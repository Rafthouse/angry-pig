# Angry Pig v1.0.0 — Free Saturation Plugin

Angry Pig is a free saturation and distortion plugin. Version 1.0.0 is the first public release, for Windows 10 and 11 (64-bit) as a VST3 plugin. It is free to download and use.

## Highlights

- Four controls: **ANGER** (drive), **COLOR** (tone), **GAIN** (level into the saturation stage) and **BYPASS**.
- Stereo VST3 plugin with four host-automatable parameters.
- Graphical installer with install, uninstall and backup of a replaced copy.
- Bundle-only download for manual installation.
- Third-party license notices are included in the bundle.

## Platforms

| Platform | Format | Status in 1.0.0 |
| --- | --- | --- |
| Windows 10 and 11, 64-bit | VST3 | Included |
| macOS, Apple Silicon | VST3, Audio Unit | Planned, not included |

## Installation

Follow [INSTALLATION.md](INSTALLATION.md). Close all hosts before installing or replacing the plugin. The Windows build needs the Microsoft Edge WebView2 Runtime.

## Known limitations

- **Windows only.** No macOS build is included in this release.
- **Stereo only.** Mono input is not supported.
- **Unsigned.** The installer and the plugin are not code-signed. Windows SmartScreen may show a warning. Check the checksums below before you run the installer.
- **Saturation headroom.** The saturation stage approaches an output ceiling of about -0.1 dBFS. At high ANGER settings, GAIN has less audible effect.
- **No oversampling.** At high drive, high-frequency harmonics can fold back (alias).
- **Testing.** Ableton Live 12 Suite was used to check loading, the interface and the controls. Automation and project recall across DAWs have not been fully verified, and other hosts have not been tested.
- **Internal file name.** The bundle folder is named `AngryPigSpike.vst3`. The plugin appears in hosts as Angry Pig 1.0.

## Checksums (SHA-256)

| File | SHA-256 |
| --- | --- |
| `AngryPig-Setup-1.0.0-UNSIGNED.exe` | `9432A1896FBBD988E82AB0B39B52A29D4C25C3A5FC7DE406AA5D6E69998E5507` |
| `AngryPig-Windows-VST3-1.0.0.zip` | `37BFF970CB31F0A3362269BBC30E8F527F2DEF3C774BF6A58481AF3EE0406054` |

The same values are in [SHA256SUMS.txt](SHA256SUMS.txt).

To check a file on Windows, run `Get-FileHash <file> -Algorithm SHA256` in PowerShell.

## Availability and license

Angry Pig 1.0.0 is free of charge. The plugin binaries may be downloaded and used free of charge. Source code is not included in this release and is not licensed for reuse. Other redistribution terms have not been published.

## Credits

Created by Mykyta Shchur. © 2026 Mykyta Shchur. Third-party components and their licenses are listed in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
