# Angry Pig Dynamic Saturation 1.1.1 beta 1 (pre-release)

**Experimental beta. Not a replacement for Angry Pig 1.0.0.** This is a separate plug-in, "Angry Pig Dynamic Saturation (candidate)", with its own VST3 class ID (`APd2`). It installs next to Angry Pig 1.0.0 and does not change it. Angry Pig 1.0.0 stays the current stable release.

## What is new compared with 1.0.0

- **Dynamic saturation.** The saturation responds to the level of the signal (stages A and B active, stage C off). Quiet passages stay cleaner, loud peaks saturate more, and the low-level "halo" around the harmonics is lower.
- **GAIN range -36 to +36 dB** (1.0.0: -24 to +18 dB).
- **macOS Audio Unit (AUv2)** next to the VST3, universal (Apple Silicon and Intel).
- Same four controls (ANGER, COLOR, GAIN, BYPASS) and the same interface.

## Downloads

| File | Platform | Contents |
| --- | --- | --- |
| `AngryPig-DynamicSaturation-1.1.1-Beta-Windows-Setup.exe` | Windows 10 and 11, 64-bit | Installer for the VST3, with uninstall |
| `AngryPig-DynamicSaturation-1.1.1-Beta-Windows-Portable.zip` | Windows 10 and 11, 64-bit | VST3 bundle for manual installation |
| `SHA256SUMS.txt` | Windows | checksums of the two Windows files |
| `AngryPig-DynamicSaturation-1.1.1-Beta-macOS.pkg` | macOS 11 or later, universal | Installer for the VST3 and the Audio Unit |
| `AngryPig-DynamicSaturation-1.1.1-Beta-macOS.zip` | macOS 11 or later, universal | VST3 and Audio Unit bundles for manual installation |
| `SHA256SUMS-macOS.txt` | macOS | checksums of the two macOS files |

Check the checksum before you run an installer: `Get-FileHash <file>` on Windows, `shasum -a 256 -c SHA256SUMS-macOS.txt` on macOS.

## Install, uninstall, reinstall

### Windows

Requires the Microsoft Edge WebView2 Runtime (already present on most Windows 10 and 11 systems).

1. Close all DAWs and plug-in hosts.
2. Run the Setup .exe. It is not code-signed, so SmartScreen may warn you. Tick the unsigned-build checkbox and choose **Install**. Windows asks once for administrator permission.
3. Restart the DAW and rescan plug-ins. The plug-in appears as **Angry Pig Dynamic Saturation (candidate)**.

The plug-in is installed to `C:\Program Files\Common Files\VST3\AngryPigDynamicSaturation.vst3`. Logs and backups are in `C:\ProgramData\AngryPigDynamicSaturation-Beta1\`.

- **Uninstall:** run the Setup .exe again and choose **Uninstall**. It removes only `AngryPigDynamicSaturation.vst3`. Angry Pig 1.0.0 (`AngryPigSpike.vst3`) stays.
- **Reinstall:** run the Setup .exe and choose **Install** again.
- **Portable:** copy `AngryPigDynamicSaturation.vst3` from the ZIP to `C:\Program Files\Common Files\VST3\`. To uninstall, delete that folder.

### macOS

1. Quit all DAWs.
2. Open the PKG. It is ad-hoc signed and **not notarized**, so macOS blocks it the first time. After you have checked the checksum, open *System Settings → Privacy & Security* and choose **Open Anyway** for this file. Do not disable Gatekeeper or System Integrity Protection.
3. The PKG installs `AngryPigDynamicSaturation.vst3` to `/Library/Audio/Plug-Ins/VST3/` and `AngryPigDynamicSaturation.component` to `/Library/Audio/Plug-Ins/Components/`.
4. Restart the DAW and rescan plug-ins. Logic Pro rescans Audio Units on start.

- **Uninstall:** delete the two bundles:
  `sudo rm -rf /Library/Audio/Plug-Ins/VST3/AngryPigDynamicSaturation.vst3 /Library/Audio/Plug-Ins/Components/AngryPigDynamicSaturation.component`
- **Reinstall:** open the PKG again.
- **ZIP:** copy the two bundles to the folders above. If macOS says a bundle is damaged, run `xattr -dr com.apple.quarantine <bundle>` on it.

## Verification

Summary below. The full record is in [docs/dyn-sat/VERIFICATION.md](dyn-sat/VERIFICATION.md).

- The DSP is the approved candidate, unchanged. Output is bit-identical to the approved build at 44.1, 48 and 96 kHz and at block sizes 1 to 512. Halo -75.82 dBc, low-frequency floor -83.38 dBc, odd harmonics H3 to H15 unchanged. Digital silence stays silent.
- CPU: about 0.5 % of one core at 44.1 kHz and 1.1 % at 96 kHz (Intel i9-9900KF). No optimization was needed. See [PERFORMANCE.md](dyn-sat/PERFORMANCE.md).
- Windows: pluginval strictness 8 passed. Steinberg VST3 validator 47 of 47 passed. Install, uninstall and reinstall: 24 of 24 checks.
- macOS (GitHub-hosted runners, Apple Silicon macOS 14.8 and Intel macOS 15.7): PKG install, `auval` AU VALIDATION SUCCEEDED, pluginval strictness 8 passed for VST3 and AU on both. Halo, low-frequency floor and harmonics identical to Windows on both architectures.

## Known limitations

- **Beta.** The dynamic saturation is new. Please report problems with sound, interface or host compatibility.
- **Not code-signed** on Windows. **Not notarized** on macOS (ad-hoc signature only).
- **macOS has not been tested on a physical Mac or in a DAW.** All macOS tests ran on GitHub-hosted runners. A test on a Mac mini M1 is prepared ([checklist](dyn-sat/MAC_MINI_M1_TEST.md)) but has not been run.
- Windows host testing: REAPER only, by the author. Other hosts are untested.
- Stereo only. No oversampling, so heavy drive can alias high frequencies. Latency 0 samples.
- The plug-in name still carries "(candidate)". Projects saved with this beta will reference this name and class ID.

## Source and build

The source of this plug-in and its build scripts are in this repository: [BUILDING.md](../BUILDING.md). No license for reuse of the source code has been granted yet; the author retains all rights. Third-party components and their licenses: [THIRD_PARTY_NOTICES.txt](../notices/THIRD_PARTY_NOTICES.txt).

## Build provenance

- Source commit: `5e6b624a8cd9647709ba82a27647d16b24db12bc`
- CI build and test run: [40900683844](https://github.com/Rafthouse/angry-pig/actions/runs/40900683844). The files of this release are the artifacts of that run.
- Windows DLL SHA-256: `404F6F7B3EAAD53C0308AA3D63DAA286FC879FC9AEEDAE7C1A41CDF48FD3407C`
