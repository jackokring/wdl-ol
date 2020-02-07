#ifndef __IPLUGPOLYSYNTH__
#define __IPLUGPOLYSYNTH__

#include "IPlug_include_in_plug_hdr.h"
#include "IMidiQueue.h"
#include "IPlugPolySynthDSP.h"

#define MAX_VOICES 16
#define TIME_MIN 2.
#define TIME_MAX 5000.

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

public:

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
  double bender[16];//as MUL relative not additive

  CVoiceState mVS[MAX_VOICES];
  CWTOsc* mOsc;
  CADSREnvL* mEnv;
  double* mTable;
  IControl* dials[kNumParams];
  IControl* labels[kNumParams];

  double oldParam[16][kNumProcessed] = { };
  double newParam[16][kNumProcessed] = { };
  double deltaParam[16][kNumProcessed] = { };
  int currentChan = 1;
  bool hackEdit = false;
  int programs[16] = { };
};

class Algorithm {
public:
    virtual double process(IPlugPolySynth* ref, CVoiceState* vs);
    virtual double makeLeft(double master);
    virtual double andMakeRight(double master);
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