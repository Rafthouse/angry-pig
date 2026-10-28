# Angry Pig Dynamic Saturation: recovery and isolated candidate

Status: isolated Windows VST3 candidate for listening and Plugin Doctor checks. Not installed, not integrated into production, no release. Production 1.0.x, Program Files, RC1.1, the Dynamic EXP outputs, macOS VST3/AU and GitHub were not touched.

## 1. Candidate artifact

| Item | Value |
|---|---|
| Deliverable bundle | `D:\AP-RE\tmp\dyn-sat-recovery\deliverable\AngryPig-DynamicSaturation-candidate\AngryPigSpike.vst3` |
| DLL | `...\AngryPigSpike.vst3\Contents\x86_64-win\AngryPigSpike.vst3` (715,776 bytes) |
| DLL SHA-256 | `38593B36A72019FBBC2F179F5EDD4F2E744094D362E027ECFF1E98D4ED4AC28B` |
| Class ID | new: `PLUG_UNIQUE_ID 'APd2'`, `PLUG_MFR_ID 'APLb'` (production and RC1.1 use `APm1`) |
| Name / version | Angry Pig Dynamic Saturation (candidate), 1.1.1-dsc |
| Source diff | `report\source-diff-candidate-vs-dynamic-exp.patch` (against the Dynamic EXP source, commit 4bce5b6) |
| Baseline references | RC1.1 DLL `9863C5C4C43B86A4CB12A14AB6C3CFA0DCD03FBE8E039F4C846EA040D8C5528A`; Dynamic EXP DLL `B91C615ABA485172805006D78EAB6D3142A27FEB5A9FC1B7772FF1F8163C28F5` |

## 2. What was recovered and what it showed

Saved Dynamic EXP mechanisms (dumps and harness logs of the final build, `f3_*`), 10 s, 999 Hz, -12 dBFS, ANGER 100%, GAIN 0 dB:

| Configuration | Halo H2..H14 (dBc) | H2 centre (dBc) | LF 20-200 Hz (dBc) | Floor 200 Hz-2 kHz / 8-22 kHz (median bin) |
|---|---|---|---|---|
| REF | -75.96 +- 0.54 | -104.2 | -83.11 | -143.9 / -155.7 |
| RC1.1 (amount 0) | -95.38 (deterministic only) | -231 | -106.7 | -233.8 / -234.0 |
| EXP early `full_1` | -68.81 +- 4.85 | -52.6 | -74.5 | -109.8 / -127.5 |
| EXP final `f3_full_1` | -95.19 +- 0.32 | -96.1 | -83.07 | -138.8 / -146.9 |
| EXP A only (slow additive bias) | -95.25 | -96.1 | -106.6 | not measured |
| EXP B only (0.19 Hz multiplicative drive) | -95.35 | -151.9 | -107.3 | not measured |
| EXP C only (noise residual) | -95.34 | -145.4 | -83.07 | -143.2 / -155.8 |

**Early versus final (26.4 dB).** The early `full_1` run had DC -42.2 dBc, LF 0.5-30 Hz at -63.9 dBc and H2 centre at -52.6 dBc. Its 1-8 Hz shell (-69.3 dBc) dominated its "halo"; its 8-32 Hz shell was -80.7 dBc. Its value was unstable (std 4.85 dB). The final build has DC -120.6 dBc and LF -87.3 dBc. Its difference from the early build is the centring of the transfer (`saturate(x+b)-saturate(b)`, silence exact) and the 3 Hz high-pass on the added term, both described in the final source patch and the EXP report. The early source is not preserved (single commit in git, no saved early DLL), so the code change itself cannot be shown. What is measured: the early value was a DC and sub-hertz artefact of the uncentred bias, not a REF-like halo. The final value is the deterministic floor, so nothing that worked was lost.

**Attribution of the existing mechanisms.**
- A (slow additive bias, 0.07 Hz sinusoids): sub-hertz sidebands only, so no skirt in the 1 Hz shells. It sets the H2 centre (-96 dBc) and nothing else in the halo.
- B (multiplicative drive, 0.19 Hz): moves only sub-hertz sidebands, and multiplies the signal so it also moves odd harmonics slightly (H3 +0.009 dB, H5 +0.034 dB). It produces no halo.
- C (noise residual, xorshift, 70 Hz and 2.5 kHz filters, gated): reproduces the REF low-frequency level (20-200 Hz -83.07 vs -83.11 dBc) and the 200 Hz-22 kHz medians, but has no structure around the harmonics. In the EXP it is the only source of the broadband background.

Result: none of A, B or C produces the +-30 Hz skirts around the even harmonics. The mechanism that does is envelope-scaled, band-limited modulation of the operating point (see section 3). That was the one missing ingredient.

## 3. The candidate

