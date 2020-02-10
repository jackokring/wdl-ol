#define PLUG_MFR "KRTL"
#define PLUG_NAME "Sinph"

#define PLUG_CLASS_NAME IPlugPolySynth

#define BUNDLE_MFR "KRTL"
#define BUNDLE_NAME "Sinph"

#define PLUG_ENTRY IPlugPolySynth_Entry
#define PLUG_VIEW_ENTRY IPlugPolySynth_ViewEntry

#define PLUG_ENTRY_STR "IPlugPolySynth_Entry"
#define PLUG_VIEW_ENTRY_STR "IPlugPolySynth_ViewEntry"

#define VIEW_CLASS IPlugPolySynth_View
#define VIEW_CLASS_STR "IPlugPolySynth_View"

// Format        0xMAJR.MN.BG - in HEX! so version 10.1.5 would be 0x000A0105
#define PLUG_VER 0x00010000
#define VST3_VER_STR "1.0.0"

#define PLUG_UNIQUE_ID 'styg'
// make sure this is not the same as BUNDLE_MFR
#define PLUG_MFR_ID 'krtl'

// ProTools stuff
#if (defined(AAX_API) || defined(RTAS_API)) && !defined(_PIDS_)
  #define _PIDS_
  const int PLUG_TYPE_IDS[2] = {'PSN1', 'PSN2'};
#endif
#define PLUG_MFR_PT "KRT\nKRT\nkrtl"
#define PLUG_NAME_PT "Sinph\nSTYG"
#define PLUG_TYPE_PT "Synth"
#define PLUG_DOES_AUDIOSUITE 0

#if (defined(AAX_API) || defined(RTAS_API)) 
#define PLUG_CHANNEL_IO "1-1 2-2"
#else
#define PLUG_CHANNEL_IO "0-2"
#endif

#define PLUG_LATENCY 0
#define PLUG_IS_INST 1

// if this is 0 RTAS can't get tempo info
#define PLUG_DOES_MIDI 1

#define PLUG_DOES_STATE_CHUNKS 0

// Unique IDs for each image resource.
#define KNOB_ID       101
#define BG_ID         102
#define ABOUTBOX_ID   103
#define WHITE_KEY_ID  104
#define BLACK_KEY_ID  105
#define KNOB_ALG_ID   106
#define SWITCH_ID     107
#define SLIDER_ID     108
#define MIDI_ID       109
#define TWO_STATE_ID  110

// Image resource locations for this plug.
#define KNOB_FN       "resources/img/knob.png"
#define BG_FN         "resources/img/bg.png"
#define ABOUTBOX_FN   "resources/img/about.png"
#define WHITE_KEY_FN  "resources/img/wk.png"
#define BLACK_KEY_FN  "resources/img/bk.png"
#define KNOB_ALG_FN   "resources/img/alg-knob.png"
#define SWITCH_FN     "resources/img/switch.png"
#define SLIDER_FN     "resources/img/slider.png"
#define MIDI_FN       "resources/img/midi.png"
#define TWO_STATE_FN  "resources/img/2state.png"

// GUI default dimensions
#define GUI_WIDTH   670
#define GUI_HEIGHT  420

// on MSVC, you must define SA_API in the resource editor preprocessor macros as well as the c++ ones
#if defined(SA_API) && !defined(OS_IOS)
#include "app_wrapper/app_resource.h"
#endif

// vst3 stuff
#define MFR_URL "kring.co.uk"
#define MFR_EMAIL "vst@kring.co.uk"
#define EFFECT_TYPE_VST3 "Instrument|Synth"

/* "Fx|Analyzer"", "Fx|Delay", "Fx|Distortion", "Fx|Dynamics", "Fx|EQ", "Fx|Filter",
"Fx", "Fx|Instrument", "Fx|InstrumentExternal", "Fx|Spatial", "Fx|Generator",
"Fx|Mastering", "Fx|Modulation", "Fx|PitchShift", "Fx|Restoration", "Fx|Reverb",
"Fx|Surround", "Fx|Tools", "Instrument", "Instrument|Drum", "Instrument|Sampler",
"Instrument|Synth", "Instrument|Synth|Sampler", "Instrument|External", "Spatial",
"Spatial|Fx", "OnlyRT", "OnlyOfflineProcess", "Mono", "Stereo",
"Surround"
*/
