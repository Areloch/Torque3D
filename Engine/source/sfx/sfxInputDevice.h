#ifndef _SFXINPUTDEVICE_H_
#define _SFXINPUTDEVICE_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _SFXCOMMON_H_
#include "sfx/sfxCommon.h"
#endif
#ifndef _THREADSAFEREF_H_
#include "platform/threads/threadSafeRefCount.h"
#endif

class SFXProvider;

class SFXInputDevice
{
public:

   typedef void Parent;


protected:

   SFXInputDevice(SFXProvider* provider, const String deviceName);

   /// The provider which created this device.
   SFXProvider* mProvider;

   /// The name of this device.
   String mName;

public:

   virtual ~SFXInputDevice();

   /// Returns the provider which created this device.
   SFXProvider* getProvider() const { return mProvider; }

   /// Returns the name of this device.
   const String getName() const { return mName; }

   virtual void startRecording() {};
   virtual void stopRecording() {};

   virtual U32 sampleCount() = 0;

   virtual void receiveSamples(const void* buffer) {};

   virtual U32 getFrameSize() = 0;

};

#endif
