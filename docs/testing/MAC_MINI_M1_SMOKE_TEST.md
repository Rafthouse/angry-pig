# macOS smoke test: Mac mini (Apple Silicon M1)

This is the manual test for the macOS VST3 and Audio Unit (AUv2) builds. It is written to be run by hand on a physical Mac mini with an M1 chip.

**Status: NOT RUN.** A row may be marked PASS only after the step was actually performed on the test machine. Record the outcome, the evidence and the time in the results table at the end.

## Scope

- Formats: VST3 and Audio Unit (AUv2), both installed from the PKG.
- Hosts: REAPER (VST3 and AU). Any other AU-compatible host (for example Logic Pro) is optional.
- Not covered: Intel Macs, AUv3, other DAWs, and Windows.

## Preconditions

1. Record the machine and OS: `sw_vers` and `sysctl -n machdep.cpu.brand_string`.
2. Use the build under test. Verify it before installing:

   ```bash
   shasum -a 256 AngryPig-<version>-macOS.pkg
   ```

   Compare the output with `SHA256SUMS-macOS.txt` from the same release.
3. Install the latest stable REAPER for macOS. Close other plug-in hosts.
4. Set up an audio device and check the sample rate (44.1 kHz and 48 kHz are required; 96 kHz is in section 8).
5. Prepare test material: a stereo file of about 30 seconds (music or pink noise), and a 1 kHz sine at -18 dBFS for level checks.

## 1. Installation

| Step | Action | Expected |
| --- | --- | --- |
| 1.1 | Download the PKG with Safari (so that the quarantine flag is set). Run `xattr -l AngryPig-<version>-macOS.pkg`. | `com.apple.quarantine` is present. Record the output. |
| 1.2 | Double-click the PKG. | Record the exact Gatekeeper message and whether **Open Anyway** is offered. |
| 1.3 | Complete the installer. Enter the administrator password when asked. | "The installation was successful." |
| 1.4 | `ls -la /Library/Audio/Plug-Ins/VST3/ /Library/Audio/Plug-Ins/Components/` | `AngryPigSpike.vst3` and `AngryPigSpike.component` exist. |
| 1.5 | `auval -v aufx APm1 APLb` | `AU VALIDATION SUCCEEDED`. |
| 1.6 | `spctl -a -vvv -t install AngryPig-<version>-macOS.pkg` | Expected: rejected (no Developer ID). Record the output. |

## 2. VST3 in REAPER

| Step | Action | Expected |
| --- | --- | --- |
| 2.1 | REAPER: Preferences, Plug-ins, VST: rescan. | "Angry Pig 1.0" is listed. |
| 2.2 | Insert it on a stereo track. Open the editor. | The GUI shows the four controls with their labels. The WebView is not blank. Take a screenshot. |
| 2.3 | Close and reopen the editor three times. | No crash. The controls show the same values each time. |
| 2.4 | Move ANGER, COLOR and GAIN with the mouse, and set BYPASS. | Each control responds, and the value text matches the position. |

## 3. Audio Unit in REAPER

| Step | Action | Expected |
| --- | --- | --- |
| 3.1 | REAPER: Preferences, Plug-ins, AU: rescan. | "Angry Pig 1.0" appears under the AU list (manufacturer AngryPigLab, type Effect). |
| 3.2 | Insert the AU on a second stereo track. Repeat steps 2.2 to 2.4. | Same results as the VST3. |

## 4. Parameters, both formats

For each of ANGER (0 to 100 %), COLOR (0 to 100 %), GAIN (-24 to +18 dB) and BYPASS (on/off):

| Step | Action | Expected |
| --- | --- | --- |
| 4.1 | Change the value in the GUI. Read it in REAPER's parameter list or the generic editor. | The host value follows the GUI. |
| 4.2 | Change the value in the generic editor. | The GUI follows the host. |
| 4.3 | Set ANGER 0 %, COLOR 0 %, GAIN 0 dB. Play the 1 kHz sine at -18 dBFS. | Output is about +1 dB above the input (record the measured difference). |
| 4.4 | Set the extreme values (min and max) of each parameter. | No crash, no silence, no clipping of the meter beyond the ceiling. |

## 5. Real audio

| Step | Action | Expected |
| --- | --- | --- |
| 5.1 | Play the 30-second file through the plugin at default settings, then at ANGER 100 %. | No clicks, dropouts or crackle. Audible change matches the settings. |
| 5.2 | Bounce the same 30 seconds once with VST3 and once with AU, same settings. | Record the peak level and the RMS of the difference between the two bounces (a null test). |
| 5.3 | Watch REAPER's CPU meter during playback. | No sustained overload. |

## 6. Project save and recall

| Step | Action | Expected |
| --- | --- | --- |
| 6.1 | Set both instances to a non-default state: ANGER 63 %, COLOR 34 %, GAIN -6 dB, BYPASS off. Save the project. | Project saved. |
| 6.2 | Quit REAPER. Reopen the project. | Both instances show exactly the same values. |
| 6.3 | Repeat with BYPASS on. Save, quit and reopen. | BYPASS state is restored. |
| 6.4 | Load the project on a second computer or a fresh user account, if one is available. | Values are restored. Record this as NOT RUN if no second machine is available. |

## 7. Automation

| Step | Action | Expected |
| --- | --- | --- |
| 7.1 | Draw an ANGER automation ramp from 0 to 100 % over four bars. Play. | The sound changes with the ramp. The parameter lane moves with playback. |
| 7.2 | Draw a GAIN ramp from -24 to +18 dB. Play. | Level follows the ramp without stepping. |
| 7.3 | Draw BYPASS automation that toggles on and off. Play. | The toggle is heard and shown in the lane. |
| 7.4 | Repeat 7.1 to 7.3 with the AU. | Same behaviour. |
| 7.5 | Bounce one automated section per format and compare. | Record the difference. Note that AU parameter changes are applied per audio block in this build, not sample-accurately. |

## 8. Stability

| Step | Action | Expected |
| --- | --- | --- |
| 8.1 | Change the project sample rate from 44.1 kHz to 96 kHz with the plugins loaded. | No crash. Output remains sane. |
| 8.2 | Toggle the track bypass 50 times in two minutes. | No crash, no stuck output. |
| 8.3 | Close REAPER while a plugin editor is open. | REAPER quits cleanly. |
| 8.4 | Open the project again. | Plugins load without errors in REAPER's log. |

## Results

Fill in one row per step. Use PASS, FAIL or NOT RUN only.

| Step | Result | Evidence (file or screenshot) | Notes |
| --- | --- | --- | --- |
| 1.1 to 1.6 | | | |
| 2.1 to 2.4 (VST3 GUI and WebView) | | | |
| 3.1 to 3.2 (AU GUI) | | | |
| 4.1 to 4.4 (parameters) | | | |
| 5.1 to 5.3 (audio) | | | |
| 6.1 to 6.4 (project recall) | | | |
| 7.1 to 7.5 (automation) | | | |
| 8.1 to 8.4 (stability) | | | |

Record with each run: macOS version, Mac mini model, REAPER version, package version, the SHA-256 of the PKG, and the tester's name or initials.
