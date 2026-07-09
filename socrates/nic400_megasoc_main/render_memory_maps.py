#!/usr/bin/env python3
"""Render NIC400 memory-map XML into an interactive HTML page."""

import argparse
import html
import math
import sys
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Any, Dict, List


def fmt_addr(value: int) -> str:
    return f"0x{value:08X}" if value >= 0 else str(value)


def parse_memory_maps(xml_path: Path) -> List[Dict[str, Any]]:
    tree = ET.parse(xml_path)
    root = tree.getroot()

    memory_maps_node = root.find(".//MemoryMaps")
    if memory_maps_node is None:
        raise ValueError(f"No MemoryMaps section found in {xml_path}")

    memory_maps = []
    for mapping in memory_maps_node.findall("MemoryMap"):
        name = (mapping.findtext("Name") or "Unnamed").strip()
        description = (mapping.findtext("Description") or "").strip()

        source_node = mapping.find("MemoryMapSource")
        source = ""
        if source_node is not None:
            source = (source_node.findtext("InterfaceRef") or "").strip()

        blocks = []
        for block in mapping.findall("MappedBlock"):
            interface = (block.findtext("InterfaceRef") or "").strip()
            offset = int(block.findtext("Offset") or "0")
            size = int(block.findtext("Range") or "0")
            visible = (block.findtext("Visibility") or "false").strip().lower() == "true"
            blocks.append(
                {
                    "interface": interface,
                    "offset": offset,
                    "size": size,
                    "end": offset + size - 1 if size > 0 else offset,
                    "visibility": visible,
                }
            )

        memory_maps.append(
            {
                "name": name,
                "description": description,
                "source": source,
                "blocks": blocks,
            }
        )

    return memory_maps


