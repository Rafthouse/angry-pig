#!/usr/bin/env python3
"""Compares the VST3 and AU renders of the same input and parameters.

usage: analyze.py <dir> [--out <dir>]
  expects in <dir>: input.wav, vst3_fixed.wav, au_fixed.wav, vst3_ramp.wav, au_ramp.wav

Criteria are fixed in this file before the first run. They are not relaxed to fit a result.
A criterion that fails is reported with the measured value and its likely cause; it is
not waived here.
"""
import json
import math
import os
import struct
import sys

import numpy as np

RATE = 48000

# Pass criteria (fixed in advance).
NULL_MAX_DB = -80.0          # residual RMS relative to output RMS, fixed parameters
SPECTRUM_MAX_DB = 0.10       # max magnitude difference over bins above -60 dB re peak
ENVELOPE_MAX_DB = 0.20       # max difference of 100 ms RMS envelopes, ramp test
ENVELOPE_MIN_CORR = 0.999    # correlation of the two envelopes, ramp test
CLICK_MAX_DB = -60.0         # largest residual sample relative to output peak
LAG_MAX_SAMPLES = 0          # cross-correlation lag between the two renders


def read_wav(path):
    with open(path, "rb") as f:
        raw = f.read()
    if raw[:4] != b"RIFF" or raw[8:12] != b"WAVE":
        raise ValueError(f"{path}: not a WAVE file")
    p = 12
    fmt = None
    data = None
    while p + 8 <= len(raw):
        cid = raw[p:p + 4]
        (size,) = struct.unpack("<I", raw[p + 4:p + 8])
        body = raw[p + 8:p + 8 + size]
        if cid == b"fmt ":
            fmt = struct.unpack("<HHIIHH", body[:16])
        elif cid == b"data":
            data = body
        p += 8 + size + (size & 1)
    if fmt is None or data is None:
        raise ValueError(f"{path}: missing fmt or data chunk")
    tag, channels, rate, _, _, bits = fmt
    if tag != 3 or bits != 32 or channels != 2:
        raise ValueError(f"{path}: expected float32 stereo, got tag={tag} bits={bits} ch={channels}")
    arr = np.frombuffer(data, dtype="<f4").reshape(-1, 2).astype(np.float64)
    return arr, rate


def db(x):
    x = float(x)
    return -999.0 if x <= 0 else 20.0 * math.log10(x)


def rms(x):
    return math.sqrt(float(np.mean(x * x))) if x.size else 0.0


def lag_of(a, b, max_lag=2048):
    """Lag (samples) that maximises the cross-correlation of channel 0, b relative to a."""
    n = min(len(a), len(b), RATE * 2)
    x = a[:n, 0] - np.mean(a[:n, 0])
    y = b[:n, 0] - np.mean(b[:n, 0])
    corr = np.correlate(y, x, mode="full")
    mid = len(x) - 1
    window = corr[mid - max_lag:mid + max_lag + 1]
    return int(np.argmax(np.abs(window)) - max_lag)


