#include "sfx/openal/sfxALInputDevice.h"
#include "platform/async/asyncUpdate.h"

SFXALInputDevice::SFXALInputDevice(SFXProvider *provider, const OPENALFNTABLE &openal, String deviceName)
   : Parent(provider, deviceName),
      mOpenAL(openal),
      mChannels(1),
      mActive(false),
      mBits(16)
{
   /// format for speex narrowband, this will probably need changed for opus.
   /// this is possible to set dynamically but for testing not necessary.
   mCaptureDevice = mOpenAL.alcCaptureOpenDevice(deviceName, 8000, AL_FORMAT_MONO16, 1024);

   Con::printf("Input Device: %s", mOpenAL.alcGetString(
      mCaptureDevice, ALC_CAPTURE_DEVICE_SPECIFIER));

   /// keeping track of openal framesize, for reasons.... :-P
   mFrameSize = mChannels * mBits / 8;

   Con::printf("Frame Size: %d", mFrameSize);

}

SFXALInputDevice::~SFXALInputDevice()
{

   mOpenAL.alcCaptureCloseDevice(mCaptureDevice);

}

void SFXALInputDevice::startRecording()
{

   if (mCaptureDevice && !mActive)
   {
      mOpenAL.alcCaptureStart(mCaptureDevice);
      mActive = true;
   }

}

void SFXALInputDevice::stopRecording()
{

   if (mCaptureDevice && mActive)
   {
      mOpenAL.alcCaptureStop(mCaptureDevice);
      mActive = false;
   }

}

U32 SFXALInputDevice::sampleCount()
{

   U32 retVal = 0;
   

   if (mCaptureDevice)
   {
      ALint samples = 0;
      mOpenAL.alcGetIntegerv(mCaptureDevice, ALC_CAPTURE_SAMPLES, sizeof(samples), &samples);
      retVal = (U32)samples;
   }

   return retVal;
   
}

void SFXALInputDevice::receiveSamples(U32 samples, char *buffer)
{

   if (mCaptureDevice)
      mOpenAL.alcCaptureSamples(mCaptureDevice, buffer, samples);

}

void SFXALInputDevice::playRawStream(U32 samples, U32 rate, const char *data)
{
   ALuint buffer;
   ALuint format = AL_FORMAT_MONO16;
   ALuint source = 0;
   ALenum state;

   mOpenAL.alGenBuffers(1, &buffer);
   mOpenAL.alBufferData(buffer, format, (ALvoid*)data, (samples * 2 * 1), rate);
   mOpenAL.alGenSources(1, &source);

   mOpenAL.alSourceQueueBuffers(source, 1, &buffer);

   mOpenAL.alSourcePlay(source);

   do {
      mOpenAL.alGetSourcei(source, AL_SOURCE_STATE, &state);
   } while (state == AL_PLAYING);

   mOpenAL.alDeleteSources(1, &source);
   mOpenAL.alDeleteBuffers(1, &buffer);

}
