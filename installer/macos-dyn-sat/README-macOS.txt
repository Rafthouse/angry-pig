ANGRY PIG - Dynamic Saturation 1.1.1 Beta 1 (macOS, VST3 + Audio Unit)
EXPERIMENTAL BETA. Universal binary (Apple silicon and Intel). Ad-hoc signed, NOT notarized.

Contents
  AngryPigDynamicSaturation.vst3        VST3 plug-in
  AngryPigDynamicSaturation.component   Audio Unit (AUv2), type aufx, subtype APd2, manufacturer APLb
  THIRD_PARTY_NOTICES.txt               third-party notices
Plugin name "Angry Pig Dynamic Saturation (candidate)". Gain range -36 to +36 dB.
Angry Pig 1.0.0 (AngryPigSpike.vst3 / AngryPigSpike.component) is a different bundle and is not replaced.

Install with the PKG (recommended)
  Open AngryPig-DynamicSaturation-1.1.1-Beta-macOS.pkg. The package is not notarized, so macOS may block it:
  right-click the file, choose Open, then confirm. It installs into /Library/Audio/Plug-Ins/VST3 and /Components.

Install by hand (ZIP)
  1. Quit all DAWs.
  2. Copy AngryPigDynamicSaturation.vst3 to /Library/Audio/Plug-Ins/VST3/
     and AngryPigDynamicSaturation.component to /Library/Audio/Plug-Ins/Components/
  3. If macOS says the plug-in is damaged or cannot be checked, remove the quarantine flag:
       xattr -dr com.apple.quarantine /Library/Audio/Plug-Ins/VST3/AngryPigDynamicSaturation.vst3
       xattr -dr com.apple.quarantine /Library/Audio/Plug-Ins/Components/AngryPigDynamicSaturation.component
  4. Restart the DAW and rescan plug-ins. Logic Pro rescans Audio Units on start.

Uninstall
  Delete the two bundles above from /Library/Audio/Plug-Ins/VST3 and /Library/Audio/Plug-Ins/Components.

Please report sound, GUI and host problems with your DAW name and version, macOS version and Mac model.
