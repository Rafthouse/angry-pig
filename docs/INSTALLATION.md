# Installing Angry Pig 1.0.0 (Windows)

Angry Pig 1.0.0 is a VST3 plugin for 64-bit Windows 10 and 11. The release has two ways to install it.

| Option | File | Use it when |
| --- | --- | --- |
| Installer (recommended) | `AngryPig-Setup-1.0.0-UNSIGNED.exe` | You want the plugin copied to the standard VST3 folder, with an uninstaller. |
| Bundle | `AngryPig-Windows-VST3-1.0.0.zip` | You want to place the VST3 bundle yourself. |

## Before you start

1. **Close every DAW and plugin host.** A host that has Angry Pig loaded keeps the plugin file locked. The installer then refuses to replace it and changes nothing.
2. **Check for the Microsoft Edge WebView2 Runtime.** Most current Windows systems have it. The installer checks for it but does not install it.
3. **Verify the download.** Compare the file's SHA-256 checksum with the value in the release notes. In PowerShell, run `Get-FileHash <file> -Algorithm SHA256`.

## Option 1: installer

1. Run `AngryPig-Setup-1.0.0-UNSIGNED.exe`.
2. Windows may show a SmartScreen warning because the installer is not code-signed yet. If the checksum matches, choose **More info**, then **Run anyway**.
3. Tick the box to confirm that you understand the build is unsigned, then choose **Install**. Windows asks for administrator permission, because the VST3 folder is under `Program Files`.
4. If another copy of Angry Pig's bundle is already installed, the installer asks before replacing it. The old copy is saved to `C:\ProgramData\AngryPig\backup\` first.
5. Rescan plugins in your host. Angry Pig appears as **Angry Pig 1.0**.

To remove Angry Pig, run the installer again and choose **Uninstall**. A backup copy stays in `C:\ProgramData\AngryPig\backup\`.

## Option 2: bundle

1. Extract `AngryPig-Windows-VST3-1.0.0.zip`. It contains one folder, `AngryPigSpike.vst3`.
2. Copy that folder to `C:\Program Files\Common Files\VST3\`. This needs administrator permission.
3. Rescan plugins in your host.

The bundle includes `THIRD_PARTY_NOTICES.txt` with the license texts of the components Angry Pig uses. The file name `AngryPigSpike.vst3` is internal. Hosts show the plugin as **Angry Pig 1.0**.

## Where things are

| Item | Location |
| --- | --- |
| Installed bundle | `C:\Program Files\Common Files\VST3\AngryPigSpike.vst3` |
| Third-party notices | `C:\Program Files\Common Files\VST3\AngryPigSpike.vst3\THIRD_PARTY_NOTICES.txt` |
| Installer log | `C:\ProgramData\AngryPig\install.log` |
| Backups | `C:\ProgramData\AngryPig\backup\` |

## Troubleshooting

- **The plugin does not appear in my host.** Rescan plugins. Confirm that the bundle is in the VST3 folder above.
- **The installer says the file is in use.** Close all hosts, then run the installer again. Nothing was changed.
- **The window is blank or does not open.** Install the Microsoft Edge WebView2 Runtime.
- **The name looks old in the host.** Some hosts cache plugin names until the next rescan.

## Removing manually

Close all hosts, then delete the `AngryPigSpike.vst3` folder from `C:\Program Files\Common Files\VST3\`.

## macOS 1.0.0 (VST3, universal)

The macOS build is a universal VST3 (arm64 for Apple Silicon, x86_64 for Intel), validated on a
GitHub macOS runner with `auval` (when the AU target is enabled) and `pluginval --strictness-level 8`.

| Option | File | Use it when |
| --- | --- | --- |
| Installer (recommended) | `AngryPig-1.0.0-macOS.pkg` | You want the plug-in copied to `/Library/Audio/Plug-Ins/VST3`. |
| Disk image | `AngryPig-1.0.0-macOS.dmg` | You want to drag the bundle into place yourself. |

The macOS builds are ad-hoc signed, not notarized. On first launch macOS may require
System Settings -> Privacy & Security -> "Open Anyway".

Checksums: `installer/macos/SHA256SUMS-macOS.txt`.

