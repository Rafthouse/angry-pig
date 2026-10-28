# Measurements: Angry Pig 1.0.0

These tables and plots describe the behaviour of the Windows VST3 build of Angry Pig 1.0.0. They were measured on the plugin itself, with the project's measurement harness. No values are simulated or invented.

## Plugin under test

| Item | Value |
| --- | --- |
| Plugin | Angry Pig 1.0.0, Windows x64 VST3 |
| DLL | `Contents_64-win\AngryPigSpike.vst3`, SHA-256 `E5CF23D351DE06DFCB7BC0A9A96A07E3A3AAFB651375B4A21F04C6D7622EE2A1` |
| Reported latency | 0 samples |
| Channels | Stereo in, stereo out |

## Measurement conditions

| Item | Value |
| --- | --- |
| Measurement tool | Project measurement harness (offline processing of the VST3 DLL). Not Plugin Doctor. |
| Sample rate | 44 100 Hz, 32-bit float |
| Analysis window | 1 s (44 100 samples), taken after a 0.54 s warm-up |
| Frequency resolution | 1 Hz (coherent DFT; test tones fall exactly on bins) |
| Test signal | Sine at the stated frequency. Levels are the sine peak amplitude in dBFS. |
| Harmonic analysis | H2 to H10 relative to the output fundamental (dBc). THD is the root sum of squares of H2 to H10 relative to the fundamental (dB). |
| Parameter units | ANGER and COLOR in percent of range (0 to 100 %). GAIN in dB (-24 to +18 dB). |
| Host | None. The DLL was loaded directly. |

Values below -150 dBc are at the numerical floor of the analysis. Even harmonics at 999 Hz were at this floor for every condition tested with COLOR 0 %.

## Files

| File | Contents |
| --- | --- |
| [`frequency_response.csv`](frequency_response.csv) | Gain versus frequency for ANGER 0 % (input -40 dBFS) and ANGER 100 % (input -60 dBFS), at COLOR 0, 50 and 100 %. |
| [`transfer_harmonics_999hz.csv`](transfer_harmonics_999hz.csv) | Input and output levels, harmonics and THD at 999 Hz across ANGER and input levels, plus the COLOR cases. |
| [`gain_law.csv`](gain_law.csv) | Output level for GAIN from -24 to +18 dB at ANGER 0 % (input -40 dBFS) and ANGER 100 % (input -40 dBFS). |

## Plots

![Transfer curve](../images/measurements/transfer-curve.png)

![Harmonic distortion](../images/measurements/harmonic-distortion.png)

![Frequency response](../images/measurements/frequency-response.png)

![GAIN law](../images/measurements/gain-law.png)

The plots are drawn from the CSV files above with matplotlib. They are not Plugin Doctor screenshots.

## Transfer and harmonics at 999 Hz (GAIN 0 dB, COLOR 0 %)

| ANGER | Input (dBFS) | Output fundamental (dBFS) | H2 (dBc) | H3 (dBc) | H5 (dBc) | THD H2-H10 (dB) | Output peak (dBFS) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 0 % | -36 | -35.00 | < -150 | < -150 | < -150 | -170.43 | -35.00 |
| 0 % | -24 | -23.00 | < -150 | < -150 | < -150 | -174.44 | -23.00 |
| 0 % | -12 | -11.00 | < -150 | < -150 | < -150 | -173.12 | -11.00 |
| 0 % | -6 | -5.00 | < -150 | < -150 | < -150 | -173.74 | -5.00 |
| 0 % | 0 | 0.53 | < -150 | -27.28 | -32.11 | -25.90 | -0.12 |
| 25 % | -36 | -25.25 | < -150 | < -150 | < -150 | -170.54 | -25.25 |
| 25 % | -24 | -13.25 | < -150 | < -150 | < -150 | -174.90 | -13.25 |
| 25 % | -12 | -1.25 | < -150 | -95.97 | -96.37 | -89.43 | -1.25 |
| 25 % | -6 | 1.06 | < -150 | -19.02 | -30.77 | -18.66 | -0.10 |
| 25 % | 0 | 1.20 | < -150 | -17.27 | -33.56 | -17.06 | -0.10 |
| 50 % | -36 | -15.50 | < -150 | < -150 | < -150 | -170.91 | -15.50 |
| 50 % | -24 | -3.50 | < -150 | < -150 | < -150 | -174.53 | -3.50 |
| 50 % | -12 | 1.37 | < -150 | -15.37 | -50.92 | -15.25 | -0.10 |
| 50 % | -6 | 1.46 | < -150 | -14.43 | -38.68 | -14.30 | -0.10 |
| 50 % | 0 | 1.53 | < -150 | -13.66 | -31.41 | -13.50 | -0.10 |
| 75 % | -36 | -5.75 | < -150 | < -150 | < -150 | -168.67 | -5.75 |
| 75 % | -24 | 1.50 | < -150 | -14.03 | -34.33 | -13.88 | -0.10 |
| 75 % | -12 | 1.63 | < -150 | -12.77 | -26.05 | -12.51 | -0.10 |
| 75 % | -6 | 1.68 | < -150 | -12.30 | -23.81 | -11.95 | -0.10 |
| 75 % | 0 | 1.72 | < -150 | -11.90 | -22.12 | -11.46 | -0.10 |
| 100 % | -36 | 1.36 | < -150 | -15.41 | -49.56 | -15.28 | -0.10 |
| 100 % | -24 | 1.70 | < -150 | -12.09 | -22.91 | -11.70 | -0.10 |
| 100 % | -12 | 1.77 | < -150 | -11.42 | -20.23 | -10.82 | -0.10 |
| 100 % | -6 | 1.80 | < -150 | -11.16 | -19.27 | -10.45 | -0.10 |
| 100 % | 0 | 1.83 | < -150 | -10.94 | -18.48 | -10.12 | -0.10 |

