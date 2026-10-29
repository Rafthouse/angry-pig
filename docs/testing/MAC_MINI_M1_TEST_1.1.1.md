# Physical test on a Mac mini M1 (Angry Pig 1.1.1)

**Status: not run yet.** CI runs on GitHub-hosted runners only. This checklist is for a real Mac mini M1 and a real DAW. Record the result in the table at the end. Until then, release notes must not call the macOS build tested on physical hardware.

## Before you start

- macOS version and Mac model: Apple menu → About This Mac.
- Downloaded files from the 1.1.1 release: `AngryPig-1.1.1-macOS.pkg` (or the ZIP) and `SHA256SUMS-macOS.txt`.
- Check the download: `shasum -a 256 -c SHA256SUMS-macOS.txt` in the download folder. The lines for the files you downloaded must say `OK`.
- Best test of the upgrade: install Angry Pig 1.0.0 first (`AngryPig-1.0.0-macOS.pkg` from the 1.0.0 release), make a short project with it in your DAW, with GAIN at +6 dB and one GAIN automation point at 0 dB, and save it.

## Install

1. Quit all DAWs.
2. Open the PKG. It is ad-hoc signed and not notarized, so Gatekeeper blocks it the first time. Open *System Settings → Privacy & Security* and choose **Open Anyway** for this file (on macOS 14 and earlier, right-click → **Open** also works). Do not disable Gatekeeper.
3. Check the files:
   ```sh
   ls -d /Library/Audio/Plug-Ins/VST3/AngryPigSpike.vst3 /Library/Audio/Plug-Ins/Components/AngryPigSpike.component
   /usr/libexec/PlistBuddy -c "Print :CFBundleShortVersionString" /Library/Audio/Plug-Ins/VST3/AngryPigSpike.vst3/Contents/Info.plist
   lipo -archs /Library/Audio/Plug-Ins/Components/AngryPigSpike.component/Contents/MacOS/AngryPigSpike
   codesign --verify --deep --strict /Library/Audio/Plug-Ins/VST3/AngryPigSpike.vst3 && echo signature OK
   ls ~/Library/Audio/Plug-Ins/VST3/ 2>/dev/null
   ```
   The version must be `1.1.1`, the architectures `x86_64 arm64`, and the signature OK. There must be no `AngryPigSpike.vst3` in `~/Library/Audio/Plug-Ins/VST3/`.

## Audio Unit

```sh
auval -v aufx APm1 APLb
```
The last lines must say `AU VALIDATION SUCCEEDED`.

## DAW smoke test (Logic Pro, REAPER, Ableton Live or another host)

| # | Step | Expected |
| --- | --- | --- |
| 1 | Rescan plug-ins, add "Angry Pig" to an audio track (AU in Logic, VST3 or AU elsewhere). | Loads without error. |
| 2 | Open the editor. | Pig artwork, ANGER, COLOR, GAIN, CLIP meter. |
| 3 | Drag GAIN to both ends. | Readout shows -36.0 dB and +36.0 dB. GAIN 0 dB sits at the centre. |
| 4 | Drag ANGER and COLOR. | Readout follows; sound changes. |
| 5 | Play a 999 Hz sine at -12 dBFS, ANGER 100 %, COLOR 0 %, GAIN 0 dB. | Saturated tone, no clicks, no dropouts. |
| 6 | Stop playback with silence on the track. | Meter shows no output (digital silence). |
| 7 | Automate GAIN and ANGER, play back. | Smooth changes, automation follows. |
| 8 | Save the project, quit, reopen. | Same parameter values. |
| 9 | Bypass on and off. | Dry signal when on. |
| 10 | Activity Monitor: CPU of the host with one instance at 48 kHz. | Note the value. |
| 11 | Open the project saved with 1.0.0 (see "Before you start"). | Opens with 1.1.1. GAIN shows +6.0 dB. The GAIN automation point recorded at 0 dB now plays at about +5.1 dB (expected, see the release notes). |

## Uninstall and reinstall

```sh
sudo rm -rf /Library/Audio/Plug-Ins/VST3/AngryPigSpike.vst3 /Library/Audio/Plug-Ins/Components/AngryPigSpike.component
```
Rescan: the plug-in is gone. Install the PKG again and repeat step 1.

## Result

| Item | Result | Notes |
| --- | --- | --- |
| Mac model / macOS |  |  |
| Checksums |  |  |
| Install checks (version, architectures, signature) |  |  |
| auval |  |  |
| DAW and version |  |  |
| Steps 1-11 |  |  |
| Uninstall / reinstall |  |  |
