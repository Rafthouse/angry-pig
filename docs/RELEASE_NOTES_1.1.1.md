# Angry Pig 1.1.1

Angry Pig 1.1.1 replaces Angry Pig 1.0.0. It is free to download and use.

## What is new

- **Dynamic saturation.** The saturation stage now moves slightly with the signal. It adds a soft halo around the harmonics, about 75 dB below the signal, that follows the input level: quiet passages get less of it, loud ones more, silence none. At the same settings the level, the transfer curve and the odd harmonics are identical to 1.0.0. [How it works and what was measured](technical/dynamic-saturation.md).
- **GAIN range -36 to +36 dB** (1.0.0: -24 to +18 dB). GAIN 0 dB is now at the centre of the knob.
- **Audio Unit (AUv2) on macOS**, next to the VST3. Both are universal (Apple Silicon and Intel).
- **Source and build.** The source code, the build scripts and the CI that built this release are in the repository. See [BUILDING.md](../BUILDING.md).
- Same four controls (ANGER, COLOR, GAIN, BYPASS) and the same interface.

The Dynamic Saturation beta (`v1.1.1-dyn-sat-beta1`, "Angry Pig Dynamic Saturation (candidate)") was a separate test plugin with its own ID. 1.1.1 has the same audio output as that beta and supersedes it. You can remove the beta: `AngryPigDynamicSaturation.vst3` (and on macOS `AngryPigDynamicSaturation.component`). Projects that used the beta need the beta to stay installed.

## Downloads

| File | Platform | Contents |
| --- | --- | --- |
| `AngryPig-Setup-1.1.1-UNSIGNED.exe` | Windows 10 and 11, 64-bit | Installer for the VST3, with backup of 1.0.0 and uninstall |
| `AngryPig-Windows-VST3-1.1.1.zip` | Windows 10 and 11, 64-bit | VST3 bundle for manual installation |
| `SHA256SUMS.txt` | Windows | Checksums of the two Windows files |
| `AngryPig-1.1.1-macOS.pkg` | macOS 11 or later, universal | Installer for the VST3 and the Audio Unit |
| `AngryPig-1.1.1-macOS.zip` | macOS 11 or later, universal | VST3 and Audio Unit bundles for manual installation |
| `SHA256SUMS-macOS.txt` | macOS | Checksums of the two macOS files |
| `THIRD_PARTY_NOTICES.txt` | all | Third-party components and their licenses |

Check the checksum before you run an installer: `Get-FileHash <file> -Algorithm SHA256` on Windows, `shasum -a 256 -c SHA256SUMS-macOS.txt` on macOS.

## Install and upgrade

Close all DAWs first. Full instructions: [INSTALLATION.md](INSTALLATION.md).

