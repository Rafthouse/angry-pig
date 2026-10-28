# Angry Pig User Guide

Angry Pig is a saturation and distortion plugin. You put it on a track, drive the sound into a saturation stage, and shape the result with four controls. This guide explains what each control does, how to set levels, and how to get subtle or strong results.

Version: 1.0.0 (Windows and macOS, VST3; no Audio Unit).

---

## 1. What Angry Pig does

Angry Pig does three things to a stereo signal:

1. **Filters the low end.** A high-pass filter removes the lowest frequencies before the drive stage. The corner is near 30 Hz.
2. **Drives the signal.** The level going into the saturation stage is raised by ANGER and GAIN. The saturation stage turns peaks into a soft, rounded curve, which adds harmonics.
3. **Controls peaks.** A level detector applies gain reduction when the driven signal is loud. Above that, the saturation curve limits the output so that its peaks approach about -0.1 dBFS.

The result is more density, edge and character. At low settings the change is small. At high settings the sound becomes clearly distorted.

## 2. Installing

See [INSTALLATION.md](INSTALLATION.md) for the installer and the manual method. In short: close your DAW, run the installer, then rescan plugins in your DAW.

## 3. Loading Angry Pig in your DAW

Angry Pig is an audio effect. Put it on an **audio track** after the sound source.

- **Ableton Live:** open the browser, choose *Plug-Ins*, find **Angry Pig 1.0**, and drag it onto the track.
- **Other DAWs:** insert Angry Pig 1.0 in the track's effects or FX slots.

Use a **stereo** track. Angry Pig processes two channels. It does not accept mono input.

Angry Pig reports zero samples of latency, so it does not need latency compensation.

## 4. Setting input levels

The level you feed into Angry Pig decides how much it changes the sound.

- **Start with the source peaking around -12 to -6 dBFS** before Angry Pig. This leaves headroom for the drive.
- **Watch the CLIP light and the meter.** The meter shows the level entering the saturation stage. A lit CLIP light means the saturation stage is working hard.
- **Compensate with GAIN or the track fader.** At ANGER 0 %, Angry Pig adds about +1 dB. At ANGER 100 %, it adds about +40 dB of small-signal gain at GAIN 0 dB. Higher ANGER settings make the output louder, so lower GAIN or the fader to keep your mix level steady.

The output does not go much above -0.1 dBFS at its peaks, so the plugin also acts as a ceiling. Keep your master below 0 dBFS as usual.

## 5. The controls

### ANGER (0 to 100 %)

ANGER sets how hard the signal is driven into the saturation stage.

- **0 %:** almost transparent. The signal passes at about +1 dB.
- **25 %:** gentle saturation. Small-signal gain is about +11 dB, so reduce GAIN to keep the level.
- **50 % and above:** clear harmonics appear. Louder peaks are reduced by the gain-reduction stage.
- **100 %:** strong saturation. About 39 dB of drive is added on top of the base gain. Quiet parts become loud, and the waveform is squared off.

### COLOR (0 to 100 %)

COLOR shapes the tone before the drive stage.

- **0 %:** no tone shaping. The response is flat.
- **50 %:** a bump of about +4 dB around 1 kHz.
- **100 %:** a bump of about +8 dB around 8 kHz.

Because COLOR boosts a band before the drive, a boosted band can enter saturation sooner than the rest of the signal. This can add distortion that would not appear at COLOR 0 %.

### GAIN (-24 to +18 dB)

GAIN sets the level going into the saturation stage. It works together with ANGER.

- Use GAIN to set how much of the drive you hear.
- At low ANGER values, GAIN changes the output level one-for-one, plus about 1 dB.
- At high ANGER values, the saturation curve limits the output, so raising GAIN changes the level much less. Above about 0 dB, the output level barely rises at ANGER 100 %.

### BYPASS

BYPASS passes the input through unchanged. Use it to compare the processed and original sound. Bypass is not a mute: the track continues to play the original signal.

