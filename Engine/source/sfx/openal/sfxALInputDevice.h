#ifndef _SFXALINPUTDEVICE_H_
#define _SFXALINPUTDEVICE_H_

class SFXProvider;

#ifndef _SFXINPUTDEVICE_H_
#include "sfx/sfxInputDevice.h"
#endif

#ifndef _SFXALVOICE_H_
#include "sfx/openal/sfxALVoice.h"
#endif

#ifndef _SFXPROVIDER_H_
#include "sfx/sfxProvider.h"
#endif

#ifndef _OPENALFNTABLE
#include "sfx/openal/LoadOAL.h"
#endif

class SFXALInputDevice : public SFXInputDevice
{

public:

   typedef SFXInputDevice Parent;

   SFXALInputDevice(SFXProvider *provider, const OPENALFNTABLE &openal, String deviceName);
   virtual ~SFXALInputDevice();

protected:
   OPENALFNTABLE mOpenAL;

   ALCdevice *mCaptureDevice;

   U32 mBits;
   U32 mChannels;
   U32 mFrameSize;

public:

   virtual void startRecording();

   virtual void stopRecording();

   virtual S32 sampleCount();

   virtual void receiveSamples(U32 samples, void *buffer);

   /// this is important to keep data in sync.
   virtual U32 getFrameSize() { return mFrameSize; }

   virtual void playRawStream(U32 samples, U32 rate, const void *data);

};

#endif
