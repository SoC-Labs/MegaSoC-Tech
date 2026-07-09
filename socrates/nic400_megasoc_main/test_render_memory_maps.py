import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT))

import render_memory_maps as renderer


class RenderMemoryMapsTests(unittest.TestCase):
    def test_render_html_contains_all_memory_maps(self) -> None:
        xml_path = ROOT / "nic400_megasoc_main.xml"
        memory_maps = renderer.parse_memory_maps(xml_path)

        self.assertGreaterEqual(len(memory_maps), 4)
        self.assertEqual(memory_maps[0]["name"], "CPU_MM")

        with tempfile.TemporaryDirectory() as tmpdir:
            output_path = Path(tmpdir) / "memory_maps.html"
            renderer.write_html(output_path, memory_maps)
            html_text = output_path.read_text(encoding="utf-8")

        self.assertIn("CPU_MM", html_text)
        self.assertIn("SDIO_MM", html_text)
        self.assertIn("EXP_MM", html_text)
        self.assertIn("ADP_MM", html_text)
        self.assertIn("memory-map-tabs", html_text)
        self.assertIn("Unmapped gap", html_text)
        self.assertIn("title=", html_text)


if __name__ == "__main__":
    unittest.main()
