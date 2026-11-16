"""Check independently extracted native/scene endpoint facts; missing evidence is UNKNOWN.

This does not extract Capture data or replace rendered review. Supply JSON with
objects, expected_endpoint_ids (independent native inventory), and
connectors_before/connectors_after (lists of native RefDes).
Geometry uses the same page coordinates for hotspot, symbol_bounds, text_bounds.
tip_vector must come from the actual transformed contour/profile, not an angle
lookup. GNDORDER additionally computes the order of all transformed ground bars;
profile_verified cannot replace those geometric facts. POWERGAP computes the gap
from actual circle ink to the object's own complete Name ink and compares it with
native reference geometry and its calibrated upper bound/quantization.

New geometry blocks are ground_geometry / power_geometry; see the matching
ENDPOINT-20261116.2 contract for their exact fields. Source, native-export and
symbol identities must be bound. Missing geometry/calibration yields UNKNOWN.
This remains a limited facts checker: it does not extract primitives, establish
that a reference was user-approved, or certify whole-page/PCB layout.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path

VERSION = "ENDPOINT-20261116.2"


def number(value):
    return isinstance(value, (int, float)) and not isinstance(value, bool) and math.isfinite(value)


def point(value):
    return isinstance(value, (list, tuple)) and len(value) == 2 and all(number(v) for v in value)


def unit(value):
    if not point(value):
        return None
    length = math.hypot(*value)
    return [v / length for v in value] if length else None


def dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def digest(value):
    return isinstance(value, str) and len(value) == 64 and all(c in "0123456789abcdefABCDEF" for c in value)


def bind_geometry(geometry, data):
    """Verify file identity; geometric extraction accuracy is still independently audited."""
    if not isinstance(geometry, dict):
        return None, "Missing independently extracted geometric facts"
    source, expected = data.get("native_source"), data.get("native_source_sha256")
    if not source or not digest(expected):
        return None, "Missing final native source path/SHA256"
    path = Path(source)
    if not path.is_file():
        return None, "Final native source cannot be read"
    if hashlib.sha256(path.read_bytes()).hexdigest() != expected:
        return False, "Final native source SHA256 mismatch"
    if geometry.get("source_xml_sha256") != expected:
        return False if geometry.get("source_xml_sha256") else None, "Geometry is not bound to final source SHA256"
    if not all(digest(geometry.get(key)) for key in ("native_export_sha256", "symbol_geometry_sha256")):
        return None, "Missing native export / actual symbol-geometry identities"
    if geometry.get("extraction_method") != "native_transformed_primitives":
        return None, "Native transformed primitive extraction provenance is required"
    return True, "Geometry bound to final native source and export/symbol identities"


def ground_order(obj, data):
    geometry = obj.get("ground_geometry")
    bound, detail = bind_geometry(geometry, data)
    if bound is not True:
        return bound, detail
    inward = unit(obj.get("circuit_vector"))
    bars = geometry.get("bars")
    inventory = geometry.get("native_bar_ids")
    tolerance = geometry.get("quantization_doc")
    if (inward is None or geometry.get("circuit_vector_source") != "native_wire_last_segment" or
            not isinstance(bars, list) or not isinstance(inventory, list) or len(inventory) < 2 or
            geometry.get("complete_bar_inventory") is not True or not number(tolerance) or
            not 0 <= tolerance < 1):
        return None, "Need native wire direction, full independent bar inventory and native quantization (<1 doc unit)"
    ids = [bar.get("id") for bar in bars if isinstance(bar, dict)]
    if (len(ids) != len(bars) or None in ids or len(set(ids)) != len(ids) or
            len(set(inventory)) != len(inventory) or set(ids) != set(inventory)):
        return False, "Bar facts do not exactly cover the independently enumerated native bars"
    outward = [-value for value in inward]
    normal = [-outward[1], outward[0]]
    measured = []
    for bar in bars:
        a, b, width = bar.get("start"), bar.get("end"), bar.get("stroke_width_doc")
        if not point(a) or not point(b) or not number(width) or width < 0:
            return None, "Every actual bar needs transformed endpoints and resolved ink stroke width"
        axis = [b[i] - a[i] for i in (0, 1)]
        length = math.hypot(*axis)
        if not length or abs(dot(axis, outward)) > tolerance + 1e-9:
            return False, "Ground bars must be real nonzero parallel bars perpendicular to the outward axis"
        center = [(a[i] + b[i]) / 2 for i in (0, 1)]
        measured.append({"id": bar["id"], "outward_position": dot(center, outward),
                         "lateral_position": dot(center, normal), "ink_length": length + width})
    measured.sort(key=lambda bar: bar["outward_position"])
    aligned = all(abs(bar["lateral_position"] - measured[0]["lateral_position"]) <= tolerance + 1e-9
                  for bar in measured)
    ordered = all(a["outward_position"] + tolerance < b["outward_position"] and
                  a["ink_length"] > b["ink_length"] + tolerance
                  for a, b in zip(measured, measured[1:]))
    return aligned and ordered, {"method": "Actual bar centers sorted from circuit toward free space; ink lengths must strictly decrease",
                                 "outward_unit": outward, "bars_near_to_far": measured,
                                 "quantization_doc": tolerance}


def circle_name_gap(circle, bounds, circuit_vector):
    inward = unit(circuit_vector)
    if not inward or not isinstance(circle, dict) or not circle.get("primitive_id") or not isinstance(bounds, (list, tuple)) or len(bounds) != 4 or not all(number(v) for v in bounds):
        return None
    center, a, b = circle.get("center"), circle.get("axis_a"), circle.get("axis_b")
    width = circle.get("stroke_width_doc")
    if not all(point(p) for p in (center, a, b)) or not number(width) or width < 0:
        return None
    la, lb = math.hypot(*a), math.hypot(*b)
    if not la or not lb or abs(dot(a, b)) > 1e-7 or abs(la - lb) > 1e-7 or bounds[0] > bounds[2] or bounds[1] > bounds[3]:
        return None
    outward = [-value for value in inward]
    # Axis-aligned Name ink bounds give exact support on horizontal/vertical ends.
    if not ((abs(outward[0]) < 1e-8 and abs(abs(outward[1]) - 1) < 1e-8) or
            (abs(outward[1]) < 1e-8 and abs(abs(outward[0]) - 1) < 1e-8)):
        return None
    circle_outer = dot(center, outward) + math.hypot(dot(a, outward), dot(b, outward)) + width / 2
    corners = [[x, y] for x in (bounds[0], bounds[2]) for y in (bounds[1], bounds[3])]
    text_near = min(dot(p, outward) for p in corners)
    return text_near - circle_outer


def perpendicular_separation(circle, bounds, circuit_vector):
    """Necessary alignment bound: a Name far above/below cannot pass on x-gap alone."""
    inward = unit(circuit_vector)
    normal = [-inward[1], inward[0]]
    center = dot(circle["center"], normal)
    values = [dot([x, y], normal) for x in (bounds[0], bounds[2])
              for y in (bounds[1], bounds[3])]
    return max(min(values) - center, center - max(values), 0)


def power_gap(obj, data):
    geometry = obj.get("power_geometry")
    bound, detail = bind_geometry(geometry, data)
    if bound is not True:
        return bound, detail
    profile = geometry.get("gap_profile")
    if (geometry.get("marker_kind") != "circle" or
            geometry.get("name_ink_source") != "actual_full_object_Name_glyph" or
            not isinstance(profile, dict)):
        return None, "Need actual circle ink, full own-Name ink and a calibrated native gap profile"
    maximum, tolerance = profile.get("max_gap_doc"), profile.get("quantization_doc")
    samples = profile.get("reference_samples")
    if (not number(maximum) or maximum < 0 or not number(tolerance) or not 0 <= tolerance < 1 or
            not profile.get("approved_reference_id") or
            not all(digest(profile.get(key)) for key in ("reference_source_sha256", "reference_native_export_sha256")) or
            not isinstance(samples, list) or not samples or
            not geometry.get("document_units") or geometry.get("document_units") != profile.get("document_units") or
            not geometry.get("font_signature") or geometry.get("font_signature") != profile.get("font_signature")):
        return None, "Missing calibrated maximum/quantization, native reference identity, reference geometry or matching units/font"
    if geometry.get("circuit_vector_source") != "native_wire_last_segment":
        return None, "Power outward axis must come from the actual owning wire last segment"
    gaps, separations = [], []
    for sample in samples:
        if not isinstance(sample, dict):
            return None, "Missing native reference sample geometry"
        gap = circle_name_gap(sample.get("circle"), sample.get("name_ink_bounds"), sample.get("circuit_vector"))
        if gap is None:
            return None, "Unsupported or incomplete native reference circle/Name ink geometry"
        gaps.append(gap)
        separations.append(perpendicular_separation(sample["circle"], sample["name_ink_bounds"], sample["circuit_vector"]))
    # A supplied large upper bound cannot override the actual approved tight example.
    if min(gaps) < -tolerance or abs(maximum - max(gaps)) > tolerance + 1e-9:
        return False, "Gap upper bound is not calibrated to the supplied native positive reference geometry"
    gap = circle_name_gap(geometry.get("circle"), obj.get("text_bounds"), obj.get("circuit_vector"))
    if gap is None:
        return None, "Missing actual circle/own-Name ink or unsupported non-axis-aligned end"
    separation = perpendicular_separation(geometry["circle"], obj["text_bounds"], obj["circuit_vector"])
    return -tolerance <= gap <= maximum + tolerance and separation <= max(separations) + tolerance, {
        "method": "Projected actual circle outer ink to nearest complete own-Name ink edge",
        "actual_gap_doc": gap, "calibrated_max_gap_doc": maximum,
        "reference_gaps_doc": gaps, "quantization_doc": tolerance,
        "actual_perpendicular_separation_doc": separation,
        "reference_max_perpendicular_separation_doc": max(separations)}


def check(data):
    results = []

    def record(obj, rule, value, detail):
        results.append({"id": obj.get("id"), "page": obj.get("page"),
                        "rule": rule, "status": "UNKNOWN" if not isinstance(value, bool) else
                        "PASS" if value else "FAIL", "detail": detail})

    objects = data.get("objects", [])
    if not objects:
        record({}, "END-COVERAGE", None, "No native/scene endpoint facts supplied")
    expected_ids = data.get("expected_endpoint_ids")
    actual_ids = [obj.get("id") for obj in objects]
    coverage = None if expected_ids is None else (
        None not in actual_ids and len(actual_ids) == len(set(actual_ids)) and
        set(actual_ids) == set(expected_ids))
    record({}, "END-INVENTORY", coverage,
           "Facts must exactly cover an independently enumerated native endpoint inventory")
    for obj in objects:
        kind = obj.get("kind")
        record(obj, "END-NATIVE", None if kind is None else kind in
               ("OffPageConnector", "Global"), "Real native scope object")
        name = obj.get("name")
        record(obj, "END-NONEMPTY-NAME", None if name is None else
               bool(str(name).strip()), "Native object must retain its actual network name")
        visible = obj.get("name_visible")
        # A native GND profile may conventionally identify itself by its glyph.
        no_text_ground = (obj.get("is_ground") is True and
                          obj.get("native_ground_without_name") is True and
                          obj.get("profile_verified") is True)
        if not no_text_ground:
            record(obj, "END-NAME", visible, "Effective object Name must be visible")
            source = obj.get("name_source")
            record(obj, "END-NAME-SOURCE", None if source is None else
                   source == "object_name", "Alias/comment cannot replace endpoint Name")
            axis, sign = obj.get("outside_axis"), obj.get("outside_sign")
            text, symbol = obj.get("text_bounds"), obj.get("symbol_bounds")
            outside = None
            if (axis in ("x", "y") and sign in (-1, 1) and
                    text is not None and symbol is not None and
                    len(text) == len(symbol) == 4):
                i = 0 if axis == "x" else 1
                outside = (text[i] >= symbol[i + 2]) if sign == 1 else (
                    text[i + 2] <= symbol[i])
            record(obj, "END-NAME-OUTSIDE", outside,
                   "Entire Name bounds must be outside the symbol toward free space")
        record(obj, "END-PROFILE", obj.get("profile_verified"),
               "Symbol identity, hotspot and transform calibrated against native reference")
        if kind == "Global":
            flag, role = obj.get("is_ground"), obj.get("global_role")
            native_ground_name = isinstance(name, str) and name.upper() == "GND"
            if role is None:
                role = "ground" if flag is True or native_ground_name else "power" if flag is False else None
            consistent = None if role not in ("ground", "power") else not (
                role == "power" and (flag is True or native_ground_name) or
                role == "ground" and flag is False)
            record(obj, "END-GLOBAL-ROLE", consistent,
                   "Native ground/power role must be classified consistently; GND cannot bypass bar checks")
            if role == "ground" or native_ground_name:
                value, detail = ground_order(obj, data)
                record(obj, "GNDORDER", value, detail)
            elif role == "power":
                value, detail = power_gap(obj, data)
                record(obj, "POWERGAP", value, detail)
            else:
                record(obj, "GNDORDER", None, "Unclassified Global role")
                record(obj, "POWERGAP", None, "Unclassified Global role")
        if kind == "OffPageConnector":
            tip, inward = obj.get("tip_vector"), obj.get("circuit_vector")
            facing = None
            if tip is not None and inward is not None and len(tip) == len(inward) == 2:
                facing = (tip[0] * inward[0] + tip[1] * inward[1] > 0)
            record(obj, "END-DIRECTION", facing,
                   "Transformed contour tip must face the circuit interior")
        record(obj, "END-RENDER", obj.get("render_reviewed"),
               "Complete glyph, text and neighboring rows reviewed in rendered output")
    before, after = data.get("connectors_before"), data.get("connectors_after")
    if before is None or after is None:
        record({}, "CONNECTOR-PRESERVE", None, "Native before/after RefDes sets required")
    else:
        removed = sorted(set(before) - set(after))
        record({}, "CONNECTOR-PRESERVE", not removed, "Missing existing connectors: " + str(removed))
    counts = {state: sum(r["status"] == state for r in results)
              for state in ("PASS", "FAIL", "UNKNOWN")}
    return {"version": VERSION, "status": "FAIL" if counts["FAIL"] else
            "UNKNOWN" if counts["UNKNOWN"] else "PASS", "counts": counts,
            "objects": len(objects), "checks": results,
            "limits": "Limited facts checker only. GNDORDER/POWERGAP compute supplied native-bound geometry; reference approval, extractor accuracy, full-scene clearance, compact grouping/lead length, electrical equivalence and whole-page certification remain separate."}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("facts", type=Path)
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()
    result = check(json.loads(args.facts.read_text(encoding="utf-8-sig")))
    payload = json.dumps(result, ensure_ascii=False, indent=2)
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(payload + "\n", encoding="utf8")
    print(json.dumps({k: result[k] for k in ("version", "status", "counts", "objects")}))
    raise SystemExit(0 if result["status"] == "PASS" else 2 if result["status"] == "UNKNOWN" else 1)


if __name__ == "__main__":
    main()
