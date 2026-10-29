# Dynamic Saturation 1.1.1: CPU profile

One targeted profile of the DSP engine (`src/native/APDspEngine.h`, `APDynamicSaturation.h`), measured with `tools/perf-bench/bench.cpp`.

**Conclusion: the engine is already efficient. No optimization was applied, and the DSP source is unchanged.**

## Setup

- Intel Core i9-9900KF, Windows 11, MSVC 19.44, `/O2 /fp:strict /MT`, the same compiler options as the plug-in.
- Stereo, program-like signal (tonal content, noise, silence, peaks up to +3 dBFS) with parameter changes every 0.37 s (smoothing, COLOR recompute, bypass).
- 20 s per run, best of 5. Time per stereo frame, and the share of one core in real time.
- Initialization (`reset()`) is measured separately and is not part of the per-block numbers.

## Results (baseline engine)

| Sample rate | Block | ns per frame | Share of one core | With Dynamic off |
| --- | --- | --- | --- | --- |
| 44.1 kHz | 1 | 130 | 0.57 % | 90 ns |
| 44.1 kHz | 64 | 113 | 0.50 % | 72 ns |
| 44.1 kHz | 512 | 109 | 0.48 % | 72 ns |
| 48 kHz | 512 | 112 | 0.54 % | 73 ns |
| 96 kHz | 512 | 116 | 1.11 % | 75 ns |
| 96 kHz | 4096 | 116 | 1.12 % | 82 ns |

- `reset()`: about 60 µs, once per sample-rate change. Most of it is the 8192-step normalization of the 32 Hz halo filter. It does not run in the audio callback.
- Dynamic A+B costs about 38 ns per frame, about a third of the total.
- Per-call cost: `pow` 13 ns, `log10` 5.7 ns, `sin` 3.3 ns, `saturate` 3.9 ns.

## What was tried

1. **Bit-exact caching** of the gain `pow()` (recomputed only when the smoothed parameters change) and skipping the compressor `log10()`/`pow()` when the envelope is certainly below the knee. Outputs were bit-identical in all 180 cases (44.1/48/96 kHz, blocks 1/7/64/512/4096, sine, program material, silence, fixed and automated parameters, Dynamic on and off). **Speed-up 0 to 7 %, within run-to-run noise.** The compressor is active on most real material, so the skipped path is rarely taken.
2. **Upper bound check** (test only, not bit-exact): removing the compressor's `log10()` and `pow()` entirely saved 20 to 28 %, about 0.1 % of one core at 48 kHz. Getting that would need approximations that change the output.

Neither result justifies a change to an approved, measured DSP, so the source was left as it is.

## Realtime safety

The audio path (`ProcessBlock`, `APDspEngine::process`) has no allocations, locks, file I/O or system calls. The only mutex in the plug-in is taken in `OnActivate` (AU wrapper) and in control-thread code, not during processing.
