#include "IPlugPolySynth.h"
#include "IPlug_include_in_plug_src.h"
#include "resource.h"

#include "IControl.h"
#include "IKeyboardControl.h"


const int kNumPrograms = 128;

#define PITCH 440.
#define TABLE_SIZE 512

#ifndef M_PI
#define M_PI 3.14159265
#endif

#define GAIN_FACTOR 0.2;

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

char* kNames[kNumParams] = {
    "Algorithm", "Modulation", "Noise", "Unison",
    "Foot", "Bend", "Data", "Volume",
    "Balance", "Warm", "Shape", "Vel Sens",
    "FX1", "FX2", "FX3", "FX4",

    "F2", "Q2", "FB", "F1",
    "Attack", "Decay", "Sustain", "Release",
    "VF2", "VQ2", "VFB", "VF1",
    "MF2", "MQ2", "MFB", "MF1"
};

IParam::EParamType kTypes[kNumParams] = {
    IParam::kTypeDouble, IParam::kTypeDouble, IParam::kTypeDouble, IParam::kTypeDouble,
    IParam::kTypeDouble, IParam::kTypeDouble, IParam::kTypeDouble, IParam::kTypeDouble,
    IParam::kTypeDouble, IParam::kTypeDouble, IParam::kTypeDouble, IParam::kTypeDouble,
    IParam::kTypeDouble, IParam::kTypeDouble, IParam::kTypeDouble, IParam::kTypeDouble,

    IParam::kTypeDouble, IParam::kTypeDouble, IParam::kTypeDouble, IParam::kTypeDouble,
    IParam::kTypeDouble, IParam::kTypeDouble, IParam::kTypeDouble, IParam::kTypeDouble,
    IParam::kTypeDouble, IParam::kTypeDouble, IParam::kTypeDouble, IParam::kTypeDouble,
    IParam::kTypeDouble, IParam::kTypeDouble, IParam::kTypeDouble, IParam::kTypeDouble
};

enum EUses {
    uTime = 0,
    uPercent,
    uAlgorithm
};

EUses kUses[kNumParams] = {
    uAlgorithm, uTime, uTime, uTime,
    uTime, uTime, uTime, uTime,
    uTime, uTime, uTime, uTime,
    uTime, uTime, uTime, uTime,

    uTime, uTime, uTime, uTime,
    uTime, uTime, uPercent, uTime,
    uTime, uTime, uTime, uTime,
    uTime, uTime, uTime, uTime
};

int getX(int c) {
    return 55 * ((c % 4) + 4 * (c / 16)) + 12;
}

int getY(int c) {
    return 57 * ((c >> 2) & 3) + 10;
}

IPlugPolySynth::IPlugPolySynth(IPlugInstanceInfo instanceInfo)
  : IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo),
    mSampleRate(44100.),
    mNumHeldKeys(0),
    mKey(-1),
    mActiveVoices(0)

