#include "sfx/sfxInputDevice.h"
#include "sfx/sfxInternal.h"

/// This is a pretty empty class, most of it happens at the api level.
SFXInputDevice::SFXInputDevice(SFXProvider* provider , const String deviceName)
   :  mName(deviceName),
      mProvider(provider)
{
   AssertFatal(provider, "We must have a provider pointer on device creation!");

}

SFXInputDevice::~SFXInputDevice()
{
}
