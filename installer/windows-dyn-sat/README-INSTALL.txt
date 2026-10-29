ANGRY PIG - Dynamic Saturation 1.1.1 Beta 1 (Windows VST3)
EXPERIMENTAL BETA. Installer package, UNSIGNED.

Contents
  AngryPig-Setup.ps1      setup window (Windows PowerShell 5.1, WinForms)
  AngryPig-Install.ps1    installer script (Status, Install, Uninstall, Rollback)
  payload.zip             AngryPigDynamicSaturation.vst3 + payload-manifest.json (SHA-256 of every bundle file)

What it installs
  C:\Program Files\Common Files\VST3\AngryPigDynamicSaturation.vst3
  Plugin name: "Angry Pig Dynamic Saturation (candidate)"   VST3 class ID: APd2
  Gain range: -36 to +36 dB (Angry Pig 1.0.0: -24 to +18 dB)
  Logs and backups: C:\ProgramData\AngryPigDynamicSaturation-Beta1\

What it does not touch
  Angry Pig 1.0.0 (AngryPigSpike.vst3, class ID APm1) is a different bundle and is not changed.
  Other plugins are not changed.

Status
  Not code-signed. Windows SmartScreen and the VST3 host may warn about the publisher.
  Dynamic Saturation (A+B active, C off) is experimental. Please report sound, GUI and host problems.

Requirements
  Windows 10/11 x64. Microsoft Edge WebView2 Runtime (the setup checks for it and does not install it).
  Close all DAWs and plugin hosts before installing or uninstalling.

How to use
  1. Run the Setup .exe and tick the unsigned-build checkbox.
  2. Install. Administrator permission is requested once (the VST3 folder is under Program Files).
  3. Restart your DAW or plugin host and rescan plugins.
  4. Uninstall removes only AngryPigDynamicSaturation.vst3. The production 1.0.0 bundle stays.
  5. Reinstall is the same as Install. Refresh shows the current state.

Exit codes and the log are described in the header of AngryPig-Install.ps1.
Log: C:\ProgramData\AngryPigDynamicSaturation-Beta1\install.log
