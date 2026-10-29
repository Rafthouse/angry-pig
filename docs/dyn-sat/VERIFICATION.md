# Dynamic Saturation 1.1.1 beta 1: verification record

What was checked before the `v1.1.1-dyn-sat-beta1` pre-release, and how. Windows and macOS are evaluated separately.

## Baseline

| Item | Value |
| --- | --- |
| Approved candidate | Windows DLL `f1d58dcb…` (Gain -36 to +36 dB), installed and listened to in REAPER by the author |
| Reference for the halo and harmonics | DLL `38593B36…` (not published) |
| DSP source in this tree | `src/native/APDspEngine.h`, `APDynamicSaturation.h`, `APMatchTables.h`: byte-identical to the `f1d58dcb` source (line endings ignored) |
| VST3 class ID | `APLb` / `APd2`, CID `F2AEE70D00DE4F4E41504C6241506432`. Angry Pig 1.0.0 (`APm1`) is a different plug-in and is not changed. |
| Parameters | ANGER 0 to 100 %, COLOR 0 to 100 %, GAIN -36 to +36 dB, BYPASS. Same IDs and serialization as the candidate. |

## Performance

One profile, no optimization applied. Details: [PERFORMANCE.md](PERFORMANCE.md). About 110 to 130 ns per stereo frame, 0.5 % of one core at 44.1 kHz and 1.1 % at 96 kHz (i9-9900KF). CPU before = CPU after, because the DSP source did not change.

## No sound regression

All renders with the pinned Windows build of this tree.

| Check | Result |
| --- | --- |
| This tree vs `f1d58dcb`: 16 cases (44.1 / 48 / 96 kHz, blocks 1 / 64 / 127, Dynamic A+B on and `AP_DYNAMIC_AMOUNT=0`, two noise seeds, digital silence, -6 / -24 dBFS, +18 dB gain) | bit-identical in all 16 |
| This tree vs reference `38593B36`, +18 dB gain and silence | bit-identical |
| This tree vs reference `38593B36`, 0 dB gain | max difference 5.96e-8 (-144.5 dBFS). The old plug-in stores 0 dB as 0.5714285714 of its -24 to +18 dB range, which is -1.2e-9 dB. |
| `AP_DYNAMIC_AMOUNT=0` vs RC 1.1 | bit-identical (Dynamic off is the static engine) |
| Halo, 999 Hz sine at -12 dBFS, ANGER 100 %, COLOR 0 %, GAIN 0 dB, 44.1 kHz | -75.82 dBc (expected -75.82; reference plug-in -75.96) |
| Low-frequency floor 20 to 200 Hz | -83.38 dBc (expected -83.38) |
| Odd harmonics H3 to H15 | identical to the reference values (max delta 0.00000 dB) |
| DC | -112.5 dBc |
| Digital silence in | exactly 0 out |
| Block size 1 vs 512 | bit-identical |

Reference values: [`tools/parity/reference/windows-x64-sine999-m12-anger100.json`](../../tools/parity/reference/windows-x64-sine999-m12-anger100.json).

The released Windows DLL (built by CI, SHA-256 `404F6F7B3EAAD53C0308AA3D63DAA286FC879FC9AEEDAE7C1A41CDF48FD3407C`) has a different file hash from the local build (different machine and compiler build). Its output is bit-identical to the local build in 7 renders (44.1 / 48 / 96 kHz, blocks 1 / 64 / 127 / 512, ANGER ramp, GAIN -36 / -6 / 0 / +18 dB, bypass).

## Windows x64 VST3

CI results from build run [40900683844](https://github.com/Rafthouse/angry-pig/actions/runs/40900683844) (commit `5e6b624`, `windows-2022`).

| Test | Result |
| --- | --- |
| Clean-checkout build on `windows-2022` (MSVC, `/fp:strict`) | pass |
| pluginval 1.0.4, strictness 8 | SUCCESS |
| Steinberg VST3 validator (built from the vendored SDK) | 47 passed, 0 failed. The two rejected sample rates (1234.568 Hz and 1234567.8 Hz) are outside the supported 8 kHz to 384 kHz range and are expected. |
| DSP metrics of the CI render (halo, LF, odd, DC, block 1) | PASS |
| Setup.exe in an isolated folder: install, status, uninstall, reinstall, 1.0.0 bundle untouched, manifest hashes | 24 of 24 checks |
| Host smoke test | REAPER on Windows 11 by the author with the approved `f1d58dcb` build, whose output this build reproduces bit for bit. Not repeated with the CI binary. |

The installer is built with IExpress and PowerShell, the same approach as the 1.0.0 Windows installer. There is no NSIS script in this project.

## macOS universal VST3 + AUv2

All results from build run [40900683844](https://github.com/Rafthouse/angry-pig/actions/runs/40900683844) (commit `5e6b624`), on GitHub-hosted runners. Logs are attached to the run.

| Test | Apple Silicon (`macos-14`, macOS 14.8) | Intel (`macos-15-intel`, macOS 15.7) |
| --- | --- | --- |
| Build with Xcode 15.4, `arm64` + `x86_64` | pass, `lipo` shows `x86_64 arm64` for both bundles | (same universal build) |
| Download checksums | OK | OK |
| Install the PKG with `installer` | pass, 1.0.0 bundle path untouched | pass |
| `auval -v aufx APd2 APLb` | AU VALIDATION SUCCEEDED | AU VALIDATION SUCCEEDED |
| pluginval 1.0.4, strictness 8, VST3 | SUCCESS | SUCCESS |
| pluginval 1.0.4, strictness 8, AU | SUCCESS | SUCCESS |
| Halo / LF / odd H3 to H15 / DC, VST3 and AU, vs Windows | identical (-75.8165 / -83.3760 / delta 0.00000 dB / -112.5 dBc) | identical |
| 999 Hz sine samples vs Windows | max difference 5.96e-8 (one float LSB, -144.5 dBFS) | bit-identical |
| Program material samples vs Windows | max difference 5.96e-8 | max difference 7.1e-15 |
| Objective-C class names | own prefix `vAngryPigDynSat`, AU view `AngryPigDynSat_View`; no name shared with Angry Pig 1.0.0 | (same binary) |
| Code signing | ad-hoc only. Not notarized. Gatekeeper rejects the package until the user allows it. | same |
| Physical Mac and a DAW | **not run.** Checklist for a Mac mini M1: [MAC_MINI_M1_TEST.md](MAC_MINI_M1_TEST.md) | not run |

The arm64 differences come from floating-point rounding in the Apple Silicon maths library and compiler; they are far below audibility and do not change any measured metric.

### Fixed during release testing

CI found two macOS-only defects in the first builds of this branch. Neither affects Windows or the DSP.

1. The bundles are renamed to `AngryPigDynamicSaturation`, but iPlug still looked them up under the compile-time identifier `...AngryPigSpike`. The AU view lookup got a NULL bundle and auval crashed in the custom-UI test; in a host the editor would have failed the same way. `BUNDLE_NAME` in `config.h` now matches the shipped identifiers, and CI checks them.
2. The Objective-C class prefix and the AU view class had the same names as in Angry Pig 1.0.0, so both plug-ins in one host could clash. They are now unique, and CI checks the class names.
