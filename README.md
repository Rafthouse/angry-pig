<div align="center">

# ANGRY PIG

**Make your sound filthy. Keep your wallet clean.**

Free saturation and distortion plugin

![Windows: VST3](https://img.shields.io/badge/Windows-VST3-B3261E) ![macOS: VST3 + AU](https://img.shields.io/badge/macOS-VST3%20%2B%20AU-6E6E6E) ![Version 1.1.1](https://img.shields.io/badge/version-1.1.1-D4A93C)

[**Download Angry Pig 1.1.1**](https://github.com/Rafthouse/angry-pig/releases/tag/v1.1.1)

![Angry Pig interface at high drive](docs/images/angry-pig-main.png)

</div>

---

## About

Angry Pig adds saturation, grit and tonal colour to a stereo signal. It has four controls: how hard the signal is driven, how the tone is shaped, how much level goes into the saturation stage, and a bypass. Use it on drums, bass, synthesizers, vocals or a mix bus when you want density and character without a complicated interface.

Angry Pig was designed and built by its author, Mykyta Shchur. It is free to download and use.

## What is new in 1.1.1

- **Dynamic saturation.** The saturation stage now moves slightly with the signal. It adds a soft halo around the harmonics, about 75 dB below the signal, that follows the input level. Level, transfer curve and odd harmonics are the same as in 1.0.0. [How it works](docs/technical/dynamic-saturation.md).
- **GAIN range -36 to +36 dB** (1.0.0: -24 to +18 dB).
- **Audio Unit on macOS.** The macOS package now contains a universal VST3 and a universal Audio Unit (AUv2).
- **Replaces 1.0.0.** Same plugin ID and file, so your projects open with 1.1.1. GAIN automation recorded with 1.0.0 plays back at different values: [details](docs/RELEASE_NOTES_1.1.1.md#projects-from-angry-pig-100).

All changes: [CHANGELOG.md](CHANGELOG.md).

## Controls

| Control | Range | What it does |
| --- | --- | --- |
| **ANGER** | 0 to 100 % | Drives the signal into the saturation stage, adding up to 39 dB of drive at 100 %. A level detector applies gain reduction on loud peaks, so the output rises more slowly than the drive. |
| **COLOR** | 0 to 100 % | Shapes the tone before the drive stage with a peaking filter. Higher settings move a boost of up to about 8 dB from roughly 1 kHz to roughly 8 kHz. |
| **GAIN** | -36 to +36 dB | Sets the level going into the saturation stage. At high drive the saturation curve limits the output, so GAIN is less audible on a heavily saturated signal. |
| **BYPASS** | on / off | Passes the input through unchanged. |

Technical details, measured values and the signal path are in [docs/technical](docs/technical/README.md).

## Features

- Stereo plugin with four parameters that can be automated from the host. VST3 on Windows; VST3 and Audio Unit on macOS.
- Low-frequency high-pass filtering on each channel before the drive stage, with a corner near 30 Hz.
- Saturation stage with a smooth transfer curve and an output that approaches about -0.1 dBFS at its peaks.
- Dynamic saturation: a level-dependent halo around the harmonics. Digital silence stays silent.
- Level meter with -12, -6, 0 and +6 dB indicators and a CLIP light for the signal entering the saturation stage.
- Parameter values are stored with the host project. Zero latency.

## Use cases

- **Drums:** more presence and snap on snares, kicks and percussion. Start with ANGER low and raise it until the transients feel fuller.
- **Bass:** warmth and audible harmonics that help a bass line cut through on small speakers. Keep COLOR moderate.
- **Synthesizers:** add grit to pads, leads and basses, or run a chord through it for a dirtier tone.
- **Mix bus and vocals:** subtle colour at low ANGER values, and character when pushed.

## Screenshots

| Default | Moderate drive | High drive |
| --- | --- | --- |
| ![Default settings](docs/images/gui-default.png) | ![ANGER 39 %, COLOR 34 %](docs/images/gui-moderate.png) | ![ANGER 100 %](docs/images/gui-high.png) |
| ANGER 0 %, COLOR 0 %, GAIN 0 dB | ANGER 39.3 %, COLOR 34.0 %, GAIN 0 dB | ANGER 100 %, COLOR 34.0 %, GAIN 0 dB |

Captured from 1.0.0. The 1.1.1 interface is the same, except that GAIN 0 dB now sits at the centre of the GAIN knob, because the range is symmetric.

## Measurements

These plots come from measurements of the Windows 1.1.1 plugin. The full tables, the conditions and the command to repeat them are in [docs/measurements](docs/measurements/README.md).

| Transfer curve | Harmonic distortion |
| --- | --- |
| ![Transfer curve](docs/images/measurements/transfer-curve.png) | ![Harmonic distortion](docs/images/measurements/harmonic-distortion.png) |
| Frequency response | GAIN law |
| ![Frequency response](docs/images/measurements/frequency-response.png) | ![GAIN law](docs/images/measurements/gain-law.png) |

The new halo, around the second harmonic, compared with 1.0.0:

![Spectrum around the 2nd harmonic, 1.0.0 and 1.1.1](docs/images/measurements/halo-h2.png)

Plugin Doctor graphs, captured from **Angry Pig 1.0.0**. 1.1.1 has the same level, transfer curve and odd harmonics; the graphs do not show the halo.

| Harmonic analysis (THD -10.8 dB at -12 dBFS, ANGER 100 %) | Transfer curve (ANGER 100 %) |
| --- | --- |
| ![Plugin Doctor harmonic analysis](docs/images/plugindoctor/plugindoctor-harmonic-anger100-color0-gain0.png) | ![Plugin Doctor transfer curve at ANGER 100](docs/images/plugindoctor/plugindoctor-transfer-anger100.png) |
| Transfer curve (ANGER 0 %) | Linear view (ANGER 0 %, Plugin Doctor default excitation) |
| ![Plugin Doctor transfer curve at ANGER 0](docs/images/plugindoctor/plugindoctor-transfer-anger0.png) | ![Plugin Doctor linear view](docs/images/plugindoctor/plugindoctor-linear-anger0-color0.png) |

The Plugin Doctor linear view differs from the harness frequency response below about 100 Hz. This discrepancy is **unresolved**: it has not been shown to be a defect of either tool. The [measurement notes](docs/measurements/README.md#linear-response-discrepancy-investigation-unresolved) give the investigation.

## Downloads

| Platform | Format | File | Status |
| --- | --- | --- | --- |
| Windows 10 and 11, 64-bit | VST3 | `AngryPig-Setup-1.1.1-UNSIGNED.exe` (installer), `AngryPig-Windows-VST3-1.1.1.zip` (bundle) | **Released** in [1.1.1](https://github.com/Rafthouse/angry-pig/releases/tag/v1.1.1). Not code-signed. |
| macOS 11 or later, Apple Silicon and Intel | VST3 and Audio Unit (universal) | `AngryPig-1.1.1-macOS.pkg` (installer), `AngryPig-1.1.1-macOS.zip` (bundles) | **Released, validated on GitHub-hosted Macs only.** Installation, auval and pluginval passed on Apple Silicon and Intel runners. Not yet tested on a physical Mac or in a DAW. Not notarized. |

Each release lists the SHA-256 checksum of every file. Check the checksum before you run an installer.

Earlier versions: [1.0.0](https://github.com/Rafthouse/angry-pig/releases/tag/v1.0.0) (VST3, GAIN -24 to +18 dB) and the [Dynamic Saturation beta](https://github.com/Rafthouse/angry-pig/releases/tag/v1.1.1-dyn-sat-beta1), a separate test plugin that 1.1.1 supersedes.

## Installation

### Windows

Windows 10 or 11 (64-bit) with a host that supports VST3, and the Microsoft Edge WebView2 Runtime. Most current Windows systems already have the runtime.

1. Close your DAW and plugin hosts.
2. Run `AngryPig-Setup-1.1.1-UNSIGNED.exe` and follow the steps. The installer copies the VST3 bundle to `C:\Program Files\Common Files\VST3\` and can uninstall it later. If 1.0.0 is installed, it asks before replacing it and keeps a backup.
3. Rescan plugins in your host. Angry Pig appears as **Angry Pig**.

The installer is not code-signed yet. Windows SmartScreen may warn you. Verify the checksum first. Full instructions, including the bundle-only option, going back to 1.0.0 and troubleshooting, are in [docs/INSTALLATION.md](docs/INSTALLATION.md).

### macOS

Apple Silicon or Intel, macOS 11.0 or later. The package contains a universal VST3 and a universal Audio Unit.

1. Verify the checksum: `shasum -a 256 -c SHA256SUMS-macOS.txt`.
2. Open `AngryPig-1.1.1-macOS.pkg`. It installs `AngryPigSpike.vst3` to `/Library/Audio/Plug-Ins/VST3/` (replacing 1.0.0) and `AngryPigSpike.component` to `/Library/Audio/Plug-Ins/Components/`, and asks for an administrator password.
3. Rescan plugins in your host. Angry Pig appears as **Angry Pig**; in Logic Pro under *Audio Units → AngryPigLab*.

The package is **not signed with a Developer ID and not notarized**. Gatekeeper rejects it. Install it only after you have verified the checksum and trust this download. If macOS blocks it, use *System Settings → Privacy & Security* and choose *Open Anyway* for this file. Do not disable Gatekeeper or System Integrity Protection. The macOS guide is in [docs/INSTALLATION.md](docs/INSTALLATION.md#macos).

## Documentation

- [User guide](docs/USER_GUIDE.md): what each control does, levels, examples, troubleshooting
- [Technical documentation](docs/technical/README.md): signal path, saturation behaviour, dynamic saturation, gain staging, limitations
- [Measurements](docs/measurements/README.md): conditions, tables, data files and how to repeat them
- [Release notes 1.1.1](docs/RELEASE_NOTES_1.1.1.md): changes, upgrading from 1.0.0, known limitations
- [Verification 1.1.1](docs/VERIFICATION_1.1.1.md): what was tested, how, and what was not
- [Building from source](BUILDING.md)
- [Release notes 1.0.0](docs/RELEASE_NOTES_1.0.0.md)
- [Third-party notices](docs/THIRD_PARTY_NOTICES.md): components and licenses

## Compatibility

| Item | Angry Pig 1.1.1 |
| --- | --- |
| Operating system | Windows 10 and 11, 64-bit; macOS 11.0 or later, universal arm64 and x86_64 (tested on GitHub-hosted macOS 14.8 and 15.7) |
| Format | VST3 on Windows. VST3 and Audio Unit (AUv2) on macOS. |
| Channels | Stereo only. Mono is not supported. |
| Sample rates | Accepted by the plugin from 8 kHz to 384 kHz. Tested at 44.1, 48 and 96 kHz. Measurements were made at 44.1 kHz. |
| Latency | Reported as 0 samples |
| Oversampling | None. Heavy drive can alias high frequencies. |
| CPU | About 0.5 % of one core at 44.1 kHz and 1.1 % at 96 kHz (Intel i9-9900KF, one instance) |
| Projects from 1.0.0 | Open with 1.1.1, values unchanged. GAIN automation plays back at different values. |
| Host testing | Windows: 1.0.0 in Ableton Live 12 Suite; the 1.1.1 sound in REAPER (with the beta build, same audio output). The 1.1.1 build itself and macOS have not been tested in a DAW yet. |

## Validation

Full record: [docs/VERIFICATION_1.1.1.md](docs/VERIFICATION_1.1.1.md). Summary:

- **Sound:** bit-identical to the approved Dynamic Saturation build at 44.1, 48 and 96 kHz and at block sizes from 1 to 512. Compared with 1.0.0 at the same settings: same level, transfer curve and odd harmonics; the halo is the only difference.
- **Windows:** clean-checkout build in CI, Steinberg VST3 validator, pluginval at strictness 8, and an isolated install test that includes the upgrade from the published 1.0.0 bundle (backup, replace, rollback).
- **macOS (GitHub-hosted Apple Silicon and Intel runners):** the published 1.0.0 PKG is installed and then upgraded with the 1.1.1 PKG; auval, pluginval at strictness 8 for VST3 and AU, and render comparison with Windows.

Not yet verified: a physical Mac and a DAW on macOS, the 1.1.1 build in a DAW on Windows, automation and project recall across DAWs, and a clean installation on a fresh Windows virtual machine.

## Support development

Angry Pig is free to use. If it is useful to you, optional donations help fund future releases. A donation link will be added here by the author.

## Credits and licensing

Created by **Mykyta Shchur**. © 2026 Mykyta Shchur. The interface, visual design and artwork are original works of the author.

Angry Pig uses third-party components, including the iPlug 2 framework and the VST 3 SDK. They are listed with their licenses in [docs/THIRD_PARTY_NOTICES.md](docs/THIRD_PARTY_NOTICES.md). The full license texts are in `THIRD_PARTY_NOTICES.txt`, which is in the Windows ZIP, inside the macOS bundles and attached to the release.

**The source code is published in this repository (`src/`) so that the builds can be checked and reproduced. It is not licensed for reuse:** no open-source license has been granted, and the author retains all rights to the source code and artwork. The compiled plugin is free to download and use. Redistribution terms beyond free binary distribution have not been published.

## Feedback

Bug reports and feature requests are welcome in the [issue tracker](https://github.com/Rafthouse/angry-pig/issues). Please use the templates and include your host, operating system and plugin version.