{
  TRACE;

  mTable = new double[TABLE_SIZE];

  for (int i = 0; i < TABLE_SIZE; i++)
  {
    mTable[i] = sin( i/double(TABLE_SIZE) * 2. * M_PI);
    //printf("mTable[%i] %f\n", i, mTable[i]);
  }

  mOsc = new CWTOsc(mTable, TABLE_SIZE);
  mEnv = new CADSREnvL();

  memset(mKeyStatus, 0, 128 * sizeof(bool));

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachBackground(BG_ID, BG_FN);

  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);
  IBitmap algKnob = pGraphics->LoadIBitmap(KNOB_ALG_ID, KNOB_ALG_FN, kAlgFrames);
  IBitmap regular = pGraphics->LoadIBitmap(WHITE_KEY_ID, WHITE_KEY_FN, 6);
  IBitmap sharp   = pGraphics->LoadIBitmap(BLACK_KEY_ID, BLACK_KEY_FN);

  IText text[kNumParams];

  for (int i = 0; i < kNumParams; ++i) {
      text[i] = IText(12, &COLOR_WHITE, "Tahoma",
          IText::kStyleNormal, IText::kAlignCenter, 0,
          IText::kQualityDefault);
      IBitmap* show = &knob;//default
      switch (kTypes[i]) {
      case IParam::kTypeDouble:
          switch (kUses[i]) {
          case uTime:
              GetParam(i)->InitDouble(kNames[i],
                  (TIME_MIN + TIME_MAX) / 2., TIME_MIN, TIME_MAX, 0.001, "ms");
              break;
          case uPercent:
              GetParam(i)->InitDouble(kNames[i],
                  50., 0., 100., 0.001, "%");
              break;
          case uAlgorithm:
              show = &algKnob;//different knob
              GetParam(i)->InitInt(kNames[i],
                  0, 0, kKnobFrames - 1, "Alg");
              break;
          default:
              break;
          }
          pGraphics->AttachControl(
              new IKnobMultiControl(this, getX(i), getY(i), i, show));
          break;
      default:
          break;
      }
      IRECT rect(getX(i) - 8, getY(i) + 35, getX(i) + 40, getY(i) + 100);
      pGraphics->AttachControl(
          new ITextControl(this, rect, &text[i], kNames[i]));
  }

  //                    C#     D#          F#      G#      A#
  int coords[12] = { 0, 7, 12, 20, 24, 36, 43, 48, 56, 60, 69, 72 };
  mKeyboard = new IKeyboardControl(this, kKeybX, kKeybY, 48, 5, &regular, &sharp, coords);

  pGraphics->AttachControl(mKeyboard);

  IBitmap about = pGraphics->LoadIBitmap(ABOUTBOX_ID, ABOUTBOX_FN);
  mAboutBox = new IBitmapOverlayControl(this, 100, 100, &about, IRECT(540, 250, 680, 290));
  pGraphics->AttachControl(mAboutBox);
  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);
}

IPlugPolySynth::~IPlugPolySynth()
{
  delete mOsc;
  delete mEnv;
  delete [] mTable;
}

int IPlugPolySynth::FindFreeVoice()
{
  int v;

  for(v = 0; v < MAX_VOICES; v++)
  {
    if(!mVS[v].GetBusy())
      return v;
  }

  int quietestVoice = 0;
  double level = 2.;

  for(v = 0; v < MAX_VOICES; v++)
  {
    double summed = mVS[v].mEnv_ctx.mPrev;

    if (summed < level)
    {
      level = summed;
      quietestVoice = v;
    }

  }

  DBGMSG("stealing voice %i\n", quietestVoice);
  return quietestVoice;
}

void IPlugPolySynth::NoteOnOff(IMidiMsg* pMsg)
{
  int v;

  int status = pMsg->StatusMsg();
  int velocity = pMsg->Velocity();
  int note = pMsg->NoteNumber();

  if (status == IMidiMsg::kNoteOn && velocity) // Note on
  {
    v = FindFreeVoice(); // or quietest
    mVS[v].mKey = note;
    mVS[v].mOsc_ctx.mPhaseIncr = (1./mSampleRate) * midi2CPS(note);
    mVS[v].mEnv_ctx.mLevel = (double) velocity / 127.;
    mVS[v].mEnv_ctx.mStage = kStageAttack;

    mActiveVoices++;
  }
  else  // Note off
  {
    for (v = 0; v < MAX_VOICES; v++)
    {
      if (mVS[v].mKey == note)
      {
        if (mVS[v].GetBusy())
        {
          mVS[v].mKey = -1;
          mVS[v].mEnv_ctx.mStage = kStageRelease;
          mVS[v].mEnv_ctx.mReleaseLevel = mVS[v].mEnv_ctx.mPrev;

          return;
        }
      }
    }
  }

}

