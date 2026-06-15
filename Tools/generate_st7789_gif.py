#!/usr/bin/env python3
"""Convert a 120x120 GIF into compact ST7789 animation data."""

from __future__ import annotations

import hashlib
from pathlib import Path
from typing import Iterable, Sequence

from PIL import Image

WIDTH = 120
HEIGHT = 120
PIXEL_SCALE = 2
PALETTE_SIZE = 16
FRAME_STEP = 3
FRAME_INTERVAL_MS = 120
FIRST_FRAME_SIZE = WIDTH * HEIGHT // 2


def load_frames(path: Path) -> list[Image.Image]:
    """Load composited GIF frames and keep every third 40 ms frame."""
    image = Image.open(path)
    if image.size != (WIDTH, HEIGHT):
        raise ValueError(f"expected {WIDTH}x{HEIGHT}, got {image.size}")
    if image.n_frames < 2:
        raise ValueError("the GIF must contain at least two frames")

    frames = []
    for index in range(0, image.n_frames, FRAME_STEP):
        image.seek(index)
        frames.append(image.convert("RGB").copy())
    return frames


def quantize_frames(
    frames: Sequence[Image.Image],
) -> tuple[list[int], list[list[int]]]:
    """Build one global 16-color palette and quantize every frame without dithering."""
    strip = Image.new("RGB", (WIDTH, HEIGHT * len(frames)))
    for index, frame in enumerate(frames):
        strip.paste(frame, (0, index * HEIGHT))

    quantized_strip = strip.quantize(
        colors=PALETTE_SIZE,
        method=Image.Quantize.MEDIANCUT,
        dither=Image.Dither.NONE,
    )
    palette_rgb = quantized_strip.getpalette()[: PALETTE_SIZE * 3]
    palette_image = Image.new("P", (1, 1))
    palette_image.putpalette(palette_rgb + [0] * (768 - len(palette_rgb)))

    indexes = []
    for frame in frames:
        quantized = frame.quantize(
            palette=palette_image,
            dither=Image.Dither.NONE,
        )
        indexes.append(list(quantized.tobytes()))

    palette_565 = []
    for offset in range(0, len(palette_rgb), 3):
        red, green, blue = palette_rgb[offset : offset + 3]
        palette_565.append(
            ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3)
        )
    return palette_565, indexes


def pack_indexes(indexes: Sequence[int]) -> list[int]:
    """Pack two four-bit palette indexes into each byte."""
    if len(indexes) & 1:
        raise ValueError("four-bit data must contain an even number of pixels")
    return [
        (indexes[offset] << 4) | indexes[offset + 1]
        for offset in range(0, len(indexes), 2)
    ]


def encode_delta(current: Sequence[int], previous: Sequence[int]) -> list[int]:
    """Encode one frame as row-local skip and literal runs."""
    encoded: list[int] = []
    for row in range(HEIGHT):
        row_start = row * WIDTH
        column = 0
        while column < WIDTH:
            unchanged = (
                current[row_start + column] == previous[row_start + column]
            )
            count = 1
            while (
                column + count < WIDTH
                and count < 128
                and (
                    current[row_start + column + count]
                    == previous[row_start + column + count]
                )
                == unchanged
            ):
                count += 1

            if unchanged:
                encoded.append(0x80 | (count - 1))
            else:
                encoded.append(count - 1)
                run = current[
                    row_start + column : row_start + column + count
                ]
                if len(run) & 1:
                    run = [*run, 0]
                encoded.extend(pack_indexes(run))
            column += count
    return encoded


