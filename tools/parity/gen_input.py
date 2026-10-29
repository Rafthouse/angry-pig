#!/usr/bin/env python3
"""Writes the shared stereo test signal for the VST3/AU parity check.

Standard library only, so the CI job needs no extra packages. The signal is
fully deterministic: a fixed-seed generator, 48 kHz, 32-bit float WAV.

Segments (10 s total):
  0.0 - 4.0 s  tonal music-like content (fundamentals and harmonics) plus low-level noise
  4.0 - 7.0 s  logarithmic sweep 20 Hz to 20 kHz at -12 dBFS peak
  7.0 - 10.0 s noise bursts at -18 dBFS RMS with silence between them
"""
import math
import struct
import sys

RATE = 48000
DURATION = 10.0
N = int(RATE * DURATION)


class Lcg:
    def __init__(self, seed):
        self.state = seed & 0xFFFFFFFF

    def next(self):
        self.state = (1664525 * self.state + 1013904223) & 0xFFFFFFFF
        return self.state / 0x80000000 - 1.0  # roughly uniform in [-1, 1)


def build(seed_l, seed_r):
    left = [0.0] * N
    right = [0.0] * N
    rng_l, rng_r = Lcg(seed_l), Lcg(seed_r)
    # Pink-ish noise via a one-pole lowpass of white noise.
    pink_l = pink_r = 0.0
    for i in range(N):
        t = i / RATE
        if t < 4.0:
            tone_l = (0.30 * math.sin(2 * math.pi * 110.0 * t)
                      + 0.20 * math.sin(2 * math.pi * 440.0 * t + 0.3)
                      + 0.10 * math.sin(2 * math.pi * 3000.0 * t + 1.1)
                      + 0.05 * math.sin(2 * math.pi * 6000.0 * t))
            tone_r = (0.30 * math.sin(2 * math.pi * 165.0 * t + 0.7)
                      + 0.20 * math.sin(2 * math.pi * 660.0 * t)
                      + 0.10 * math.sin(2 * math.pi * 2500.0 * t + 2.0)
                      + 0.05 * math.sin(2 * math.pi * 8000.0 * t + 0.5))
            pink_l = 0.95 * pink_l + 0.05 * rng_l.next()
            pink_r = 0.95 * pink_r + 0.05 * rng_r.next()
            left[i] = tone_l + 0.03 * pink_l
            right[i] = tone_r + 0.03 * pink_r
        elif t < 7.0:
            u = (t - 4.0) / 3.0
            f = 20.0 * (1000.0 ** u)
            phase = 2 * math.pi * 20.0 * 3.0 / math.log(1000.0) * (1000.0 ** u - 1.0)
            s = 0.25 * math.sin(phase)
            left[i] = s
            right[i] = 0.25 * math.sin(phase + 0.2)
        else:
            burst = int((t - 7.0) / 0.5) % 2 == 0
            if burst:
                left[i] = 0.12 * rng_l.next()
                right[i] = 0.12 * rng_r.next()
    return left, right


def write_wav(path, left, right):
    data = bytearray()
    for l, r in zip(left, right):
        data += struct.pack("<ff", l, r)
    with open(path, "wb") as f:
        f.write(b"RIFF" + struct.pack("<I", 36 + len(data)) + b"WAVE")
        f.write(b"fmt " + struct.pack("<IHHIIHH", 16, 3, 2, RATE, RATE * 8, 8, 32))
        f.write(b"data" + struct.pack("<I", len(data)) + data)


if __name__ == "__main__":
    out = sys.argv[1] if len(sys.argv) > 1 else "input.wav"
    l, r = build(12345, 67890)
    peak = max(max(abs(x) for x in l), max(abs(x) for x in r))
    write_wav(out, l, r)
    print(f"wrote {out}: {N} frames/channel, {RATE} Hz, float32, peak {peak:.4f}")
