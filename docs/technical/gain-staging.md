# Gain staging and operating levels

## Level relationships (measured)

| Setting | Effect on level |
| --- | --- |
| ANGER 0 %, GAIN 0 dB | Output = input + 1.0 dB |
| ANGER 0 %, GAIN changed | Output = input + GAIN + 1.0 dB, within 0.01 dB |
| ANGER 25 %, GAIN 0 dB | Output = input + 10.7 dB at low levels |
| ANGER 100 %, GAIN 0 dB | Output = input + 40.0 dB at low levels |
| ANGER 100 %, GAIN raised from 0 to +18 dB | Output rises by only about +1.8 dB, because the saturation stage limits it |

The gain set by ANGER applies to small signals. Louder signals reach the saturation stage, where the output flattens. Above that point, changes to GAIN and ANGER are much less audible.

## Practical operating levels

- **Input around -12 to -6 dBFS** is a practical starting point. At ANGER 0 % and GAIN 0 dB, the CLIP light stays off for inputs below about -2 dBFS. Higher ANGER values light it sooner.
- **Subtle settings (ANGER 0 to 25 %)** change the level by 1 to 11 dB. To keep the level steady, lower GAIN by about the same amount as the increase.
- **Strong settings (ANGER 60 to 100 %)** can push the output to its ceiling. Lower GAIN or the track fader so the next stage is not overloaded.
- **The output ceiling** is about -0.1 dBFS at the peaks. Use the master bus for final level control.

## The meter

The meter shows the level entering the saturation stage, relative to unity. 0 dB corresponds to a level of 1.0 at the saturation stage's input.

| Indicator | Lights when |
| --- | --- |
| -12 | the peak is at or above -12 dB |
| -6 | the peak is at or above -6 dB |
| 0 | the peak is at or above 0 dB |
| +6 | the peak is at or above +6 dB |
| CLIP | the peak is above about -0.9 dB |

The indicators show drive into the saturation stage, not the final output level. A lit CLIP light means the stage is compressing the signal, which is normal for strong settings.
