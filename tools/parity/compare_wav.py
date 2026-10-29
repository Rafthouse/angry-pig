#!/usr/bin/env python3
"""Sample comparison of two float32 WAV renders (for example macOS vs Windows).

usage: compare_wav.py <a.wav> <b.wav> [max_abs_tolerance=1e-4]
Prints max and RMS differences; exit 1 if the max difference exceeds the tolerance.
"""
import sys

import numpy as np

from halo_metrics import read_wav


def main():
    a, _ = read_wav(sys.argv[1])
    b, _ = read_wav(sys.argv[2])
    tol = float(sys.argv[3]) if len(sys.argv) > 3 else 1e-4
    n = min(len(a), len(b))
    d = np.abs(a[:n] - b[:n])
    mx, rms = float(np.max(d)), float(np.sqrt(np.mean(d ** 2)))
    print(f"frames={n} max_abs={mx:.3e} rms={rms:.3e} bit_identical={mx == 0.0} tol={tol:.0e}")
    print("RESULT", "PASS" if mx <= tol else "FAIL")
    return 0 if mx <= tol else 1


if __name__ == "__main__":
    sys.exit(main())
