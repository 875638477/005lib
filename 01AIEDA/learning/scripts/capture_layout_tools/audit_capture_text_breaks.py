"""Inventory XML text controls; native wrapping is deliberately left UNKNOWN.

Read-only. An explicit per-object allowlist may classify intentional multiline
content; this does not prove native appearance or neighboring clearance.
"""
import argparse
import hashlib
import json
import re
import xml.etree.ElementTree as ET
from pathlib import Path

BREAK = re.compile(r"\r\n|[\r\n\u2028\u2029]|\\[nr]")
TEXT_KEYS = {"name", "val", "number"}


def inventory(path, allowed=None):
    path = Path(path)
    allowed = allowed or {}
    root = ET.parse(path).getroot()
    rows = []

    def walk(node, route):
        counts = {}
        for child in node:
            i = counts.get(child.tag, 0)
            counts[child.tag] = i + 1
            current = f"{route}/{child.tag}[{i}]"
            if child.tag == "Defn":
                for key, text in child.attrib.items():
                    if key not in TEXT_KEYS:
                        continue
                    identity = current + "/@" + key
                    breaks = BREAK.findall(text)
                    if breaks:
                        reason = allowed.get(identity)
                        rows.append(dict(id=identity, text=text, controls=breaks,
                                         status="INTENTIONAL_MULTILINE" if reason else "REVIEW_REQUIRED",
                                         reason=reason))
            walk(child, current)

    walk(root, root.tag)
    return dict(rule_version="MODULETEXT-20261117.1", source=str(path.resolve()),
                sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                control_breaks=rows, unclassified=sum(r["status"] == "REVIEW_REQUIRED" for r in rows),
                native_wrapping="UNKNOWN_REQUIRES_SAVED_NATIVE_RENDER",
                whole_text_acceptance="UNKNOWN", read_only=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source")
    parser.add_argument("output")
    parser.add_argument("--intentional", help="JSON object mapping exact inventory IDs to reasons")
    args = parser.parse_args()
    allowed = json.loads(Path(args.intentional).read_text(encoding="utf-8")) if args.intentional else None
    result = inventory(args.source, allowed)
    Path(args.output).write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding="utf-8")
    print(json.dumps(dict(unclassified=result["unclassified"], whole_text_acceptance=result["whole_text_acceptance"])))
