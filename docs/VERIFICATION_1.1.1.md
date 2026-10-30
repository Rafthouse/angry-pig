# Angry Pig 1.1.1: verification record

What was checked before the `v1.1.1` release, how, and what was not checked. Windows and macOS are evaluated separately. All CI results are from build run [40988178744](https://github.com/Rafthouse/angry-pig/actions/runs/40988178744) (commit `dc509fc`); its logs are attached to the run.

## What 1.1.1 is

| Item | Value |
| --- | --- |
| Audio code | `src/native/APDspEngine.h`, `APDynamicSaturation.h`, `APMatchTables.h`: unchanged from the approved Dynamic Saturation build (`f1d58dcb…`, beta `v1.1.1-dyn-sat-beta1`) |
| Identity | Name "Angry Pig", version 1.1.1. VST3 class ID `F2AEE70D00DE4F4E41504C6241506D31` (`APLb`/`APm1`), bundle `AngryPigSpike.vst3`, macOS bundle IDs `com.AngryPigLab.vst3.AngryPigSpike` and `com.AngryPigLab.audiounit.AngryPigSpike`, AU `aufx APm1 APLb`. The same class ID, bundle name and VST3 bundle ID as Angry Pig 1.0.0. |
| Parameters | ANGER 0 to 100 %, COLOR 0 to 100 %, GAIN -36 to +36 dB, BYPASS. Same IDs, order and state format (four values in real units) as 1.0.0. |
| Windows DLL | SHA-256 `72AE70730FB8CFF776C47A7D59089B12C306B2DFEC849F69632A4EC2BBF038F5` |

## Sound

| Check | Result |
| --- | --- |
| 1.1.1 CI DLL vs the approved beta DLL: 20 renders (999 Hz sine at 44.1, 48 and 96 kHz with blocks 1, 64, 127 and 512; program material with GAIN -36, -6, 0, +18 and +36 dB; ANGER ramp; bypass; digital silence at GAIN +36 dB) | **bit-identical in all 20** (output files byte for byte) |
| Halo, 999 Hz at -12 dBFS, ANGER 100 %, 44.1 kHz (CI render vs stored reference) | -75.8165 dBc, delta 0.0000 |
| Low frequencies 20 to 200 Hz | -83.3760 dBc, delta 0.0000 |
| Odd harmonics H3 to H15 | max delta 0.00000 dB |
| DC | -112.5 dBc |
| Block size 1 vs 512 (CI) | bit-identical |
| Digital silence in | exactly 0 out |
| CPU | about 0.5 % of one core at 44.1 kHz and 1.1 % at 96 kHz (i9-9900KF); unchanged, the audio code did not change ([details](dyn-sat/PERFORMANCE.md)) |

## Compared with Angry Pig 1.0.0

Same settings, the installed 1.0.0 DLL (`E5CF23D3…`) and the 1.1.1 DLL, 10 settings (ANGER 0 to 100 %, COLOR 0 to 100 %, GAIN -24 to +18 dB), 999 Hz sine and program material:

| Check | Result |
| --- | --- |
| Output level | identical (difference 0.00 dB) |
| Odd harmonics, THD | identical (to 0.01 dB) |
| Difference signal (1.1.1 minus 1.0.0) | -74 to -76 dB relative to the output: the halo |
| Measurement tables (`docs/measurements`) | transfer curve, frequency response and GAIN law identical within the 1.0.0 range; H2 -92 to -95 dBc when saturating (1.0.0: below -150 dBc) |
| Saved state | same 36-byte format with values in real units; every 1.0.0 GAIN value (-24 to +18 dB) is inside the 1.1.1 range and loads unchanged (from the source of both versions) |
| GAIN automation (normalized) | shifts: 0 dB in 1.0.0 plays as +5.1 dB in 1.1.1. Measured level change on program material: +5.0 dB at ANGER 0 %, +0.3 dB at ANGER 50 %. Table in the [release notes](RELEASE_NOTES_1.1.1.md#projects-from-angry-pig-100). |

The measurement tool `tools/measurements/measure.py` reproduces all published 1.0.0 tables from the 1.0.0 DLL within 0.001 dB, so the 1.1.1 tables are comparable.

## Windows x64 VST3

| Test | Result |
| --- | --- |
| Clean-checkout build on `windows-2022` (MSVC, `/fp:strict`) | pass |
| Steinberg VST3 validator (built from the vendored SDK) | 47 passed, 0 failed; class ID `…41504C6241506D31` |
| Class ID of 1.1.1 = class ID of the installed 1.0.0 (validator, locally) | both `F2AEE70D00DE4F4E41504C6241506D31` |
| pluginval 1.0.4, strictness 8 | SUCCESS |
| DSP metrics of the CI render | PASS |
| Setup.exe in an isolated folder: install, refusal without consent, uninstall, reinstall, conflict, backup, rollback, manifest hashes | 24 of 24 checks |
| Upgrade from the published 1.0.0 bundle (`AngryPig-Windows-VST3-1.0.0.zip`, checksum checked): refused without consent, 1.0.0 untouched; replaced with consent; backup holds the 1.0.0 DLL; rollback restores 1.0.0; upgrade again | 10 of 10 checks |
| Host smoke test | The audio output was checked by the author in REAPER on Windows 11 with the approved beta build, whose output 1.1.1 reproduces bit for bit. **The 1.1.1 build itself has not been loaded in a DAW.** |

## macOS universal VST3 + AUv2

GitHub-hosted runners. Both run the same universal build.

| Test | Apple Silicon (`macos-14`, macOS 14.8) | Intel (`macos-15-intel`, macOS 15.7) |
| --- | --- | --- |
| Build with Xcode 15.4, `arm64` + `x86_64` | pass, `lipo` shows `x86_64 arm64` for both bundles | (same build) |
| Package checksums | OK | OK |
| Install the published 1.0.0 PKG (checksum checked), then the 1.1.1 PKG | "The upgrade was successful" | same |
| Installed VST3 version / package receipt `com.angrypiglab.angrypig` | 1.1.1 / 1.1.1 | 1.1.1 / 1.1.1 |
| Installed bundles equal the packaged bundles (no 1.0.0 file left) | yes (`diff -r` empty) | yes |
| `codesign --verify --deep --strict` on the installed bundles | valid | valid |
| `auval -v aufx APm1 APLb` | AU VALIDATION SUCCEEDED | AU VALIDATION SUCCEEDED |
| pluginval 1.0.4, strictness 8, VST3 | SUCCESS | SUCCESS |
| pluginval 1.0.4, strictness 8, AU | SUCCESS | SUCCESS |
| Halo / LF / odd H3 to H15 / DC, VST3 and AU, vs Windows | identical (delta 0.0000) | identical |
| 999 Hz sine samples vs Windows | max difference 5.96e-8 (one float LSB) | bit-identical |
| Program material samples vs Windows | max difference 5.96e-8 | max difference 7.1e-15 |
| Objective-C class names | own prefix `vAngryPig111`, AU view `AngryPig111_View`; none shared with 1.0.0 or the beta | (same binary) |
| PKG payload | only `AngryPigSpike.vst3` and `AngryPigSpike.component`; bundles not relocatable; preinstall removes old bundles | (same package) |
| Code signing | ad-hoc only. Not notarized. Gatekeeper rejects the package until the user allows it. | same |
| Physical Mac and a DAW | **not run.** Checklist: [MAC_MINI_M1_TEST_1.1.1.md](testing/MAC_MINI_M1_TEST_1.1.1.md) | not run |

## Not verified

- macOS on a physical Mac, and in any DAW.
- The 1.1.1 build loaded in a DAW on Windows (only the identical-output beta was).
- GAIN automation recall in real hosts. The shift is derived from the VST3 normalized-value rule and measured with the offline host; it was not observed in a DAW.
- Hosts other than REAPER, and automation or project recall across DAWs.
- A clean installation on a fresh Windows virtual machine.
- Code signing and notarization: not done.
