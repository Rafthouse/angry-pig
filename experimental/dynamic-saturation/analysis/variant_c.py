import sys, os
sys.path.insert(0, '.')
import numpy as np
import dsc_metrics as dm
import make_report as mr
R = r"D:\AP-RE\tmp\dyn-sat-recovery\runs"
def run(n): return dm.load(os.path.join(R, n + ".ap"))["outL"]
cases = {
  "REF": dm.ref_out("L999_m12_p0")[0],
  "C-only (EXP)": dm.load(dm.EVI + r"\f3_C_1.ap")["outL"],
  "AB (C off)": run("v2_ab"),
  "ABC (C full)": run("v2_abc"),
  "ABC C-HF only": run("v2_abc_hf"),
}
out = {}
for k, y in cases.items():
    s = dm.summarize(y, k); f = mr.floor_bands(y)
    out[k] = (s, f)
    print(f"{k:14s} halo {s['halo_mean']:7.2f} H2 {s['per_h']['2']:7.2f} H4 {s['per_h']['4']:7.2f} H6 {s['per_h']['6']:7.2f} | LF20-200 {s['LF_20_200']:7.2f} | "
          f"floor 20-200 {f['20-200']:7.2f} 200-2k {f['200-2k']:7.2f} 2-8k {f['2k-8k']:7.2f} 8-22k {f['8k-22k']:7.2f} | DC {s['DC']:7.1f} | E32 {'-':>1}")
