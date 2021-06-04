#include "sfx/openal/sfxALInputDevice.h"
#include "platform/async/asyncUpdate.h"

SFXALInputDevice::SFXALInputDevice(SFXProvider *provider, const OPENALFNTABLE &openal, String deviceName)
   : Parent(provider, deviceName),
      mOpenAL(openal),
      mChannels(1),
      mBits(16)
{
   mCaptureDevice = mOpenAL.alcCaptureOpenDevice(deviceName, 48000, AL_FORMAT_MONO16, 1024);

   Con::printf("Input Device: %s", mOpenAL.alcGetString(
      mCaptureDevice, ALC_CAPTURE_DEVICE_SPECIFIER));

   mFrameSize = mChannels * mBits / 8;

}

SFXALInputDevice::~SFXALInputDevice()
{
   mOpenAL.alcCaptureCloseDevice(mCaptureDevice);
}

void SFXALInputDevice::startRecording()
{
   if(mCaptureDevice)
      mOpenAL.alcCaptureStart(mCaptureDevice);
}

void SFXALInputDevice::stopRecording()
{
   if (mCaptureDevice)
      mOpenAL.alcCaptureStop(mCaptureDevice);
}

U32 SFXALInputDevice::sampleCount()
{
   return 0;
}

void SFXALInputDevice::receiveSamples(const void * buffer)
{
}
