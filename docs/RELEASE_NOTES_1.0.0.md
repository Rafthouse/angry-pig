# Angry Pig v1.0.0 — Free Saturation Plugin

Angry Pig is a free saturation and distortion plugin. Version 1.0.0 is the first public release. It is a VST3 plugin for Windows 10 and 11 (64-bit) and for macOS 11.0 or later on Apple Silicon and Intel. It is free to download and use.

## Highlights

- Four controls: **ANGER** (drive), **COLOR** (tone), **GAIN** (level into the saturation stage) and **BYPASS**.
- Stereo VST3 plugin with four host-automatable parameters.
- Windows: graphical installer with install, uninstall and backup of a replaced copy, plus a bundle-only download for manual installation.
- macOS: installer package (`.pkg`) and disk image (`.dmg`) with a universal VST3 bundle (arm64 and x86_64).

## Platforms

| Platform | Format | Status in 1.0.0 |
| --- | --- | --- |
| Windows 10 and 11, 64-bit | VST3 | Released |
| macOS, Apple Silicon and Intel | VST3 (universal) | Available, validation pending |
| macOS | Audio Unit | Not included in 1.0.0 |

macOS is marked *validation pending* because the DAW and GUI smoke tests have not been run. Installation and pluginval passed on both architectures. See [Validation](#validation).

## Installation

Follow [INSTALLATION.md](https://github.com/Rafthouse/angry-pig/blob/main/docs/INSTALLATION.md). Close all hosts before you install or replace the plugin.

- **Windows:** run `AngryPig-Setup-1.0.0-UNSIGNED.exe`, or copy the bundle to `C:\Program Files\Common Files\VST3\`. The Windows build needs the Microsoft Edge WebView2 Runtime.
- **macOS:** verify the checksum, then run `AngryPig-1.0.0-macOS.pkg`. It installs `AngryPigSpike.vst3` to `/Library/Audio/Plug-Ins/VST3/`. The package is not notarized, so Gatekeeper warns or blocks the first launch. See [Gatekeeper](https://github.com/Rafthouse/angry-pig/blob/main/docs/INSTALLATION.md#gatekeeper). Do not disable Gatekeeper or System Integrity Protection.

Tested macOS versions: 14.8 on Apple Silicon and 15.7 on Intel. The binary declares a minimum of macOS 11.0.

## Validation

### Windows 1.0.0

- VST3 Validator: 47 of 47 tests passed on the Windows bundle.
- The four parameter identifiers, default values and ranges were checked on the built plugin.

### macOS 1.0.0

The [macOS verification workflow](https://github.com/Rafthouse/angry-pig/actions/workflows/macos-release-verify.yml) (run [40629905813](https://github.com/Rafthouse/angry-pig/actions/runs/40629905813)) downloaded the published PKG and DMG on two isolated GitHub-hosted runners and checked them:

| Check | Apple Silicon (macOS 14.8) | Intel (macOS 15.7) |
| --- | --- | --- |
| Downloaded files match the published SHA-256 values | PASS | PASS |
| Architectures of the VST3 binary (`lipo`) | x86_64 arm64 | x86_64 arm64 |
| Installation with `installer` (PKG) | PASS | PASS |
| pluginval, strictness level 8, on the installed VST3 | PASS | PASS |
| auval | NOT RUN (no Audio Unit in the package) | NOT RUN (no Audio Unit in the package) |
| Code signature | Ad-hoc only | Ad-hoc only |
| Notarization | Not notarized (no stapled ticket) | Not notarized (no stapled ticket) |
| Gatekeeper (`spctl`) | Rejected | Rejected |
| DAW or GUI smoke test | NOT RUN | NOT RUN |

The universal VST3 was built on a GitHub-hosted macOS runner (macOS 14, Xcode 15.4) in the author's private build workflow (run 40625541370). The published PKG and DMG are byte-identical to that build's output.

Not verified on macOS:

- A DAW or GUI smoke test.
- Gatekeeper acceptance of a quarantined download opened by double-click on a physical Mac. The check above uses `spctl`, which rejects the files.
- Developer ID signing and notarization.
- Audio Unit. There is none in 1.0.0.

## Known limitations

- **macOS is not notarized.** The macOS packages are ad-hoc signed or unsigned, and Gatekeeper rejects them. Check the checksum before you open them.
- **VST3 only.** There is no Audio Unit in 1.0.0 on either platform.
- **Unsigned Windows installer.** SmartScreen may show a warning. Check the checksum before you run the installer.
- **macOS packages do not include `THIRD_PARTY_NOTICES.txt`.** The Windows bundle includes it. The full notices are in [docs/THIRD_PARTY_NOTICES.txt](https://github.com/Rafthouse/angry-pig/blob/main/docs/THIRD_PARTY_NOTICES.txt).
- **Test script in the bundles.** Both bundles contain `Contents/Resources/web/spike-test.js`. It does nothing unless a test host calls it.
- **Stereo only.** Mono input is not supported.
- **Saturation headroom.** The saturation stage approaches an output ceiling of about -0.1 dBFS. At high ANGER settings, GAIN has less audible effect.
- **No oversampling.** At high drive, high-frequency harmonics can fold back (alias).
- **Testing.** Ableton Live 12 Suite was used on Windows to check loading, the interface and the controls. Automation and project recall across DAWs have not been fully verified, and other hosts have not been tested.
- **Measurement discrepancy.** Plugin Doctor and the measurement harness disagree on the low-frequency linear response near 35 Hz. The cause is unresolved and has not been shown to be a defect of either tool. See the [measurement notes](https://github.com/Rafthouse/angry-pig/blob/main/docs/measurements/README.md#linear-response-discrepancy-investigation-unresolved).
- **Internal file name.** The bundle folder is named `AngryPigSpike.vst3`. The plugin appears in hosts as Angry Pig 1.0.

## Checksums (SHA-256)

| File | SHA-256 |
| --- | --- |
| `AngryPig-Setup-1.0.0-UNSIGNED.exe` | `9432A1896FBBD988E82AB0B39B52A29D4C25C3A5FC7DE406AA5D6E69998E5507` |
| `AngryPig-Windows-VST3-1.0.0.zip` | `37BFF970CB31F0A3362269BBC30E8F527F2DEF3C774BF6A58481AF3EE0406054` |
| `AngryPig-1.0.0-macOS.pkg` | `4793A2432D8FBCB5FC025CEDD09F22588056AC869F17B66481F37F660B94D32F` |
| `AngryPig-1.0.0-macOS.dmg` | `7D4B3BA21E4A0A1CC17B94DF141127059325790062611B78214BA5EF2FE1565B` |

The same values are in [SHA256SUMS.txt](https://github.com/Rafthouse/angry-pig/blob/main/docs/SHA256SUMS.txt). The macOS checksum file `SHA256SUMS-macOS.txt` is attached to this release.

To check a file on Windows, run `Get-FileHash <file> -Algorithm SHA256` in PowerShell. On macOS, run `shasum -a 256 <file>` in Terminal.

## Availability and license

Angry Pig 1.0.0 is free of charge. The plugin binaries may be downloaded and used free of charge. Source code is not included in this release and is not licensed for reuse. Other redistribution terms have not been published.

## Credits

Created by Mykyta Shchur. © 2026 Mykyta Shchur. Third-party components and their licenses are listed in [THIRD_PARTY_NOTICES.md](https://github.com/Rafthouse/angry-pig/blob/main/docs/THIRD_PARTY_NOTICES.md).
