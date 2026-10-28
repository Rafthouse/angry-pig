# Overview

## Purpose

Angry Pig adds saturation and distortion to a stereo signal, with simple controls for drive, tone and level. It is an insert effect for audio tracks.

## Signal path

The processing is applied per sample, in this order.

1. **High-pass filter** (implementation). Two cascaded second-order high-pass sections on each channel. Their corner frequencies depend on the sample rate. At 44.1 kHz they are about 29 Hz and 19.5 Hz, with Q near 0.65.
2. **COLOR filter** (implementation). A peaking filter. Its centre frequency and boost are set by COLOR. See [Controls](controls.md).
3. **Drive gain** (implementation). A linear gain set by ANGER and GAIN, plus a fixed +1.0 dB base gain.
4. **Level detector and gain reduction** (implementation). Peak detection on both channels, linked. Attack time constant 2.45 ms, release time constant 30 ms. Hard-knee reduction above a threshold set by ANGER. See [Controls](controls.md#anger-0-to-100--default-0-).
5. **Saturation** (implementation). A memoryless waveshaping curve. See [Saturation and harmonics](saturation.md).
6. **Bypass** (implementation). When BYPASS is on, the input is passed through unchanged.

Parameter changes are smoothed with a time constant of about 6 ms, so moving a control does not produce clicks.

## Channels

Stereo input and stereo output only (implementation). The processing requires two input and two output channels. Mono input is not supported.

## Sample rates

The plugin accepts sample rates from 8 kHz to 384 kHz (implementation). The high-pass corner frequencies are chosen from five sample-rate ranges, so the low-end response stays close to the same frequencies across rates.

## Oversampling

None. The saturation is computed at the session sample rate (implementation). See [Limitations](limitations.md).

## Latency

Zero samples. The plugin reports zero latency to the host (measured). It has no look-ahead buffer.

## Bypass

When BYPASS is on, the output equals the input (implementation). Bypass is not a mute: the track keeps playing the original signal.
