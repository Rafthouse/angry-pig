# Installing Angry Pig 1.0.0

Angry Pig 1.0.0 is a VST3 plugin for 64-bit Windows 10 and 11, and for macOS 11.0 or later. This page covers Windows first, then macOS. The release has no Audio Unit for either platform.

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

## macOS

**Supported:** macOS 11.0 or later, on Apple Silicon (arm64) or Intel (x86_64). The VST3 bundle is universal. Audio Unit is **not** included in 1.0.0.

**Status:** the package is **not signed with a Developer ID and not notarized**. Gatekeeper rejects it. See [Gatekeeper](#gatekeeper) before you install.

| Option | File | Use it when |
| --- | --- | --- |
| Installer | `AngryPig-1.0.0-macOS.pkg` | You want the bundle installed to `/Library/Audio/Plug-Ins/VST3/`. |
| Disk image | `AngryPig-1.0.0-macOS.dmg` | You want to copy the bundle yourself. |

### Verify the download

```bash
shasum -a 256 AngryPig-1.0.0-macOS.pkg AngryPig-1.0.0-macOS.dmg
```

Compare the output with the values in [`installer/macos/SHA256SUMS-macOS.txt`](../installer/macos/SHA256SUMS-macOS.txt) and in the release notes.

### Install the package

1. Close all DAWs and plugin hosts.
2. Double-click `AngryPig-1.0.0-macOS.pkg` and follow the installer. It asks for an administrator password.
3. The bundle is installed to `/Library/Audio/Plug-Ins/VST3/AngryPigSpike.vst3`.
4. Rescan plugins in your host. Angry Pig appears as **Angry Pig 1.0**.

### Install from the disk image

1. Open `AngryPig-1.0.0-macOS.dmg`.
2. Copy `AngryPigSpike.vst3` to `/Library/Audio/Plug-Ins/VST3/` (for the current user only: `~/Library/Audio/Plug-Ins/VST3/`).
3. Do not drag it into Applications. The Applications shortcut in the disk image is not the plugin folder.
4. Rescan plugins in your host.

### Gatekeeper

The package and the bundle are ad-hoc signed or unsigned, and they are not notarized. macOS shows a warning and may block the first launch. Before you open the file:

- Check the SHA-256 checksum.
- Only continue if you trust this download.
- If macOS blocks it, open *System Settings → Privacy & Security* and choose *Open Anyway* for this file, if the option is offered.

Do not disable Gatekeeper or System Integrity Protection to install Angry Pig.

### Uninstall on macOS

Quit all DAWs and plugin hosts. Then delete `/Library/Audio/Plug-Ins/VST3/AngryPigSpike.vst3` with administrator rights.

### Known macOS notes

- The bundle includes a test script, `Contents/Resources/web/spike-test.js`. It does nothing unless a test host calls it. The Windows bundle includes it as well.
- The third-party notices file is **not** yet included in the macOS packages. The full notices are in the repository as [`docs/THIRD_PARTY_NOTICES.txt`](THIRD_PARTY_NOTICES.txt).
