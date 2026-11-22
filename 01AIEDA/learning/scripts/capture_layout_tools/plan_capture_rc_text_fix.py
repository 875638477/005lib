"""Plan native Capture Ref/Value edits using the calibrated Arial 9 display frame."""

import csv
from dataclasses import dataclass, field
from functools import lru_cache
import itertools
import math
from pathlib import Path
import sys

from compare_capture_rc_text_ink import unescape
from font_metrics_gdi import measure


FONT = ("Arial", "-9", "4", "0", "0", "400", "0", "0", "0", "0", "7", "0", "1", "16")
FONT_FIELDS = (
    "name", "height", "width", "escapement", "orientation", "weight", "italic",
    "underline", "strikeout", "charset", "out_precision", "clip_precision", "quality",
    "pitch_and_family",
)
ROLES = {"part reference": "ref", "refdes": "ref", "value": "value", "user_value": "value"}
# Original DP_C5 print at 100 document units/inch, separately for each caption.
APPROVED_GAP = {"ref": 0.37369, "value": 2.22222}
# Native anchors measured from each end of the body along its physical pin axis.
# H: approved DP_C5. V: the 14 matching P20 capacitors in the user's example.
FIXED_LAYOUT = {
    "H": (0, -45, 15, -10),
    "V": (3, -30, 15, 0),
}


@dataclass
class Caption:
    part: int
    id: int
    name: str
    value: str
    mode: int
    rotation: int
    x: int
    y: int
    justification: int
    font: tuple

    @property
    def role(self):
        return ROLES.get(self.name.casefold())

    @property
    def supported(self):
        return (self.mode == 1 and self.justification == 0 and self.font == FONT
                and bool(self.value.strip()) and self.value.isascii()
                and not any(c in self.value for c in "\r\n\t"))


@dataclass
class Part:
    id: int
    ref: str
    locked: bool
    x: int
    y: int
    axis: str = ""
    across: int = 0
    lead: int = 0
    low: int = 0
    high: int = 0


@dataclass
class Page:
    id: int
    name: str
    units: float
    width: int
    height: int
    grid: int
    parts: list = field(default_factory=list)
    captions: list = field(default_factory=list)
    boxes: list = field(default_factory=list)
    unresolved: list = field(default_factory=list)


def expanded(bounds, margin):
    return (min(bounds[0], bounds[2]) - margin, min(bounds[1], bounds[3]) - margin,
            max(bounds[0], bounds[2]) + margin, max(bounds[1], bounds[3]) + margin)


def intersects(first, second):
    return (first[0] < second[2] and second[0] < first[2]
            and first[1] < second[3] and second[1] < first[3])


def read_scene(path):
    pages = {}
    with path.open(encoding="utf-8", newline="") as stream:
        for row in csv.reader(stream, delimiter="\t", quoting=csv.QUOTE_NONE):
            record, page_id, *values = map(unescape, row)
            page_id = int(page_id)
            if record == "PAGE":
                name, units, width, height, grid = values
                pages[page_id] = Page(page_id, name, float(units), int(width), int(height), int(grid))
                continue
            page = pages[page_id]
            if record == "PART":
                id_, ref, locked, x, y, *axis = values
                geometry = (axis[0], *map(int, axis[1:])) if axis else ()
                page.parts.append(Part(int(id_), ref, bool(int(locked)), int(x), int(y), *geometry))
            elif record == "TEXT":
                part, id_, name, value, mode, rotation, x, y, justification, *font = values
                page.captions.append(Caption(int(part), int(id_), name, value, int(mode), int(rotation),
                                             int(x), int(y), int(justification), tuple(font)))
            elif record == "BOX":
                kind, *bounds = values
                cushion = 16 if kind == "labelled_pin" else .45 if kind == "endpoint" else .25
                page.boxes.append((kind, expanded(tuple(map(int, bounds)), cushion)))
            elif record == "REVIEW":
                page.unresolved.append(values[0])
    return pages.values()


@lru_cache(maxsize=4096)
def metric(value, font):
    try:
        request = dict(zip(FONT_FIELDS, (font[0], *map(int, font[1:]))))
        rendered = measure(value, request)
        if rendered["font_family_substituted"] or rendered["missing_primary_font_glyphs"]:
            return None
        return rendered["ink_bounds"], rendered["advance_width"], rendered["cell_height"]
    except (ValueError, KeyError, OSError, IndexError):
        return None


