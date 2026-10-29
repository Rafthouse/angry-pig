# Changelog

## Angry Pig 1.1.1 (`v1.1.1`)

Replaces Angry Pig 1.0.0: same VST3 class ID (`APm1`) and bundle name (`AngryPigSpike.vst3`). Supersedes the Dynamic Saturation beta and has the same audio output.

- Added dynamic saturation: a level-dependent halo around the harmonics. Level, transfer curve and odd harmonics are unchanged from 1.0.0.
- GAIN range is now -36 to +36 dB (1.0.0: -24 to +18 dB). Saved values load unchanged; GAIN automation recorded with 1.0.0 plays back at different values.
- Added the macOS Audio Unit (AUv2). macOS builds are universal.
- Plugin name is now "Angry Pig" (1.0.0: "Angry Pig 1.0").
- Windows setup backs up an existing 1.0.0 before it replaces it. macOS PKG upgrades the 1.0.0 PKG.
- Measurement tool `tools/measurements/measure.py`; measurement tables regenerated for 1.1.1. The THD column of the 1.0.0 tables is relabelled H2-H15 (the values were H2-H15).
- Notes: [docs/RELEASE_NOTES_1.1.1.md](docs/RELEASE_NOTES_1.1.1.md)

## Angry Pig Dynamic Saturation 1.1.1 beta 1 (pre-release, `v1.1.1-dyn-sat-beta1`)

A separate beta plug-in next to Angry Pig 1.0.0 (VST3 class ID `APd2`). It does not replace 1.0.0.

- Added dynamic saturation (stages A and B).
- GAIN range is now -36 to +36 dB.
- Added the macOS Audio Unit (AUv2). macOS builds are universal.
- Windows Setup.exe and portable ZIP. macOS PKG and ZIP.
- Published the source and clean-checkout build for Windows and macOS, with CI.
- Notes: [docs/RELEASE_NOTES_1.1.1-dyn-sat-beta1.md](docs/RELEASE_NOTES_1.1.1-dyn-sat-beta1.md)

## Angry Pig 1.0.0 (`v1.0.0`)

First public release. VST3 for Windows 10 and 11 (64-bit) and macOS (universal).

- Notes: [docs/RELEASE_NOTES_1.0.0.md](docs/RELEASE_NOTES_1.0.0.md)