Changes from the Dynamic EXP source (`APDynamicSaturation.h`, `APDspEngine.h`, `config.h`):

1. **B replaced.** The multiplicative 0.19 Hz drive is now an additive term added to the saturator input: `b = amount * kappa * env * u`. `u` is seeded white noise (xorshift) through a 2nd-order Butterworth low-pass at 32 Hz, normalised to unit variance. `env` is a 1 ms average of |input|, so the intensity follows the signal and is zero in silence. `kappa = 2.0e-4`. The output term is `saturate(shaped + b + A-bias) - saturate(A-bias) - saturate(shaped) + residual`, high-passed at 3 Hz as before, so odd harmonics are not modulated to first order (S' is even).
2. **C off by default.** C's low-frequency part duplicates B's (see section 4). `AP_DYNAMIC_COMPONENTS=ABC` re-enables it.
3. **Default amount 1.** `AP_DYNAMIC_AMOUNT=0` gives the RC1.1 output bit for bit (verified: SHA-256 of the 10 s dump `7223103D...`, identical to the RC1.1 dump).
4. **New class ID** `APd2` and a new name/version, so Plugin Doctor and the host do not merge the candidate with production or RC1.1 (the production, RC1, RC1.1, r2/r3 and Dynamic EXP binaries all carry the `APm1` class ID).
5. Unchanged: ClipCurve/`saturate`, Hard Knee, filters, detector, parameter mapping, GUI and meter path, the A centring and the 3 Hz high-pass.

`kappa` was set once from the offline calibration on the integrated H2..H14 energy of the -12 dBFS sine (seed 1). It was not refit against the DSP output and no FFT bins were fitted.

## 4. Measurements (existing harness, 10 s dumps, same settings as the reports: 44.1 kHz, bin 999, 3 s warm-up discarded)

### 4.1 Halo at -12 dBFS (dBc, 1-128 Hz shells around H2..H14, mean over 10 s)

| Case | Halo | H2 | H4 | H6 | H8 | H10 | H12 | H14 | Shells 1-8 / 8-32 / 32-128 |
|---|---|---|---|---|---|---|---|---|---|
| REF | -75.96 +- 0.54 | -78.29 | -81.36 | -87.99 | -107.53 | -92.40 | -91.25 | -95.61 | -84.2 / -78.2 / -82.2 |
| **Candidate (default)** | **-75.82 +- 0.38** | -78.19 | -81.23 | -87.78 | -104.95 | -92.14 | -90.81 | -94.56 | -83.1 / -78.2 / -82.5 |
| Candidate seed 2 | -76.14 +- 0.48 | -78.51 | -81.56 | -88.10 | -105.27 | -92.47 | -91.11 | -94.83 | -83.6 / -78.6 / -82.4 |
| Candidate seed 3 | -76.30 +- 0.62 | -78.67 | -81.72 | -88.28 | -105.36 | -92.62 | -91.28 | -95.01 | -83.6 / -78.9 / -82.3 |
| RC1.1 | -95.38 (deterministic) | -105.26 | -102.13 | -103.13 | -109.43 | -108.43 | -102.42 | -101.73 | -211 / -95.4 / -121.9 |

### 4.2 Width and shape

| Case | Energy within 32 Hz (dB of halo) | Density 16-32 Hz (dB/Hz) | Density 64-127 Hz (dB/Hz) | Slope 16-127 Hz (dB/oct) |
|---|---|---|---|---|
| REF | -1.19 | -92.15 | -109.06 | -8.46 |
| **Candidate** | **-1.10** | **-92.66** | **-109.73** | **-8.53** |
| EXP final | -0.12 | -129.59 | -130.58 | -0.50 |
| EXP early | -0.17 | -96.19 | -104.92 | -4.37 |

### 4.3 Levels (same candidate coefficients at every level; REF records L999_m6/m12/m24)

| Level | REF halo | Candidate halo | H2 REF / cand | H4 REF / cand | H6 REF / cand | H3 REF / cand | H5 REF / cand | LF 20-200 REF / cand |
|---|---|---|---|---|---|---|---|---|
| -6 dBFS | -75.85 +- 0.68 | -75.45 +- 0.38 | -78.47 / -78.10 | -81.06 / -80.73 | -86.38 / -86.04 | -11.156 / -11.158 | -19.258 / -19.266 | -83.45 / -83.42 |
| -12 dBFS | -75.96 +- 0.54 | -75.82 +- 0.38 | -78.29 / -78.19 | -81.36 / -81.23 | -87.99 / -87.78 | -11.416 / -11.419 | -20.218 / -20.227 | -83.11 / -83.38 |
| -24 dBFS | -76.58 +- 0.71 | -76.58 +- 0.38 | -78.34 / -78.36 | -82.66 / -82.69 | -93.99 / -93.91 | -12.086 / -12.089 | -22.893 / -22.908 | -82.63 / -83.23 |

The RC1.1 path has no halo at any level (-94.5, -95.4, -100.8 dBc).

### 4.4 Broadband floor (median bin, dBc, harmonic bins +-2 Hz excluded)

| Case | 20-200 Hz | 200 Hz-2 kHz | 2-8 kHz | 8-22 kHz |
|---|---|---|---|---|
| REF | -117.6 | -143.9 | -148.0 | -155.7 |
| **Candidate (C off)** | **-117.9** | **-144.3** | **-147.3** | **-152.5** |
| Candidate, C on (ABC) | -109.8 | -138.8 | -144.1 | -149.9 |
| EXP final | -110.8 | -138.8 | -138.6 | -146.9 |
| RC1.1 | -233.5 | -233.8 | -233.8 | -234.0 |

Why C is off: with C on the floor is 4-8 dB above REF, because B alone already reproduces the REF floor (B-only: -117.97 / -144.3 / -147.4 / -152.6).

### 4.5 Checks

| Check | Result |
|---|---|
| Amount 0 versus RC1.1 | bit-identical (`7223103D...`) |
| Silence (input exactly zero, -1000 dBFS) | output exactly 0.0 |
| Block size 1 / 64 / 127 / 128 | bit-identical output |
| Independent realisations (seeds) | halo -75.8 / -76.1 / -76.3 dBc; per-second std 0.38-0.62 dB (REF 0.54) |
| Onset after reset | none: first second is 0.21 dB from the mean of seconds 2-10 (REF -0.22); the candidate-minus-RC1.1 rms in the first 50 ms (1.9e-4) is the same order as at 1-2 s (1.6e-4) |
| DC | candidate -112.5 dBc (REF -103.2 dBc) |
| Peak / ceiling | 0.98866 peak versus ceiling 0.98855 (0.001 dB over, not a clip event) |
| Sample step of candidate minus RC1.1 | max 1.26e-3 (EXP 2.88e-3) |
| Odd harmonics H3..H15 versus RC1.1 | identical to 0.001 dB (H3 -11.419, H5 -20.227, H7 -33.188, H9 -39.675, H11 -34.883, H13 -37.726, H15 -47.615) |
| CPU | ~15 extra operations and one extra saturator call per channel per sample. Harness timing, 10 s of audio: 0.09 s with amount 0, 0.10 s with amount 1 (includes harness overhead; an upper bound of about 0.1 % of one core) |

## 5. Acceptance criteria, judged separately

| Criterion | Result | Note |
|---|---|---|
| Halo energy within +-3 dB of REF | PASS: -75.82 versus -75.96 (+0.14 dB); -6 dBFS +0.40 dB; -24 dBFS 0.00 dB | |
| Width close to REF | PASS: energy within 32 Hz -1.10 versus -1.19 dB; slope -8.53 versus -8.46 dB/oct; density 16-32 Hz within 0.5 dB | |
| No unacceptable change to main harmonics | PASS: odd harmonics identical to RC1.1; H3 and H5 match REF to 0.01 dB | |
| Even-harmonic distribution | PASS with note: H2, H4, H6, H10, H12 within 0.3 dB; H14 +1.0 dB; H8 2.6 dB shallower than REF (-104.9 versus -107.5) | |
| Normal dynamic response to music | NOT TESTED: the harness only feeds sines. The change is -76 dBc and leaves odd harmonics untouched, so the expected effect on programme material is small, but this is not measured | Needs a music test |
| No audible noise in silence and decay | Silence: PASS (exact zero). Decay: not measured; the envelope decays to exact zero below 1e-12 | Needs a signal-to-silence test |
| Manageable CPU | PASS by estimate and by harness timing (upper bound about 0.1 % of one core) | |

Known deviations, stated without compensation:
- 8-22 kHz floor is 3.2 dB below REF (-152.5 versus -155.7 dBc). C supplies that floor but adds too much in the 20 Hz-8 kHz range. A variant with only C's high-frequency path was not tested; that would be a new change and needs an owner decision.
- H8 is 2.6 dB shallower than REF.

## 6. Safe testing

1. Do not copy the bundle into `C:\Program Files\Common Files\VST3`. Load it from `D:\AP-RE\tmp\dyn-sat-recovery\deliverable\AngryPig-DynamicSaturation-candidate\`.
2. The candidate runs with amount 1 by default. To check the RC1.1-equivalent path, set `AP_DYNAMIC_AMOUNT=0` in the environment of the process that starts Plugin Doctor before it loads the plugin. Set it to `1` for the candidate.
3. In Plugin Doctor: 44.1 kHz, Harmonic analysis, 999 Hz at -12 dB, ANGER 100%, COLOR 0%, GAIN 0 dB. Turn off "Reload last state on start" first (the EXP guide warns about stale slots).
4. Compare against the RC1.1 DLL loaded by its own path. Both show the Angry Pig name, but the candidate has a different class ID (`APd2`), so its scan record is separate.

## 7. Not done

- No listening test, no music test, no Plugin Doctor screenshots, no decay test.
- The artifact searched for as `ANGRYPIG_RC1_1_SPECTRAL_RESIDUAL_INVESTIGATION.md` is not in the saved folders.
- The early Dynamic EXP build (`full_1`) was not reproduced: its source is not saved, so the cause of the 26.4 dB difference is attributed from the measurements, not from the code.
- The candidate was calibrated once on integrated energy at -12 dBFS. Levels -6 and -24 dBFS and the seeds were used as checks, not for calibration.

## 8. C high-frequency variant (approved test)

Test build with a test-only switch AP_DYNAMIC_C_LOWPATH=0 (C without its 70 Hz low-frequency path; only the 2.5 kHz branch). Test DLL SHA-256 $h. Default behaviour is unchanged: the default-configuration dump is bit-identical to the shipped candidate (D7E56F56…), and ABC with the switch at its default reproduces the earlier ABC dump ( D623D04…). The shipped deliverable DLL is still 38593B36….

| Case (-12 dBFS) | Halo H2..H14 | LF 20-200 Hz | Floor 20-200 | Floor 200 Hz-2 kHz | Floor 2-8 kHz | Floor 8-22 kHz |
|---|---|---|---|---|---|---|
| REF | -75.96 | -83.11 | -117.6 | -143.9 | -148.0 | -155.7 |
| AB (C off, shipped default) | -75.82 | -83.38 | -117.9 | -144.3 (+0.4) | -147.3 (+0.6) | -152.5 (+3.1) |
| ABC (C full) | -75.82 | -80.30 | -109.8 | -138.8 (+5.1) | -144.1 (+3.9) | -149.9 (+5.8) |
| ABC, C high-frequency only | -75.82 | -83.38 | -118.1 | -139.5 (+4.4) | -144.1 (+3.9) | -150.0 (+5.7) |

Result: rejected. The high-frequency-only C keeps the low-frequency level right, but its 2.5 kHz branch overshoots the 200 Hz-22 kHz floor by 4-6 dB. The shipped default stays AB (C off). The remaining deviation of AB is the 8-22 kHz floor at -3.1 dB. Closing it would need a new level setting for C's 2.5 kHz branch, which is a new coefficient and needs an owner decision.

## 9. Host checks: Class ID, Plugin Doctor and REAPER (status)

- **Class ID (verified from the binary).** The candidate's unique-ID word is 41506432 ("APd2"). Production 1.0.0 (installed), RC1.1 and Dynamic EXP carry 41506D31 ("APm1"). The first three words of the class ID are the same in all of them (B9410008 41504C62 202444C7), so only the last word separates the candidate from the others. The bytes are 2dPA 17 times in the candidate and 1mPA 17 times in production, RC1.1 and EXP, and 1mPA 0 times in the candidate. Plugin Doctor's own uniqueId for the candidate is still to be read from its log.
- **Plugin Doctor.** The launcher Start-PluginDoctor-Candidate.ps1 validates the SHA-256 and starts Plugin Doctor with the candidate path as its argument. The first run did not load the candidate: Plugin Doctor restored its last plugin, the Dynamic EXP (AngryPig-RC1.1-Dynamic-EXP.vst3, class ID APm1, the same class ID as production), and opened its editor. Its log says "starting to load last plugin...". This process (PID 51728) was started by the launcher and was asked to close; it was still running afterwards with the EXP loaded. The option that restores the last plugin is not in the settings XML, so it must be turned off in Plugin Doctor's interface.
- **Harmonic comparison REF versus AB in Plugin Doctor.** Not done. Plugin Doctor's windows were not visible in the screen captures, so no controls could be set.
- **REAPER.** Granted for control, but the REAPER window was not visible either. No project was created and no listening render was made. Setting the candidate's ANGER control needs the plugin window, so the render cannot be automated yet.
- **THD versus halo, from the saved dumps (H2..H15, 10 s):** REF -10.821 dB, RC1.1 -10.824 dB, candidate -10.824 dB. THD is identical within 0.003 dB, while the halo differs by about 20 dB. THD does not show the halo.
- **Width, shape, temporal mobility (existing measurements, no new runs):** energy within 32 Hz -1.10 dB (REF -1.19); density slope -8.53 dB/oct (REF -8.46); halo per second 0.38-0.62 dB std (REF 0.54); the first second is within 0.21 dB of the rest, so no onset.
