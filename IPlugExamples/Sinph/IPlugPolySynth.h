#ifndef __IPLUGPOLYSYNTH__
#define __IPLUGPOLYSYNTH__

#include "IPlug_include_in_plug_hdr.h"
#include "IMidiQueue.h"
#include "IPlugPolySynthDSP.h"

#define MAX_VOICES 16
#define TIME_MIN 2.
#define TIME_MAX 5000.
constexpr double semitone = 1.0594630943592952646;

enum EParams
{
    kAlgSelect = 0,
    kModGain,
    kNoise,
    kUnison,

    kFoot,
    kEnvFreqMod,
    kData,
    kVolume,

    kBalance,
    kWarm,
    kPanShapes,
    kVelSens,

    kEF1,
    kEF2,
    kEF3,
    kEF4,

    kF2,
    kQ2,
    kFB,
    kF1,

    kAttack,
    kDecay,
    kSustain,
    kRelease,

    kVF2,
    kVQ2,
    kVFB,
    KVF1,

    kMF2,
    kMQ2,
    kMFB,
    kMF1,

    kNumParams
};

class IPlugPolySynth : public IPlug
{
public:

  IPlugPolySynth(IPlugInstanceInfo instanceInfo);
  ~IPlugPolySynth();

  void Reset();
  void OnParamChange(int paramIdx);

  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
  bool HostRequestingAboutBox();

  int GetNumKeys();
  bool GetKeyStatus(int key);
  void ProcessMidiMsg(IMidiMsg* pMsg);
  void NoteOnOff(IMidiMsg* pMsg);

private:

  //void NoteOnOffPoly(IMidiMsg* pMsg);
  int FindFreeVoice();

  IBitmapOverlayControl* mAboutBox;
  IControl* mKeyboard;

  IMidiQueue mMidiQueue;

  int mActiveVoices;
  int mKey;
  int mNumHeldKeys;
  bool mKeyStatus[128]; // array of on/off for each key

  double mSampleRate;
  double bender = 0.0;

  CVoiceState mVS[MAX_VOICES];
  CWTOsc* mOsc;
  CADSREnvL* mEnv;
  double* mTable;
  IControl* dials[kNumParams];
  IControl* labels[kNumParams];
};

enum ELayout
{
  kWidth = GUI_WIDTH,  // width of plugin window
  kHeight = GUI_HEIGHT, // height of plugin window

  kKeybX = 1,
  kKeybY = 233,

  kKnobFrames = 256,
  kAlgFrames = 31
};

#endif //__IPLUGPOLYSYNTH__