## Gain law (GAIN 0 dB reference)

Output gain is the output fundamental minus the input level. The output is in dB.

| GAIN setting (dB) | ANGER 0 %: output gain (dB) | ANGER 100 %: output fundamental (dBFS) | ANGER 100 %: output gain (dB) |
| --- | --- | --- | --- |
| -24 | -23.00 | -24.00 | +16.00 |
| -12 | -11.00 | -12.00 | +28.00 |
| -6 | -5.00 | -6.00 | +34.00 |
| +0 | +1.00 | -0.08 | +39.92 |
| +6 | +7.00 | 1.61 | +41.62 |
| +12 | +13.00 | 1.67 | +41.67 |
| +18 | +19.00 | 1.71 | +41.71 |

## Frequency response (GAIN 0 dB)

Gain in dB relative to the input, measured per frequency.

| Frequency (Hz) | ANGER 0 %, COLOR 0 % | ANGER 0 %, COLOR 50 % | ANGER 0 %, COLOR 100 % | ANGER 100 %, COLOR 0 % | ANGER 100 %, COLOR 50 % | ANGER 100 %, COLOR 100 % |
| --- | --- | --- | --- | --- | --- | --- |
| 20 | -10.49 | -10.49 | -10.49 | +28.51 | +28.51 | +28.51 |
| 30 | -3.71 | -3.69 | -3.71 | +35.30 | +35.31 | +35.30 |
| 50 | -0.23 | -0.20 | -0.23 | +38.77 | +38.80 | +38.77 |
| 100 | +0.78 | +0.92 | +0.79 | +39.78 | +39.92 | +39.79 |
| 200 | +0.95 | +1.52 | +0.97 | +39.95 | +40.52 | +39.97 |
| 500 | +0.99 | +3.90 | +1.11 | +39.99 | +42.91 | +40.12 |
| 1000 | +1.00 | +4.43 | +1.48 | +40.00 | +43.43 | +40.48 |
| 2000 | +1.00 | +2.12 | +2.74 | +40.00 | +41.12 | +41.74 |
| 3000 | +1.00 | +1.50 | +4.43 | +40.00 | +40.50 | +43.43 |
| 5000 | +1.00 | +1.17 | +7.80 | +40.00 | +40.17 | +46.80 |
| 8000 | +1.00 | +1.06 | +8.47 | +40.00 | +40.06 | +47.47 |
| 10000 | +1.00 | +1.03 | +6.64 | +40.00 | +40.03 | +45.64 |
| 15000 | +1.00 | +1.01 | +2.83 | +40.00 | +40.01 | +41.83 |
| 20000 | +1.00 | +1.00 | +1.15 | +40.00 | +40.00 | +40.15 |

## Effect of COLOR on distortion

COLOR can raise the level of the band it boosts. If that band enters the saturation stage, distortion appears that would not appear at COLOR 0 %.

| ANGER | COLOR | Input (dBFS) | Output fundamental (dBFS) | H3 (dBc) | THD H2-H10 (dB) |
| --- | --- | --- | --- | --- | --- |
| 50 % | 0 % | -24 | -3.50 | < -150 | -174.53 |
| 50 % | 0 % | -12 | 1.37 | -15.37 | -15.25 |
| 50 % | 0 % | -6 | 1.46 | -14.43 | -14.30 |
| 50 % | 50 % | -24 | -0.13 | -43.31 | -40.12 |
| 50 % | 50 % | -12 | 1.42 | -14.80 | -14.68 |
| 50 % | 50 % | -6 | 1.50 | -13.97 | -13.83 |
| 50 % | 100 % | -24 | -3.02 | < -150 | -173.50 |
| 50 % | 100 % | -12 | 1.38 | -15.29 | -15.16 |
| 50 % | 100 % | -6 | 1.46 | -14.36 | -14.23 |
| 100 % | 0 % | -24 | 1.70 | -12.09 | -11.70 |
| 100 % | 0 % | -12 | 1.77 | -11.42 | -10.82 |
| 100 % | 0 % | -6 | 1.80 | -11.16 | -10.45 |
| 100 % | 50 % | -24 | 1.72 | -11.87 | -11.43 |
| 100 % | 50 % | -12 | 1.79 | -11.26 | -10.61 |
| 100 % | 50 % | -6 | 1.82 | -11.03 | -10.26 |
| 100 % | 100 % | -24 | 1.70 | -12.06 | -11.66 |
| 100 % | 100 % | -12 | 1.78 | -11.40 | -10.79 |
| 100 % | 100 % | -6 | 1.81 | -11.14 | -10.43 |

