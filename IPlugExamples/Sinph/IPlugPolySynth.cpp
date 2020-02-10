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

#define GAIN_FACTOR 0.1;

char* kNames[kNumParams] = {
    "Algorithm", "Modulation", "Noise", "Unison",
    "Foot", "Env Bend", "Polytouch", "Volume",
    "Balance", "Warm", "Pan Warm", "Expression",
    "FX1", "FX2", "?", "?",

    "F2", "Q2", "F1", "Invert",
    "Attack", "Decay", "Sustain", "Release",
    "Vel F2", "Vel Q2", "Vel F1", "Vel Invert",
    "Env F2", "Env Q2", "Env F1", "Env Invert",

    "Hold", "Porto", "Sustenuto", "Soft",
    "Legato", "Hold 2", "Variation", "Timbre",
    "Release 2", "Attack 2", "Brightness", "P6",
    "P7", "P8", "P9", "P10",

    //switches
    "S1", "S2", "S3", "S4",

    //Notes C# D# F# G# A# (for sliders)
    "C", "C#", "D", "D#",
    "E", "F", "F#", "G",
    "G#", "A", "A#", "B",

    //Upto the special controllers for parameter entry
    //And if used these will come later

    //Special at KNumProcessed
    "Write", "Channel"
};

enum EUses {
    uNul = 0,
    uNulSW,
    uNulSL,
    uEdit,//no write back
    uChan,//for midi?
    uTime,//double
    uPercent,//double
    uAlgorithm//int
};

EUses kUses[kNumParams] = {
    uAlgorithm, uNul, uNul, uNul,
    uNul, uNul, uNul, uNul,
    uNul, uNul, uNul, uNul,
    uNul, uNul, uNul, uNul,

    uNul, uNul, uNul, uNul,
    uTime, uTime, uPercent, uTime,
    uNul, uNul, uNul, uNul,
    uNul, uNul, uNul, uNul,

    //third block
    uNulSW, uNulSW, uNulSW, uNulSW,
    uNulSW, uNulSW, uNul, uNul,
    uNul, uNul, uNul, uNul,
    uNul, uNul, uNul, uNul,

    //switches
    uNulSW, uNulSW, uNulSW, uNulSW,

    //notes
    uNulSL, uNulSL, uNulSL, uNulSL,
    uNulSL, uNulSL, uNulSL, uNulSL,
    uNulSL, uNulSL, uNulSL, uNulSL,

    //"Dummies"
    uEdit, uChan
};

Algorithm sound[kAlgFrames] = {
    Algorithm(),//default

    Algorithm(), Algorithm(), Algorithm(),
    Algorithm(), Algorithm(), Algorithm(),
    Algorithm(), Algorithm(), Algorithm(),
    Algorithm(), Algorithm(), Algorithm(),
    Algorithm(), Algorithm(), Algorithm(),

    Algorithm(), Algorithm(), Algorithm(),
    Algorithm(), Algorithm(), Algorithm(),
    Algorithm(), Algorithm(), Algorithm(),
    Algorithm(), Algorithm(), Algorithm(),
    Algorithm(), Algorithm(), Algorithm()
};

int getX(int c) {
    if (c > 47) {
        if (c > 51) {
            c -= 52;
            return getX(c % 4 + 16 * (c / 4));//for sliders
        }
        return getX(c - 4);//for extra switches
    }
    return 55 * ((c % 4) + 4 * (c / 16)) + 16;
}

