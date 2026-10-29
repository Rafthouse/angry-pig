# Physical test on a Mac mini M1 (Dynamic Saturation 1.1.1 beta)

**Status: not run yet.** CI runs on GitHub-hosted runners only. This checklist is for a real Mac mini M1 and a real DAW. Record the result in the table at the end. Until then, release notes must not call the macOS build tested on physical hardware.

## Before you start

- macOS version and Mac model: Apple menu → About This Mac.
- Downloaded files: `AngryPig-DynamicSaturation-1.1.1-Beta-macOS.pkg` (or the ZIP) and `SHA256SUMS-macOS.txt` from the pre-release.
- Check the download: `shasum -a 256 -c SHA256SUMS-macOS.txt` in the download folder. Every line must say `OK`.

## Install

1. Quit all DAWs.
2. Open the PKG. It is ad-hoc signed and not notarized, so Gatekeeper blocks it the first time. Open *System Settings → Privacy & Security* and choose **Open Anyway** for this file (on macOS 14 and earlier, right-click → **Open** also works). Do not disable Gatekeeper.
3. Check the files:
   ```sh
   ls -d /Library/Audio/Plug-Ins/VST3/AngryPigDynamicSaturation.vst3 /Library/Audio/Plug-Ins/Components/AngryPigDynamicSaturation.component
   lipo -archs /Library/Audio/Plug-Ins/Components/AngryPigDynamicSaturation.component/Contents/MacOS/AngryPigSpike
   ```
   The second command must print `x86_64 arm64`.
4. If Angry Pig 1.0.0 was installed, `AngryPigSpike.vst3` must still be there and unchanged.

## Audio Unit

```sh
auval -v aufx APd2 APLb
```
The last lines must say `AU VALIDATION SUCCEEDED`.

## DAW smoke test (Logic Pro, REAPER, Ableton Live or another host)

| # | Step | Expected |
| --- | --- | --- |
| 1 | Rescan plug-ins, add "Angry Pig Dynamic Saturation (candidate)" to an audio track (AU in Logic, VST3 or AU elsewhere). | Loads without error. |
| 2 | Open the editor. | Pig artwork, ANGER, COLOR, GAIN, CLIP meter. |
| 3 | Drag GAIN to both ends. | Readout shows -36.0 dB and +36.0 dB. |
| 4 | Drag ANGER and COLOR. | Readout follows; sound changes. |
| 5 | Play a 999 Hz sine at -12 dBFS, ANGER 100 %, COLOR 0 %, GAIN 0 dB. | Saturated tone, no clicks, no dropouts. |
| 6 | Stop playback with silence on the track. | Meter shows no output (digital silence). |
| 7 | Automate GAIN and ANGER, play back. | Smooth changes, automation follows. |
| 8 | Save the project, quit, reopen. | Same parameter values. |
| 9 | Bypass on and off. | Dry signal when on. |
| 10 | Activity Monitor: CPU of the host with one instance at 48 kHz. | Note the value. |
| 11 | Both versions in one project (if 1.0.0 is installed). | Both load, both work, no crash. |

## Uninstall and reinstall

```sh
sudo rm -rf /Library/Audio/Plug-Ins/VST3/AngryPigDynamicSaturation.vst3 /Library/Audio/Plug-Ins/Components/AngryPigDynamicSaturation.component
```
Rescan: the plug-in is gone, Angry Pig 1.0.0 is not affected. Install the PKG again and repeat step 1.

## Result

| Item | Result | Notes |
| --- | --- | --- |
| Mac model / macOS |  |  |
| Checksums |  |  |
| auval |  |  |
| DAW and version |  |  |
| Steps 1-11 |  |  |
| Uninstall / reinstall |  |  |
