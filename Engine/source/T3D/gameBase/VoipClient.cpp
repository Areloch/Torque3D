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
   U32 quality = 0;
   U32 complex = 10;
   speex_encoder_ctl(speexEncoder, SPEEX_SET_QUALITY, &quality);
   speex_encoder_ctl(speexEncoder, SPEEX_SET_COMPLEXITY, &complex);

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
   voiceDeCompress(speexDecoder, frames, sampleRate, length, decoderBits, mClientDev, mConnection, data);
}

void VoipClient::CompressJob::rawCompress(void* speexEncoder, U32 frameSize, SpeexBits encoderBits, SFXInputDevice *clientDev, GameConnection *conn)
{
   //if (isServerObject()) return;
   /// create our data buffers.
   Con::printf("write voip");
   char enBuf[1024];
   char frameBuf[256];
   dMemset(enBuf, 0, 1024);
   S16 buff[2048];
   dMemset(buff, 0, 2048);
   /// get our sample count.
   U32 samples = clientDev->sampleCount();
   U32 pos = 0;
   U32 bPos = 0;
   U32 frames = 0;
   /// 12 samples should = 240ms
   if (samples > (frameSize * 4))
      samples = (frameSize * 4);

   samples -= samples % frameSize;

   Con::printf("Samples: %d", samples);

   clientDev->receiveSamples(samples, (char *)&buff);

   speex_bits_init(&encoderBits);
   speex_bits_reset(&encoderBits);
   while (samples > 0)
   {
      S16 *sampPtr = &buff[pos];
      S8 len = 0;
      /// we have to encode as ints.
      speex_encode_int(speexEncoder, sampPtr, &encoderBits);
      /// outputs the length of the encoding.
      len = speex_bits_write(&encoderBits, frameBuf, sizeof(frameBuf));
      AssertFatal(len > 0 && len < 256, avar("invalid length: %i", len));
      for(U32 i = 0; i < len; i++)
         enBuf[bPos + i] = frameBuf[i];
      bPos += len +1;
      pos += frameSize;
      samples -= frameSize;
      frames++;
   }

   //getNextPow2(bPos);

   ThreadPool* pThreadPool = &ThreadPool::GLOBAL();
   VoipEvent *ev = new VoipEvent(enBuf, frames, bPos);
   ThreadSafeRef<CompressJobResult> item(new CompressJobResult(ev, conn));
   pThreadPool->queueWorkItemOnMainThread(item);

}

void VoipClient::DeCompressJob::rawDeCompress(void* speexDecoder, U32 frames, U32 sampleRate, U32 length, SpeexBits decoderBits, SFXInputDevice *clientDev, GameConnection *conn, const char *data)
{
   /// this codeblock is eventually going to hold the sender id aswell.

   S16 decode[4096];
   U32 outSize = 0;
   Con::printf("Data Received: %s Frames: %d Length: %d", data, frames, length);
   speex_bits_init(&decoderBits);
   speex_bits_reset(&decoderBits);
   for (U32 i = 0; i < frames; i++)
   {
      speex_bits_read_from(&decoderBits, data, length);
      speex_decode_int(speexDecoder, &decoderBits, decode + outSize);
      outSize += 160;
   }

   clientDev->playRawStream(outSize, sampleRate, (const char*)decode);

   ThreadPool* pThreadPool = &ThreadPool::GLOBAL();
   ThreadSafeRef<DeCompressJobResult> item(new DeCompressJobResult(clientDev, conn));
   pThreadPool->queueWorkItemOnMainThread(item);

}

void VoipClient::CompressJobResult::execute()
{
   mConn->postNetEvent(mEvent);
}

void VoipClient::DeCompressJobResult::execute()
{
   if (mDev->isActive())
   {
      mConn->getVoipClient()->clientWriteVoip();
   }
   else if (mDev->sampleCount() > 160)
   {
      mConn->getVoipClient()->clientWriteVoip();
   }
   else
      return;
}
