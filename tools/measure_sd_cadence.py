#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Strictly decode an SD take and measure intervals in the video PTS time base."""
import argparse
from fractions import Fraction
import json
import math
from pathlib import Path
import re
from statistics import median
import subprocess


def measure(path, ffmpeg):
    command = [ffmpeg, '-nostdin', '-hide_banner', '-nostats', '-xerror', '-i', str(path),
               '-map', '0:v:0', '-map', '0:a:0', '-vf', 'showinfo=checksum=0',
               '-fps_mode', 'passthrough', '-f', 'null', '-']
    decoded = subprocess.run(command, capture_output=True, text=True)
    if decoded.returncode:
        raise ValueError(decoded.stderr[-2000:])
    timebase = Fraction(re.search(r'config in time_base: (\d+/\d+)', decoded.stderr)[1])
    pts = [int(p) for p in re.findall(r'\bn:\s*\d+ pts:\s*(-?\d+) ', decoded.stderr)]
    intervals = sorted(float((b - a) * timebase * 1000) for a, b in zip(pts, pts[1:]))
    if not intervals or intervals[0] <= 0:
        raise ValueError('Expected at least two frames with increasing presentation timestamps')
    return dict(frames=len(pts), fps=1000 * len(intervals) / sum(intervals),
                median_ms=median(intervals), p95_ms=intervals[math.ceil(.95 * len(intervals)) - 1],
                longest_ms=intervals[-1])


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('clip', type=Path)
    parser.add_argument('--ffmpeg', default='ffmpeg')
    args = parser.parse_args()
    print(json.dumps(measure(args.clip, args.ffmpeg), indent=2))