def build_html(memory_maps: List[Dict[str, Any]], title: str) -> str:
    tabs = []
    panels = []

    color_palette = [
        "#2563eb",
        "#0f766e",
        "#7c3aed",
        "#d97706",
        "#dc2626",
        "#0891b2",
        "#4f46e5",
        "#be185d",
        "#15803d",
    ]

    for idx, mapping in enumerate(memory_maps):
        name = html.escape(mapping["name"])
        description = html.escape(mapping.get("description", ""))
        source = html.escape(mapping.get("source", ""))

        blocks = sorted(mapping.get("blocks", []), key=lambda item: item["offset"])
        sections = []
        previous_end = None
        for index, block in enumerate(blocks):
            if previous_end is not None and block["offset"] > previous_end + 1:
                gap_start = previous_end + 1
                gap_end = block["offset"] - 1
                sections.append(
                    {
                        "kind": "gap",
                        "start": gap_start,
                        "end": gap_end,
                        "size": gap_end - gap_start + 1,
                    }
                )

            sections.append(
                {
                    "kind": "block",
                    "interface": block["interface"],
                    "start": block["offset"],
                    "end": block["end"],
                    "size": block["size"],
                    "visibility": block["visibility"],
                }
            )
            previous_end = block["end"]

        if not sections:
            sections.append({"kind": "empty", "start": 0, "end": 0, "size": 0})

        max_addr = max([section["end"] for section in sections if section["kind"] != "empty"], default=0)
        if max_addr <= 0:
            max_addr = 1

        bar_segments = []
        for section in sections:
            if section["kind"] == "empty":
                continue
            if section["kind"] == "gap":
                width = max(1, int((section["size"] / max_addr) * 100)) if max_addr else 0
                bar_segments.append(
                    f"<div class='bar-segment gap' style='flex-basis:{width}%;' title='Unmapped gap: {fmt_addr(section['start'])} to {fmt_addr(section['end'])}' data-tooltip='Unmapped gap: {fmt_addr(section['start'])} to {fmt_addr(section['end'])}'></div>"
                )
            else:
                width = max(1, int((section["size"] / max_addr) * 100)) if max_addr else 0
                color = color_palette[len(bar_segments) % len(color_palette)]
                bar_segments.append(
                    f"<div class='bar-segment mapped' style='flex-basis:{width}%;background:{color};' title='{html.escape(section['interface'])}: {fmt_addr(section['start'])} to {fmt_addr(section['end'])}' data-tooltip='{html.escape(section['interface'])}: {fmt_addr(section['start'])} to {fmt_addr(section['end'])}'></div>"
                )

        detail_items = []
        for section in sections:
            if section["kind"] == "empty":
                continue
            if section["kind"] == "gap":
                detail_items.append(
                    f"<li><span class='legend gap'></span><strong>Unmapped gap</strong> {fmt_addr(section['start'])} to {fmt_addr(section['end'])} ({fmt_addr(section['size'])} bytes)</li>"
                )
            else:
                detail_items.append(
                    f"<li><span class='legend mapped' style='background:{color_palette[len(detail_items) % len(color_palette)]};'></span><strong>{html.escape(section['interface'])}</strong> {fmt_addr(section['start'])} to {fmt_addr(section['end'])} ({fmt_addr(section['size'])} bytes)</li>"
                )

        active_class = "active" if idx == 0 else ""
        content_id = f"panel-{idx}"
        tabs.append(
            f"<button class='tab-button {active_class}' data-target='{content_id}'>{name}</button>"
        )
        panels.append(
            f"""
            <section id='{content_id}' class='tab-panel {active_class}'>
              <h2>{name}</h2>
              <p><strong>Source:</strong> {source}</p>
              <p><strong>Description:</strong> {description or 'No description provided.'}</p>
              <div class='map-card'>
                <div class='bar-labels'>
                  <span>{fmt_addr(0)}</span>
                  <div class='zoom-controls'>
                    <button class='zoom-button' onclick='zoomOut(this)'>−</button>
                    <input class='zoom-slider' type='range' min='100' max='400' step='25' value='100' oninput='setZoom(this, this.value)'>
                    <button class='zoom-button' onclick='zoomIn(this)'>+</button>
                  </div>
                  <span>{fmt_addr(max_addr)}</span>
                </div>
                <div class='map-viewport'>
                  <div class='map-scale' data-scale='1'>
                    <div class='address-bar'>
                      {''.join(bar_segments)}
                    </div>
                  </div>
                </div>
                <div class='legend-row'>
                  <span class='legend-item'><span class='legend'></span>Mapped region</span>
                  <span class='legend-item'><span class='legend gap'></span>Unmapped gap</span>
                </div>
              </div>
              <ul class='detail-list'>
                {''.join(detail_items)}
              </ul>
            </section>
            """
        )

    return f"""<!DOCTYPE html>
<html lang='en'>
<head>
  <meta charset='utf-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <title>{html.escape(title)}</title>
  <style>
    body {{ font-family: Arial, sans-serif; margin: 0; background: #f6f8fb; color: #17212b; }}
    .wrap {{ max-width: 1440px; margin: 0 auto; padding: 24px; }}
    h1 {{ margin-bottom: 8px; font-size: 1.8rem; }}
    .intro {{ color: #4b5563; margin-bottom: 20px; font-size: 1rem; }}
    .tab-bar {{ display: flex; flex-wrap: wrap; gap: 8px; margin-bottom: 16px; }}
    .tab-button {{ border: 1px solid #cbd5e1; background: #fff; padding: 10px 14px; border-radius: 10px; cursor: pointer; font-weight: 600; color: #334155; }}
    .tab-button.active {{ background: #2563eb; color: white; border-color: #2563eb; box-shadow: 0 4px 12px rgba(37, 99, 235, 0.2); }}
    .tab-panel {{ display: none; background: white; border: 1px solid #dbe4ee; border-radius: 14px; padding: 20px; box-shadow: 0 8px 24px rgba(15, 23, 42, 0.06); }}
    .tab-panel.active {{ display: block; }}
    .map-card {{ margin-top: 16px; padding: 18px; border: 2px solid #e2e8f0; border-radius: 14px; background: linear-gradient(180deg, #f8fbff 0%, #f3f7fb 100%); }}
    .map-viewport {{ overflow-x: auto; overflow-y: hidden; padding-bottom: 8px; cursor: grab; user-select: none; }}
    .map-viewport.dragging {{ cursor: grabbing; }}
    .map-scale {{ width: max-content; min-width: 100%; transform-origin: left top; transition: transform 0.2s ease; }}
    .address-bar {{ display: flex; width: 100%; min-width: 100%; height: 42px; border-radius: 999px; overflow: hidden; border: 2px solid #94a3b8; background: #e2e8f0; box-shadow: inset 0 1px 2px rgba(0,0,0,0.08); }}
    .bar-segment {{ height: 100%; min-width: 4px; position: relative; }}
    .bar-segment.mapped {{ border-right: 2px solid rgba(255,255,255,0.8); }}
    .bar-segment.gap {{ background: repeating-linear-gradient(90deg, #fef2f2 0 6px, #fde68a 6px 12px); }}
    .bar-segment:hover::after {{ content: attr(data-tooltip); position: absolute; top: -38px; left: 50%; transform: translateX(-50%); white-space: nowrap; background: #0f172a; color: white; padding: 6px 8px; border-radius: 6px; font-size: 0.8rem; z-index: 10; }}
    .bar-labels {{ display: flex; justify-content: space-between; margin-top: 8px; color: #475569; font-size: 0.92rem; font-weight: 600; }}
    .legend-row {{ display: flex; flex-wrap: wrap; gap: 12px; margin-top: 12px; padding: 10px 12px; border-radius: 10px; background: #f8fafc; border: 1px solid #e2e8f0; }}
    .legend-item {{ display: flex; align-items: center; font-size: 0.92rem; color: #334155; }}
    .detail-list {{ margin-top: 16px; padding-left: 18px; color: #334155; }}
    .detail-list li {{ margin-bottom: 8px; line-height: 1.5; }}
    .legend {{ display: inline-block; width: 14px; height: 14px; border-radius: 50%; margin-right: 8px; vertical-align: middle; border: 1px solid rgba(0,0,0,0.15); }}
    .legend.gap {{ background: #f59e0b; }}
    .zoom-controls {{ display: flex; align-items: center; gap: 8px; margin-left: auto; }}
    .zoom-button {{ border: 1px solid #cbd5e1; background: #fff; padding: 6px 10px; border-radius: 8px; cursor: pointer; font-size: 0.9rem; }}
    .zoom-slider {{ width: 140px; accent-color: #2563eb; }}
    code {{ background: #f3f6fa; padding: 2px 6px; border-radius: 4px; }}
  </style>
</head>
<body>
  <div class='wrap'>
    <h1>{html.escape(title)}</h1>
    <p class='intro'>Select a memory map to inspect the mapped address ranges and interfaces.</p>
    <div class='tab-bar memory-map-tabs'>
      {''.join(tabs)}
    </div>
    {''.join(panels)}
  </div>
  <script>
    function setZoom(input, value) {{
      const card = input.closest('.map-card');
      const scaleBox = card.querySelector('.map-scale');
      const viewport = card.querySelector('.map-viewport');
      const scale = Number(value) / 100;
      scaleBox.style.transform = 'scaleX(' + scale + ')';
      scaleBox.dataset.scale = scale;
      scaleBox.style.width = (100 / scale) + '%';
      viewport.scrollLeft = 0;
      viewport.scrollTop = 0;
    }}

    function zoomIn(button) {{
      const card = button.closest('.map-card');
      const slider = card.querySelector('.zoom-slider');
      const next = Math.min(400, Number(slider.value) + 25);
      slider.value = next;
      setZoom(slider, next);
    }}

    function zoomOut(button) {{
      const card = button.closest('.map-card');
      const slider = card.querySelector('.zoom-slider');
      const next = Math.max(100, Number(slider.value) - 25);
      slider.value = next;
      setZoom(slider, next);
    }}

    let isPanning = false;
    let panStartX = 0;
    let panStartScrollLeft = 0;

    document.querySelectorAll('.map-viewport').forEach(viewport => {{
      viewport.addEventListener('mousedown', (event) => {{
        if (event.button !== 0) return;
        isPanning = true;
        viewport.classList.add('dragging');
        panStartX = event.clientX;
        panStartScrollLeft = viewport.scrollLeft;
      }});

      viewport.addEventListener('mousemove', (event) => {{
        if (!isPanning) return;
        const deltaX = event.clientX - panStartX;
        viewport.scrollLeft = panStartScrollLeft - deltaX;
      }});

      window.addEventListener('mouseup', () => {{
        if (!isPanning) return;
        isPanning = false;
        viewport.classList.remove('dragging');
      }});
    }});

    document.querySelectorAll('.tab-button').forEach(btn => {{
      btn.addEventListener('click', () => {{
        document.querySelectorAll('.tab-button').forEach(b => b.classList.remove('active'));
        document.querySelectorAll('.tab-panel').forEach(p => p.classList.remove('active'));
        btn.classList.add('active');
        document.getElementById(btn.dataset.target).classList.add('active');
      }});
    }});
  </script>
</body>
</html>
"""


def write_html(output_path: Path, memory_maps: List[Dict[str, Any]], title: str = "NIC400 Memory Maps") -> Path:
    output_path.write_text(build_html(memory_maps, title), encoding="utf-8")
    return output_path


def main() -> int:
    parser = argparse.ArgumentParser(description="Render NIC400 memory-map XML as an interactive HTML page")
    parser.add_argument("xml", nargs="?", default=str(Path(__file__).with_name("nic400_megasoc_main.xml")), help="Path to the input XML file")
    parser.add_argument("-o", "--output", default=str(Path(__file__).with_name("memory_maps.html")), help="Path to the generated HTML file")
    parser.add_argument("--title", default="NIC400 Memory Maps", help="Title for the generated web page")
    args = parser.parse_args()

    xml_path = Path(args.xml).resolve()
    output_path = Path(args.output).resolve()
    if not xml_path.exists():
        print(f"Input XML not found: {xml_path}", file=sys.stderr)
        return 2

    memory_maps = parse_memory_maps(xml_path)
    write_html(output_path, memory_maps, args.title)
    print(f"Wrote {len(memory_maps)} memory maps to {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
