"""Shared pieces for the offline halo-engine prototypes (Model A, Model B) and the production proxy.

Offline only. Reads saved REF records and the saved static ramp; writes nothing outside D:\\AP-RE\\tmp\\halo-engine.
Nothing here touches production 1.0.x, RC1.1, Dynamic EXP v2, installed VST3s or macOS/GitHub artifacts.
"""
import sys
from pathlib import Path

import numpy as np
from scipy.interpolate import PchipInterpolator
from scipy.signal import butter, lfilter

sys.path.insert(0, r"D:\AP-RE\tmp\ref-spec")
from analyze_dumps import load  # noqa: E402

FS = 44100
K = 999                              # test tone, Hz
W0 = 24000                           # harness warm-up samples
T_SEC = 60                           # analysis length, s (one integer-period window)
NW = T_SEC * FS
C = 0.98854828                       # output ceiling (-0.10 dBFS)
TAU = 0.525387299                    # output lead over input at 999 Hz (measured, L999_m12_p0 phase)
HALO_TARGET_DBC = -75.9              # REF integrated energy H2..H14 (LONG-RUN-CONCLUSION / this study)
ONS = Path(r"D:\AP-RE\tmp\exp-dynamic-h2\onset-investigation")
OUT = Path(r"D:\AP-RE\tmp\halo-engine")
LONG = ONS / "long-records"
NOISE = ONS / "noise-records"
RAMP = ONS / "static-curve" / "RAMP_0p01Hz_m48_to_0_2periods.ref"
EVEN_H = list(range(2, 15, 2))       # 2,4,...,14 (even harmonics carrying the halo)
ALL_H = list(range(2, 15))


def amp(db):
    return 10 ** (db / 20)


# ------------------------------------------------------------------ static curve (per level)

_ramp = None


def _ramp_arrays():
    global _ramp
    if _ramp is None:
        d = load(RAMP)
        n = np.arange(d["T"])
        ph = np.mod(n / (100 * FS), 1.0)
        tri = np.where(ph < 0.5, 2 * ph, 2 * (1 - ph))
        _ramp = (d["inL"], d["outL"], -48 + 48 * tri)
    return _ramp


def _lagrange_shift(x, idx, tau):
    t = idx + tau
    m = np.floor(t).astype(np.int64)
    f = t - m
    w = (-f * (f - 1) * (f - 2) / 6, (f + 1) * (f - 1) * (f - 2) / 2,
         -(f + 1) * f * (f - 2) / 2, (f + 1) * f * (f - 1) / 6)
    return sum(w[j] * x[m - 1 + j] for j in range(4))


def _odd_pchip(xm, ym):
    pos = xm > 0
    u = xm[pos]
    yp = 0.5 * (ym[pos] - np.interp(-u, xm, ym))
    o = np.argsort(u)
    u, yp = u[o], np.maximum.accumulate(yp[o])
    j = int(np.argmax(yp >= C - 1e-4))
    if yp[j] < C - 1e-4:
        raise ValueError("table never reaches the ceiling")
    u_k = u[j]
    uu = np.r_[0.0, u[:j], u_k]
    yy = np.r_[0.0, yp[:j], C]
    P = PchipInterpolator(uu, yy)

    def S(x):
        x = np.asarray(x, dtype=float)
        a = np.abs(x)
        p = np.where(a < u_k, P(np.minimum(a, u_k)), C)
        return np.sign(x) * p
    return S


_static_cache = {}


def static_curve(db, nb=300):
    """Odd, monotone, ceiling-limited static transfer measured at a fixed sine level (aligned ramp table)."""
    if db in _static_cache:
        return _static_cache[db]
    x, y, L = _ramp_arrays()
    sel = np.where(np.abs(L - db) < 0.05)[0]
    sel = sel[(sel > 2) & (sel < len(x) - 3)]
    xe = _lagrange_shift(x, sel, TAU)
    ys = y[sel]
    a = amp(db)
    edges = np.linspace(-a * 1.02, a * 1.02, nb + 1)
    k = np.digitize(xe, edges) - 1
    xm, ym = [], []
    for i in range(nb):
        m = k == i
        if m.sum() > 20:
            xm.append(xe[m].mean())
            ym.append(ys[m].mean())
    S = _odd_pchip(np.array(xm), np.array(ym))
    _static_cache[db] = S
    return S


# ------------------------------------------------------------------ signals

def sine_window(db=-12.0, total_samples=None):
    """Pure 999 Hz sine from file index 0 (harness phase reference), with the output lead applied."""
    n = np.arange(W0 + NW) if total_samples is None else np.arange(total_samples)
    x = amp(db) * np.sin(2 * np.pi * K * n / FS)
    return _shift_wrap(x, TAU)


def _shift_wrap(x, tau):
    N = len(x)
    t = np.arange(N) + tau
    m = np.floor(t).astype(np.int64)
    f = t - m
    w = (-f * (f - 1) * (f - 2) / 6, (f + 1) * (f - 1) * (f - 2) / 2,
         -(f + 1) * f * (f - 2) / 2, (f + 1) * f * (f - 1) / 6)
    return sum(w[j] * x[(m - 1 + j) % N] for j in range(4))


def shift_wrap(x, tau=TAU):
    return _shift_wrap(x, tau)


# ------------------------------------------------------------------ modulation and envelope

