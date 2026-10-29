# Angry Pig Dynamic Saturation: experimental candidate

**Status: experimental and not released.** This folder documents an offline-measured candidate for a
spectral "halo" around the even harmonics. It is not part of Angry Pig 1.0.0 or any release, it is not
validated for listening, and no binary is included here.

## What is in this folder

| Path | Contents |
| --- | --- |
| `src/native/` | Candidate source, the Dynamic Saturation module and the engine it is attached to. |
| `deliverable/SHA256.txt` | SHA-256 of the test DLL that was built from this source. The DLL is **not** in this repository. |
| `deliverable/Start-PluginDoctor-Candidate.ps1` | Local launcher for Plugin Doctor. It expects the DLL next to it, so it works only with a local build. |
| `report/` | `REPORT.md`, metrics (JSON), figures and the source diff against the Dynamic EXP. |
| `analysis/`, `halo-engine/`, `run_candidate.ps1`, `*.json` | Measurement and calibration scripts and their results. |

## Identity and controls

- Name: **Angry Pig Dynamic Saturation (candidate)**, version `1.1.1-dsc`.
- `PLUG_UNIQUE_ID 'APd2'` gives a VST3 class ID that differs from production (`APm1`).
- `AP_DYNAMIC_AMOUNT` (default `1`). `0` disables the experimental path; the output is then the RC1.1 path, bit for bit.
- `AP_DYNAMIC_COMPONENTS` (default `AB`). `A` is a slow additive bias, `B` is the envelope-scaled 32 Hz modulation, `C` is a broadband background that is **off** by default because it raised the floor above the reference.
- Test-only variables, read once at instance creation: `AP_DYNAMIC_C_LOWPATH`, `AP_DYNAMIC_HALO_SCALE`, `AP_DYNAMIC_SEED`. They are not VST3 parameters and are not saved in state.

## Committed source versus the tested DLL

The DLL with SHA-256 `38593B36A72019FBBC2F179F5EDD4F2E744094D362E027ECFF1E98D4ED4AC28B` was built **before**
the test-only `AP_DYNAMIC_C_LOWPATH` switch was added to `APDynamicSaturation.h`. The source here therefore
contains that switch, and the DLL does not. With default settings the output is identical: a harness dump of the
default configuration matched the dump of the tested build bit for bit. A build from this source is expected to
match the tested DLL's behaviour, but its binary is not the same file and has not been compared.

## Measurements (sine, 999 Hz, -12 dBFS, offline harness)

Summary of `report/REPORT.md`; see the report for conditions and limits.

- Halo H2..H14 (1-128 Hz): candidate -75.8 dBc, reference -76.0 dBc. Seed-to-seed spread about 0.5 dB (-75.8, -76.1, -76.3 dBc).
- Halo shape: energy within 32 Hz and the density slope match the reference within 0.1 dB and 0.1 dB/oct.
- Odd harmonics H3..H15 are identical to the RC1.1 path to 0.001 dB. THD is unchanged within 0.003 dB, which is why THD alone does not show the halo.
- 8-22 kHz broadband floor is about 3 dB above the reference (-152.5 versus -155.7 dBc; the known remaining deviation).
- Silence gives exact zero output; block sizes 1, 64, 127 and 128 give bit-identical output; no onset after reset.

Not done: music or programme-material listening tests, decay tests, and a completed Harmonic comparison in
Plugin Doctor against the reference plugin. Plugin Doctor loads the candidate under the identity `bc2af9b1`, distinct
from production's `bc2b02b0`.

## External dependencies (not included)

- **Build:** Visual Studio 2022, CMake 3.24 or later, the iPlug2 tree pinned under `src/deps` (with its VST3 SDK and WebView2 loader), and the `src/qualification` and `src/dsp` trees from the same snapshot. None of these are included here. The CMake project looks for them relative to `src/native`.
- **Analysis:** Python 3 with numpy, scipy and matplotlib. The scripts use absolute paths on the author's machine (for example `D:\AP-RE\tmp\ref-spec`, `D:\AP-RE\tmp\halo-engine` and the Dynamic EXP evidence folder). The harness `ap-host-pcm.exe` and the saved dump files are not included.
- **Reference plugin:** the comparisons use a third-party reference plugin (SausageFattener). Its binary and dumps are not included, and only numeric summaries of its measurements appear here.
- **Measurement-derived data:** `src/native/APMatchTables.h` contains coefficients derived from black-box measurements of that reference plugin, and its header names the reference file's SHA-256. This is data, not a binary, and it is included as part of the existing engine. Remove it before any redistribution if the reference owner's terms require it.

## Safety

Do not copy any build into `C:\Program Files\Common Files\VST3`. The production 1.0.0 plugin and the 1.0.1 candidate are not changed by this folder.
