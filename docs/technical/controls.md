# Controls

All four parameters are exposed to the host and can be automated. Default values are in brackets.

## ANGER (0 to 100 %, default 0 %)

**Implementation: drive.** The linear drive gain is 10^((39 × A + G) / 20) × 1.122, where A is ANGER as a fraction from 0 to 1 and G is GAIN in dB. The factor 1.122 is the fixed +1.0 dB base gain. At 100 %, ANGER adds 39 dB of drive.

**Implementation: gain reduction.** A level detector follows the driven signal. The threshold is 5 × ANGER dB above unity: 0 dB at 0 % and +5 dB at 100 %. Above the threshold the stage applies about 0.9 dB of gain reduction per dB of excess. The output above the threshold therefore rises about 0.1 dB per dB of input: a hard-knee ratio of about 10:1.

**Implementation: timing.** Attack time constant 2.45 ms, release time constant 30 ms.

**Measured: small-signal gain** (999 Hz, GAIN 0 dB, input -60 dBFS):

| ANGER | Gain |
| --- | --- |
| 0 % | +1.0 dB |
| 25 % | +10.7 dB |
| 50 % | +20.5 dB |
| 75 % | +30.2 dB |
| 100 % | +40.0 dB |

The small-signal gain is 1.0 dB plus 0.39 dB for each percent of ANGER.

## COLOR (0 to 100 %, default 0 %)

**Implementation.** COLOR sets a peaking filter before the drive stage. Its boost is 8 dB at 100 % and 0 dB at 0 %. Its centre frequency moves with COLOR, and its bandwidth is about 1.9 octaves.

**Measured** (GAIN 0 dB, ANGER 0 %, relative to the input):

| COLOR | Peak gain | Frequency of the peak |
| --- | --- | --- |
| 0 % | +1.0 dB (flat) | none |
| 50 % | +4.4 dB | about 1 kHz |
| 100 % | +8.5 dB | about 8 kHz |

At ANGER 100 %, the peak is about 47.5 dB at 8 kHz: the 40 dB small-signal gain plus the COLOR boost.

Because COLOR boosts a band before the drive stage, that band reaches saturation sooner. Measured at ANGER 50 %, input -24 dBFS: total harmonic distortion is -174.5 dB at COLOR 0 % and -40.1 dB at COLOR 50 %. See the [measurements](../measurements/README.md#effect-of-color-on-distortion).

## GAIN (-36 to +36 dB, default 0 dB)

**Implementation.** GAIN is added in dB to the drive gain, before the level detector and the saturation stage. The range is -36 to +36 dB since 1.1.1 (1.0.0: -24 to +18 dB). A saved GAIN value keeps its dB value; host automation recorded with 1.0.0 does not (see the [release notes](../RELEASE_NOTES_1.1.1.md#projects-from-angry-pig-100)).

**Measured.** At ANGER 0 %, the output equals the input plus GAIN plus 1.0 dB, within 0.01 dB across the range. At ANGER 100 %, the saturation stage limits the output: raising GAIN from 0 to +36 dB changes the output by about +1.9 dB. See [Gain staging](gain-staging.md).

## BYPASS (off by default)

**Implementation.** When BYPASS is on, the output equals the input and the processing is not applied.
