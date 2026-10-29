# Technical documentation: Angry Pig 1.1.1

This section describes how Angry Pig processes audio and what its controls do. It is written for producers who want to understand the sound, and for engineers who want the details.

Each page separates two kinds of statements:

- **Implementation:** behaviour defined by the plugin's processing code (signal path, time constants, ranges).
- **Measured:** values measured on the plugin with the measurement harness. The data are in [../measurements](../measurements/README.md).

## Pages

| Page | Contents |
| --- | --- |
| [Overview](overview.md) | Signal path, stereo processing, sample rates, latency, bypass |
| [Controls](controls.md) | ANGER, COLOR, GAIN and BYPASS: behaviour, ranges, measured effect |
| [Saturation and harmonics](saturation.md) | Transfer curve, harmonic structure, output ceiling, measured distortion |
| [Dynamic saturation](dynamic-saturation.md) | The halo around the harmonics, new in 1.1.1, compared with 1.0.0 |
| [Gain staging](gain-staging.md) | Level relationships, practical operating levels, the meter |
| [Limitations](limitations.md) | Confirmed limitations of version 1.1.1 |