## Notes

- Small-signal gain is 1.0 dB plus 0.39 dB for each percent of ANGER (measured 1.0 dB at ANGER 0 % and 40.0 dB at ANGER 100 %).
- At ANGER 0 %, the output equals the input plus GAIN plus 1.0 dB, within 0.01 dB across -24 to +18 dB.
- When saturation engages, the output peak stays at -0.10 dBFS. The fundamental in the Fourier analysis can exceed 0 dBFS, because the waveform is squared off.
- The gain reduction stage and the attack and release times are not separately measured here. Their values are described from the implementation in `docs/technical/`.
- Plugin Doctor graphs are shown in the next section. Plugin Doctor uses its own test signals, so the two sets of values are compared with each other there. One difference, in the Linear view, is not resolved; see the investigation below.

## Plugin Doctor graphs

Captured in Plugin Doctor 2.7.3 (DDMF) with the Angry Pig 1.0 plug-in loaded in Chain #1. Plugin Doctor generated the test signal. The plug-in is the same installed 1.0.0 build as above.

![Harmonic analysis, Plugin Doctor](../images/plugindoctor/plugindoctor-harmonic-anger100-color0-gain0.png)

**Harmonic analysis.** Sine at 998.6 Hz (Plugin Doctor's generator), level -12 dBFS, ANGER 100 %, COLOR 0 %, GAIN 0 dB. Plugin Doctor reads THD -10.8 dB. The harness gives -10.82 dB (H2 to H10) at 999 Hz under the same settings, so the two agree.

![Transfer curve at ANGER 100, Plugin Doctor](../images/plugindoctor/plugindoctor-transfer-anger100.png)

**Transfer curve, ANGER 100 %.** Static level sweep from -20 to 0 dB, COLOR 0 %, GAIN 0 dB. The output flattens near 0 dB, which is the saturation ceiling.

![Transfer curve at ANGER 0, Plugin Doctor](../images/plugindoctor/plugindoctor-transfer-anger0.png)

**Transfer curve, ANGER 0 %.** The same sweep at ANGER 0 %. The output sits about 1 dB above the unity line, the base gain. It bends near -2 dB input as the saturation stage starts.

![Linear view, Plugin Doctor](../images/plugindoctor/plugindoctor-linear-anger0-color0.png)

**Linear view, ANGER 0 %, COLOR 0 %.** Plugin Doctor's Linear view shows a low-frequency rise of about +4.6 dB near 35 Hz, relative to its own 1 kHz reading. The harness, using sine tones, shows no such rise. This is an unresolved measurement discrepancy. It has not been shown to be a defect of either tool, and no DSP change was made for it. The investigation is below.

### Linear-response discrepancy: investigation (unresolved)

| Factor | Plugin Doctor (2.7.3) | Harness (sine tones) | Finding |
| --- | --- | --- | --- |
| Test signal | Default Linear excitation; PD does not document it. The "Random" option switches the view to a noise spectrum, which is not a transfer function. | One sine per frequency, 1 Hz bins, 1 s analysis window. | The default Linear mode is the only one comparable to the harness. The "Random" view was checked and is not comparable. |
| Level | A level value of 3.58 dB is shown in the Linear view; its meaning is not documented. | Sine peaks from -40 to 0 dBFS. | At 35 Hz the harness gain stays at -2.18 dB from -40 to -3 dBFS. Only 0 dBFS changes the 1 kHz reference, through the gain-reduction stage. Level does not produce the rise. |
| Plugin settings | ANGER 0 %, COLOR 0 %, GAIN 0 dB | ANGER 0 %, COLOR 0 %, GAIN 0 dB | Identical. |
| Sample rate | Not shown in the Plugin Doctor interface or its log. | 44.1, 48 and 96 kHz | 35 Hz gain: -2.18, -2.03 and -2.08 dB. 20 Hz gain: -10.49, -10.65 and -10.68 dB. No rise at any rate. |
| Measurement method | Averaged spectral analysis; method not documented. | Coherent single-tone DFT. | Results are not directly comparable. |
| Absolute offset | Empty chain: flat line at +3.58 dB. Plugin chain: 0 dB at high frequencies. | +1.0 dB at 1 kHz for the plugin. | Plugin Doctor's absolute scale does not match the harness. Only the shape can be compared, and the shape still differs. |
| Control | Plugin removed (empty chain): no feature near 35 Hz. | Not applicable. | The rise is produced when the plugin is in Plugin Doctor's chain. Plugin Doctor's own signal path adds no rise. |

Relative to 1 kHz, the 35 Hz point is about +4.6 dB in Plugin Doctor and -3.2 dB in the harness, a difference of about 7.8 dB.

**Status.** Unresolved. Not established: whether the cause lies in Plugin Doctor's excitation or analysis, or in the plugin's response to broadband signals. Not done: a broadband (noise or multi-tone) test with the harness, and reading Plugin Doctor's test level and sample rate, which the interface does not show. For small-signal frequency response, use the harness table above.