void IPlugPolySynth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us
  IKeyboardControl* pKeyboard = (IKeyboardControl*) mKeyboard;

  if (pKeyboard->GetKey() != mKey)
  {
    IMidiMsg msg;

    if (mKey >= 0)
    {
      msg.MakeNoteOffMsg(mKey + 48, 0, 0);
      mMidiQueue.Add(&msg);
    }

    mKey = pKeyboard->GetKey();

    if (mKey >= 0)
    {
      msg.MakeNoteOnMsg(mKey + 48, pKeyboard->GetVelocity(), 0, 0);
      mMidiQueue.Add(&msg);
    }
  }

  if (mActiveVoices > 0 || !mMidiQueue.Empty()) // block not empty
  {
    double* out1 = outputs[0];
    double* out2 = outputs[1];

    double output;
    CVoiceState* vs;

    for (int s = 0; s < nFrames; ++s)
    {
      while (!mMidiQueue.Empty())
      {
        IMidiMsg* pMsg = mMidiQueue.Peek();

        if (pMsg->mOffset > s) break;

        int status = pMsg->StatusMsg(); // get the MIDI status byte

        switch (status)
        {
          case IMidiMsg::kNoteOn:
          case IMidiMsg::kNoteOff:
          {
            NoteOnOff(pMsg);
            break;
          }
          case IMidiMsg::kPitchWheel:
          {
            //TODO
            break;
          }
        }

        mMidiQueue.Remove();
      }

      output = 0.;

      for(int v = 0; v < MAX_VOICES; v++) // for each vs
      {
        vs = &mVS[v];

        if (vs->GetBusy())
        {
            output += mOsc->process(&vs->mOsc_ctx) * mEnv->process(&vs->mEnv_ctx);
        }
      }

      output *= GAIN_FACTOR;

      *out1++ = output;
      *out2++ = output;
    }

    mMidiQueue.Flush(nFrames);
  }
}

void IPlugPolySynth::Reset()
{
  TRACE;
  IMutexLock lock(this);

  mSampleRate = GetSampleRate();
  mMidiQueue.Resize(GetBlockSize());
  mEnv->setSampleRate(mSampleRate);
}

void IPlugPolySynth::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);

  switch (paramIdx)
  {
    case kAttack:
      mEnv->setStageTime(kStageAttack, GetParam(kAttack)->Value());
      break;
    case kDecay:
      mEnv->setStageTime(kStageDecay, GetParam(kDecay)->Value());
      break;
    case kSustain:
      mEnv->setSustainLevel(GetParam(kSustain)->Value() / 100.);
      break;
    case kRelease:
      mEnv->setStageTime(kStageRelease, GetParam(kRelease)->Value());
      break;
    default:
      break;
  }
}

void IPlugPolySynth::ProcessMidiMsg(IMidiMsg* pMsg)
{
  int status = pMsg->StatusMsg();
  int velocity = pMsg->Velocity();
  
  switch (status)
  {
    case IMidiMsg::kNoteOn:
    case IMidiMsg::kNoteOff:
      // filter only note messages
      if (status == IMidiMsg::kNoteOn && velocity)
      {
        mKeyStatus[pMsg->NoteNumber()] = true;
        mNumHeldKeys += 1;
      }
      else
      {
        mKeyStatus[pMsg->NoteNumber()] = false;
        mNumHeldKeys -= 1;
      }
      break;
    default:
      return; // if !note message, nothing gets added to the queue
  }
  

  mKeyboard->SetDirty();
  mMidiQueue.Add(pMsg);
}

// Should return non-zero if one or more keys are playing.
int IPlugPolySynth::GetNumKeys()
{
  IMutexLock lock(this);
  return mNumHeldKeys;
}

// Should return true if the specified key is playing.
bool IPlugPolySynth::GetKeyStatus(int key)
{
  IMutexLock lock(this);
  return mKeyStatus[key];
}

//Called by the standalone wrapper if someone clicks about
bool IPlugPolySynth::HostRequestingAboutBox()
{
  IMutexLock lock(this);
  if(GetGUI())
  {
    // get the IBitmapOverlay to show
    mAboutBox->SetValueFromPlug(1.);
    mAboutBox->Hide(false);
  }
  return true;
}