def relative_ink(caption, rotation):
    result = metric(caption.value, caption.font)
    if not result or not result[0]:
        return None
    (x1, y1, x2, y2), _, _ = result
    # The q0 TOP and q3 vertical frames were fitted independently to native
    # PScript glyphs. Unscaled GDI is deliberately wider along the text axis.
    if rotation == 0:
        return (x1, y1 + 8.1, x2, y2 + 8.0)
    return (3.55 - y2, x1, 3.55 - y1, x2)


def moved(bounds, x, y):
    return (bounds[0] + x, bounds[1] + y, bounds[2] + x, bounds[3] + y)


def current_bounds(caption):
    if caption.supported and caption.rotation in (0, 3):
        bounds = relative_ink(caption, caption.rotation)
        if bounds:
            return moved(expanded(bounds, .55), caption.x, caption.y)
    if not caption.value.strip():
        return None
    if caption.mode == 3:
        value = caption.name
    elif caption.mode != 1:
        value = f"{caption.name} = {caption.value}"
    else:
        value = caption.value
    rendered = metric(value, caption.font)
    if rendered and rendered[0]:
        bounds, advance, height = rendered
        width = max(advance, bounds[2] - bounds[0])
    else:
        height = abs(int(caption.font[1])) * 2 if caption.font else 24
        width = max(len(value), 1) * height
    if caption.rotation in (0, 3) and caption.justification == 0:
        relative = (-3, -3, width + 3, height + 3)
        if caption.rotation == 3:
            relative = (-3, -3, height + 3, width + 3)
    elif caption.rotation % 2:
        relative = (-height - 3, -width - 3, height + 3, width + 3)
    else:
        relative = (-width - 3, -height - 3, width + 3, height + 3)
    return moved(relative, caption.x, caption.y)


def on_page(bounds, page):
    return (2 <= bounds[0] < bounds[2] <= page.width - 2
            and 2 <= bounds[1] < bounds[3] <= page.height - 2)


def lead_gap(bounds, part):
    across = part.across
    return max(part.lead - bounds[across + 2], bounds[across] - part.lead)


def allowed(bounds, part, role, grid, steps):
    across = part.across
    along = 1 - across
    # Use the nearest edge of the full ink and keep both ends associated.
    gap = lead_gap(bounds, part)
    if not (.25 < gap and gap + .5 <= APPROVED_GAP[role] + grid):
        return False
    distance = part.low - bounds[along + 2] if role == "ref" else bounds[along] - part.high
    return 0 <= distance <= steps * grid


def placements(caption, part, page, grid, overflow):
    rotation = 0 if part.axis == "H" else 3
    ink = relative_ink(caption, rotation)
    if not ink:
        return []
    across = part.across
    along = 1 - across
    steps = 10 if overflow else 6
    if caption.role == "ref":
        edge = math.floor((part.low - ink[along + 2]) / grid) * grid
        endpoints = [edge - step * grid for step in range(steps + 1)]
    else:
        edge = math.ceil((part.high - ink[along]) / grid) * grid
        endpoints = [edge + step * grid for step in range(steps + 1)]
    center = round(part.lead / grid) * grid
    offsets = (0, 1, -1, 2, -2, 3, -3)
    positions = [(center + offset * grid, endpoint) for offset, endpoint in itertools.product(offsets, endpoints)]
    if rotation == caption.rotation and caption.x % grid == caption.y % grid == 0:
        xy = (caption.x, caption.y)
        positions.insert(0, (xy[across], xy[along]))
    results = []
    for lateral, longitudinal in dict.fromkeys(positions):
        x, y = (longitudinal, lateral) if across == 1 else (lateral, longitudinal)
        box = moved(ink, x, y)
        if allowed(box, part, caption.role, grid, steps) and (overflow or on_page(box, page)):
            results.append((x, y, rotation, expanded(box, .35)))
    return sorted(results, key=lambda item: not on_page(item[3], page))


def proposals(part, page, captions, obstacles, grid, overflow):
    choices = []
    for caption in captions:
        valid = [placement for placement in placements(caption, part, page, grid, overflow)
                 if not any(intersects(placement[3], box) for box in obstacles)]
        choices.append(valid)
    return next((pair for pair in itertools.product(*choices)
                 if len(pair) == 1 or not intersects(pair[0][3], pair[1][3])), None)


def place_groups(groups, page, parts, fixed, grid, overflow):
    active = dict(groups)
    # Treat a dense row as a simultaneous edit. Reinstate blocked instances
    # and recalculate if their old caption rectangles constrain the new plan.
    while active:
        obstacles = [*fixed, *[b for group in groups if group not in active
                               for c in groups[group] if (b := current_bounds(c))]]
        blocked = []
        selected = {}
        for group, captions in active.items():
            part = parts[captions[0].part]
            pair = proposals(part, page, captions, obstacles, grid, overflow)
            if pair is None:
                blocked.append(group)
            else:
                obstacles.extend(item[3] for item in pair)
                selected[group] = list(zip(captions, pair))
        if not blocked:
            return selected
        for group in blocked:
            active.pop(group)
    return {}


