#include "sfx/openal/sfxALInputDevice.h"
#include "platform/async/asyncUpdate.h"

SFXALInputDevice::SFXALInputDevice(SFXProvider *provider, const OPENALFNTABLE &openal, String deviceName)
   : Parent(provider, deviceName),
      mOpenAL(openal),
      mChannels(1),
      mBits(16)
{
   /// format for speex narrowband, this will probably need changed for opus.
   /// this is possible to set dynamically but for testing not necessary.
   mCaptureDevice = mOpenAL.alcCaptureOpenDevice(deviceName, 8000, AL_FORMAT_MONO16, 4096);

   Con::printf("Input Device: %s", mOpenAL.alcGetString(
      mCaptureDevice, ALC_CAPTURE_DEVICE_SPECIFIER));

   /// keeping track of openal framesize, for reasons.... :-P
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

S32 SFXALInputDevice::sampleCount()
{

   ALint samples = 0;

   if(mCaptureDevice)
      mOpenAL.alcGetIntegerv(mCaptureDevice, ALC_CAPTURE_SAMPLES, sizeof(samples), &samples);

   return (S32)samples;
   
}

void SFXALInputDevice::receiveSamples(U32 samples, void *buffer)
{

   if (mCaptureDevice)
      mOpenAL.alcCaptureSamples(mCaptureDevice, buffer, samples);

}

void SFXALInputDevice::playRawStream(U32 samples,U32 rate, const void *data)
{
   ALuint buffer;
   ALuint format = AL_FORMAT_MONO16;
   ALuint source = 0;
   ALenum state;

   mOpenAL.alGenBuffers(1, &buffer);
   mOpenAL.alBufferData(buffer, format, (ALvoid*)data, (samples * 2 * 1), rate);
   mOpenAL.alGenSources(1, &source);
   mOpenAL.alSourcei(source, AL_BUFFER, buffer);

   mOpenAL.alSourcePlay(source);

   do {
      mOpenAL.alGetSourcei(source, AL_SOURCE_STATE, &state);
   } while (state == AL_PLAYING);

   mOpenAL.alDeleteSources(1, &source);
   mOpenAL.alDeleteBuffers(1, &buffer);

}
