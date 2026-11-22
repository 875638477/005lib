"""Compare a Capture quick scan with same-save native printed glyph evidence."""

import argparse
import csv
import hashlib
import json
import math
from pathlib import Path
import re


VERSION = "RC-20261117.1"
ROLES = {"part reference": "ref", "refdes": "ref", "value": "value", "user_value": "value"}
FONT_FIELDS = (
    "name", "height", "width", "escapement", "orientation", "weight", "italic",
    "underline", "strikeout", "charset", "outprecision", "clipprecision", "quality",
    "pitchandfamily",
)


def sha256(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def unescape(cell):
    escapes = {"t": "\t", "r": "\r", "n": "\n", "\\": "\\"}
    return re.sub(r"\\([trn\\])", lambda match: escapes[match[1]], cell)


def read_scan(path):
    with path.open(encoding="utf-8", newline="") as stream:
        version, source = next(csv.reader(stream, delimiter="\t", quoting=csv.QUOTE_NONE))
        if version != f"# {VERSION}":
            raise ValueError("Unexpected quick scan version")
        rows = list(csv.DictReader(stream, delimiter="\t", quoting=csv.QUOTE_NONE))
    if not rows or rows[-1]["schematic"] != "# COMPLETE":
        raise ValueError("Quick scan did not finish")
    return Path(unescape(source)), [
        {key: unescape(value) for key, value in row.items()}
        for row in rows[:-1]
    ]


def evidence(path):
    report = json.loads(path.read_text(encoding="utf-8-sig"))
    if report["rule_version"] != VERSION:
        raise ValueError("Unexpected glyph evidence version")
    artifacts = {}
    for entry in report["raw_evidence"]:
        artifact = Path(entry["path"])
        if sha256(artifact) != entry["sha256"]:
            raise ValueError(f"Stale native evidence: {artifact}")
        if artifact.name in artifacts:
            raise ValueError(f"Duplicate native evidence name: {artifact.name}")
        artifacts[artifact.name] = artifact
    if sha256(Path(report["source_xml"])) != report["source_xml_sha256"]:
        raise ValueError("Stale source XML")
    if not all(name in artifacts for name in ("CapOutput.ps", "displayprops.tsv", "page_sizes.tsv")):
        raise ValueError("Expected bound native print, caption and page evidence")
    with artifacts["displayprops.tsv"].open(encoding="utf-8-sig", newline="") as stream:
        display = index(csv.DictReader(stream, delimiter="\t", quoting=csv.QUOTE_NONE),
                        ("page", "owner_id", "dp_id"))
    with artifacts["page_sizes.tsv"].open(encoding="utf-8-sig", newline="") as stream:
        pages = index(csv.DictReader(stream, delimiter="\t"), ("page",))
    glyphs = index(report["rows"], ("page", "id", "dp_id"))
    return report["native_print"]["source_dsn_sha256"].lower(), glyphs, display, pages


def index(rows, fields):
    result = {}
    for row in rows:
        key = tuple(str(row[field]) for field in fields)
        if key in result:
            raise ValueError(f"Ambiguous evidence for {key}")
        result[key] = row
    return result


def ink(glyph, axis, pin_hotspots):
    if (glyph["status"] != "MATCHED_NATIVE_PRINTED_GLYPHS"
            or glyph.get("match_count") != 1 or not glyph.get("own_printed_pins_match")
            or glyph["axis"] != axis
            or sorted(map(tuple, glyph["physical_pins"])) != sorted(pin_hotspots)):
        raise ValueError("Native glyphs and physical pins did not bind")
    bounds = tuple(glyph["ink_bounds"])
    if len(bounds) != 4 or not all(math.isfinite(n) for n in bounds):
        raise ValueError("Invalid glyph bounds")
    if not (bounds[0] < bounds[2] and bounds[1] < bounds[3]):
        raise ValueError("Empty glyph bounds")
    return bounds


def gap(bounds, axis, line):
    across = 0 if axis == "V" else 1
    return max(line - bounds[across + 2], bounds[across] - line)


def font_signature(display):
    return tuple(display[f"font_{field}"] for field in FONT_FIELDS) + (
        display["justification"], display["display_type"],
    )


def approved_gaps(reference, data):
    _, glyphs, displays, pages = data
    profiles = {}
    for key, glyph in glyphs.items():
        role = ROLES.get(glyph["property"].lower())
        if glyph["ref"] != reference or not role:
            continue
        pins = list(map(tuple, glyph["physical_pins"]))
        if role in profiles or len(pins) != 2 or pins[0][1] != pins[1][1]:
            raise ValueError("Approved instance must be unique and horizontal")
        bounds = ink(glyph, "H", pins)
        distance = gap(bounds, "H", pins[0][1])
        if distance < 0:
            raise ValueError("Approved text crosses its own pin axis")
        profiles[role] = (distance, font_signature(displays[key]), pages[(key[0],)]["doc_units_per_inch"])
    if set(profiles) != {"ref", "value"}:
        raise ValueError("Approved instance needs a measured Ref and Value")
    return profiles


def proximity(row, glyphs, displays, pages, profiles):
    if row["axis_check"] != "PASS":
        return {"status": "UNKNOWN", "reason": "Direction check needs review"}
    key = (row["page"], row["part_id"], row["dp_id"])
    glyph = glyphs[key]
    display = displays[key]
    if (glyph["ref"], glyph["property"], glyph["text"], str(glyph["native_rotation"])) != (
        row["ref"], row["property"], row["text"], row["rotation"],
    ) or display["value_string"] != row["text"]:
        raise ValueError("Printed text, identity or rotation differs from scan")
    if tuple(map(float, row["anchor_page"].split())) != (
        float(display["absolute_add_candidate_x"]), float(display["absolute_add_candidate_y"]),
    ):
        raise ValueError("Display anchor differs from scan")
    # The Tcl report stores each native lead as {pin-number {start-x start-y} {hot-x hot-y}}.
    lead_points = re.findall(r"\{(-?\d+) (-?\d+)\}", row["pin_leads"])
    if len(lead_points) != 4:
        raise ValueError("Expected two physical leads")
    starts = [tuple(map(int, point)) for point in lead_points[::2]]
    hotspots = [tuple(map(int, point)) for point in lead_points[1::2]]
    axis = row["part_axis"]
    bounds = ink(glyph, axis, hotspots)
    limit, font, units = profiles[row["role"]]
    if (float(row["doc_units_per_inch"]) != float(units)
            or float(pages[(key[0],)]["doc_units_per_inch"]) != float(units)
            or font_signature(display) != font):
        raise ValueError("Approved font, justification, mode or document units differ")
    grid = float(row["grid_doc"])
    if not math.isfinite(grid) or grid <= 0:
        raise ValueError("Current document grid required")
    across = 0 if axis == "V" else 1
    along = 1 - across
    start_min, start_max = sorted(point[along] for point in starts)
    increasing = row["rotation"] in {"0", "3"}
    low_side = (row["role"] == "ref") == increasing
    on_own_side = bounds[along + 2] <= start_min if low_side else bounds[along] >= start_max
    distance = gap(bounds, axis, hotspots[0][across])
    maximum = limit + grid
    passed = on_own_side and 0 < distance <= maximum
    return {
        "status": "PASS" if passed else "FAIL",
        "gap_doc": distance, "max_gap_doc": maximum, "on_own_side": on_own_side,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scan", required=True, type=Path)
    parser.add_argument("--native-ink", required=True, type=Path)
    parser.add_argument("--approved-ink", required=True, type=Path)
    parser.add_argument("--approved-ref", required=True)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    dsn, rows = read_scan(args.scan)
    dsn_hash, glyphs, displays, pages = evidence(args.native_ink)
    if sha256(dsn) != dsn_hash:
        parser.error("The native glyph report is for a different DSN save")
    profiles = approved_gaps(args.approved_ref, evidence(args.approved_ink))
    counts = {"PASS": 0, "FAIL": 0, "UNKNOWN": 0}
    results = []
    for row in rows:
        try:
            result = proximity(row, glyphs, displays, pages, profiles)
        except (KeyError, ValueError) as error:
            result = {"status": "UNKNOWN", "reason": str(error)}
        counts[result["status"]] += 1
        results.append({
            **{key: row[key] for key in ("schematic", "page", "part_id", "ref", "role", "axis_check")},
            **result,
        })
    output = {"rule_version": VERSION, "scope": "Native ink proximity and lead side",
              "source_dsn_sha256": dsn_hash, "scan_sha256": sha256(args.scan),
              "native_ink_sha256": sha256(args.native_ink),
              "approved_ink_sha256": sha256(args.approved_ink),
              "approved_reference": args.approved_ref, "counts": counts, "rows": results}
    with args.output.open("x", encoding="utf-8") as stream:
        json.dump(output, stream, ensure_ascii=False, indent=2)
        stream.write("\n")
    print(counts)
    return 2 if counts["FAIL"] or counts["UNKNOWN"] or not results else 0


if __name__ == "__main__":
    raise SystemExit(main())
