#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Synthetic visual fixtures and C/Python wire-contract tests; no hardware access."""
import io
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
import zlib
from PIL import Image, ImageDraw
from decode_xbox_diagnostic import decode

COLORS = [(255, 0, 0), (0, 255, 0), (0, 0, 255), (255, 255, 255),
          (0, 0, 0), (128, 128, 128), (255, 255, 0), (0, 255, 255)]


def payload(state):
    players = state.get('players', [dict(present=True, held=0, last=0, presses=0, releases=0)] * 4)
    out = b'XKD1' + struct.pack('<III', state.get('frames', 100), state.get('elapsed_ms', 2000), 1000)
    for p in players:
        out += struct.pack('<BHHII', p['present'], p['held'], p['last'], p['presses'], p['releases'])
    return out + struct.pack('<I', zlib.crc32(out))


def render(raw, size=(640, 480), padding=0, bad_colors=False):
    image = Image.new('RGB', (640, 480), (32, 32, 32))
    draw = ImageDraw.Draw(image)
    draw.rectangle((120, 188, 519, 419), fill=(255, 0, 255))
    for bit in range(576):
        x, y = 128 + bit % 32 * 12, 196 + bit // 32 * 12
        v = 255 if raw[bit // 8] & (1 << (7 - bit % 8)) else 0
        draw.rectangle((x, y, x + 11, y + 11), fill=(v, v, v))
    for index, color in enumerate(COLORS):
        draw.rectangle((64 + index * 64, 436, 127 + index * 64, 459),
                       fill=(0, 0, 0) if bad_colors else color)
    image = image.resize(size)
    if padding:
        padded = Image.new('RGB', (size[0] + padding * 2, size[1] + padding * 2))
        padded.paste(image, (padding, padding)); image = padded
    out = io.BytesIO(); image.save(out, format='JPEG', quality=80)
    return out.getvalue()


class VisualTests(unittest.TestCase):
    def test_scaled_padded_jpeg(self):
        for size in [(640, 480), (720, 480), (1280, 720), (480, 360)]:
            with self.subTest(size=size):
                value = decode(render(payload({}), size, padding=20))
                self.assertEqual(value['frames'], 100)
                self.assertTrue(value['pattern_ok'])
                self.assertEqual(len(value['players']), 4)

    def test_corrupt_payload_rejected(self):
        raw = bytearray(payload({})); raw[8] ^= 1
        with self.assertRaisesRegex(ValueError, 'CRC'):
            decode(render(raw))

    def test_wrong_screen_rejected(self):
        out = io.BytesIO(); Image.new('RGB', (720, 480)).save(out, format='JPEG')
        with self.assertRaisesRegex(ValueError, 'not found'):
            decode(out.getvalue())

    def test_color_failure_not_hidden_by_valid_crc(self):
        self.assertFalse(decode(render(payload({}), bad_colors=True))['pattern_ok'])

    def test_c_protocol_and_transitions(self):
        header = Path(__file__).resolve().parents[1] / 'diagnostics/xbox'
        source = r'''
#include <assert.h>
#include <stdio.h>
#include "telemetry.h"
int main(void) {
    struct diag_pad pads[4] = {{0}};
    pads[0].present = 1;
    diag_sample(&pads[0], 256); diag_sample(&pads[0], 256);
    assert(pads[0].presses == 1);
    diag_sample(&pads[0], 0); diag_sample(&pads[0], 0);
    assert(pads[0].releases == 1 && pads[0].last == 256);
    uint8_t out[DIAG_BYTES]; diag_encode(out, 123, 456, 789, pads);
    return fwrite(out, 1, sizeof(out), stdout) != sizeof(out);
}
'''
        with tempfile.TemporaryDirectory() as directory:
            src, exe = Path(directory) / 'fixture.c', Path(directory) / 'fixture'
            src.write_text(source)
            subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror', '-I', str(header), str(src), '-o', str(exe)], check=True)
            raw = subprocess.check_output([str(exe)])
        result = decode(render(raw))
        self.assertEqual((result['frames'], result['elapsed_ms'], result['free_pages']), (123, 456, 789))
        self.assertEqual(result['players'][0], dict(present=True, held=0, last=256, presses=1, releases=1))


if __name__ == '__main__':
    if len(sys.argv) > 1 and sys.argv[1] == '--render-fixture':
        state = json.load(sys.stdin)
        sys.stdout.buffer.write(render(payload(state), bad_colors=state.get('bad_colors', False)))
    else:
        unittest.main()
