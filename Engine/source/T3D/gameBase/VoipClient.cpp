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

   speex_bits_init(&encoderBits);
   /// create encoder in narrowband mode, the settings in openal input.
   speexEncoder = speex_encoder_init(&speex_nb_mode);
   /// set encoder quality to 8 (15kbps)
   U32 quality = 8;
   U32 complex = 10;
   U32 vbr = 0;
   speex_encoder_ctl(speexEncoder, SPEEX_SET_QUALITY, &quality);
   speex_encoder_ctl(speexEncoder, SPEEX_SET_COMPLEXITY, &complex);
   speex_encoder_ctl(speexEncoder, SPEEX_SET_VBR, &vbr);

   /// after quality is set get sample rate and frame size.
   speex_encoder_ctl(speexEncoder, SPEEX_GET_FRAME_SIZE, &frameSize);
   speex_encoder_ctl(speexEncoder, SPEEX_GET_SAMPLING_RATE, &sampleRate);

   speex_bits_init(&decoderBits);
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
   /// create our data buffers.
   char enBuf[160];
   dMemset(enBuf, 0, 160);
   S16 buff[480];
   dMemset(buff, 0, 480);
   U32 pos = 0;
   /// get our sample count.
   U32 samples = clientDev->sampleCount();
   /// 3 samples should = 60ms
   if (samples > (frameSize * 3))
      samples = (frameSize * 3);

   samples -= samples % frameSize;

   clientDev->receiveSamples(samples, (char *)&buff);

   
   while (samples > 0)
   {
      S16 *sampPtr = &buff[pos];
      U32 len = 0;
      speex_bits_reset(&encoderBits);
      speex_encode_int(speexEncoder, sampPtr, &encoderBits);

      len = speex_bits_write(&encoderBits, enBuf, 160);

      ThreadPool* pThreadPool = &ThreadPool::GLOBAL();
      VoipEvent *ev = new VoipEvent(enBuf, len, sizeof(enBuf));
      ThreadSafeRef<CompressJobResult> item(new CompressJobResult(ev, conn));
      pThreadPool->queueWorkItemOnMainThread(item);

      samples -= frameSize;
      pos += frameSize;
   }

}

void VoipClient::DeCompressJob::rawDeCompress(void* speexDecoder, U32 frames, U32 sampleRate, U32 length, SpeexBits decoderBits, SFXInputDevice *clientDev, GameConnection *conn, const char *data)
{
   /// this codeblock is eventually going to hold the sender id aswell.
   S16 decode[160];
   speex_bits_reset(&decoderBits);
   speex_bits_read_from(&decoderBits, data, frames);

   speex_decode_int(speexDecoder, &decoderBits, decode);

   clientDev->playRawStream(frames, sampleRate, (const char*)decode);

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
