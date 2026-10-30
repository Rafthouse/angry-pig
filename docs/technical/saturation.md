# Saturation and harmonics

## Transfer curve

**Implementation.** The saturation stage is a memoryless curve applied to each sample:

| Input magnitude | Behaviour |
| --- | --- |
| up to 0.85 (-1.4 dBFS) | Linear: output equals input |
| 0.85 to 1.6 | Smooth curve, approaching the ceiling |
| above 1.6 (+4.1 dBFS) | Constant at the ceiling, 0.98855 (-0.10 dBFS) |

The sign of each sample is preserved. The curve is read from a table of 768 segments, so it is smooth between points.

**Measured.** 999 Hz sine, GAIN 0 dB, COLOR 0 %. Output fundamental in dBFS:

| Input (dBFS) | ANGER 0 % | ANGER 25 % | ANGER 50 % | ANGER 75 % | ANGER 100 % |
| --- | --- | --- | --- | --- | --- |
| -60 | -59.00 | -49.25 | -39.50 | -29.75 | -20.00 |
| -36 | -35.00 | -25.25 | -15.50 | -5.75 | 1.36 |
| -24 | -23.00 | -13.25 | -3.50 | 1.50 | 1.70 |
| -12 | -11.00 | -1.25 | 1.37 | 1.63 | 1.77 |
| -6 | -5.00 | 1.06 | 1.46 | 1.68 | 1.80 |
| 0 | 0.53 | 1.20 | 1.53 | 1.72 | 1.83 |

The output fundamental can exceed 0 dBFS. A waveform that is squared off has a fundamental larger than its peak. The output peak stays at -0.10 dBFS when saturation is engaged (measured).

## Harmonic structure

Measured at 999 Hz, GAIN 0 dB, COLOR 0 %:

| ANGER | Input (dBFS) | H2 (dBc) | H3 (dBc) | H5 (dBc) | THD H2 to H15 (dB) |
| --- | --- | --- | --- | --- | --- |
| 50 % | -12 | -92.33 | -15.37 | -50.92 | -15.25 |
| 50 % | -6 | -92.58 | -14.43 | -38.68 | -14.30 |
| 100 % | -24 | -93.98 | -12.09 | -22.91 | -11.70 |
| 100 % | -12 | -94.77 | -11.42 | -20.23 | -10.82 |
| 100 % | -6 | -95.19 | -11.16 | -19.27 | -10.45 |

What this shows:

- The saturation is nearly symmetric. The second harmonic is about -92 to -95 dBc when the stage saturates. It comes from the slow bias of the [dynamic saturation](dynamic-saturation.md). In 1.0.0 even harmonics were below -150 dBc; the odd harmonics and THD are identical in both versions.
- Odd harmonics dominate. The third harmonic is the strongest.
- Total harmonic distortion depends on ANGER and level. At full drive it is about -10 to -12 dB relative to the fundamental.

## Aliasing

The saturation runs at the session sample rate without oversampling (implementation). Harmonics that fall above half the sample rate fold back into the audible range. The effect grows with drive and with high-frequency content. Aliasing was not measured for version 1.1.1.
