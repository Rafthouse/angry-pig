import sys
sys.path.insert(0, '.')
import numpy as np
import dsc_metrics as dm
from reference_table import show
R = r"D:\AP-RE\tmp\dyn-sat-recovery\runs"
tags = sys.argv[1:] or ["c0_a1"]
for t in tags:
    d = dm.load(R + "\\" + t + ".ap")
    y = d["outL"]
    if t.endswith("sil"):
        print(t, "silence: max |out| =", float(np.max(np.abs(y[24000:]))), " max |in| =", float(np.max(np.abs(d["inL"]))))
        continue
    show(dm.summarize(y, t))