def apply_delta(encoded: Sequence[int], previous: Sequence[int]) -> list[int]:
    """Decode one generated delta stream for an exact round-trip check."""
    current = list(previous)
    position = 0
    for row in range(HEIGHT):
        column = 0
        row_start = row * WIDTH
        while column < WIDTH:
            control = encoded[position]
            position += 1
            count = (control & 0x7F) + 1
            if control & 0x80:
                column += count
                continue

            payload_size = (count + 1) // 2
            payload = encoded[position : position + payload_size]
            position += payload_size
            for run_index in range(count):
                packed = payload[run_index // 2]
                palette_index = packed >> 4 if run_index % 2 == 0 else packed & 0x0F
                current[row_start + column + run_index] = palette_index
            column += count

    if position != len(encoded):
        raise ValueError("delta round-trip left unused bytes")
    return current


def format_bytes(data: Iterable[int], indent: str = "    ") -> str:
    """Format bytes as compact ARMCC-compatible C initializers."""
    values = list(data)
    return ",\n".join(
        indent
        + ", ".join(f"0x{value:02X}U" for value in values[start : start + 16])
        for start in range(0, len(values), 16)
    )


def format_words(data: Iterable[int], indent: str = "    ") -> str:
    """Format 16-bit values as C initializers."""
    values = list(data)
    return ",\n".join(
        indent
        + ", ".join(f"0x{value:04X}U" for value in values[start : start + 8])
        for start in range(0, len(values), 8)
    )


def format_dwords(data: Iterable[int], indent: str = "    ") -> str:
    """Format 32-bit values as C initializers."""
    values = list(data)
    return ",\n".join(
        indent
        + ", ".join(f"{value}U" for value in values[start : start + 8])
        for start in range(0, len(values), 8)
    )


def build_header(frame_count: int, delta_size: int, source_hash: str) -> str:
    """Build declarations and format metadata for the generated animation."""
    return f"""#ifndef __ANIM_FRAMES_H
#define __ANIM_FRAMES_H

#include "stm32f10x.h"

#define ANIM_FRAME_WIDTH          {WIDTH}U    // Indexed source width
#define ANIM_FRAME_HEIGHT         {HEIGHT}U    // Indexed source height
#define ANIM_PIXEL_SCALE          {PIXEL_SCALE}U      // Output scale to 240x240
#define ANIM_FRAME_COUNT          {frame_count}U     // Number of retained GIF frames
#define ANIM_FRAME_INTERVAL_MS    {FRAME_INTERVAL_MS}U     // Display time per retained frame
#define ANIM_FIRST_FRAME_SIZE     {FIRST_FRAME_SIZE}U   // Packed first-frame bytes
#define ANIM_DELTA_DATA_SIZE      {delta_size}U  // Encoded bytes for frames 1..N
#define ANIM_SOURCE_SHA256        "{source_hash}"

extern const uint16_t anim_palette[16];
extern const uint8_t anim_first_frame[ANIM_FIRST_FRAME_SIZE];
extern const uint32_t anim_delta_offsets[ANIM_FRAME_COUNT];
extern const uint8_t anim_delta_data[ANIM_DELTA_DATA_SIZE];

#endif
"""


def build_source(
    palette: Sequence[int],
    first_frame: Sequence[int],
    offsets: Sequence[int],
    delta_data: Sequence[int],
) -> str:
    """Build constant RGB565 palette, first frame, offsets, and delta stream."""
    return f"""#include "anim_frames.h"

const uint16_t anim_palette[16] =
{{
{format_words(palette)}
}};

const uint8_t anim_first_frame[ANIM_FIRST_FRAME_SIZE] =
{{
{format_bytes(first_frame)}
}};

const uint32_t anim_delta_offsets[ANIM_FRAME_COUNT] =
{{
{format_dwords(offsets)}
}};

const uint8_t anim_delta_data[ANIM_DELTA_DATA_SIZE] =
{{
{format_bytes(delta_data)}
}};
"""


def main() -> int:
    """Generate firmware animation data from the local cat GIF."""
    project_root = Path(__file__).resolve().parent.parent
    source = project_root / "小猫图.gif"
    output_c = project_root / "User" / "anim_frames.c"
    output_h = project_root / "User" / "anim_frames.h"

    frames = load_frames(source)
    palette, indexes = quantize_frames(frames)
    first_frame = pack_indexes(indexes[0])

    offsets = [0]
    delta_data: list[int] = []
    for frame_index in range(1, len(indexes)):
        encoded = encode_delta(indexes[frame_index], indexes[frame_index - 1])
        if apply_delta(encoded, indexes[frame_index - 1]) != indexes[frame_index]:
            raise ValueError(f"delta round-trip failed for frame {frame_index}")
        delta_data.extend(encoded)
        offsets.append(len(delta_data))

    source_hash = hashlib.sha256(source.read_bytes()).hexdigest()
    output_h.write_text(
        build_header(len(indexes), len(delta_data), source_hash),
        encoding="gbk",
        newline="\n",
    )
    output_c.write_text(
        build_source(palette, first_frame, offsets, delta_data),
        encoding="gbk",
        newline="\n",
    )
    print(
        f"generated {len(indexes)} frames: "
        f"{len(first_frame)} first-frame bytes + "
        f"{len(delta_data)} delta bytes"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
