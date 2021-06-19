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
   speex_encoder_ctl(speexEncoder, SPEEX_SET_QUALITY, &quality);

   /// after quality is set get sample rate and frame size.
   speex_encoder_ctl(speexEncoder, SPEEX_GET_FRAME_SIZE, &frameSize);
   speex_encoder_ctl(speexEncoder, SPEEX_GET_SAMPLING_RATE, &sampleRate);

   speex_bits_init(&decoderBits);
   speexDecoder = speex_decoder_init(&speex_nb_mode);

   U32 en = 1;
   speex_decoder_ctl(speexDecoder, SPEEX_SET_ENH, &en);

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
   U32 bufflen = frameSize*8;
   /// create our data buffers.
   char *enBuf = (char*)dMalloc(bufflen);
   dMemset(enBuf, 0, bufflen);
   S16 *buff = (S16*)dMalloc(bufflen*2);
   dMemset(buff, 0, bufflen*2);
   U32 pos = 0;
   U32 frames = 0;
   U32 nbBytes = 0;
   /// get our sample count.
   U32 samples = clientDev->sampleCount();
   /// 4 samples should = 80ms
   if (samples > (bufflen))
      samples = (bufflen);

   samples -= samples % frameSize;

   clientDev->receiveSamples(samples, buff);
   
   while (samples > 0)
   {
      S16 *sampPtr = &buff[pos];
      U16 len = 0;
      speex_bits_reset(&encoderBits);
      speex_encode_int(speexEncoder, sampPtr, &encoderBits);
      len = speex_bits_write(&encoderBits, &enBuf[nbBytes+1], bufflen - (nbBytes+1));
      Con::printf("Length Encoded: %d", len);
      enBuf[nbBytes] = (char)len;
      nbBytes += len + 1;
      samples -= frameSize;
      pos += frameSize;
      frames++;
   }

   ThreadPool* pThreadPool = &ThreadPool::GLOBAL();
   if (nbBytes < 1)
   {
      ThreadSafeRef<DeCompressJobResult> item(new DeCompressJobResult(clientDev, conn));
      pThreadPool->queueWorkItemOnMainThread(item);
   }
   else
   {

      VoipEvent *ev = new VoipEvent(enBuf, frames, nbBytes + 1);
      ThreadSafeRef<CompressJobResult> item(new CompressJobResult(ev, conn));
      pThreadPool->queueWorkItemOnMainThread(item);
   }

   free(enBuf);
   free(buff);
}

void VoipClient::DeCompressJob::rawDeCompress(void* speexDecoder, U32 frames, U32 sampleRate, U32 length, SpeexBits decoderBits, SFXInputDevice *clientDev, GameConnection *conn, const char *data)
{
   /// this codeblock is eventually going to hold the sender id aswell.
   /// no data escape
   ThreadPool* pThreadPool = &ThreadPool::GLOBAL();
   if (length < 1)
   {
      
      ThreadSafeRef<DeCompressJobResult> item(new DeCompressJobResult(clientDev, conn));
      pThreadPool->queueWorkItemOnMainThread(item);
   }
   else
   {
      U32 fSize;
      U32 written = 0;
      U16 pos = 0;
      //enough for each frame.
      speex_decoder_ctl(speexDecoder, SPEEX_GET_FRAME_SIZE, &fSize);
      U32 bufferLen = fSize * frames;
      S16* decode = (S16*)dMalloc(bufferLen * 2);
      dMemset(decode, 0, bufferLen * 2);
      Con::printf("Frames: %d", frames);
      for (U32 i = 0; i < frames; i++)
      {
         speex_bits_reset(&decoderBits);
         Con::printf("Length Decoded: %d", data[pos]);
         speex_bits_read_from(&decoderBits, &data[pos+1], data[pos]);
         speex_decode_int(speexDecoder, &decoderBits, decode + written);
         pos += data[pos] + 1;
         written += fSize;
      }
      clientDev->playRawStream(written, sampleRate, (const char*)decode);
      ThreadSafeRef<DeCompressJobResult> item(new DeCompressJobResult(clientDev, conn));
      pThreadPool->queueWorkItemOnMainThread(item);
      free(decode);
   }
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
