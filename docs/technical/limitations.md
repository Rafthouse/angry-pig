# Limitations of version 1.0.0

These limitations are confirmed by the implementation or by measurement.

- **Windows only** in this release. macOS builds are planned.
- **Stereo only.** Mono input is not processed.
- **No oversampling.** Saturation runs at the session sample rate. Heavy drive can produce aliasing, where harmonics above half the sample rate fold back into the audible range.
- **Output ceiling of about -0.1 dBFS.** The saturation stage limits output peaks. GAIN has little audible effect at high drive.
- **Even harmonics are very low.** At 999 Hz with COLOR 0 %, even harmonics were below the measurement floor.
- **COLOR can add distortion** in the band it boosts. See [Controls](controls.md#color-0-to-100--default-0).
- **Automation and project recall** have not been fully verified across DAWs.
- **Host coverage.** Ableton Live 12 Suite was used to check loading, the interface and the controls. Other hosts have not been tested.
- **Unsigned installer and binaries.** Windows may show a warning.
- **Measurement scope.** Measurements cover 44.1 kHz and the conditions in [the measurement tables](../measurements/README.md). Other sample rates and program material were not measured.
