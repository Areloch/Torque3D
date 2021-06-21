#include "platform/platform.h"
#include "T3D/gameBase/VoipClient.h"
#include "T3D/gameBase/voipEvent.h"
#include "T3D/gameBase/gameConnection.h"
#include "platform/threads/threadPool.h"

#include <opus.h>

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
   S32 err;
   opusEncoder = opus_encoder_create(8000, 1, OPUS_APPLICATION_VOIP, &err);
   if (err < 0)
   {
      Con::printf("Opus failed to create encoder: %s\n", opus_strerror(err));
   }

   sampleRate = 8000;
   frameSize = FRAME_SIZE;

   err = opus_encoder_ctl((OpusEncoder*)opusEncoder, OPUS_SET_BITRATE(16000));
   if (err < 0)
   {
      Con::printf("Opus failed to set bitrate: %s\n", opus_strerror(err));
   }

   opusDecoder = opus_decoder_create(8000, 1, &err);

   if(err<0)
   {
      Con::printf("Opus failed to create decoder: %s\n", opus_strerror(err));
   }

   /// need this for quick access to samples.
   mClientDev = SFX->getInputDevice();

   Con::printf("VoipClient Created");

}

VoipClient::~VoipClient()
{

   /// stop any recording just in case.
   /// this needs to check if recording is happening.
   mClientDev->stopRecording();
   /// shutdown opus
   opus_encoder_destroy((OpusEncoder*)opusEncoder);
   opus_decoder_destroy((OpusDecoder*)opusDecoder);

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
   voiceCompress(opusEncoder, frameSize, mClientDev, mConnection);
}

void VoipClient::clientReadVoip(U8 *data, U32 frames, U32 length)
{
   voiceDeCompress(opusDecoder, frames, sampleRate, length, mClientDev, mConnection, data);
}

void VoipClient::CompressJob::rawCompress(void* opsEncoder, U32 frameSize, SFXInputDevice *clientDev, GameConnection *conn)
{
   /// create our data buffers.
   U8 enBuf[MAX_FRAME_SIZE];
   S16 buff[MAX_FRAME_SIZE*2];
   U32 pos = 0;
   U32 frames = 0;
   U32 nbBytes = 0;
   /// get our sample count.
   U32 samples = clientDev->sampleCount();
   /// 4 samples should = 80ms
   if (samples > (MAX_FRAME_SIZE))
      samples = (MAX_FRAME_SIZE);

   samples -= samples % FRAME_SIZE;

   clientDev->receiveSamples(samples, buff);
   
   while (samples > 0)
   {
      S16 *sampPtr = &buff[pos];
      U16 len = 0;
      len = opus_encode((OpusEncoder*)opsEncoder, sampPtr, FRAME_SIZE, &enBuf[nbBytes+1], MAX_FRAME_SIZE - (nbBytes +1));
      Con::printf("Length Encoded: %d", len);
      enBuf[nbBytes] = (char)len;
      nbBytes += len + 1;
      samples -= FRAME_SIZE;
      pos += FRAME_SIZE;
      frames++;
   }

   ThreadPool* pThreadPool = &ThreadPool::GLOBAL();
   if (nbBytes < 1 || frames > 6 || nbBytes > MAX_FRAME_SIZE)
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

   //free(enBuf);
   //free(buff);
}

void VoipClient::DeCompressJob::rawDeCompress(void* opsDecoder, U32 frames, U32 sampleRate, U32 length, SFXInputDevice *clientDev, GameConnection *conn, U8 *data)
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
      U32 fSize = 0;
      U32 written = 0;
      U16 pos = 0;
      S16 decode[MAX_FRAME_SIZE * 2];
      Con::printf("Frames: %d", frames);
      for (U32 i = 0; i < frames; i++)
      {
         fSize = opus_decode((OpusDecoder*)opsDecoder, &data[pos + 1], data[pos], &decode[written], FRAME_SIZE, 0);
         Con::printf("Length Decoded: %d", data[pos]);
         pos += data[pos] + 1;
         written += fSize;
      }
      clientDev->playRawStream(written, sampleRate, (const char*)decode);
      ThreadSafeRef<DeCompressJobResult> item(new DeCompressJobResult(clientDev, conn));
      pThreadPool->queueWorkItemOnMainThread(item);
      //free(decode);
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
   else if (mDev->sampleCount() > FRAME_SIZE)
   {
      mConn->getVoipClient()->clientWriteVoip();
   }
   else
      return;
}
