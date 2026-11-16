"""Check independently extracted native/scene endpoint facts; missing evidence is UNKNOWN.

This does not extract Capture data or replace rendered review. Supply JSON with
objects, expected_endpoint_ids (independent native inventory), and
connectors_before/connectors_after (lists of native RefDes).
Geometry uses the same page coordinates for hotspot, symbol_bounds, text_bounds.
tip_vector must come from the actual transformed contour/profile, not an angle
lookup. A verified profile can document a library GND with no visible text.
"""
import argparse
import json
from pathlib import Path

VERSION = "ENDPOINT-20261116.1"


def check(data):
    results = []

    def record(obj, rule, value, detail):
        results.append({"id": obj.get("id"), "page": obj.get("page"),
                        "rule": rule, "status": "UNKNOWN" if value is None else
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
            "limits": "Facts checker only; it cannot prove extractor accuracy, electrical equivalence, or unsupplied scene coverage."}


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
