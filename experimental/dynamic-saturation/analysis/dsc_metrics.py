"""Read-only metrics for Dynamic Saturation dumps (harness .ap files and REF records).

Halo metrics use 1 s windows (1 Hz bins). Energy is the sum of bin powers at distances 1..127 Hz from
H2..H14, referenced to the 999 Hz line of the same window. Deterministic intermodulation lines are
included in these numbers and reported separately where it matters.
"""
import sys

import numpy as np

sys.path.insert(0, r"D:\AP-RE\tmp\halo-engine")
sys.path.insert(0, r"D:\AP-RE\tmp\ref-spec")
import halo_common as hc  # noqa: E402
from analyze_dumps import load  # noqa: E402

W0 = hc.W0
FS = hc.FS
EVI = r"D:\Codex\Snapshots\2026-10-26\referenced-chatgpt-conversation-this-is-an\work\dynamic-saturation\evidence"
SECONDS = 10


def out_channel(path):
    d = load(path)
    return d["outL"], d["inL"]


def ref_out(name="L999_m12_p0"):
    inp, out = hc.load_ref(name)      # halo_common.load_ref returns (inL, outL)
    return out, inp


def second_spectra(y, seconds=SECONDS):
    for j in range(seconds):
        seg = y[W0 + j * FS:W0 + (j + 1) * FS]
        yield np.fft.rfft(seg) * 2 / len(seg)


def summarize(y, label, seconds=SECONDS):
    fr = np.arange(FS // 2 + 1).astype(float)
    dist, _ = hc._dist_even(fr)
    halo, per_h = [], {k: [] for k in hc.EVEN_H}
    shells = {"1-8": [], "8-32": [], "32-128": []}
    dens = []
    h2c, h3, h5, lf, lf200, dc = [], [], [], [], [], []  # lf200: 20-200 Hz integrated, dBc
    for X in second_spectra(y, seconds):
        a1 = abs(X[hc.K])
        P = np.abs(X) ** 2
        m = (dist >= 1.0) & (dist < 128.0)
        halo.append(10 * np.log10(np.sum(P[m]) / a1 ** 2 + 1e-30))
        d_idx = np.floor(dist[m]).astype(int)
        dens.append(np.bincount(d_idx, weights=P[m], minlength=129)[:129] / a1 ** 2)
        _, kidx = hc._dist_even(fr)
        for k in hc.EVEN_H:
            mk = m & (kidx == k)
            per_h[k].append(10 * np.log10(np.sum(P[mk]) / a1 ** 2 + 1e-30))
        for name, lo, hi in (("1-8", 1, 8), ("8-32", 8, 32), ("32-128", 32, 128)):
            mm = (dist >= lo) & (dist < hi)
            shells[name].append(10 * np.log10(np.sum(P[mm]) / a1 ** 2 + 1e-30))
        h2c.append(20 * np.log10(abs(X[2 * hc.K]) / a1 + 1e-30))
        h3.append(20 * np.log10(abs(X[3 * hc.K]) / a1 + 1e-30))
        h5.append(20 * np.log10(abs(X[5 * hc.K]) / a1 + 1e-30))
        lf_m = (fr >= 0.5) & (fr < 30.0)
        lf.append(10 * np.log10(np.sum(P[lf_m]) / a1 ** 2 + 1e-30))
        lf200_m = (fr >= 20.0) & (fr < 200.0)
        lf200.append(10 * np.log10(np.sum(P[lf200_m]) / a1 ** 2 + 1e-30))
        dc.append(20 * np.log10(abs(X[0]) / a1 + 1e-30))
    dens_mean = np.mean(np.array(dens), axis=0)
    dens_dB = 10 * np.log10(dens_mean[1:128] + 1e-30)
    # width: distance (Hz) where the 1 Hz density first falls 3 dB below its 4..16 Hz level, searched beyond 16 Hz
    plateau = np.mean(dens_dB[3:16])
    dist_axis = np.arange(1, 128)
    below = np.where((dist_axis >= 16) & (dens_dB < plateau - 3.0))[0]
    width = float(dist_axis[below[0]]) if len(below) else float("nan")
    lf200_mean = float(np.mean(lf200))
    return {
        "label": label,
        "halo_mean": float(np.mean(halo)), "halo_std": float(np.std(halo, ddof=1)),
        "per_h": {str(k): float(np.mean(v)) for k, v in per_h.items()},
        "shells": {k: float(np.mean(v)) for k, v in shells.items()},
        "width_3dB_Hz": width,
        "density_dB": {str(d): float(dens_dB[d - 1]) for d in (1, 4, 8, 16, 32, 64, 127)},
        "H2_center": float(np.mean(h2c)), "H3": float(np.mean(h3)), "H5": float(np.mean(h5)),
        "LF_0p5_30": float(np.mean(lf)), "LF_20_200": lf200_mean,
        "DC": float(np.mean(dc)),
    }


def halo_series(y, seconds=SECONDS):
    fr = np.arange(FS // 2 + 1).astype(float)
    dist, _ = hc._dist_even(fr)
    out = []
    for X in second_spectra(y, seconds):
        a1 = abs(X[hc.K])
        P = np.abs(X) ** 2
        m = (dist >= 1.0) & (dist < 128.0)
        out.append(10 * np.log10(np.sum(P[m]) / a1 ** 2 + 1e-30))
    return np.array(out)
