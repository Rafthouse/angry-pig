# Building Angry Pig Dynamic Saturation

This tree builds the **Angry Pig Dynamic Saturation 1.1.1 beta** (VST3 class ID `APLb`/`APd2`) from a clean checkout on Windows and macOS. It does not build or change Angry Pig 1.0.0.

Everything the build needs is in the repository or pinned by version and SHA-256:

| Dependency | Where | Pin |
| --- | --- | --- |
| iPlug 2 with the VST 3 SDK (MIT) | `deps/iplug2.tar.gz` | SHA-256 `ca6f2be74c522e5a8eeeac1859a356c09764f2cca0fad02d91bc3b0a439808bd` |
| Microsoft WebView2 SDK (Windows only) | NuGet `Microsoft.Web.WebView2` | 1.0.2903.40, SHA-256 `ef128016dd1e51c59178c827ed5b8aa3322c57afa8675d930f8109505542ad74` |
| Windows Implementation Library (Windows only) | fetched by CMake from GitHub | tag `v1.0.240803.1` |

The archive contains placeholders only for the AAX and VST2 SDKs; neither format is built. The WebView2 SDK is downloaded during the build and is not stored in this repository.

## Windows (x64 VST3)

Requirements: Windows 10 or 11, Visual Studio 2022 (Desktop C++), CMake 3.24 or later, Git, 7-Zip, Python 3.

```powershell
mkdir src\deps
tar -xzf deps\iplug2.tar.gz -C src\deps
Invoke-WebRequest https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/1.0.2903.40 -OutFile webview2.zip
(Get-FileHash webview2.zip).Hash   # must be EF128016DD1E51C59178C827ED5B8AA3322C57AFA8675D930F8109505542AD74
Expand-Archive webview2.zip -DestinationPath src\deps\webview2
cmake -S src\native -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target AngryPigSpike-vst3
```

The bundle is `build\out\AngryPigSpike.vst3`. Package it (Setup.exe, portable ZIP, SHA256SUMS.txt):

```powershell
powershell -ExecutionPolicy Bypass -File installer\windows-dyn-sat\build-packages.ps1 -Bundle build\out\AngryPigSpike.vst3 -OutDir pkg -Version 1.1.1-Beta
```

The packages install the bundle as `AngryPigDynamicSaturation.vst3`, next to Angry Pig 1.0.0 (`AngryPigSpike.vst3`). Only the bundle and DLL are renamed; their bytes are unchanged.

## macOS (universal VST3 and AUv2)

Requirements: macOS 13 or later, Xcode 15 or later, CMake 3.24 or later.

```sh
mkdir -p src/deps && tar -xzf deps/iplug2.tar.gz -C src/deps
cmake -S src/native -B build -G Xcode \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
  -DIPLUG2_DIR="$PWD/src/deps/iplug2" -DIPLUG_DEPLOY_PLUGINS=OFF -DAP_BUILD_AU=ON
cmake --build build --config Release
```

The release workflow renames the bundles to `AngryPigDynamicSaturation.vst3` and `AngryPigDynamicSaturation.component`, ad-hoc signs them and builds the PKG and ZIP. See `.github/workflows/build.yml`.

## Tests

| Test | Command |
| --- | --- |
| Isolated install / uninstall / reinstall (Windows) | `tools\qa\windows-install-qa.ps1 -SetupExe <Setup.exe> -ExpectedDllSha256 <sha> -Work <empty folder>` |
| DSP metrics of a render (halo, LF, odd harmonics, DC) | `python tools/parity/gen_sine.py sine.wav 44100 999 -12 10.54421768707483`, render with `tools/parity/parity_render`, then `python tools/parity/halo_metrics.py out.wav --expect tools/parity/reference/windows-x64-sine999-m12-anger100.json` |
| Cross-platform render comparison | `python tools/parity/compare_wav.py mac.wav windows.wav 1e-4` |
| CPU profile of the DSP engine (x86-64) | build `tools/perf-bench/bench.cpp` with `/O2 /fp:strict`, run `bench time` and `bench compare` |

CI runs all of these on every push to a `release/**` branch: `.github/workflows/build.yml`.

## Build options that must stay as they are

- `/fp:strict` on MSVC and no fast-math on any compiler. The saturation curve and the halo are defined by exact floating-point behaviour.
- `AP_MATCH_DSP=ON`, `AP_DIAGNOSTICS=OFF`, `AP_TIMING=OFF` for release builds. The diagnostic options need lab sources that are not part of this repository.
