#!/usr/bin/env python3
"""Measurement tables and plots for docs/measurements (Windows VST3, offline, no host).

usage: measure.py <parity_render.exe> <plugin.vst3> <out_dir> [--legacy-gain] [--compare <other.vst3>]
  --legacy-gain       the plug-in has the Angry Pig 1.0.0 GAIN range (-24 to +18 dB)
  --compare <bundle>  Angry Pig 1.0.0 bundle (legacy GAIN range); adds it to the halo plot

Method (the same as the 1.0.0 tables): 44.1 kHz, 32-bit float stereo sine, 2 s per condition.
Analysis on 1 s (44 100 samples) after a 24 000-sample warm-up, coherent DFT with 1 Hz bins, so
every test tone falls on a bin. Levels are sine peaks in dBFS. Harmonics in dBc of the output
fundamental; THD is the root sum of squares of H2 to H15. Requires numpy and matplotlib.
"""
import csv
import math
import os
import struct
import subprocess
import sys

import numpy as np

FS = 44100
TOTAL = 88200
W0 = 24000
N = 44100
FREQS = [20, 30, 50, 100, 200, 500, 1000, 2000, 3000, 5000, 8000, 10000, 15000, 20000]
LEVELS = [-60, -48, -36, -30, -24, -18, -12, -6, -3, 0]


def write_wav(path, x):
    data = np.repeat(x.astype("<f4")[:, None], 2, axis=1).tobytes()
    with open(path, "wb") as f:
        f.write(b"RIFF" + struct.pack("<I", 36 + len(data)) + b"WAVE")
        f.write(b"fmt " + struct.pack("<IHHIIHH", 16, 3, 2, FS, FS * 8, 8, 32))
        f.write(b"data" + struct.pack("<I", len(data)) + data)


def read_wav(path):
    b = open(path, "rb").read()
    i, ch, data = 12, 2, None
    while i + 8 <= len(b):
        cid, ln = b[i:i + 4], struct.unpack("<I", b[i + 4:i + 8])[0]
        if cid == b"fmt ":
            ch = struct.unpack("<H", b[i + 10:i + 12])[0]
        elif cid == b"data":
            data = b[i + 8:i + 8 + ln]
        i += 8 + ln + (ln & 1)
    return np.frombuffer(data, dtype="<f4").reshape(-1, ch)[:, 0].astype(np.float64)


class Plugin:
    def __init__(self, exe, bundle, legacy, work):
        self.exe, self.bundle, self.legacy, self.work = exe, bundle, legacy, work

    def render(self, x, anger, color, gain_db):
        # parity_render maps GAIN to (g + 36) / 72; a 1.0.0 plug-in needs (g + 24) / 42.
        g = 72.0 * (gain_db + 24.0) / 42.0 - 36.0 if self.legacy else gain_db
        src, dst = os.path.join(self.work, "in.wav"), os.path.join(self.work, "out.wav")
        write_wav(src, x)
        subprocess.run([self.exe, "vst3", self.bundle, src, dst, repr(anger), repr(color), repr(g), "0", "none"],
                       check=True, stdout=subprocess.DEVNULL)
        return read_wav(dst)


def sine(freq, dbfs):
    n = np.arange(TOTAL)
    return 10 ** (dbfs / 20) * np.sin(2 * np.pi * freq * n / FS)


def spectrum(y):
    return np.fft.rfft(y[W0:W0 + N]) * 2 / N


def db(a):
    return 20 * math.log10(abs(a) + 1e-30)


def harmonics(plugin, anger, color, gain, level, f0=999):
    x = sine(f0, level)
    y = plugin.render(x, anger, color, gain)
    X, Y = spectrum(x), spectrum(y)
    a1 = abs(Y[f0])
    h = {k: db(Y[k * f0] / a1) for k in range(2, 11)}
    thd = 10 * math.log10(sum(abs(Y[k * f0]) ** 2 for k in range(2, 16)) / a1 ** 2 + 1e-30)
    peak = db(np.max(np.abs(y[W0:W0 + N])))
    return [anger, color, gain, level, round(db(X[f0]), 3), round(db(a1), 3), round(h[2], 2), round(h[3], 2),
            round(h[4], 2), round(h[5], 2), round(thd, 2), round(peak, 3), 0]


def gain_at(plugin, anger, color, gain, level, f):
    x = sine(f, level)
    y = plugin.render(x, anger, color, gain)
    return db(spectrum(y)[f]) - db(spectrum(x)[f])


