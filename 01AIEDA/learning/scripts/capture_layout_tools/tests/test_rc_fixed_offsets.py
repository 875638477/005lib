from pathlib import Path
import sys
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import plan_capture_rc_text_fix as repair


def ink(caption, rotation):
    if rotation == 0:
        return (0.5, 2, len(caption.value) * 4, 9)
    return (2, 0.5, 9, len(caption.value) * 4)


def example(axis):
    across = 1 if axis == "H" else 0
    part = repair.Part(100, "C12", False, 100, 100, axis, across, 110, 100, 110)
    ref = repair.Caption(part.id, 1, "Part Reference", "C12", 1, 0, 0, 0, 0, repair.FONT)
    value = repair.Caption(part.id, 2, "Value", "1uF", 1, 0, 0, 0, 0, repair.FONT)
    return repair.Page(1, "test", 100, 200, 200, 5, [part], [ref, value])


class FixedOffsets(unittest.TestCase):
    @patch.object(repair, "relative_ink", side_effect=ink)
    def test_wire_and_value_length_do_not_shift_fixed_positions(self, unused):
        page = example("V")
        before, rows = repair.fixed_page(page, True)
        self.assertEqual(before, [(1, 1, 10, -30, 3), (1, 2, 10, 25, 3)])
        self.assertEqual(rows[0][-1], "")

        page.captions[1].value = "100nF/0201/25V/10%"
        page.boxes.append(("wire", (100, 160, 130, 161)))
        after, rows = repair.fixed_page(page, True)
        self.assertEqual(after, before)
        self.assertEqual(rows[0][-1], "value:wire")

    @patch.object(repair, "relative_ink", side_effect=ink)
    def test_horizontal_offsets_do_not_snap_to_another_grid(self, unused):
        page = example("H")
        edits, _ = repair.fixed_page(page, True)
        self.assertEqual(edits, [(1, 1, -45, 0, 0), (1, 2, 25, 0, 0)])
        page.grid = 10
        edits, rows = repair.fixed_page(page, True)
        self.assertEqual(edits, [])
        self.assertEqual(rows[0][2], "FIXED_OFF_GRID")

    @patch.object(repair, "relative_ink", side_effect=ink)
    def test_fixed_body_overlap_blocks_instead_of_moving_text(self, unused):
        page = example("V")
        page.boxes.append(("body", (108, 124, 122, 143)))
        edits, rows = repair.fixed_page(page, True)
        self.assertEqual(edits, [])
        self.assertEqual(rows[0][2], "FIXED_BODY_OVERLAP")


if __name__ == "__main__":
    unittest.main()
