import sys, json, os
sys.path.insert(0, '.')
import numpy as np
import dsc_metrics as dm
R = r"D:\AP-RE\tmp\dyn-sat-recovery\runs"
def run(name):
    return dm.load(os.path.join(R, name + ".ap"))["outL"]
rows = {}
levels = [("-6", "L999_m6_p0", "fin_a1_m6", "fin_a0_6"),
          ("-12", "L999_m12_p0", "fin_a1", "fin_a0"),
          ("-24", "L999_m24_p0", "fin_a1_m24", "fin_a0_24")]
for db, ref_name, cand_name, rc_name in levels:
    y_ref, _ = dm.ref_out(ref_name)
    for lab, y in (("REF", y_ref), ("RC1.1 path", run(rc_name)), ("CANDIDATE", run(cand_name))):
        s = dm.summarize(y, f"{lab} {db}")
        rows[f"{lab} {db}"] = s
        print(f"{lab:11s} {db:>4s}: halo {s['halo_mean']:7.2f}+-{s['halo_std']:4.2f} H2 {s['per_h']['2']:7.2f} H4 {s['per_h']['4']:7.2f} H6 {s['per_h']['6']:7.2f} H8 {s['per_h']['8']:7.2f} H10 {s['per_h']['10']:7.2f} | H3 {s['H3']:8.3f} H5 {s['H5']:8.3f} | LF20-200 {s['LF_20_200']:7.2f} | DC {s['DC']:7.1f}")
json.dump(rows, open(os.path.join(r"D:\AP-RE\tmp\dyn-sat-recovery\report", "levels.json"), "w", encoding="utf-8"), indent=1, default=float)
