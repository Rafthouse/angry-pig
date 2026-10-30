# Installing Angry Pig 1.1.1

Angry Pig 1.1.1 is a VST3 plugin for 64-bit Windows 10 and 11, and a VST3 and Audio Unit (AUv2) plugin for macOS 11.0 or later. This page covers Windows first, then macOS.

**Upgrading from 1.0.0?** 1.1.1 replaces 1.0.0. It uses the same plugin file and the same plugin ID, so your projects open with 1.1.1. Read [Projects from Angry Pig 1.0.0](#projects-from-angry-pig-100) first: GAIN automation recorded with 1.0.0 plays back at different values.

| Option | File | Use it when |
| --- | --- | --- |
| Windows installer (recommended) | `AngryPig-Setup-1.1.1-UNSIGNED.exe` | You want the plugin copied to the standard VST3 folder, with a backup of 1.0.0 and an uninstaller. |
| Windows bundle | `AngryPig-Windows-VST3-1.1.1.zip` | You want to place the VST3 bundle yourself. |
| macOS installer (recommended) | `AngryPig-1.1.1-macOS.pkg` | You want the VST3 and the Audio Unit installed to `/Library/Audio/Plug-Ins/`. |
| macOS bundles | `AngryPig-1.1.1-macOS.zip` | You want to copy the bundles yourself. |

## Before you start

1. **Close every DAW and plugin host.** A host that has Angry Pig loaded keeps the plugin file locked. The Windows installer then refuses to replace it and changes nothing.
2. **Windows: check for the Microsoft Edge WebView2 Runtime.** Most current Windows systems have it. The installer checks for it but does not install it.
3. **Verify the download.** Compare the file's SHA-256 checksum with `SHA256SUMS.txt` (Windows) or `SHA256SUMS-macOS.txt` (macOS) on the release page. In PowerShell: `Get-FileHash <file> -Algorithm SHA256`. On macOS: `shasum -a 256 -c SHA256SUMS-macOS.txt`.

## Windows

### Option 1: installer

1. Run `AngryPig-Setup-1.1.1-UNSIGNED.exe`.
2. Windows may show a SmartScreen warning because the installer is not code-signed yet. If the checksum matches, choose **More info**, then **Run anyway**.
3. Tick the box to confirm that you understand the build is unsigned, then choose **Install**. Windows asks for administrator permission, because the VST3 folder is under `Program Files`.
4. If Angry Pig 1.0.0 (or any other copy of `AngryPigSpike.vst3`) is installed, the installer asks before replacing it. The old copy is saved to `C:\ProgramData\AngryPig\backup\<date-time>\` first.
5. Rescan plugins in your host. Angry Pig appears as **Angry Pig**.

To remove Angry Pig, run the installer again and choose **Uninstall**. Backups stay in `C:\ProgramData\AngryPig\backup\`.

**Going back to 1.0.0.** Run the 1.0.0 installer from the [1.0.0 release](https://github.com/Rafthouse/angry-pig/releases/tag/v1.0.0). Or close all hosts, delete `C:\Program Files\Common Files\VST3\AngryPigSpike.vst3`, and copy the `AngryPigSpike.vst3` folder from the backup (`C:\ProgramData\AngryPig\backup\<date-time>\`) back to `C:\Program Files\Common Files\VST3\`.

### Option 2: bundle

1. Extract `AngryPig-Windows-VST3-1.1.1.zip`. It contains the folder `AngryPigSpike.vst3`, a readme and `THIRD_PARTY_NOTICES.txt`.
2. If Angry Pig 1.0.0 is installed and you want to keep a copy, move `C:\Program Files\Common Files\VST3\AngryPigSpike.vst3` somewhere outside the VST3 folder first.
3. Copy the `AngryPigSpike.vst3` folder to `C:\Program Files\Common Files\VST3\`. This needs administrator permission.
4. Rescan plugins in your host.

The file name `AngryPigSpike.vst3` is internal and the same as in 1.0.0. Hosts show the plugin as **Angry Pig**.

### Where things are

| Item | Location |
| --- | --- |
| Installed bundle | `C:\Program Files\Common Files\VST3\AngryPigSpike.vst3` |
| Installer log | `C:\ProgramData\AngryPig\install.log` |
| Backups (for example of 1.0.0) | `C:\ProgramData\AngryPig\backup\` |

### Troubleshooting

- **The plugin does not appear in my host.** Rescan plugins. Confirm that the bundle is in the VST3 folder above.
- **The installer says the file is in use.** Close all hosts, then run the installer again. Nothing was changed.
- **The window is blank or does not open.** Install the Microsoft Edge WebView2 Runtime.
- **The host still shows "Angry Pig 1.0".** Hosts cache plugin names. Rescan plugins; in some hosts, clear the plugin cache.

### Removing manually

Close all hosts, then delete the `AngryPigSpike.vst3` folder from `C:\Program Files\Common Files\VST3\`.

## macOS

**Supported:** macOS 11.0 or later, on Apple Silicon (arm64) or Intel (x86_64). Both bundles are universal.

**Status:** the package is ad-hoc signed, **not signed with a Developer ID and not notarized**. Gatekeeper rejects it. See [Gatekeeper](#gatekeeper) before you install. It has been tested on GitHub-hosted Macs (macOS 14.8 on Apple Silicon, macOS 15.7 on Intel), not yet on a physical Mac or in a DAW.

### Install the package

1. Quit all DAWs and plugin hosts.
2. Open `AngryPig-1.1.1-macOS.pkg` and follow the installer. It asks for an administrator password. If macOS blocks the file, see [Gatekeeper](#gatekeeper).
3. It installs:
   - `/Library/Audio/Plug-Ins/VST3/AngryPigSpike.vst3` (VST3). If Angry Pig 1.0.0 is installed there, it is replaced.
   - `/Library/Audio/Plug-Ins/Components/AngryPigSpike.component` (Audio Unit, new in 1.1.1).
4. If you copied 1.0.0 from its disk image to your user folder (`~/Library/Audio/Plug-Ins/VST3/AngryPigSpike.vst3`), delete that copy. Both copies have the same plugin ID, and a host may load the old one.
5. Restart the DAW and rescan plugins. Logic Pro rescans Audio Units when it starts.

### Install from the ZIP

1. Open `AngryPig-1.1.1-macOS.zip`.
2. Copy `AngryPigSpike.vst3` to `/Library/Audio/Plug-Ins/VST3/` and `AngryPigSpike.component` to `/Library/Audio/Plug-Ins/Components/`. Replace 1.0.0 if asked.
3. If macOS says a bundle is damaged or cannot be checked, remove the quarantine flag: `xattr -dr com.apple.quarantine <bundle>`.
4. Restart the DAW and rescan plugins.

### Gatekeeper

The package and the bundles are ad-hoc signed and not notarized. macOS shows a warning and blocks the first open. Before you open the file:

- Check the SHA-256 checksum.
- Only continue if you trust this download.
- Open *System Settings → Privacy & Security* and choose *Open Anyway* for this file. On macOS 14 and earlier, right-click the file and choose *Open* also works.

Do not disable Gatekeeper or System Integrity Protection to install Angry Pig.

### Uninstall on macOS

Quit all DAWs and plugin hosts. Then delete the two bundles with administrator rights:

```bash
sudo rm -rf /Library/Audio/Plug-Ins/VST3/AngryPigSpike.vst3 /Library/Audio/Plug-Ins/Components/AngryPigSpike.component
```

**Going back to 1.0.0:** uninstall, then install the 1.0.0 PKG from the [1.0.0 release](https://github.com/Rafthouse/angry-pig/releases/tag/v1.0.0).

## Projects from Angry Pig 1.0.0

1.1.1 has the same plugin ID as 1.0.0, so hosts open old projects with 1.1.1.

- **Saved settings load unchanged.** ANGER, COLOR, GAIN and BYPASS keep their values.
- **The sound is the same apart from the new halo.** At the same settings, level, transfer curve and odd harmonics are identical. The halo is about 75 dB below the signal.
- **GAIN automation changes.** Hosts store automation as a position from 0 to 1 of the control's range, and the GAIN range changed from -24…+18 dB to -36…+36 dB. Recorded GAIN automation therefore plays back at different values:

| Automation point recorded with 1.0.0 | Played back by 1.1.1 |
| --- | --- |
| -24 dB | -36 dB |
| -12 dB | -15.4 dB |
| -6 dB | -5.1 dB |
| 0 dB | +5.1 dB |
| +6 dB | +15.4 dB |
| +12 dB | +25.7 dB |
| +18 dB | +36 dB |

  At ANGER 0 % a point at 0 dB plays back 5 dB louder. At high ANGER the saturation limits the change in level, but the sound is driven harder. MIDI-learn and controller mappings of GAIN change the same way. ANGER, COLOR and BYPASS automation is not affected.

**What to do:** open each project that automates GAIN and redraw the GAIN automation, or keep 1.0.0 for that project.
