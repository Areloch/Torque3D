#include "platform/platform.h"
#include "T3D/gameBase/VoipClient.h"
#include "T3D/gameBase/voipEvent.h"
#include "T3D/gameBase/gameConnection.h"
#include "platform/threads/threadPool.h"

#include <speex/speex.h>

IMPLEMENT_CONOBJECT(VoipClient);

ConsoleDocClass(VoipClient,
   "@brief A Voip client class for voip functions"
);

DefineEngineMethod(VoipClient, startRecording, void, (), ,
   "@brief Start recording the mic.\n\n")
{
   object->clientWriteVoip();
}

DefineEngineMethod(VoipClient, stopRecording, void, (), ,
   "@brief Stop recording the mic.\n\n")
{
   object->stopRecordingVoip();
}

VoipClient::VoipClient()
{
   /// create encoder in narrowband mode, the settings in openal input.
   speexEncoder = speex_encoder_init(&speex_nb_mode);
   /// set encoder quality to 8 (15kbps)
   U32 quality = 8;
   speex_encoder_ctl(speexEncoder, SPEEX_SET_QUALITY, &quality);

   /// after quality is set get sample rate and frame size.
   speex_encoder_ctl(speexEncoder, SPEEX_GET_FRAME_SIZE, &frameSize);
   speex_encoder_ctl(speexEncoder, SPEEX_GET_SAMPLING_RATE, &sampleRate);

   speexDecoder = speex_decoder_init(&speex_nb_mode);

   U32 en = 1;
   speex_encoder_ctl(speexDecoder, SPEEX_SET_ENH, &en);

   /// need this for quick access to samples.
   mClientDev = SFX->getInputDevice();

   Con::printf("VoipClient Created");

}

VoipClient::~VoipClient()
{

   /// stop any recording just in case.
   /// this needs to check if recording is happening.
   mClientDev->stopRecording();
   /// shutdown speex encoder
   speex_encoder_destroy(speexEncoder);
   speex_bits_destroy(&encoderBits);
   /// shutdown speex decoder
   speex_decoder_destroy(speexDecoder);
   speex_bits_destroy(&decoderBits);

}

void VoipClient::startRecordingVoip()
{
   if (mClientDev)
   {
      mClientDev->startRecording();
   }
}

void VoipClient::stopRecordingVoip()
{
   if (mClientDev)
   {
      mClientDev->stopRecording();
   }
}


void VoipClient::clientWriteVoip()
{
   voiceCompress(speexEncoder, frameSize, encoderBits, mClientDev, mConnection);
}

void VoipClient::clientReadVoip(const char *data, U32 frames, U32 length)
{
   voiceDeCompress(speexDecoder, frames, sampleRate, length, decoderBits, mClientDev, data);
}

void VoipClient::CompressJob::rawCompress(void* speexEncoder, U32 frameSize, SpeexBits encoderBits, SFXInputDevice *clientDev, GameConnection *conn)
{
   //if (isServerObject()) return;
   /// create our data buffers.
   Con::printf("write voip");
   char enBuf[1024];
   dMemset(enBuf, 0, 1024);
   static S16 buff[2048];
   dMemset(buff, 0, 2048);
   /// get our sample count.
   U32 samples = mClientDev->sampleCount();
   U32 pos = 0;
   U32 bPos = 0;

   /// 12 samples should = 240ms
   if (samples > (frameSize * 12))
      samples = (frameSize * 12);

   samples -= samples % frameSize;

   Con::printf("Samples: %d", samples);

   mClientDev->receiveSamples(samples, (char *)&buff);

   speex_bits_init(&encoderBits);
   while (samples > 0)
   {
      S16 *sampPtr = &buff[pos];
      U32 len;
      speex_bits_reset(&encoderBits);
      /// we have to encode as ints.
      speex_encode_int(speexEncoder, sampPtr, &encoderBits);
      /// outputs the length of the encoding.
      len = speex_bits_write(&encoderBits, enBuf, sizeof(enBuf));

      mConnection->postNetEvent(new VoipEvent(enBuf, frameSize, sizeof(enBuf)));

      bPos += len + 1;
      pos += frameSize;
      samples -= frameSize;

   }

}

void VoipClient::DeCompressJob::rawDeCompress(void* speexDecoder, U32 frames, U32 sampleRate, U32 length, SpeexBits decoderBits, SFXInputDevice *clientDev, const char *data)
{
   /// this codeblock is eventually going to hold the sender id aswell.

   S16 decode[4096];
   char encoded[1024];
   for (U32 i = 0; i < length; i++)
      encoded[i] = data[i];

   speex_bits_init(&decoderBits);

   speex_bits_reset(&decoderBits);
   speex_bits_read_from(&decoderBits, encoded, length);
   speex_decode_int(speexDecoder, &decoderBits, decode);

   clientDev->playRawStream(frames, sampleRate, (const char*)decode);
}