def candidate_groups(page):
    eligible = {}
    outcomes = {}
    parts = {p.id: p for p in page.parts}
    by_part = {}
    for caption in page.captions:
        if caption.part in parts and caption.role:
            by_part.setdefault(caption.part, []).append(caption)
    for part in sorted(page.parts, key=lambda p: (p.y, p.x, p.id)):
        captions = by_part.get(part.id, [])
        if part.locked:
            outcomes[part.id] = "LOCKED"
        elif not part.axis:
            outcomes[part.id] = "PIN_AXIS_UNAVAILABLE"
        elif page.units != 100 or len(captions) != 2 or {c.role for c in captions} != {"ref", "value"}:
            outcomes[part.id] = "UNSUPPORTED_PAGE_OR_DISPLAY_PAIR"
        elif any(not c.supported or not relative_ink(c, 0) for c in captions):
            outcomes[part.id] = "UNCALIBRATED_TEXT_FRAME"
        else:
            eligible[part.id] = captions
    return parts, eligible, outcomes


def plan_page(page, grid, overflow):
    parts, eligible, outcomes = candidate_groups(page)
    fixed = [*(bounds for _, bounds in page.boxes), *(b for c in page.captions if c not in eligible.get(c.part, ())
                           and (b := current_bounds(c)))]
    selected = place_groups(eligible, page, parts, fixed, grid, overflow)
    remaining = {id_: captions for id_, captions in eligible.items() if id_ not in selected}
    extra = [*fixed, *(layout[3] for group in selected.values() for _, layout in group)]
    individual = {}
    while remaining:
        before = len(individual)
        pending = [c for group in remaining.values() for c in group]
        for role in ("ref", "value"):
            stage = {c.id: [c] for c in pending if c.role == role and c.id not in individual}
            others = [c for c in pending if c.role != role and c.id not in individual]
            stage_fixed = [*extra, *(b for c in others if (b := current_bounds(c)))]
            resolved = place_groups(stage, page, parts, stage_fixed, grid, overflow)
            extra.extend(row[0][1][3] for row in resolved.values())
            individual.update(resolved)
        if len(individual) == before:
            break
    changes = []
    for id_, captions in eligible.items():
        part = parts[id_]
        items = selected.get(id_, [individual[c.id][0] for c in captions if c.id in individual])
        edits = []
        for caption, (x, y, rotation, _) in items:
            if (x, y, rotation) != (caption.x, caption.y, caption.rotation):
                edits.append((page.id, caption.id, x - part.x, y - part.y, rotation))
        changes.extend(edits)
        if len(items) == 2:
            outcomes[id_] = "PLANNED" if edits else "KEPT"
        elif edits:
            pending = {c.role for c in captions} - {c.role for c, _ in items}
            outcomes[id_] = "PARTIAL_" + next(iter(pending)).upper() + "_NEEDS_SPACE"
        else:
            outcomes[id_] = "NO_CLEAR_POSITION"
    overflows = {id_: ",".join(c.role for c, layout in items if not on_page(layout[3], page))
                 for id_, items in selected.items()}
    for items in individual.values():
        caption, layout = items[0]
        if not on_page(layout[3], page):
            overflows[caption.part] = caption.role
    return changes, [(page.name, p.ref, outcomes[p.id], overflows.get(p.id, "")) for p in page.parts]


def fixed_targets(part, captions):
    rotation, ref_shift, value_shift, cross_shift = FIXED_LAYOUT[part.axis]
    positions = {"ref": part.low + ref_shift, "value": part.high + value_shift}
    lateral = part.lead + cross_shift
    targets = []
    for caption in captions:
        longitudinal = positions[caption.role]
        x, y = (longitudinal, lateral) if part.axis == "H" else (lateral, longitudinal)
        bounds = moved(relative_ink(caption, rotation), x, y)
        targets.append((caption, x, y, rotation, bounds))
    return targets


def fixed_clearance(bounds, part, role, grid):
    along = 1 - part.across
    on_own_end = bounds[along + 2] <= part.low if role == "ref" else bounds[along] >= part.high
    # The native horizontal reference has a measured 0.37-unit gap. The
    # conservative GDI estimate can be smaller, so keep a positive model gap.
    return on_own_end and 0 < lead_gap(bounds, part) <= APPROVED_GAP[role] + grid


