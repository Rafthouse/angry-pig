#define PLUG_NAME "Angry Pig Dynamic Saturation (candidate)"
#define PLUG_MFR "Mykyta Shchur"
#define PLUG_VERSION_HEX 0x00010101
#define PLUG_VERSION_STR "1.1.1-beta1"
#define PLUG_UNIQUE_ID 'APd2'
#define PLUG_MFR_ID 'APLb'
#define PLUG_URL_STR "https://iplug2.github.io"
#define PLUG_EMAIL_STR ""
#define PLUG_COPYRIGHT_STR "Created by Mykyta Shchur"
#define PLUG_CLASS_NAME AngryPigSpike

// Must match CFBundleIdentifier in the Info.plists: iPlug finds the macOS bundle (AU view, web UI) by it.
#define BUNDLE_NAME "AngryPigDynamicSaturation"
#define BUNDLE_MFR "AngryPigLab"
#define BUNDLE_DOMAIN "com"

#define SHARED_RESOURCES_SUBPATH "AngryPigSpike"

#define PLUG_CHANNEL_IO "2-2"

#define PLUG_LATENCY 0
#define PLUG_TYPE 0
#define PLUG_DOES_MIDI_IN 0
#define PLUG_DOES_MIDI_OUT 0
#define PLUG_DOES_MPE 0
#define PLUG_DOES_STATE_CHUNKS 0
#define PLUG_HAS_UI 1
#define PLUG_WIDTH 800
#define PLUG_HEIGHT 600
#define PLUG_FPS 60
#define PLUG_SHARED_RESOURCES 0
#define PLUG_HOST_RESIZE 1

#define AUV2_ENTRY AngryPigSpike_Entry
#define AUV2_ENTRY_STR "AngryPigSpike_Entry"
#define AUV2_FACTORY AngryPigSpike_Factory
#define AUV2_VIEW_CLASS AngryPigDynSat_View
#define AUV2_VIEW_CLASS_STR "AngryPigDynSat_View"

#define AAX_TYPE_IDS 'IPWV'
#define AAX_TYPE_IDS_AUDIOSUITE 'IPWA'
#define AAX_PLUG_MFR_STR "Acme"
#define AAX_PLUG_NAME_STR "AngryPigSpike\nIPEF"
#define AAX_PLUG_CATEGORY_STR "Effect"
#define AAX_DOES_AUDIOSUITE 1

#define VST3_SUBCATEGORY "Fx"

#define APP_NUM_CHANNELS 2
#define APP_N_VECTOR_WAIT 0
#define APP_MULT 1
#define APP_COPY_AUV3 0
#define APP_SIGNAL_VECTOR_SIZE 64