int getY(int c) {
    if (c > 47) {
        if (c > 51) {
            return getY(51) + 63;//for sliders
        }
        return getY(c - 4) + 60;//for extra switches
    }
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

  memset(mKeyStatus, 0, 128 * 16 * sizeof(bool));

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachBackground(BG_ID, BG_FN);

  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);
  IBitmap algKnob = pGraphics->LoadIBitmap(KNOB_ALG_ID, KNOB_ALG_FN, kAlgFrames);
  IBitmap switchKnob = pGraphics->LoadIBitmap(SWITCH_ID, SWITCH_FN, kSwitchFrames);
  IBitmap sliderKnob = pGraphics->LoadIBitmap(SLIDER_ID, SLIDER_FN, kSliderFrames);

  IBitmap regular = pGraphics->LoadIBitmap(WHITE_KEY_ID, WHITE_KEY_FN, 6);
  IBitmap sharp   = pGraphics->LoadIBitmap(BLACK_KEY_ID, BLACK_KEY_FN);

  IText text[kNumParams];

  for (int i = 0; i < kNumParams; ++i) {
      text[i] = IText(12, &COLOR_WHITE, "Tahoma",
          IText::kStyleNormal, IText::kAlignCenter, 0,
          IText::kQualityDefault);
      IBitmap* show = &knob;//default
      bool ok = true;
      bool voids = false;
      int yOff = 0;
      switch (kUses[i]) {
      case uTime:
          GetParam(i)->InitDouble(kNames[i],
              (TIME_MIN + TIME_MAX) / 2., TIME_MIN, TIME_MAX, 0.001, "ms");
          break;
      case uPercent:
          GetParam(i)->InitDouble(kNames[i],
              50., 0., 100., 0.001, "%");
          break;;
      case uAlgorithm:
          show = &algKnob;//different knob
          GetParam(i)->InitEnum(kNames[i],
              0, kKnobFrames, "Alg");
          break;
      case uNul:
          ok = false;
          GetParam(i)->InitEnum(kNames[i], 0, 1, "N/A");
          break;
      case uNulSW:
          ok = false;
          show = &switchKnob;
          GetParam(i)->InitEnum(kNames[i], 0, 16, "N/A");
          break;
      case uNulSL:
          ok = false;
          show = &sliderKnob;
          yOff = 65;
          GetParam(i)->InitEnum(kNames[i], 128, 256, "N/A");
          break;
          //MIDI select and edit
      case uEdit:
          voids = true;
          GetParam(i)->InitBool(kNames[i], false, "Allow");
          break;
      case uChan:
          voids = true;
          GetParam(i)->InitInt(kNames[i], 1, 1, 16, "Chan");
          break;
      default:
          break;
      }
      pGraphics->AttachControl(
          dials[i] = new IKnobMultiControl(this, getX(i), getY(i), i, show));
      if (!ok) {
          dials[i]->GrayOut(true);
      }
      if (!voids) {
          IRECT rect(getX(i) - 8, getY(i) + 35 + yOff,
              getX(i) + 40, getY(i) + 100 + yOff);
          pGraphics->AttachControl(
              labels[i] = new ITextControl(this, rect, &text[i], kNames[i]));
      }
      if(i < kNumProcessed)
          for (int j = 0; j < 16; ++j) {//start with defaults
              oldParam[j][i] = newParam[j][i] = GetParam(i)->Value();
              bender[j] = 1.;//initial here as GNU syntax [ ... ]
          }
      else {
          GetParam(i)->SetCanSave(false);
      }
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
  /* delete mKeyboard;
  for (int i = 0; i < kNumParams; ++i) {
      delete dials[i];
      delete labels[i];
  } */
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
  int chan = pMsg->Channel();

  if (status == IMidiMsg::kNoteOn && velocity) // Note on
  {
    v = FindFreeVoice(); // or quietest
    mVS[v].mKey = note;
    mVS[v].mOsc_ctx.mPhaseIncr = (1./mSampleRate) * midi2CPS(note);
    mVS[v].mEnv_ctx.mLevel = (double) velocity / 127.;
    mVS[v].mEnv_ctx.mStage = kStageAttack;
    mVS[v].chan = chan;//set channel

    mActiveVoices++;
  }
  else  // Note off
  {
    for (v = 0; v < MAX_VOICES; v++)
    {
      if (mVS[v].mKey == note && mVS[v].chan == chan)
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

double Algorithm::process(IPlugPolySynth *ref, CVoiceState* vs) {
    double env = ref->mEnv->process(&vs->mEnv_ctx,
        ref->oldParam[vs->chan]);
    return ref->mOsc->process(&vs->mOsc_ctx, env,
        ref->oldParam[vs->chan], ref->bender[vs->chan]) * env;
}

double Algorithm::makeLeft(double master) {
    return master;
}

double Algorithm::andMakeRight(double master) {
    return master;
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
      msg.MakeNoteOffMsg(mKey + 48, 0, currentChan);
      mMidiQueue.Add(&msg);
    }

    mKey = pKeyboard->GetKey();

    if (mKey >= 0)
    {
      msg.MakeNoteOnMsg(mKey + 48, pKeyboard->GetVelocity(), 0, currentChan);
      mMidiQueue.Add(&msg);
    }
  }

  if (mActiveVoices > 0 || !mMidiQueue.Empty()) // block not empty
  {
    double* out1 = outputs[0];
    double* out2 = outputs[1];

    double output[kAlgFrames] = { };
    CVoiceState* vs;
    for(int j = 0; j < 16; ++j) 
        for (int i = 0; i < kNumProcessed; ++i) {
            deltaParam[j][i] = (newParam[j][i] - oldParam[j][i]) / (double)nFrames;
        }

    for (int s = 0; s < nFrames; ++s)
    {
      while (!mMidiQueue.Empty())
      {
        IMidiMsg* pMsg = mMidiQueue.Peek();

        if (pMsg->mOffset > s) break;
        NoteOnOff(pMsg);
        mMidiQueue.Remove();
      }

      for(int v = 0; v < MAX_VOICES; v++) // for each vs
      {
        vs = &mVS[v];

        if (vs->GetBusy())
        {
            int alg = oldParam[vs->chan][kAlgSelect];
            output[alg] += sound[alg].process(this, vs);
        }
      }

      *out1 = 0.;
      *out2 = 0.;

      for (int i = 0; i < kAlgFrames; ++i) {
          *out1 += sound[i].makeLeft(output[i]);
          *out2 += sound[i].andMakeRight(output[i]);
      }

      *out1++ *= GAIN_FACTOR;
      *out2++ *= GAIN_FACTOR;

      for (int j = 0; j < 16; ++j)
          for (int i = 0; i < kNumProcessed; ++i) {
              oldParam[j][i]  += deltaParam[j][i];
          }
    }

    mMidiQueue.Flush(nFrames);
  }

  for (int j = 0; j < 16; ++j)
      for (int i = 0; i < kNumProcessed; ++i) {
          oldParam[j][i] = newParam[j][i];
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
  double* param = &newParam[currentChan][paramIdx];

  switch (kUses[paramIdx]) {
  case uTime:
      *param = GetParam(paramIdx)->Value();
      break;
  case uPercent:
      *param = GetParam(paramIdx)->Value() / 100.;
      break;
  case uAlgorithm:
      *param = GetParam(paramIdx)->Int();
      break;
  case uNul:
      *param = 0.;
      break;
  case uEdit://not controllable
  case uChan:
  default:
      break;
  }


  //DELETE LATER TODO:--
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
  IMidiMsg::EControlChangeMsg idx = pMsg->ControlChangeIdx();
  int chan = pMsg->Channel();
  double val;
  
  switch (status)
  {
  case IMidiMsg::kNoteOn:
  case IMidiMsg::kNoteOff:
      // filter only note messages
      if (status == IMidiMsg::kNoteOn && velocity)
      {
          mKeyStatus[chan][pMsg->NoteNumber()] = true;
          mNumHeldKeys += 1;
      }
      else
      {
          mKeyStatus[chan][pMsg->NoteNumber()] = false;
          mNumHeldKeys -= 1;
      }
      break;//only add notes
  case IMidiMsg::kControlChange:
      //process control change
      if (idx < kNumProcessed) {
          if (idx < 32) {
              val = GetParam(idx)->Value();
              GetParam(idx)->SetNormalized(pMsg->ControlChange(idx));
          }
          else if (idx < 64) {
              idx = (IMidiMsg::EControlChangeMsg)(idx & 31);
              val = GetParam(idx)->Value();
              double current = GetParam(idx)->GetNormalized();
              current += pMsg->ControlChange(idx) / 128.;
              GetParam(idx)->SetNormalized(current);//14 bit controllers
          }
          else {
              idx = (IMidiMsg::EControlChangeMsg)(idx - 16);//to control
              val = GetParam(idx)->Value();
              GetParam(idx)->SetNormalized(pMsg->ControlChange(idx));
              //Seem to be outdated by 14 bit use??
              //6 on/off [64]-[69]
              //10 general [70]-[79]
              //4 buttons [80]-[83]
              //Who knows the 12 notes? [84]-[95]
              /* UPTO HERE <--------------
                Plus 2 extras for MIDI chan and edit
              ------------------------> */

              //Data entry and param selects [96]-[101] -- SPECIAL USE CASES
              //Unassigned controllers [102]-[119]
              //Specials [120]-[128] -- N.B. DO NOT USE!!!
          }
          newParam[chan][idx] = GetParam(idx)->Value();
          if (chan == currentChan) {
              dials[idx]->SetDirty();
              InformHostOfParamChange(idx, GetParam(idx)->GetNormalized());
              RedrawParamControls();
          }
          else {
              GetParam(idx)->Set(val);//restore
          }
      }
      return;
  case IMidiMsg::kPitchWheel:
      //frequency multiplier
      bender[chan] = pow(semitone, 2.0 * pMsg->PitchWheel());
      return;
  case IMidiMsg::kPolyAftertouch:
      //pMsg->PolyAfterTouch();
      //pMsg->NoteNumber();
      return;
  case IMidiMsg::kChannelAftertouch:
      //pMsg->ChannelAfterTouch();
      return;
  case IMidiMsg::kProgramChange:
      if (hackEdit) {
          RestorePreset(programs[chan]);
          ModifyCurrentPreset();//save current
      }
      RestorePreset(programs[chan] = pMsg->Program());//just in case
      for (int i = 0; i < kNumProcessed; ++i) {
          newParam[chan][i] = GetParam(i)->Value();
      }
      for (int i = 0; i < kNumProcessed; ++i) {
          GetParam(i)->Set(newParam[currentChan][i]);
      }
      DirtyParameters();
      RedrawParamControls();
      return;
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
  return mKeyStatus[currentChan][key];
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