def fixed_page(page, overflow):
    parts, eligible, outcomes = candidate_groups(page)
    body_boxes = [bounds for kind, bounds in page.boxes if kind == "body"]
    selected = {}
    changes = []
    for id_, captions in eligible.items():
        part = parts[id_]
        targets = fixed_targets(part, captions)
        if any(x % page.grid or y % page.grid for _, x, y, _, _ in targets):
            outcomes[id_] = "FIXED_OFF_GRID"
            continue
        if not overflow and any(not on_page(bounds, page) for _, _, _, _, bounds in targets):
            outcomes[id_] = "FIXED_OFF_PAGE"
            continue
        if any(intersects(expanded(bounds, .35), body)
               for _, _, _, _, bounds in targets for body in body_boxes):
            outcomes[id_] = "FIXED_BODY_OVERLAP"
            continue
        if any(not fixed_clearance(bounds, part, caption.role, page.grid)
               for caption, _, _, _, bounds in targets):
            outcomes[id_] = "FIXED_CAPTION_CLEARANCE_REVIEW"
            continue
        edits = [(page.id, c.id, x - part.x, y - part.y, rotation)
                 for c, x, y, rotation, _ in targets if (x, y, rotation) != (c.x, c.y, c.rotation)]
        changes.extend(edits)
        outcomes[id_] = "PLANNED" if edits else "KEPT"
        selected[id_] = targets

    final_text = {c.id: current_bounds(c) for c in page.captions}
    final_text.update({c.id: expanded(b, .35) for targets in selected.values() for c, _, _, _, b in targets})
    rows = []
    for part in page.parts:
        positions = {}
        exceeded = []
        reviews = []
        for caption, x, y, _, bounds in selected.get(part.id, []):
            role = caption.role
            positions[role] = f"{x - part.x},{y - part.y}"
            if not on_page(bounds, page):
                exceeded.append(role)
            guard = expanded(bounds, .35)
            hits = {kind for kind, other in page.boxes if kind != "body" and intersects(guard, other)}
            if any(other is not None and id_ != caption.id and intersects(guard, other)
                   for id_, other in final_text.items()):
                hits.add("text")
            reviews.extend(f"{role}:{kind}" for kind in sorted(hits))
        rows.append((page.name, part.ref, outcomes[part.id], ",".join(exceeded), page.grid,
                     positions.get("ref", ""), positions.get("value", ""), ",".join(reviews)))
    return changes, rows


def main(snapshot, plan, report, page_boundary, layout="avoid"):
    if layout not in {"avoid", "fixed"}:
        raise ValueError("Unknown caption layout mode")
    fixed = layout == "fixed"
    overflow = page_boundary == "allow-overflow"
    changes = []
    results = []
    for page in read_scene(Path(snapshot)):
        empty_cells = ("", "", "") if fixed else ()
        if page.grid <= 0:
            results.extend((page.name, part.ref, "PAGE_GRID_UNSUPPORTED", "", "", *empty_cells)
                           for part in page.parts)
            continue
        if fixed:
            edits, rows = fixed_page(page, overflow)
        else:
            edits, rows = plan_page(page, page.grid, overflow)
            rows = [(*row, page.grid) for row in rows]
        changes.extend(edits)
        results.extend(rows)
        results.extend((page.name, "", warning, "", page.grid, *empty_cells)
                       for warning in sorted(set(page.unresolved)))
    with Path(plan).open("x", encoding="utf-8", newline="") as stream:
        csv.writer(stream, delimiter="\t", lineterminator="\n").writerows(changes)
    with Path(report).open("x", encoding="utf-8", newline="") as stream:
        out = csv.writer(stream, delimiter="\t", lineterminator="\n")
        columns = ("page", "ref", "result", "overflows_page", "grid_doc")
        if fixed:
            columns += ("ref_anchor_dxdy", "value_anchor_dxdy", "fixed_position_review")
        out.writerow(columns)
        out.writerows(results)
    planned = sum(row[2] == "PLANNED" for row in results)
    partial = sum(row[2].startswith("PARTIAL_") for row in results)
    kept = sum(row[2] == "KEPT" for row in results)
    exceeded = sum(bool(row[3]) for row in results)
    print(f"RC text repair: {planned} parts planned, {partial} partial, {kept} kept, "
          f"{len(results) - planned - kept - partial} review entries, {exceeded} page overflow.")
    if fixed:
        print(f"Fixed-position neighbor review: {sum(bool(row[-1]) for row in results)} parts.")


if __name__ == "__main__":
    main(*sys.argv[1:])