- **Windows:** run the Setup .exe. It is not code-signed, so SmartScreen may warn you. Tick the unsigned-build checkbox and choose **Install**. If Angry Pig 1.0.0 is installed, Setup asks before replacing it and saves it to `C:\ProgramData\AngryPig\backup\` first. Requires the Microsoft Edge WebView2 Runtime.
- **macOS:** open the PKG. It is ad-hoc signed and **not notarized**, so macOS blocks it the first time: after checking the checksum, open *System Settings → Privacy & Security* and choose **Open Anyway**. It installs `AngryPigSpike.vst3` to `/Library/Audio/Plug-Ins/VST3/` (replacing 1.0.0) and `AngryPigSpike.component` to `/Library/Audio/Plug-Ins/Components/`. If you copied 1.0.0 to `~/Library/Audio/Plug-Ins/VST3/`, delete that copy.
- Restart the DAW and rescan plugins. The plugin is listed as **Angry Pig** (1.0.0 was listed as "Angry Pig 1.0").
- **Uninstall:** Windows: run the Setup .exe and choose **Uninstall**. macOS: delete the two bundles.
- **Going back to 1.0.0:** install 1.0.0 from its [release](https://github.com/Rafthouse/angry-pig/releases/tag/v1.0.0), or on Windows restore the backup. See [INSTALLATION.md](INSTALLATION.md).

## Projects from Angry Pig 1.0.0

1.1.1 has the same plugin ID (VST3 class ID `APLb`/`APm1`) and the same file name (`AngryPigSpike.vst3`) as 1.0.0, so your host opens 1.0.0 projects with 1.1.1.

- **Saved settings load unchanged.** ANGER, COLOR, GAIN and BYPASS are stored in real units and load with the same values.
- **The sound is the same apart from the new halo.** Measured on the same settings with the two Windows DLLs: output level identical (0.00 dB), harmonics and THD identical; the difference between the two outputs is about 75 dB below the signal.
- **GAIN automation plays back at different values.** Hosts record automation as a position from 0 to 1 of a control's range. The GAIN range is now 72 dB wide instead of 42 dB, so the same position means a different value:

| GAIN automation point recorded with 1.0.0 | Played back by 1.1.1 | Level change at ANGER 0 % (music test signal) | at ANGER 50 % |
| --- | --- | --- | --- |
| -24 dB | -36 dB | -12.0 dB | -12.0 dB |
| -12 dB | -15.4 dB | -3.4 dB | -2.8 dB |
| -6 dB | -5.1 dB | +0.9 dB | +0.2 dB |
| 0 dB | +5.1 dB | +5.0 dB | +0.3 dB |
| +6 dB | +15.4 dB | +3.2 dB | +0.3 dB |
| +12 dB | +25.7 dB | +1.4 dB | +0.3 dB |
| +18 dB | +36 dB | +0.9 dB | +0.4 dB |

At high ANGER the saturation limits the level change, but the signal is driven harder. MIDI-learn and controller mappings of GAIN change in the same way. Automation of ANGER, COLOR and BYPASS is not affected.

**What to do:** in projects that automate GAIN, check the GAIN automation and redraw it. A project without GAIN automation needs nothing.

## Verification

Summary. The full record is in [VERIFICATION_1.1.1.md](VERIFICATION_1.1.1.md).

- **Sound:** the audio code is the approved Dynamic Saturation code, unchanged. The output is bit-identical to the approved build at 44.1, 48 and 96 kHz and at block sizes from 1 to 512. Halo -75.82 dBc, low-frequency floor -83.38 dBc, odd harmonics H3 to H15 unchanged, digital silence stays silent.
- **CPU:** about 0.5 % of one core at 44.1 kHz and 1.1 % at 96 kHz (Intel i9-9900KF).
- **Windows:** Steinberg VST3 validator, pluginval at strictness 8, and an isolated install test, including the upgrade from the published 1.0.0 bundle with backup and rollback.
- **macOS (GitHub-hosted Apple Silicon macOS 14.8 and Intel macOS 15.7):** the published 1.0.0 PKG is installed and then upgraded with the 1.1.1 PKG; `auval`, pluginval at strictness 8 for VST3 and AU, and the render matches the Windows render.

## Known limitations

- **Not code-signed** on Windows. **Not notarized** on macOS (ad-hoc signature only).
- **macOS has not been tested on a physical Mac or in a DAW.** All macOS tests ran on GitHub-hosted runners. A checklist for a Mac mini M1 is prepared: [MAC_MINI_M1_TEST_1.1.1.md](testing/MAC_MINI_M1_TEST_1.1.1.md).
- **Host testing on Windows:** the 1.1.1 sound was checked by the author in REAPER, with the beta build that has the same audio output. The 1.1.1 build itself has not been loaded in a DAW yet. Other hosts are untested.
- **GAIN automation from 1.0.0 projects** plays back at different values (see above).
- Stereo only. No oversampling, so heavy drive can alias high frequencies. Latency 0 samples.

## Source and build

The source of the plugin and its build scripts are in this repository: [BUILDING.md](../BUILDING.md). No license for reuse of the source code has been granted; the author retains all rights. Third-party components and their licenses: [THIRD_PARTY_NOTICES.txt](../notices/THIRD_PARTY_NOTICES.txt).

## Build provenance

- Source commit: `67c9480379b222f128015e19c08f8669fa9872a4`
- CI build and test run: [40924734612](https://github.com/Rafthouse/angry-pig/actions/runs/40924734612). The files of this release are the artifacts of that run.
- Windows DLL SHA-256: `73BD774610016D9EFD33D0E221F25DB3527E36842599D90F8C21E4C55B1F8CA4`
