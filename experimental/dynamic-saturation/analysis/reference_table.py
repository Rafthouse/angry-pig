import sys
sys.path.insert(0, '.')
import dsc_metrics as dm
EVI = dm.EVI
def show(r):
    print(f"{r['label']:18s} halo {r['halo_mean']:7.2f}±{r['halo_std']:4.2f}  width {r['width_3dB_Hz']:5.1f}  "
          f"H2 {r['per_h']['2']:7.2f} H4 {r['per_h']['4']:7.2f} H6 {r['per_h']['6']:7.2f} H8 {r['per_h']['8']:7.2f} H10 {r['per_h']['10']:7.2f} H12 {r['per_h']['12']:7.2f} H14 {r['per_h']['14']:7.2f} | "
          f"sh1-8 {r['shells']['1-8']:7.2f} 8-32 {r['shells']['8-32']:7.2f} 32-128 {r['shells']['32-128']:7.2f} | "
          f"H2c {r['H2_center']:7.2f} H3 {r['H3']:7.3f} H5 {r['H5']:7.3f} | LF0.5-30 {r['LF_0p5_30']:7.2f} LF20-200 {r['LF_20_200']:7.2f} DC {r['DC']:7.1f}")
if __name__ == '__main__':
    y_ref, _ = dm.ref_out("L999_m12_p0")
    show(dm.summarize(y_ref, "REF"))
    show(dm.summarize(dm.load(EVI + r"\null_base.ap")["outL"], "RC1.1"))
    show(dm.summarize(dm.load(EVI + r"\full_1.ap")["outL"], "EXP early full_1"))
    show(dm.summarize(dm.load(EVI + r"\f3_full_1.ap")["outL"], "EXP final f3"))