_BUT = butter(2, 32.0, btype="low", fs=FS)          # bias bandwidth: 2nd-order Butterworth at 32 Hz


def unit_lowpass_noise(n, seed):
    """Seeded white Gaussian noise through the 32 Hz low-pass, scaled to unit variance."""
    rng = np.random.default_rng(seed)
    w = rng.standard_normal(n)
    b, a = _BUT
    imp = np.zeros(8192)
    imp[0] = 1.0
    g = np.sqrt(np.sum(lfilter(b, a, imp) ** 2))
    return lfilter(b, a, w) / g


def envelope(x, tau_s=0.001):
    """One-pole average of |x| (1 ms). Zero input gives exactly zero envelope."""
    a = np.exp(-1.0 / (tau_s * FS))
    return lfilter([1.0 - a], [1.0, -a], np.abs(x))


GATE_FLOOR = 1e-4                    # -80 dBFS presence gate for Model A


# ------------------------------------------------------------------ models (offline)

def production_proxy(x, S):
    """Static-only proxy of the production DSP (AP 1.0.x static behaviour, no halo)."""
    return S(x)


def model_A(x, S, sigma, seed):
    """Dynamic asymmetric bias: y = S(x + G * sigma * u), u = 32 Hz low-pass noise, G = presence gate."""
    u = unit_lowpass_noise(len(x), seed)
    G = np.minimum(1.0, envelope(x) / GATE_FLOOR)
    return S(x + sigma * G * u)


def model_B(x, S, kappa, seed):
    """Signal-dependent modulation: y = S(x + kappa * env(x) * u). Zero intensity when the envelope is zero."""
    u = unit_lowpass_noise(len(x), seed)
    return S(x + kappa * envelope(x) * u)


# ------------------------------------------------------------------ metrics

def spectrum(y_win):
    return np.fft.rfft(y_win) * 2 / len(y_win)


def _dist_even(fr):
    d = np.full(len(fr), np.inf)
    kidx = np.zeros(len(fr), dtype=int)
    for k in EVEN_H:
        dk = np.abs(fr - k * K)
        m = dk < d
        d[m] = dk[m]
        kidx[m] = k
    return d, kidx


def halo_metrics(y_win, ref_a1=None):
    """Metrics of one 60 s window. Harmonic levels are referenced to the 999 Hz line of the same window."""
    X = spectrum(y_win)
    fr = np.arange(len(X)) / T_SEC
    a1 = np.abs(X[K * T_SEC])
    P = np.abs(X) ** 2
    d_even, k_even = _dist_even(fr)
    m = (d_even >= 1.0) & (d_even < 128.0)
    band = np.floor(d_even[m]).astype(int)
    # power per 1 Hz band, summed over all even harmonics, referenced to the fundamental power
    dens = np.bincount(band, weights=P[m], minlength=129)[:129] / a1 ** 2   # 1 Hz bins, linear
    dens_dB = 10 * np.log10(dens[1:128] + 1e-30)
    plateau = np.mean(dens_dB[1:7])                                         # distances 2..7 Hz
    below = np.where(dens_dB < plateau - 3.0)[0]
    width = float(below[0] + 1) if len(below) else float("nan")
    per_h = {}
    for k in EVEN_H:
        mk = m & (k_even == k)
        per_h[k] = float(10 * np.log10(np.sum(P[mk]) / a1 ** 2 + 1e-30))
    total = 10 * np.log10(np.sum(P[m]) / a1 ** 2 + 1e-30)
    odd = {k: float(20 * np.log10(np.abs(X[k * K * T_SEC]) / a1 + 1e-30)) for k in (3, 5, 7, 9, 11, 13)}
    lf_mask = (fr > 0.5) & (fr < 30.0)
    lf = float(10 * np.log10(np.sum(P[lf_mask]) / a1 ** 2 + 1e-30))
    dc = float(20 * np.log10(np.abs(X[0]) / a1 + 1e-30))
    return {
        "a1_dBFS": float(20 * np.log10(a1)),
        "halo_sum_dBc": float(total),
        "halo_per_harmonic_dBc": {str(k): v for k, v in per_h.items()},
        "density_dB_at_d": {str(d): float(dens_dB[d - 1]) for d in (1, 4, 8, 16, 32, 48, 64, 96, 127)},
        "width_3dB_Hz": width,
        "odd_dBc": {str(k): v for k, v in odd.items()},
        "lf_0p5_30_dBc": lf,
        "dc_dBc": dc,
    }


def per_second_halo(y_full, start=W0, seconds=T_SEC):
    """Halo energy (H2..H14, distances 1..127 Hz) in each 1 s window, dBc."""
    vals = []
    for j in range(seconds):
        seg = y_full[start + j * FS:start + (j + 1) * FS]
        X = np.fft.rfft(seg) * 2 / len(seg)
        a1 = np.abs(X[K])
        fr = np.arange(len(X))
        d_even, _ = _dist_even(fr.astype(float))
        m = (d_even >= 1.0) & (d_even < 128.0)
        vals.append(10 * np.log10(np.sum(np.abs(X[m]) ** 2) / a1 ** 2 + 1e-30))
    v = np.array(vals)
    return float(v.mean()), float(v.std(ddof=1))


def load_ref(name):
    d = load(LONG / f"{name}.ref")
    return d["inL"], d["outL"]


def load_ref_noise(name):
    d = load(NOISE / f"{name}.ref")
    return d["inL"], d["outL"]
