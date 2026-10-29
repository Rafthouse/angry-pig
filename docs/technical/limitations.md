# Limitations of version 1.1.1

These limitations are confirmed by the implementation or by measurement.

- **Formats.** VST3 on Windows, VST3 and Audio Unit (AUv2) on macOS (11.0 or later). No AAX, no CLAP, no Audio Unit on Windows.
- **macOS is not notarized.** The macOS packages are ad-hoc signed, and Gatekeeper rejects them. See [INSTALLATION.md](../INSTALLATION.md#macos).
- **macOS has not been tested on a physical Mac or in a DAW.** All macOS tests ran on GitHub-hosted runners.
- **GAIN automation from 1.0.0 projects plays back at different values**, because the GAIN range changed. Saved values are not affected. See the [release notes](../RELEASE_NOTES_1.1.1.md#projects-from-angry-pig-100).
- **Stereo only.** Mono input is not processed.
- **No oversampling.** Saturation runs at the session sample rate. Heavy drive can produce aliasing, where harmonics above half the sample rate fold back into the audible range.
- **Output ceiling of about -0.1 dBFS.** The saturation stage limits output peaks. GAIN has little audible effect at high drive.
- **Even harmonics are low.** At 999 Hz with COLOR 0 %, the second harmonic is about -92 to -95 dBc when the stage saturates, and lower otherwise.
- **COLOR can add distortion** in the band it boosts. See [Controls](controls.md#color-0-to-100--default-0).
- **Automation and project recall** have not been fully verified across DAWs.
- **Host coverage.** 1.0.0 was checked in Ableton Live 12 Suite. The 1.1.1 sound was checked by the author in REAPER on Windows, with the beta build that has the same audio output. The 1.1.1 build itself has not been loaded in a DAW. Other hosts have not been tested.
- **Unsigned binaries.** The Windows installer is not code-signed, so Windows SmartScreen may warn. The macOS packages are not notarized.
- **Measurement scope.** Measurements cover 44.1 kHz and the conditions in [the measurement tables](../measurements/README.md). Other sample rates and program material were not measured.
