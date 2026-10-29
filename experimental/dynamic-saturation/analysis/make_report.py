"""Builds the comparison report for the isolated Dynamic Saturation candidate (read-only on all inputs)."""
import json
import sys

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

sys.path.insert(0, r"D:\AP-RE\tmp\dyn-sat-recovery\analysis")
import dsc_metrics as dm  # noqa: E402
import halo_common as hc  # noqa: E402

R = r"D:\AP-RE\tmp\dyn-sat-recovery\runs"
REP = r"D:\AP-RE\tmp\dyn-sat-recovery\report"
EVI = dm.EVI
FS = hc.FS


def load_out(path):
    return dm.load(path)["outL"]


def halo_ps(y):
    return dm.halo_series(y, 10)


def floor_bands(y):
    fr = np.arange(FS // 2 + 1).astype(float)
    dist, kidx = hc._dist_even(fr)
    bands = {"20-200": (20, 200), "200-2k": (200, 2000), "2k-8k": (2000, 8000), "8k-22k": (8000, 22050)}
    out = {k: [] for k in bands}
    for X in dm.second_spectra(y, 10):
        a1 = abs(X[hc.K])
        P = np.abs(X) ** 2
        mask_h = dist < 2.0
        for k, (lo, hi) in bands.items():
            m = (fr >= lo) & (fr < hi) & ~mask_h
            out[k].append(20 * np.log10(np.median(np.sqrt(P[m])) / a1 + 1e-30))
    return {k: float(np.mean(v)) for k, v in out.items()}


def density_profile(y):
    fr = np.arange(FS // 2 + 1).astype(float)
    dist, _ = hc._dist_even(fr)
    acc = []
    for X in dm.second_spectra(y, 10):
        a1 = abs(X[hc.K])
        P = np.abs(X) ** 2
        m = (dist >= 1) & (dist < 128)
        d = np.floor(dist[m]).astype(int)
        acc.append(np.bincount(d, weights=P[m], minlength=129)[:129] / a1 ** 2)
    dens = np.mean(acc, axis=0)
    return 10 * np.log10(dens[1:128] + 1e-30)


def main():
    cases = {}
    y_ref, _ = dm.ref_out("L999_m12_p0")
    cases["REF"] = y_ref
    cases["RC1.1"] = load_out(EVI + r"\null_base.ap")
    cases["EXP early (full_1)"] = load_out(EVI + r"\full_1.ap")
    cases["EXP final (f3)"] = load_out(EVI + r"\f3_full_1.ap")
    cases["CANDIDATE"] = load_out(R + r"\fin_a1.ap")
    cases["CANDIDATE seed 2"] = load_out(R + r"\fin_a1_s2.ap")
    cases["CANDIDATE seed 3"] = load_out(R + r"\fin_a1_s3.ap")
    cases["CANDIDATE B+C (C on)"] = load_out(R + r"\c0_comp_ABC_on.ap") if False else None
    cases = {k: v for k, v in cases.items() if v is not None}

    summ = {k: dm.summarize(v, k) for k, v in cases.items()}
    floors = {k: floor_bands(v) for k, v in cases.items()}
    dens = {k: density_profile(v) for k, v in cases.items()}

    # odd harmonics H3..H15 vs RC1.1 and clicks / peaks / DC on the candidate
    y_rc = cases["RC1.1"]
    y_c = cases["CANDIDATE"]
    odd = {}
    for k in (3, 5, 7, 9, 11, 13, 15):
        X_c = np.fft.rfft(y_c[hc.W0:hc.W0 + FS]) * 2 / FS
        X_r = np.fft.rfft(y_rc[hc.W0:hc.W0 + FS]) * 2 / FS
        odd[f"H{k}"] = (20 * np.log10(abs(X_c[k * hc.K]) / abs(X_c[hc.K])),
                        20 * np.log10(abs(X_r[k * hc.K]) / abs(X_r[hc.K])))
    diff = y_c[hc.W0:] - y_rc[hc.W0:]
    clicks = {
        "max |candidate - RC1.1| sample step": float(np.max(np.abs(np.diff(diff)))),
        "peak candidate": float(np.max(np.abs(y_c[hc.W0:]))),
        "peak RC1.1": float(np.max(np.abs(y_rc[hc.W0:]))),
        "DC candidate (dBc)": summ["CANDIDATE"]["DC"],
        "DC REF (dBc)": summ["REF"]["DC"],
    }

    result = {"summary": summ, "floors": floors, "odd_candidate_vs_RC1": odd, "checks": clicks}
    with open(REP + r"\metrics.json", "w", encoding="utf-8") as fh:
        json.dump(result, fh, indent=1, default=float)

    # ---- figures
    names = list(cases.keys())
    fig, ax = plt.subplots(figsize=(9, 4.5))
    hs = [2, 4, 6, 8, 10, 12, 14]
    w = 0.16
    for i, k in enumerate(names):
        vals = [summ[k]["per_h"][str(h)] for h in hs]
        ax.bar(np.arange(len(hs)) + (i - len(names) / 2) * w, vals, w, label=k)
    ax.set_xticks(np.arange(len(hs)))
    ax.set_xticklabels([f"H{h}" for h in hs])
    ax.set_ylabel("halo energy per harmonic, 1-128 Hz (dBc)")
    ax.legend(fontsize=7, ncol=2)
    fig.tight_layout()
    fig.savefig(REP + r"\fig1_halo_per_harmonic.png", dpi=130)
    plt.close(fig)

    fig, ax = plt.subplots(figsize=(9, 4.5))
    dd = np.arange(1, 128)
    for k in names:
        ax.plot(dd, dens[k], label=k)
    ax.set_xscale("log")
    ax.set_xlabel("distance from H2..H14 (Hz)")
    ax.set_ylabel("density per 1 Hz (dB re fundamental)")
    ax.set_ylim(-160, -70)
    ax.axvline(32, ls=":", c="k", lw=0.8)
    ax.legend(fontsize=7)
    fig.tight_layout()
    fig.savefig(REP + r"\fig2_halo_density.png", dpi=130)
    plt.close(fig)

    fig, ax = plt.subplots(figsize=(9, 4.5))
    bl = ["20-200", "200-2k", "2k-8k", "8k-22k"]
    for i, k in enumerate(["REF", "CANDIDATE", "EXP final (f3)", "RC1.1"]):
        ax.bar(np.arange(len(bl)) + (i - 1.5) * 0.2, [floors[k][b] for b in bl], 0.2, label=k)
    ax.set_xticks(np.arange(len(bl)))
    ax.set_xticklabels(bl)
    ax.set_ylabel("median bin level (dBc)")
    ax.legend(fontsize=8)
    fig.tight_layout()
    fig.savefig(REP + r"\fig3_broadband_floor.png", dpi=130)
    plt.close(fig)

    fig, ax = plt.subplots(figsize=(9, 3.8))
    for k in ["REF", "CANDIDATE", "CANDIDATE seed 2", "EXP final (f3)", "EXP early (full_1)"]:
        ax.plot(np.arange(10), halo_ps(cases[k]), "o-", label=k)
    ax.set_xlabel("second")
    ax.set_ylabel("halo H2..H14 (dBc)")
    ax.legend(fontsize=7)
    fig.tight_layout()
    fig.savefig(REP + r"\fig4_halo_stability.png", dpi=130)
    plt.close(fig)
    print("report figures and metrics written")


if __name__ == "__main__":
    main()
