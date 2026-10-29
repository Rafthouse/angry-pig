ANGRY PIG 1.1.1 (Windows VST3)
Installer package, UNSIGNED.

Contents
  AngryPig-Setup.ps1      setup window (Windows PowerShell 5.1, WinForms)
  AngryPig-Install.ps1    installer script (Status, Install, Uninstall, Rollback)
  payload.zip             AngryPigSpike.vst3 + payload-manifest.json (SHA-256 of every bundle file)

What it installs
  C:\Program Files\Common Files\VST3\AngryPigSpike.vst3
  Plugin name: "Angry Pig"   VST3 class ID: APm1 (the same as Angry Pig 1.0.0)
  Logs and backups: C:\ProgramData\AngryPig\

Upgrade from Angry Pig 1.0.0
  1.1.1 replaces 1.0.0: same bundle name and class ID, so your projects open with 1.1.1.
  Setup asks before it replaces 1.0.0 and first copies it to C:\ProgramData\AngryPig\backup\.
  Saved ANGER, COLOR, GAIN and BYPASS values load unchanged.
  GAIN now goes from -36 to +36 dB (1.0.0: -24 to +18 dB). Host automation of GAIN that was
  recorded with 1.0.0 plays back at different values (0 dB in 1.0.0 becomes +5.1 dB). Check GAIN
  automation in old projects. See the release notes for the full table.

What it does not touch
  Other plugins, including the Dynamic Saturation beta (AngryPigDynamicSaturation.vst3).

Requirements
  Windows 10/11 x64. Microsoft Edge WebView2 Runtime (the setup checks for it and does not install it).
  Close all DAWs and plugin hosts before installing or uninstalling.

How to use
  1. Run the Setup .exe and tick the unsigned-build checkbox.
  2. Install. Administrator permission is requested once (the VST3 folder is under Program Files).
  3. Restart your DAW or plugin host and rescan plugins.
  4. Uninstall removes AngryPigSpike.vst3. Backups stay in C:\ProgramData\AngryPig\backup\.
  5. To go back to 1.0.0, run the 1.0.0 setup, or close all hosts, delete the installed
     AngryPigSpike.vst3 and copy AngryPigSpike.vst3 from C:\ProgramData\AngryPig\backup\<stamp>     back to C:\Program Files\Common Files\VST3\.
  6. Reinstall is the same as Install. Refresh shows the current state.

Exit codes and the log are described in the header of AngryPig-Install.ps1.
Log: C:\ProgramData\AngryPig\install.log
