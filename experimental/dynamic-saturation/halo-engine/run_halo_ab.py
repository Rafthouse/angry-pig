"""Halo test of Model A (dynamic asymmetric bias) and Model B (signal-dependent modulation) against REF.

Calibration: one scalar per model (sigma for A, kappa for B), set analytically from the static
derivative and then corrected once on seed 1 so that the integrated H2..H14 halo equals -75.9 dBc.
Seeds 2 and 3, the independent REF starts, the other levels and the per-second statistics are
NOT used for calibration.
"""
import json
import sys

import numpy as np

sys.path.insert(0, r"D:\AP-RE\tmp\halo-engine")
import halo_common as hc  # noqa: E402

NTOT = hc.W0 + hc.NW
WIN = slice(hc.W0, hc.W0 + hc.NW)


def static_fourier(S, db):
    """Even Fourier coefficients of S'(x) and the fundamental of S(x) at the given sine level."""
    N = 4096
    th = np.linspace(0, 2 * np.pi, N, endpoint=False)
    a = hc.amp(db)
    x = a * np.sin(th)
    h = 1e-7
    dS = (S(x + h) - S(x - h)) / (2 * h)
    c = {k: 2 / N * np.sum(dS * np.cos(k * th)) for k in hc.EVEN_H}
    a1 = 2 / N * np.sum(S(x) * np.sin(th))
    return c, a1, float(np.mean(dS))


def analytic_scale(S, db, target_db):
    c, a1, s0 = static_fourier(S, db)
    csum = np.sqrt(sum(v * v for v in c.values()))
    return a1 * 10 ** (target_db / 20) / csum, {k: float(v) for k, v in c.items()}, float(a1), s0


def run_model(kind, S, scale, seed, db):
    x = hc.sine_window(db, total_samples=NTOT)
    if kind == "A":
        y = hc.model_A(x, S, scale, seed)
    elif kind == "B":
        y = hc.model_B(x, S, scale, seed)
    else:
        y = hc.production_proxy(x, S)
    return y[WIN]


def main():
    res = {}
    S12 = hc.static_curve(-12.0)
    sigma0, c12, a1_12, s0_12 = analytic_scale(S12, -12.0, hc.HALO_TARGET_DBC)
    res["static_12"] = {"c_even": c12, "a1_lin": a1_12, "s0_mean_slope": s0_12, "sigma_analytic": sigma0}
    print("analytic sigma (A)", sigma0, "a1", a1_12, "s0", s0_12, flush=True)

    # ---- Model A calibration on seed 1 (single scalar, integrated energy only)
    yA1 = run_model("A", S12, sigma0, 1, -12.0)
    mA1 = hc.halo_metrics(yA1)
    sigmaA = sigma0 * 10 ** ((hc.HALO_TARGET_DBC - mA1["halo_sum_dBc"]) / 20)
    print("A seed1 before calibration", round(mA1["halo_sum_dBc"], 2), "-> sigma", sigmaA, flush=True)

    # ---- Model B: envelope mean of the -12 sine sets kappa (same analytic route, then one correction)
    env_mean = float(np.mean(hc.envelope(hc.sine_window(-12.0, total_samples=NTOT))[WIN]))
    kappaB0 = sigma0 / env_mean
    yB1 = run_model("B", S12, kappaB0, 1, -12.0)
    mB1 = hc.halo_metrics(yB1)
    kappaB = kappaB0 * 10 ** ((hc.HALO_TARGET_DBC - mB1["halo_sum_dBc"]) / 20)
    print("B seed1 before calibration", round(mB1["halo_sum_dBc"], 2), "-> kappa", kappaB, flush=True)
    res["calibration"] = {"sigma_A": sigmaA, "kappa_B": kappaB, "env_mean_m12": env_mean,
                          "A_seed1_before": mA1["halo_sum_dBc"], "B_seed1_before": mB1["halo_sum_dBc"]}

    rows = {}
    # ---- -12 dBFS, production proxy, REF (three independent starts), models (seeds 2, 3 for checks)
    rows["PRODUCTION_proxy_m12"] = hc.halo_metrics(run_model("P", S12, None, 0, -12.0))
    for name in ("L999_m12_p0", "L999_m12_p90", "L999_m12_p180"):
        xi, yr = hc.load_ref(name)
        rows[f"REF_{name}"] = hc.halo_metrics(yr[WIN])
    rows["MODEL_A_m12_seed1"] = mA1
    rows["MODEL_B_m12_seed1"] = mB1
    for s in (2, 3):
        rows[f"MODEL_A_m12_seed{s}"] = hc.halo_metrics(run_model("A", S12, sigmaA, s, -12.0))
        rows[f"MODEL_B_m12_seed{s}"] = hc.halo_metrics(run_model("B", S12, kappaB, s, -12.0))

    # ---- per-second statistics (independent seeds, REF starts)
    ps = {}
    for name in ("L999_m12_p0", "L999_m12_p90", "L999_m12_p180"):
        xi, yr = hc.load_ref(name)
        ps[f"REF_{name}"] = hc.per_second_halo(yr)
    for s in (1, 2, 3):
        ps[f"MODEL_A_seed{s}"] = hc.per_second_halo(run_model_full("A", S12, sigmaA, s, -12.0))
        ps[f"MODEL_B_seed{s}"] = hc.per_second_halo(run_model_full("B", S12, kappaB, s, -12.0))
    res["per_second_mean_std_dB"] = ps

    # ---- levels -6 and -24 dBFS: level-matched static (from the ramp), the -12 calibrated scales kept
    for db, ref_name in ((-6.0, "L999_m6_p0"), (-24.0, "L999_m24_p0")):
        S = hc.static_curve(db)
        tag = f"m{int(-db)}"
        rows[f"PRODUCTION_proxy_{tag}"] = hc.halo_metrics(run_model("P", S, None, 0, db))
        xi, yr = hc.load_ref(ref_name)
        rows[f"REF_{ref_name}"] = hc.halo_metrics(yr[WIN])
        rows[f"MODEL_A_{tag}"] = hc.halo_metrics(run_model("A", S, sigmaA, 1, db))
        rows[f"MODEL_B_{tag}"] = hc.halo_metrics(run_model("B", S, kappaB, 1, db))

    res["metrics"] = rows
    with open(hc.OUT / "results" / "halo_ab_results.json", "w", encoding="utf-8") as fh:
        json.dump(res, fh, indent=1, default=float)
    for k, v in rows.items():
        print(f"{k:30s} halo {v['halo_sum_dBc']:8.2f}  width {v['width_3dB_Hz']:6.1f}  "
              f"H2 {v['halo_per_harmonic_dBc']['2']:8.2f} H4 {v['halo_per_harmonic_dBc']['4']:8.2f} "
              f"H6 {v['halo_per_harmonic_dBc']['6']:8.2f}  LF {v['lf_0p5_30_dBc']:8.2f}  DC {v['dc_dBc']:8.2f}  "
              f"H3 {v['odd_dBc']['3']:8.3f} H5 {v['odd_dBc']['5']:8.3f} a1 {v['a1_dBFS']:7.3f}", flush=True)
    print("per-second (mean, std dB):")
    for k, v in ps.items():
        print(f"  {k:28s} {v[0]:8.2f}  std {v[1]:5.2f}")


def run_model_full(kind, S, scale, seed, db):
    """Same as run_model but returns the whole record (warm-up included) for per-second statistics."""
    x = hc.sine_window(db, total_samples=NTOT)
    if kind == "A":
        y = hc.model_A(x, S, scale, seed)
    else:
        y = hc.model_B(x, S, scale, seed)
    return y


if __name__ == "__main__":
    main()
