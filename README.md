<div align="center">

# ANGRY PIG

**Make your sound filthy. Keep your wallet clean.**

Free saturation and distortion plugin

![Platform: Windows x64 VST3](https://img.shields.io/badge/platform-Windows%20x64%20VST3-B3261E) ![macOS: VST3 available](https://img.shields.io/badge/macOS-VST3%20available-6E6E6E) ![Version 1.0.0](https://img.shields.io/badge/version-1.0.0-D4A93C)

[**Download Angry Pig 1.0.0 for Windows**](https://github.com/Rafthouse/angry-pig/releases/tag/v1.0.0)

![Angry Pig interface at high drive](docs/images/angry-pig-main.png)

</div>

---

## About

Angry Pig adds saturation, grit and tonal colour to a stereo signal. It has four controls: how hard the signal is driven, how the tone is shaped, how much level goes into the saturation stage, and a bypass. Use it on drums, bass, synthesizers, vocals or a mix bus when you want density and character without a complicated interface.

Angry Pig was designed and built by its author, Mykyta Shchur. It is free to download and use.

## Controls

| Control | Range | What it does |
| --- | --- | --- |
| **ANGER** | 0 to 100 % | Drives the signal into the saturation stage, adding up to 39 dB of drive at 100 %. A level detector applies gain reduction on loud peaks, so the output rises more slowly than the drive. |
| **COLOR** | 0 to 100 % | Shapes the tone before the drive stage with a peaking filter. Higher settings move a boost of up to about 8 dB from roughly 1 kHz to roughly 8 kHz. |
| **GAIN** | -24 to +18 dB | Sets the level going into the saturation stage. At high drive the saturation curve limits the output, so GAIN is less audible on a heavily saturated signal. |
| **BYPASS** | on / off | Passes the input through unchanged. |

Technical details, measured values and the signal path are in [docs/technical](docs/technical/README.md).

## Features

- Stereo VST3 plugin with four parameters that can be automated from the host.
- Low-frequency high-pass filtering on each channel before the drive stage, with a corner near 30 Hz.
- Saturation stage with a smooth transfer curve and an output that approaches about -0.1 dBFS at its peaks.
- Level meter with -12, -6, 0 and +6 dB indicators and a CLIP light for the signal entering the saturation stage.
- Parameter values are stored with the host project.

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

## Measurements

These plots come from measurements of the Windows 1.0.0 plugin. The full tables and conditions are in [docs/measurements](docs/measurements/README.md).

| Transfer curve | Harmonic distortion |
| --- | --- |
| ![Transfer curve](docs/images/measurements/transfer-curve.png) | ![Harmonic distortion](docs/images/measurements/harmonic-distortion.png) |
| Frequency response | GAIN law |
| ![Frequency response](docs/images/measurements/frequency-response.png) | ![GAIN law](docs/images/measurements/gain-law.png) |

Plugin Doctor graphs, captured from the same build:

| Harmonic analysis (THD -10.8 dB at -12 dBFS, ANGER 100 %) | Transfer curve (ANGER 100 %) |
| --- | --- |
| ![Plugin Doctor harmonic analysis](docs/images/plugindoctor/plugindoctor-harmonic-anger100-color0-gain0.png) | ![Plugin Doctor transfer curve at ANGER 100](docs/images/plugindoctor/plugindoctor-transfer-anger100.png) |
| Transfer curve (ANGER 0 %) | Linear view (ANGER 0 %, Plugin Doctor default excitation) |
| ![Plugin Doctor transfer curve at ANGER 0](docs/images/plugindoctor/plugindoctor-transfer-anger0.png) | ![Plugin Doctor linear view](docs/images/plugindoctor/plugindoctor-linear-anger0-color0.png) |

The Plugin Doctor linear view differs from the harness frequency response below about 100 Hz. This discrepancy is **unresolved**: it has not been shown to be a defect of either tool. The [measurement notes](docs/measurements/README.md#linear-response-discrepancy-investigation-unresolved) give the investigation.

## Downloads

| Platform | Format | Status |
| --- | --- | --- |
| Windows 10 and 11, 64-bit | VST3 (installer and bundle) | **Released** in [1.0.0](https://github.com/Rafthouse/angry-pig/releases/tag/v1.0.0) |
| macOS, Apple Silicon and Intel | VST3 (universal PKG and DMG) | **Available, validation pending.** Installation and pluginval passed on Apple Silicon and Intel runners. DAW and GUI smoke tests not yet run. Not notarized. |
| macOS | Audio Unit (AUv2) | **Not included in 1.0.0** |

Each release lists the SHA-256 checksum of every file. Check the checksum before you run an installer.

## Installation

Windows 10 or 11 (64-bit) with a host that supports VST3, and the Microsoft Edge WebView2 Runtime. Most current Windows systems already have the runtime.

1. Close your DAW and plugin hosts.
2. Run `AngryPig-Setup-1.0.0-UNSIGNED.exe` and follow the steps. The installer copies the VST3 bundle to `C:\Program Files\Common Files\VST3\` and can uninstall it later.
3. Rescan plugins in your host. Angry Pig appears as **Angry Pig 1.0**.

The installer is not code-signed yet. Windows SmartScreen may warn you. Verify the checksum first. Full instructions, including the bundle-only option and troubleshooting, are in [docs/INSTALLATION.md](docs/INSTALLATION.md).

### macOS

Apple Silicon or Intel. The package is a universal VST3 (arm64 and x86_64). The binary declares a minimum of macOS 11.0. Tested on macOS 14.8 (Apple Silicon) and macOS 15.7 (Intel).

1. Verify the checksum: `shasum -a 256 AngryPig-1.0.0-macOS.pkg`.
2. Run `AngryPig-1.0.0-macOS.pkg`. It installs `AngryPigSpike.vst3` to `/Library/Audio/Plug-Ins/VST3/` and asks for an administrator password.
3. Rescan plugins in your host. Angry Pig appears as **Angry Pig 1.0**.

The package is **not signed with a Developer ID and not notarized**. Gatekeeper rejects it. Install it only after you have verified the checksum and trust this download. If macOS blocks it, use *System Settings → Privacy & Security* and choose *Open Anyway* for this file. Do not disable Gatekeeper or System Integrity Protection. The disk image (`AngryPig-1.0.0-macOS.dmg`) contains the same bundle. Copy it to `/Library/Audio/Plug-Ins/VST3/`, not to Applications. The macOS guide is in [docs/INSTALLATION.md](docs/INSTALLATION.md#macos).

## Documentation

- [User guide](docs/USER_GUIDE.md): what each control does, levels, examples, troubleshooting
- [Technical documentation](docs/technical/README.md): signal path, saturation behaviour, gain staging, limitations
- [Measurements](docs/measurements/README.md): conditions, tables and data files
- [Release notes 1.0.0](docs/RELEASE_NOTES_1.0.0.md): changes, known limitations, checksums
- [Third-party notices](docs/THIRD_PARTY_NOTICES.md): components and licenses

## Compatibility

| Item | Angry Pig 1.0.0 |
| --- | --- |
| Operating system | Windows 10 and 11, 64-bit; macOS (universal arm64 and x86_64; the binary declares a minimum of macOS 11.0, tested on macOS 14.8 and 15.7) |
| Format | VST3 on Windows and macOS. No Audio Unit in 1.0.0. |
| Channels | Stereo only. Mono is not supported. |
| Sample rates | Accepted by the plugin from 8 kHz to 384 kHz. Measurements were made at 44.1 kHz. |
| Latency | Reported as 0 samples |
| Oversampling | None. Heavy drive can alias high frequencies. |
| Host testing | Ableton Live 12 Suite was used to check loading, the interface and the controls on Windows. No macOS host or DAW smoke test has been recorded. Testing in other hosts has not been completed. |

## Validation

**Windows 1.0.0**

- VST3 Validator: 47 of 47 tests passed on the Windows 1.0.0 bundle.
- The four parameter identifiers, default values and ranges were checked on the built plugin.
- Measurements summarised above and in [docs/measurements](docs/measurements/README.md).

**macOS 1.0.0** (published PKG and DMG; checked on isolated GitHub-hosted runners by the [macOS verification workflow](https://github.com/Rafthouse/angry-pig/actions/workflows/macos-release-verify.yml), run [40629905813](https://github.com/Rafthouse/angry-pig/actions/runs/40629905813))

- Checksums: the downloaded files match the published SHA-256 values.
- Architectures: the VST3 binary is universal, `x86_64 arm64`.
- Installation with `installer` (PKG): passed on Apple Silicon (macOS 14.8) and Intel (macOS 15.7).
- pluginval, strictness level 8, on the installed VST3: passed on both.
- auval: not run. The package contains no Audio Unit.
- Code signing: ad-hoc only. Notarization: not done. Gatekeeper (`spctl`): rejected.
- DAW or GUI smoke test on macOS: not yet run.

Not yet verified: automation and project recall across DAWs, macOS in a DAW, and a clean installation on a fresh Windows virtual machine.

## Support development

Angry Pig is free to use. If it is useful to you, optional donations help fund future releases. A donation link will be added here by the author.

## Credits and licensing

Created by **Mykyta Shchur**. © 2026 Mykyta Shchur. The interface, visual design and artwork are original works of the author.

Angry Pig uses third-party components, including the iPlug 2 framework and the VST 3 SDK. They are listed with their licenses in [docs/THIRD_PARTY_NOTICES.md](docs/THIRD_PARTY_NOTICES.md). The full license texts are in the Windows bundle and in the `THIRD_PARTY_NOTICES.txt` file attached to the release. The macOS 1.0.0 packages do not contain them.

**Source code is not published in this repository and is not licensed for reuse.** The author retains all rights to the source code and artwork. The compiled plugin is free to download and use. Redistribution terms beyond free binary distribution have not been published.

## Feedback

Bug reports and feature requests are welcome in the [issue tracker](https://github.com/Rafthouse/angry-pig/issues). Please use the templates and include your host, operating system and plugin version.
