#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Decode the XKD1 diagnostic panel from a screenshot; no OCR or device access."""
import io
import json
import struct
import sys
import zlib
from PIL import Image, ImageChops, ImageStat


def decode(data):
    if len(data) > 4 * 1024 * 1024:
        raise ValueError('Screenshot exceeds limit')
    image = Image.open(io.BytesIO(data))
    if image.width * image.height > 4096 * 2160:
        raise ValueError('Screenshot dimensions exceed limit')
    image = image.convert('RGB')
    r, g, b = image.split()
    marker = ImageChops.multiply(ImageChops.multiply(
        r.point(lambda v: 255 if v > 140 else 0),
        b.point(lambda v: 255 if v > 140 else 0)),
        g.point(lambda v: 255 if v < 110 else 0))
    box = marker.getbbox()
    if box is None:
        raise ValueError('Diagnostic marker not found')
    left, top, right, bottom = box
    sx, sy = (right - left) / 400, (bottom - top) / 232
    if sx < 0.4 or sy < 0.4:
        raise ValueError('Diagnostic marker too small')

    def sample(x, y):
        # Map title coordinates using the visible border, including HDMI padding/stretch.
        px = round(left + (x - 120) * sx)
        py = round(top + (y - 188) * sy)
        if not (1 <= px < image.width - 1 and 1 <= py < image.height - 1):
            raise ValueError('Reference region outside screenshot')
        return ImageStat.Stat(image.crop((px - 1, py - 1, px + 2, py + 2))).mean

    payload = bytearray(72)
    for bit in range(576):
        channels = sample(128 + (bit % 32) * 12 + 6, 196 + (bit // 32) * 12 + 6)
        if max(channels) - min(channels) > 65:
            raise ValueError('Colored/corrupted diagnostic cell')
        level = sum(channels) / 3
        if 85 < level < 170:
            raise ValueError('Ambiguous diagnostic cell')
        payload[bit // 8] |= int(level >= 170) << (7 - bit % 8)
    if payload[:4] != b'XKD1' or zlib.crc32(payload[:68]) != struct.unpack_from('<I', payload, 68)[0]:
        raise ValueError('Diagnostic signature/CRC mismatch (wrong or torn frame)')
    frames, elapsed_ms, free_pages = struct.unpack_from('<III', payload, 4)
    players = []
    for index in range(4):
        present, held, last, presses, releases = struct.unpack_from('<BHHII', payload, 16 + index * 13)
        if present not in (0, 1):
            raise ValueError('Invalid controller presence')
        players.append(dict(present=bool(present), held=held, last=last, presses=presses, releases=releases))
    colors = [(255, 0, 0), (0, 255, 0), (0, 0, 255), (255, 255, 255),
              (0, 0, 0), (128, 128, 128), (255, 255, 0), (0, 255, 255)]
    observed = [sample(96 + index * 64, 448) for index in range(8)]
    pattern_ok = all(max(abs(a - b) for a, b in zip(actual, expected)) <= 65
                     for actual, expected in zip(observed, colors))
    return dict(schema=1, frames=frames, elapsed_ms=elapsed_ms, free_pages=free_pages,
                players=players, marker_box=box, pattern_ok=pattern_ok,
                color_samples=[[round(v) for v in color] for color in observed])


if __name__ == '__main__':
    try:
        print(json.dumps(decode(sys.stdin.buffer.read(4 * 1024 * 1024 + 1))))
    except Exception as exc:
        print(str(exc), file=sys.stderr)
        sys.exit(1)