def average_spectrum(x, nfft=8192, hop=4096):
    win = np.hanning(nfft)
    frames = []
    for start in range(0, len(x) - nfft, hop):
        seg = x[start:start + nfft] * win
        frames.append(np.abs(np.fft.rfft(seg)) ** 2)
    return np.mean(frames, axis=0) if frames else np.zeros(nfft // 2 + 1)


def spectral_diff_db(a_pow, b_pow, freqs, floor_db=-60.0):
    peak = np.max(a_pow)
    mask = (freqs >= 20) & (freqs <= 20000)
    a_db = 10 * np.log10(a_pow + 1e-30)
    b_db = 10 * np.log10(b_pow + 1e-30)
    keep = mask & (a_db > 10 * np.log10(peak) + floor_db)
    if not np.any(keep):
        return 0.0, 0.0
    diff = np.abs(a_db[keep] - b_db[keep])
    idx = np.argmax(diff)
    return float(diff[idx]), float(freqs[keep][idx])


ENVELOPE_FLOOR_DB = -120.0   # digital silence is floored here
ENVELOPE_COMPARE_DB = -60.0  # a window is compared only when both renders are above this level


def envelope_db(x, win=RATE // 10):
    n = len(x) // win
    env = np.sqrt(np.mean(x[:n * win].reshape(n, win, 2) ** 2, axis=(1, 2)))
    return np.maximum(20 * np.log10(env + 1e-12), ENVELOPE_FLOOR_DB)


def check_pair(name, v, a, rep):
    res = {}
    res["nan_inf"] = int(np.sum(~np.isfinite(v)) + np.sum(~np.isfinite(a)))
    res["vst3_rms_dbfs"] = [db(rms(v[:, c])) for c in range(2)]
    res["au_rms_dbfs"] = [db(rms(a[:, c])) for c in range(2)]
    res["vst3_peak_dbfs"] = [db(np.max(np.abs(v[:, c]))) for c in range(2)]
    res["au_peak_dbfs"] = [db(np.max(np.abs(a[:, c]))) for c in range(2)]
    res["level_delta_db"] = [res["au_rms_dbfs"][c] - res["vst3_rms_dbfs"][c] for c in range(2)]
    res["dc_vst3"] = [float(np.mean(v[:, c])) for c in range(2)]
    res["dc_au"] = [float(np.mean(a[:, c])) for c in range(2)]
    r = v - a
    out_rms = max(rms(v), 1e-12)
    res["residual_rms_dbfs"] = db(rms(r))
    res["null_rel_db"] = db(rms(r) / out_rms)
    res["residual_peak_dbfs"] = db(np.max(np.abs(r)))
    res["click_rel_db"] = db(np.max(np.abs(r)) / max(np.max(np.abs(v)), 1e-12))
    res["lag_samples"] = lag_of(v, a)
    freqs = np.fft.rfftfreq(8192, 1 / RATE)
    diffs = []
    for c in range(2):
        pv = average_spectrum(v[:, c])
        pa = average_spectrum(a[:, c])
        d, f = spectral_diff_db(pv, pa, freqs)
        diffs.append({"max_db": d, "at_hz": f})
    res["spectrum_max_diff"] = diffs
    rs = average_spectrum(r[:, 0])
    vs = average_spectrum(v[:, 0])
    res["residual_spectrum_peak"] = {
        "hz": float(freqs[int(np.argmax(rs[1:])) + 1]),
        "rel_to_signal_db": 10 * math.log10(np.max(rs[1:]) / (np.max(vs) + 1e-30) + 1e-30),
    }
    rep[name] = res


def check_ramp(v, a, rep):
    ev = envelope_db(v)
    ea = envelope_db(a)
    keep = np.minimum(ev, ea) > ENVELOPE_COMPARE_DB
    diff = np.abs(ev - ea)[keep]
    corr = float(np.corrcoef(ev[keep], ea[keep])[0, 1]) if np.count_nonzero(keep) > 2 else 0.0
    rep["ramp"] = {
        "windows": int(len(ev)),
        "windows_compared": int(np.count_nonzero(keep)),
        "envelope_max_diff_db": float(np.max(diff)) if diff.size else 0.0,
        "envelope_mean_diff_db": float(np.mean(diff)) if diff.size else 0.0,
        "envelope_corr": corr,
        "vst3_envelope_start_end_db": [float(ev[0]), float(ev[-1])],
        "au_envelope_start_end_db": [float(ea[0]), float(ea[-1])],
        "residual_rms_dbfs": db(rms(v - a)),
    }


def verdicts(rep):
    f = rep["fixed"]
    rp = rep["ramp"]
    checks = {
        "no_nan_inf": f["nan_inf"] == 0,
        "null_fixed_le_threshold": f["null_rel_db"] <= NULL_MAX_DB,
        "spectrum_fixed_le_threshold": max(d["max_db"] for d in f["spectrum_max_diff"]) <= SPECTRUM_MAX_DB,
        "click_fixed_le_threshold": f["click_rel_db"] <= CLICK_MAX_DB,
        "lag_fixed_is_zero": abs(f["lag_samples"]) <= LAG_MAX_SAMPLES,
        "ramp_envelope_le_threshold": rp["envelope_max_diff_db"] <= ENVELOPE_MAX_DB,
        "ramp_envelope_corr_ge_threshold": rp["envelope_corr"] >= ENVELOPE_MIN_CORR,
    }
    return checks


def main():
    d = sys.argv[1]
    out_dir = sys.argv[sys.argv.index("--out") + 1] if "--out" in sys.argv else d
    os.makedirs(out_dir, exist_ok=True)
    inp, _ = read_wav(os.path.join(d, "input.wav"))
    v_fix, _ = read_wav(os.path.join(d, "vst3_fixed.wav"))
    a_fix, _ = read_wav(os.path.join(d, "au_fixed.wav"))
    v_ramp, _ = read_wav(os.path.join(d, "vst3_ramp.wav"))
    a_ramp, _ = read_wav(os.path.join(d, "au_ramp.wav"))
    rep = {
        "input_rms_dbfs": [db(rms(inp[:, c])) for c in range(2)],
        "frames": int(len(v_fix)),
        "settings_fixed": {"anger": 60, "color": 40, "gain": -6, "bypass": 0},
        "settings_ramp": {"anger": "0 to 100 over the file", "color": 40, "gain": -6, "bypass": 0},
    }
    check_pair("fixed", v_fix, a_fix, rep)
    check_ramp(v_ramp, a_ramp, rep)
    checks = verdicts(rep)
    rep["criteria"] = {k: bool(v) for k, v in checks.items()}
    rep["overall"] = "PASS" if all(checks.values()) else "FAIL"
    rep["thresholds"] = {
        "null_max_db": NULL_MAX_DB,
        "spectrum_max_db": SPECTRUM_MAX_DB,
        "envelope_max_db": ENVELOPE_MAX_DB,
        "envelope_min_corr": ENVELOPE_MIN_CORR,
        "click_max_db": CLICK_MAX_DB,
        "lag_max_samples": LAG_MAX_SAMPLES,
    }
    with open(os.path.join(out_dir, "parity-results.json"), "w", encoding="utf-8") as f:
        json.dump(rep, f, indent=2)
    lines = [json.dumps(rep, indent=2)]
    text = "\n".join(lines)
    with open(os.path.join(out_dir, "parity-results.txt"), "w", encoding="utf-8") as f:
        f.write(text + "\n")
    print(text)
    return 0 if rep["overall"] == "PASS" else 1


if __name__ == "__main__":
    sys.exit(main())
