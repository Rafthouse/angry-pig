ANGRY PIG 1.1.1 (Windows VST3)
Portable package, UNSIGNED. No installer, no registry changes.

Contents
  AngryPigSpike.vst3        VST3 bundle, the verified build
                            plugin name "Angry Pig", VST3 class ID APm1 (the same as Angry Pig 1.0.0)
  THIRD_PARTY_NOTICES.txt   third-party notices

Install by hand
  1. Close all DAWs and plugin hosts.
  2. If Angry Pig 1.0.0 is installed, move C:\Program Files\Common Files\VST3\AngryPigSpike.vst3
     out of the VST3 folder first if you want to keep a copy. 1.1.1 replaces it.
  3. Copy the folder AngryPigSpike.vst3 to
         C:\Program Files\Common Files\VST3\
     Writing there needs administrator rights. Your host must scan this folder.
  4. Rescan plugins in the host.

Uninstall by hand
  Delete the folder C:\Program Files\Common Files\VST3\AngryPigSpike.vst3

Projects from Angry Pig 1.0.0
  They open with 1.1.1. Saved values load unchanged. GAIN now goes from -36 to +36 dB
  (1.0.0: -24 to +18 dB), so GAIN automation recorded with 1.0.0 plays back at different
  values (0 dB becomes +5.1 dB). Check GAIN automation in old projects.

Alternative: the Setup .exe does the same copy with integrity checks, a backup of 1.0.0, a log
and a clean uninstall. Use one or the other, not both.

Requirements
  Windows 10/11 x64. Microsoft Edge WebView2 Runtime (the GUI needs it; this package does not install it).
  If Windows marked the ZIP as downloaded: right-click it, Properties, Unblock, then extract.

Notes
  Not code-signed. SmartScreen and the host may warn about the publisher.
  Report sound, GUI and host problems with your DAW name and version, and your Windows version.