## 6. Subtle saturation

For a small amount of warmth or density:

1. Set **ANGER** to about 10 to 25 %.
2. Keep **COLOR** at 0 % or below 30 %.
3. Reduce **GAIN** so the output level matches the input. At ANGER 25 %, a GAIN setting of about -11 dB keeps the level similar.
4. Use BYPASS to compare. Adjust until the difference is pleasant, not obvious.

## 7. Stronger distortion

For a clear, gritty sound:

1. Raise **ANGER** to 60 to 100 %.
2. Use **GAIN** to avoid overloading the next plugin or the master. Lower it if the sound is too loud.
3. Try **COLOR** at 30 to 60 % to emphasize the upper mids and presence.
4. Check the CLIP light. Occasional lighting is normal for aggressive sounds.

At ANGER 100 %, the signal is strongly saturated and the output stays near -0.1 dBFS at its peaks. Odd harmonics dominate. Even harmonics are very small.

## 8. Practical examples

These are starting points, not presets. Adjust them to your material.

**Drums**

- *Snare:* ANGER 30 to 50 %, COLOR 30 to 50 %, GAIN as needed. Adds snap and presence.
- *Kick:* ANGER 15 to 30 %, COLOR 0 to 20 %. Keep it subtle to protect the low end.
- *Drum bus:* ANGER 10 to 25 %, COLOR 0 %. Glue and warmth without changing the character too much.

**Bass**

- *Synth or electric bass:* ANGER 15 to 35 %, COLOR 0 to 30 %. Adds harmonics that help the bass stand out on small speakers.
- *Heavy bass:* ANGER 60 to 80 %, GAIN lowered to keep the level steady.

**Synthesizers**

- *Pads:* ANGER 10 to 25 %, COLOR 20 to 40 %. Adds texture without making the pad harsh.
- *Leads:* ANGER 40 to 70 %, COLOR 30 to 60 %. Adds edge and bite.
- *Noise and percussive sounds:* ANGER 70 to 100 %. Use GAIN to control the level.

## 9. Saving sessions and settings

- Angry Pig's settings are saved with your DAW project. Reopening the project restores the controls.
- Angry Pig 1.0 does not include factory presets. Save your own settings using your DAW's preset or project features.
- Automation: the four parameters can be automated from your DAW. Automation and project recall have not been fully tested across DAWs yet, so check your results after reopening a project.

## 10. Troubleshooting

| Problem | What to try |
| --- | --- |
| Angry Pig does not appear in the DAW | Rescan plugins. Confirm that the VST3 bundle is in `C:\Program Files\Common Files\VST3\`. |
| The window is blank or does not open | Install the Microsoft Edge WebView2 Runtime, then open the plugin again. |
| The installer says a host is holding the file | Close all DAWs and plugin hosts, then run the installer again. |
| No sound after Angry Pig | Check that BYPASS is off, the track is stereo, and the track's input and output are set correctly. |
| The output is too loud | Lower GAIN or the track fader. At high ANGER values the level rises quickly. |
| The sound is harsh at the top | Lower COLOR, or lower ANGER. Very high drive can alias high frequencies. |
| The name looks old in the DAW | Some DAWs cache plugin names. Rescan plugins. |

## 11. Known limitations

- Windows and macOS (11.0 or later) VST3 only. There is no Audio Unit in 1.0.0. The macOS packages are not notarized; see [INSTALLATION.md](INSTALLATION.md#macos).
- Stereo only. Mono input is not supported.
- Saturation runs at the session sample rate without oversampling. At high drive, high-frequency harmonics can fold back (alias).
- The installer and the plugin are not code-signed yet. Windows may show a warning.
- Automation and project recall have not been verified across all DAWs.
- Plugin hosts other than Ableton Live 12 Suite have not been tested.

Report problems through the [issue tracker](https://github.com/Rafthouse/angry-pig/issues).
