#!/usr/bin/env python3
"""Convert an animated GIF to SSD1306 128x64 page-format C arrays."""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Iterable, Sequence

WIDTH = 128
HEIGHT = 64
FRAME_SIZE = WIDTH * HEIGHT // 8


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Convert GIF frames to SSD1306 128x64 C arrays."
    )
    parser.add_argument("input_gif", type=Path, help="input GIF path")
    parser.add_argument("output_c", type=Path, help="output C source path")
    parser.add_argument(
        "--max-frames",
        type=int,
        default=20,
        help="maximum number of frames to export (default: 20)",
    )
    parser.add_argument(
        "--threshold",
        type=int,
        default=128,
        help="black/white threshold from 0 to 255 (default: 128)",
    )
    parser.add_argument(
        "--interval",
        type=int,
        default=100,
        help="firmware frame interval in milliseconds (default: 100)",
    )
    parser.add_argument(
        "--invert",
        action="store_true",
        help="invert black and white output pixels",
    )
    return parser.parse_args()


def validate_args(args: argparse.Namespace) -> None:
    if args.max_frames <= 0:
        raise ValueError("--max-frames must be greater than zero")
    if not 0 <= args.threshold <= 255:
        raise ValueError("--threshold must be between 0 and 255")
    if args.interval <= 0:
        raise ValueError("--interval must be greater than zero")
    if args.output_c.suffix.lower() != ".c":
        raise ValueError("output path must end with .c")


def pack_page_pixels(pixels: Sequence[int]) -> list[int]:
    """Pack row-major monochrome pixels into SSD1306 page bytes."""
    if len(pixels) != WIDTH * HEIGHT:
        raise ValueError(f"expected {WIDTH * HEIGHT} pixels, got {len(pixels)}")

    packed: list[int] = []
    for page in range(HEIGHT // 8):
        for x in range(WIDTH):
            value = 0
            for bit in range(8):
                y = page * 8 + bit
                if pixels[y * WIDTH + x]:
                    value |= 1 << bit
            packed.append(value)

    return packed


def load_gif_frames(
    input_path: Path, max_frames: int, threshold: int, invert: bool
) -> list[list[int]]:
    try:
        from PIL import Image, ImageOps, ImageSequence
    except ImportError as exc:
        raise RuntimeError(
            "Pillow is required. Install it with: py -m pip install pillow"
        ) from exc

    frames: list[list[int]] = []
    with Image.open(input_path) as gif:
        for index, frame in enumerate(ImageSequence.Iterator(gif)):
            if index >= max_frames:
                break

            rgba = frame.convert("RGBA")
            background = Image.new("RGBA", rgba.size, (0, 0, 0, 255))
            background.alpha_composite(rgba)
            fitted = ImageOps.fit(
                background.convert("L"),
                (WIDTH, HEIGHT),
                method=Image.Resampling.LANCZOS,
            )

            pixels = []
            for luminance in fitted.getdata():
                is_on = luminance >= threshold
                pixels.append(int(not is_on if invert else is_on))

            frames.append(pack_page_pixels(pixels))

    if not frames:
        raise ValueError("the input GIF contains no frames")

    return frames


def format_bytes(data: Iterable[int], indent: str = "        ") -> str:
    values = list(data)
    lines = []
    for start in range(0, len(values), 16):
        chunk = values[start : start + 16]
        lines.append(indent + ", ".join(f"0x{value:02X}U" for value in chunk))
    return ",\n".join(lines)


def build_header(frame_count: int, interval: int) -> str:
    return f"""#ifndef __ANIM_FRAMES_H
#define __ANIM_FRAMES_H

#include "stm32f10x.h"

#define ANIM_FRAME_WIDTH          {WIDTH}U    // 动画帧宽度
#define ANIM_FRAME_HEIGHT         {HEIGHT}U     // 动画帧高度
#define ANIM_FRAME_SIZE           {FRAME_SIZE}U   // 每帧页格式字节数
#define ANIM_FRAME_COUNT          {frame_count}U     // GIF 转换后的动画帧数
#define ANIM_FRAME_INTERVAL_MS    {interval}U    // 动画帧间隔，单位 ms

extern const uint8_t anim_frames[ANIM_FRAME_COUNT][ANIM_FRAME_SIZE];

#endif
"""


def build_source(header_name: str, frames: Sequence[Sequence[int]]) -> str:
    frame_blocks = []
    for index, frame in enumerate(frames):
        frame_blocks.append(
            "    {\n"
            f"        /* 动画帧 {index} */\n"
            f"{format_bytes(frame)}\n"
            "    }"
        )

    return (
        f'#include "{header_name}"\n\n'
        "const uint8_t anim_frames[ANIM_FRAME_COUNT][ANIM_FRAME_SIZE] =\n"
        "{\n"
        + ",\n".join(frame_blocks)
        + "\n};\n"
    )


def write_outputs(
    output_c: Path, frames: Sequence[Sequence[int]], interval: int
) -> tuple[Path, Path]:
    output_h = output_c.with_suffix(".h")
    output_c.parent.mkdir(parents=True, exist_ok=True)
    output_h.write_text(
        build_header(len(frames), interval), encoding="gbk", newline="\n"
    )
    output_c.write_text(
        build_source(output_h.name, frames), encoding="gbk", newline="\n"
    )
    return output_c, output_h


def main() -> int:
    args = parse_args()
    try:
        validate_args(args)
        frames = load_gif_frames(
            args.input_gif, args.max_frames, args.threshold, args.invert
        )
        output_c, output_h = write_outputs(args.output_c, frames, args.interval)
    except (OSError, RuntimeError, ValueError) as exc:
        raise SystemExit(f"error: {exc}") from exc

    print(f"exported {len(frames)} frame(s)")
    print(f"C source: {output_c}")
    print(f"header:   {output_h}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
