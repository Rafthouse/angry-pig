#!/usr/bin/env python3
"""Writes a stereo float32 WAV sine for the halo / harmonic parity check.

usage: gen_sine.py <out.wav> [rate=44100] [freq=999] [dbfs=-12] [seconds=10.5]
Standard library only. Deterministic.
"""
import math
import struct
import sys


def main():
    out = sys.argv[1]
    rate = int(sys.argv[2]) if len(sys.argv) > 2 else 44100
    freq = float(sys.argv[3]) if len(sys.argv) > 3 else 999.0
    dbfs = float(sys.argv[4]) if len(sys.argv) > 4 else -12.0
    seconds = float(sys.argv[5]) if len(sys.argv) > 5 else 10.5
    n = int(round(rate * seconds))
    amp = 10.0 ** (dbfs / 20.0)
    frames = bytearray()
    for i in range(n):
        v = amp * math.sin(2.0 * math.pi * freq * i / rate)
        frames += struct.pack("<ff", v, v)
    with open(out, "wb") as f:
        data = bytes(frames)
        fmt = struct.pack("<HHIIHH", 3, 2, rate, rate * 8, 8, 32)
        f.write(b"RIFF" + struct.pack("<I", 4 + 8 + len(fmt) + 8 + len(data)) + b"WAVE")
        f.write(b"fmt " + struct.pack("<I", len(fmt)) + fmt)
        f.write(b"data" + struct.pack("<I", len(data)) + data)
    print(f"wrote {out}: {n} frames, {rate} Hz, {freq} Hz, {dbfs} dBFS")


if __name__ == "__main__":
    main()
