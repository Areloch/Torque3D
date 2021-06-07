#include "platform/platform.h"
#include "T3D/gameBase/VoipClient.h"
#include "T3D/gameBase/voipEvent.h"

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
   /// speex initialization
   /// encoder bits can and probably should be set each time
   /// a recording starts but we need them ready.
   speex_bits_init(&encoderBits);
   speex_bits_reset(&encoderBits);
   /// create encoder in narrowband mode, the settings in openal input.
   speexEncoder = speex_encoder_init(&speex_nb_mode);
   /// set encoder quality to 8 (15kbps)
   U32 quality = 8;
   speex_encoder_ctl(speexEncoder, SPEEX_SET_QUALITY, &quality);

   /// after quality is set get sample rate and frame size.
   speex_encoder_ctl(speexEncoder, SPEEX_GET_FRAME_SIZE, &frameSize);
   speex_encoder_ctl(speexEncoder, SPEEX_GET_SAMPLING_RATE, &sampleRate);

   speex_bits_init(&decoderBits);
   speex_bits_reset(&decoderBits);
   speexDecoder = speex_decoder_init(&speex_nb_mode);

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
      mClientDev->stopRecording();
}


void VoipClient::clientWriteVoip()
{
   /// create our data buffers.
   Con::printf("write voip");
   char enBuf[1024];
   S16 buff[2048];
   /// position pointers
   U32 bPos = 0;
   U32 pos = 0;
   /// speex frame pointer.
   U32 frames = 0;
   /// get our sample count.
   U32 samples = mClientDev->sampleCount();

   /// 12 samples should = 240ms
   if (samples > (frameSize * 12))
      samples = (frameSize * 12);

   Con::printf("Samples: %d", samples);

   samples -= samples % frameSize;
   mClientDev->receiveSamples(samples, (void *) &buff);

   while (samples > 0)
   {
      S16 *sampPtr = &buff[pos];
      U32 len;
      /// reset bits for data.
      speex_bits_reset(&encoderBits);
      /// we have to encode as ints.
      speex_encode_int(speexEncoder, sampPtr, &encoderBits);
      /// outputs the length of the encoding.
      len = speex_bits_write(&encoderBits, &enBuf[bPos +1], sizeof(enBuf) - (bPos +1));
      Con::printf("Encode Buffer: %s", enBuf);

      bPos += len + 1;
      pos += frameSize;
      samples -= frameSize;
      frames++;
   }

   Con::printf("Length: %d Frames: %d", sizeof(enBuf), frames);

   mConnection->postNetEvent(new VoipEvent(enBuf, frames, sizeof(enBuf)));

}

void VoipClient::clientReadVoip(char *data, U32 frames, U32 length)
{
   /// this codeblock is eventually going to hold the sender id aswell.

   S16 decode[4096];
   S16 written = 0;

   speex_bits_reset(&decoderBits);

   for (U32 i = 0; i < frames; i++)
   {
      speex_bits_read_from(&decoderBits, data, length);
      speex_decode_int(speexDecoder, &decoderBits, (spx_int16_t*)decode + written);

      written += frameSize;
   }

   if (written > 0)
   {
      mClientDev->playRawStream(written, sampleRate, (const void*)decode);
   }

}
