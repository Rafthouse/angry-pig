import sys, json
sys.path.insert(0, '.')
import numpy as np
import dsc_metrics as dm
from reference_table import show
R = r"D:\AP-RE\tmp\dyn-sat-recovery\runs"
EVI = dm.EVI
rows = []
def add(label, y):
    r = dm.summarize(y, label); rows.append(r); show(r)
y_ref, _ = dm.ref_out("L999_m12_p0")
add("REF", y_ref)
add("RC1.1", dm.load(EVI + r"\null_base.ap")["outL"])
add("EXP early full_1", dm.load(EVI + r"\full_1.ap")["outL"])
add("EXP final f3", dm.load(EVI + r"\f3_full_1.ap")["outL"])
add("EXP f3 A-only", dm.load(EVI + r"\f3_A_1.ap")["outL"])
add("EXP f3 B-only", dm.load(EVI + r"\f3_B_1.ap")["outL"])
add("EXP f3 C-only", dm.load(EVI + r"\f3_C_1.ap")["outL"])
add("CAND (ABC) seed1", dm.load(R + r"\c0_a1.ap")["outL"])
add("CAND seed2", dm.load(R + r"\c0_a1_s2.ap")["outL"])
add("CAND seed3", dm.load(R + r"\c0_a1_s3.ap")["outL"])
add("CAND B-only", dm.load(R + r"\c0_comp_B.ap")["outL"])
add("CAND BC (no A)", dm.load(R + r"\c0_comp_BC.ap")["outL"])
add("CAND AB (no C)", dm.load(R + r"\c0_comp_AB.ap")["outL"])
json.dump(rows, open(r"D:\AP-RE\tmp\dyn-sat-recovery\results_12db.json", "w", encoding="utf-8"), default=float, indent=1)
