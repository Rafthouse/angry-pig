ANGRY PIG 1.1.1 (macOS, VST3 + Audio Unit)
Universal binary (Apple silicon and Intel). Ad-hoc signed, NOT notarized.

Contents
  AngryPigSpike.vst3        VST3 plug-in, class ID APm1 (the same as Angry Pig 1.0.0)
  AngryPigSpike.component   Audio Unit (AUv2), type aufx, subtype APm1, manufacturer APLb
  THIRD_PARTY_NOTICES.txt   third-party notices
Plugin name "Angry Pig". Gain range -36 to +36 dB.

Upgrade from Angry Pig 1.0.0
  1.1.1 replaces the 1.0.0 VST3 in /Library/Audio/Plug-Ins/VST3 (same bundle name and class ID), so
  projects open with 1.1.1. Saved values load unchanged. GAIN automation recorded with 1.0.0
  (GAIN -24 to +18 dB) plays back at different values (0 dB becomes +5.1 dB). Check GAIN
  automation in old projects. The Audio Unit is new in 1.1.1.

Install with the PKG (recommended)
  Open AngryPig-1.1.1-macOS.pkg. The package is not notarized, so macOS blocks it the
  first time. Verify the checksum, then open System Settings > Privacy & Security and choose Open Anyway for
  this file (on macOS 14 and earlier, right-click the file and choose Open also works). Do not disable
  Gatekeeper. It installs into /Library/Audio/Plug-Ins/VST3 and /Library/Audio/Plug-Ins/Components.

Install by hand (ZIP)
  1. Quit all DAWs.
  2. Copy AngryPigSpike.vst3 to /Library/Audio/Plug-Ins/VST3/
     and AngryPigSpike.component to /Library/Audio/Plug-Ins/Components/
  3. If macOS says the plug-in is damaged or cannot be checked, remove the quarantine flag:
       xattr -dr com.apple.quarantine /Library/Audio/Plug-Ins/VST3/AngryPigSpike.vst3
       xattr -dr com.apple.quarantine /Library/Audio/Plug-Ins/Components/AngryPigSpike.component
  4. Restart the DAW and rescan plug-ins. Logic Pro rescans Audio Units on start.

Uninstall
  Delete the two bundles above from /Library/Audio/Plug-Ins/VST3 and /Library/Audio/Plug-Ins/Components.

Please report sound, GUI and host problems with your DAW name and version, macOS version and Mac model.
