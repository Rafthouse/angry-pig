import sys, json
sys.path.insert(0, '.')
import numpy as np
import dsc_metrics as dm
R = r"D:\AP-RE\tmp\dyn-sat-recovery\runs"
EVI = dm.EVI
FS = dm.FS
def floor_and_width(y):
    fr = np.arange(FS // 2 + 1).astype(float)
    dist, _ = dm._dist_even(fr) if False else (None, None)
    import halo_common as hc
    dist, _ = hc._dist_even(fr)
    bands = {"20-200": (20, 200), "200-2k": (200, 2000), "2k-8k": (2000, 8000), "8k-22k": (8000, 22050)}
    med = {k: [] for k in bands}
    frac32, frac16, slope = [], [], []
    dens_all = []
    for X in dm.second_spectra(y, 10):
        a1 = abs(X[999]); P = np.abs(X) ** 2
        mask_h = dist < 2.0                       # harmonic bins (+-2 Hz) excluded from the floor
        for k, (lo, hi) in bands.items():
            m = (fr >= lo) & (fr < hi) & ~mask_h
            med[k].append(20 * np.log10(np.median(np.sqrt(P[m])) / a1 + 1e-30))
        m1 = (dist >= 1) & (dist < 128)
        tot = np.sum(P[m1]); e32 = np.sum(P[(dist >= 1) & (dist < 32)])
        frac32.append(10 * np.log10(e32 / tot + 1e-30))
        d = np.floor(dist[m1]).astype(int)
        dens = np.bincount(d, weights=P[m1], minlength=129)[:129] / a1 ** 2
        dens_all.append(dens)
    dens = np.mean(dens_all, axis=0)
    db = 10 * np.log10(dens[1:128] + 1e-30)
    # density slope (dB per octave) between 16-32 Hz and 64-127 Hz
    s = (np.mean(db[63:127]) - np.mean(db[15:31])) / np.log2(96.0 / 24.0)
    out = {k: float(np.mean(v)) for k, v in med.items()}
    out["frac_energy_in_32Hz_dB"] = float(np.mean(frac32))
    out["density_slope_16_127_dB_per_oct"] = float(s)
    out["density_16_32_dB"] = float(np.mean(db[15:31])); out["density_64_127_dB"] = float(np.mean(db[63:127]))
    return out
cases = {
 "REF": dm.ref_out("L999_m12_p0")[0],
 "RC1.1": dm.load(EVI + r"\null_base.ap")["outL"],
 "EXP early full_1": dm.load(EVI + r"\full_1.ap")["outL"],
 "EXP final f3": dm.load(EVI + r"\f3_full_1.ap")["outL"],
 "EXP C-only": dm.load(EVI + r"\f3_C_1.ap")["outL"],
 "CAND ABC": dm.load(R + r"\c0_a1.ap")["outL"],
 "CAND AB (no C)": dm.load(R + r"\c0_comp_AB.ap")["outL"],
 "CAND B-only": dm.load(R + r"\c0_comp_B.ap")["outL"],
}
res = {}
for k, y in cases.items():
    r = floor_and_width(y); res[k] = r
    print(f"{k:18s} med20-200 {r['20-200']:8.2f}  200-2k {r['200-2k']:8.2f}  2-8k {r['2k-8k']:8.2f}  8-22k {r['8k-22k']:8.2f} | "
          f"E(<32Hz) {r['frac_energy_in_32Hz_dB']:7.2f} | dens16-32 {r['density_16_32_dB']:7.2f} dens64-127 {r['density_64_127_dB']:7.2f} slope {r['density_slope_16_127_dB_per_oct']:6.2f} dB/oct")
json.dump(res, open(r"D:\AP-RE\tmp\dyn-sat-recovery\floor_width.json", "w", encoding="utf-8"), indent=1)
