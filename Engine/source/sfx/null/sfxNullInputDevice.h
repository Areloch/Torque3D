#ifndef _SFXNULLINPUTDEVICE_H_
#define _SFXNULLINPUTDEVICE_H_

class SFXProvider;

#ifndef _SFXINPUTDEVICE_H_
#include "sfx/sfxInputDevice.h"
#endif

#ifndef _SFXPROVIDER_H_
#include "sfx/sfxProvider.h"
#endif

class SFXNULLInputDevice : public SFXInputDevice
{

public:

   typedef SFXInputDevice Parent;

   SFXNULLInputDevice(SFXProvider * provider, const String deviceName);
   virtual ~SFXNULLInputDevice();

protected:

public:

   virtual void startRecording() {};

   virtual void stopRecording() {};

   virtual S32 sampleCount() { return 0; }

   virtual void receiveSamples(U32 samples, char *buffer) {};

   virtual U32 getFrameSize() { return 0; }

   virtual void playRawStream(U32 samples, U32 rate, const char *data) {};

};

#endif
