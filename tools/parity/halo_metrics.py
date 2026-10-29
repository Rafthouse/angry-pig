#!/usr/bin/env python3
"""Halo / harmonic / LF metrics of a render of the 999 Hz test sine (tools/parity/gen_sine.py),
and a cross-platform comparison against a reference render.

Same definitions as the Dynamic Saturation reports: 1 s windows (1 Hz bins) after a 24000-sample
warm-up, 44.1 kHz. Halo = energy at distances 1..127 Hz from the even harmonics H2..H14, relative to
the 999 Hz line of the same window, averaged over 10 windows (dBc).

usage:
  halo_metrics.py <render.wav>                         print metrics as JSON
  halo_metrics.py <render.wav> --ref <reference.wav>   compare; exit 1 if a tolerance is exceeded
  halo_metrics.py <render.wav> --expect <metrics.json> compare metrics with stored values
Requires numpy.
"""
import json
import struct
import sys

import numpy as np

FS = 44100
K = 999
W0 = 24000
SECONDS = 10
EVEN_H = list(range(2, 15, 2))

# Tolerances for a different CPU architecture or math library (not bit-exact by design).
TOL = {"halo_mean": 0.05, "LF_20_200": 0.05, "odd_dBc": 0.01, "DC_floor": -100.0, "max_abs_diff": 1e-5}


def read_wav(path):
    b = open(path, "rb").read()
    if b[:4] != b"RIFF" or b[8:12] != b"WAVE":
        raise ValueError(f"not a WAV file: {path}")
    i, fmt, data = 12, None, None
    while i + 8 <= len(b):
        cid, ln = b[i:i + 4], struct.unpack("<I", b[i + 4:i + 8])[0]
        body = b[i + 8:i + 8 + ln]
        if cid == b"fmt ":
            fmt = struct.unpack("<HHIIHH", body[:16])
        elif cid == b"data":
            data = body
        i += 8 + ln + (ln & 1)
    tag, ch, rate, _, _, bits = fmt
    if tag != 3 or bits != 32:
        raise ValueError("expected 32-bit float WAV")
    x = np.frombuffer(data, dtype="<f4").reshape(-1, ch).astype(np.float64)
    return x, rate


def dist_even(fr):
    d = np.full(len(fr), np.inf)
    for k in EVEN_H:
        d = np.minimum(d, np.abs(fr - k * K))
    return d


def metrics(y):
    fr = np.arange(FS // 2 + 1).astype(float)
    dist = dist_even(fr)
    halo_m = (dist >= 1.0) & (dist < 128.0)
    lf_m = (fr >= 20.0) & (fr < 200.0)
    halo, lf, dc, odd = [], [], [], {k: [] for k in range(3, 16, 2)}
    for j in range(SECONDS):
        seg = y[W0 + j * FS:W0 + (j + 1) * FS]
        X = np.fft.rfft(seg) * 2 / len(seg)
        a1 = abs(X[K])
        P = np.abs(X) ** 2
        halo.append(10 * np.log10(np.sum(P[halo_m]) / a1 ** 2 + 1e-30))
        lf.append(10 * np.log10(np.sum(P[lf_m]) / a1 ** 2 + 1e-30))
        dc.append(20 * np.log10(abs(X[0]) / a1 + 1e-30))
        for k in odd:
            odd[k].append(20 * np.log10(abs(X[k * K]) / a1 + 1e-30))
    return {
        "halo_mean": float(np.mean(halo)),
        "LF_20_200": float(np.mean(lf)),
        "DC": float(np.mean(dc)),
        "odd_dBc": {f"H{k}": float(np.mean(v)) for k, v in odd.items()},
    }


def compare(m, r, diff=None):
    fails = []
    for key in ("halo_mean", "LF_20_200"):
        d = abs(m[key] - r[key])
        print(f"{key:10s} render={m[key]:.4f} ref={r[key]:.4f} delta={d:.4f} tol={TOL[key]}")
        if d > TOL[key]:
            fails.append(key)
    for h, v in m["odd_dBc"].items():
        d = abs(v - r["odd_dBc"][h])
        if d > TOL["odd_dBc"]:
            fails.append(h)
    print("odd H3..H15 max delta %.5f dB (tol %.2f)" % (max(abs(v - r["odd_dBc"][h]) for h, v in m["odd_dBc"].items()), TOL["odd_dBc"]))
    print(f"DC render={m['DC']:.2f} dBc (must stay below {TOL['DC_floor']})")
    if m["DC"] > TOL["DC_floor"]:
        fails.append("DC")
    if diff is not None:
        print(f"max abs sample difference {diff:.3e} (tol {TOL['max_abs_diff']:.0e}), bit-identical={diff == 0.0}")
        if diff > TOL["max_abs_diff"]:
            fails.append("samples")
    print("RESULT", "FAIL " + ",".join(fails) if fails else "PASS")
    return 1 if fails else 0


def main():
    x, rate = read_wav(sys.argv[1])
    if rate != FS:
        raise SystemExit(f"expected {FS} Hz, got {rate}")
    m = metrics(x[:, 0])
    if len(sys.argv) == 2:
        print(json.dumps(m, indent=1))
        return 0
    if sys.argv[2] == "--ref":
        r_x, _ = read_wav(sys.argv[3])
        n = min(len(r_x), len(x))
        diff = float(np.max(np.abs(r_x[:n] - x[:n])))
        return compare(m, metrics(r_x[:, 0]), diff)
    if sys.argv[2] == "--expect":
        return compare(m, json.load(open(sys.argv[3])))
    raise SystemExit(__doc__)


if __name__ == "__main__":
    sys.exit(main())