def write_csv(path, header, rows):
    with open(path, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(header)
        w.writerows(rows)


def measure(p, out):
    fr = [[a, c, 0.0, lv, f, round(gain_at(p, a, c, 0.0, lv, f), 3), 0]
          for a, lv in ((0, -40.0), (100, -60.0)) for c in (0, 50, 100) for f in FREQS]
    write_csv(os.path.join(out, "frequency_response.csv"),
              ["anger_pct", "color_pct", "gain_db", "input_level_dbfs", "frequency_hz", "gain_db_rel_input", "latency_samples"], fr)
    tr = [harmonics(p, a, 0, 0.0, lv) for a in (0, 25, 50, 75, 100) for lv in LEVELS]
    tr += [harmonics(p, a, c, 0.0, lv) for a in (50, 100) for c in (50, 100) for lv in (-24, -12, -6)]
    write_csv(os.path.join(out, "transfer_harmonics_999hz.csv"),
              ["anger_pct", "color_pct", "gain_db", "input_level_dbfs", "input_fundamental_dbfs", "output_fundamental_dbfs",
               "h2_dbc", "h3_dbc", "h4_dbc", "h5_dbc", "thd_h2_h15_db", "output_peak_dbfs", "latency_samples"], tr)
    gains = list(range(-36, 37, 6)) if not p.legacy else [-24, -12, -6, 0, 6, 12, 18]
    gl = []
    for a in (0, 100):
        for g in gains:
            out_db = round(-40.0 + gain_at(p, a, 0, g, -40.0, 999), 3)
            gl.append([a, 0, g, -40.0, out_db, round(out_db + 40.0, 3), 0])
    write_csv(os.path.join(out, "gain_law.csv"),
              ["anger_pct", "color_pct", "gain_db", "input_level_dbfs", "output_fundamental_dbfs", "output_gain_db_rel_input", "latency_samples"], gl)
    return fr, tr, gl


def halo_spectrum(p):
    y = p.render(sine(999, -12.0), 100, 0, 0.0)
    seg = y[W0:W0 + N]
    X = np.fft.rfft(seg * np.hanning(N))
    X /= np.abs(X[999])
    return 20 * np.log10(np.abs(X) + 1e-30)


def plots(out, fr, tr, gl, halo, halo_ref):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    bg, ink, grid = "#141414", "#e8e8e8", "#333333"
    cols = {0: "#6a8fa3", 25: "#8fb3c4", 50: "#d4a93c", 75: "#e07a3c", 100: "#b3261e"}
    plt.rcParams.update({"figure.facecolor": bg, "axes.facecolor": bg, "axes.edgecolor": "#555555", "axes.labelcolor": ink,
                         "text.color": ink, "xtick.color": ink, "ytick.color": ink, "grid.color": grid,
                         "legend.facecolor": "#1e1e1e", "legend.edgecolor": "#555555", "font.size": 11})
    img = os.path.join(out, "images")
    os.makedirs(img, exist_ok=True)
    floor = -120.0

    fig, ax = plt.subplots(figsize=(10, 6))
    for a in (0, 25, 50, 75, 100):
        r = [t for t in tr if t[0] == a and t[1] == 0]
        ax.plot([t[3] for t in r], [t[5] for t in r], "o-", color=cols[a], lw=2, ms=5, label=f"ANGER {a} %")
    ax.set(title="Transfer curve at 999 Hz (COLOR 0 %, GAIN 0 dB)", xlabel="Input level (sine peak, dBFS)",
           ylabel="Output fundamental (dBFS)", ylim=(-70, 5))
    ax.title.set_ha("left"); ax.title.set_x(0)
    ax.grid(True); ax.legend(loc="upper left")
    fig.tight_layout(); fig.savefig(os.path.join(img, "transfer-curve.png"), dpi=120); plt.close(fig)

    fig, ax = plt.subplots(figsize=(10, 6))
    for a in (50, 100):
        r = [t for t in tr if t[0] == a and t[1] == 0]
        lv = [t[3] for t in r]
        ax.plot(lv, [max(t[7], floor) for t in r], "o-", color=cols[a], lw=2, ms=5, label=f"H3, ANGER {a} %")
        ax.plot(lv, [max(t[9], floor) for t in r], "o--", color=cols[a], lw=2, ms=5, label=f"H5, ANGER {a} %")
        ax.plot(lv, [max(t[10], floor) for t in r], "o:", color=cols[a], lw=2, ms=5, label=f"THD (H2-H15), ANGER {a} %")
    ax.set(title="Harmonic distortion at 999 Hz (COLOR 0 %, GAIN 0 dB)", xlabel="Input level (sine peak, dBFS)",
           ylabel="Level relative to fundamental (dBc)", ylim=(-125, 5))
    ax.title.set_ha("left"); ax.title.set_x(0)
    ax.grid(True); ax.legend(loc="lower right")
    fig.text(0.01, 0.01, "Values below -120 dBc are shown at -120 dBc. Even harmonics are not plotted; the highest measured are H2 -92 dBc and H4 -98 dBc.",
             fontsize=9, color="#999999")
    fig.tight_layout(rect=(0, 0.03, 1, 1)); fig.savefig(os.path.join(img, "harmonic-distortion.png"), dpi=120); plt.close(fig)

    fig, axs = plt.subplots(1, 2, figsize=(13, 6))
    for ax, (a, lv) in zip(axs, ((0, -40.0), (100, -60.0))):
        for c, col in ((0, cols[0]), (50, cols[50]), (100, cols[100])):
            r = [t for t in fr if t[0] == a and t[1] == c]
            ax.semilogx([t[4] for t in r], [t[5] for t in r], "o-", color=col, lw=2, ms=4, label=f"COLOR {c} %")
        ax.set(title=f"ANGER {a} %, input {lv:.0f} dBFS", xlabel="Frequency (Hz)")
        ax.title.set_ha("left"); ax.title.set_x(0)
        ax.grid(True, which="major"); ax.legend(loc="lower right")
    axs[0].set_ylabel("Gain relative to input (dB)")
    fig.suptitle("Frequency response, GAIN 0 dB, COLOR 0 / 50 / 100 %", x=0.01, ha="left")
    fig.tight_layout(); fig.savefig(os.path.join(img, "frequency-response.png"), dpi=120); plt.close(fig)

    fig, ax = plt.subplots(figsize=(10, 6))
    for a in (0, 100):
        r = [t for t in gl if t[0] == a]
        ax.plot([t[2] for t in r], [t[4] for t in r], "o-", color=cols[a], lw=2, ms=5, label=f"ANGER {a} %")
    ax.set(title="GAIN law at -40 dBFS input (999 Hz, COLOR 0 %)", xlabel="GAIN setting (dB)", ylabel="Output fundamental (dBFS)")
    ax.title.set_ha("left"); ax.title.set_x(0)
    ax.grid(True); ax.legend(loc="upper left")
    fig.tight_layout(); fig.savefig(os.path.join(img, "gain-law.png"), dpi=120); plt.close(fig)

    fig, ax = plt.subplots(figsize=(10, 6))
    f = np.arange(len(halo))
    m = (f >= 1700) & (f <= 2300)
    if halo_ref is not None:
        ax.plot(f[m], halo_ref[m], color=cols[0], lw=1.2, label="Angry Pig 1.0.0")
    ax.plot(f[m], halo[m], color=cols[100], lw=1.2, label="Angry Pig 1.1.1")
    ax.set(title="Around the 2nd harmonic: 999 Hz at -12 dBFS, ANGER 100 %, COLOR 0 %, GAIN 0 dB",
           xlabel="Frequency (Hz)", ylabel="Level relative to fundamental (dBc, Hann window)", ylim=(-200, -60))
    ax.title.set_ha("left"); ax.title.set_x(0)
    ax.grid(True); ax.legend(loc="upper right")
    fig.tight_layout(); fig.savefig(os.path.join(img, "halo-h2.png"), dpi=120); plt.close(fig)


def main():
    args = sys.argv[1:]
    legacy = "--legacy-gain" in args
    ref = args[args.index("--compare") + 1] if "--compare" in args else None
    exe, bundle, out = args[0], args[1], args[2]
    work = os.path.join(out, "work")
    os.makedirs(work, exist_ok=True)
    p = Plugin(exe, bundle, legacy, work)
    fr, tr, gl = measure(p, out)
    halo = halo_spectrum(p)
    halo_ref = halo_spectrum(Plugin(exe, ref, True, work)) if ref else None
    plots(out, fr, tr, gl, halo, halo_ref)
    print(f"wrote {len(fr)} frequency-response, {len(tr)} transfer and {len(gl)} gain-law rows to {out}")


if __name__ == "__main__":
    main()
