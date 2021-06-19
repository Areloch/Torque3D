#ifndef _VOIPCLIENT_H_
#define _VOIPCLIENT_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#ifndef _GAMECONNECTION_H_
#include "T3D/gameBase/gameConnection.h"
#endif

#ifndef _VOIPEVENT_H_
#include "T3D/gameBase/voipEvent.h"
#endif // !_VOIPEVENT_H_


#ifndef _SFXSYSTEM_H_
#include "sfx/sfxSystem.h"
#endif // !_SFXSYSTEM_H_

#ifndef _SFXINPUTDEVICE_H_
#include "sfx/sfxInputDevice.h"
#endif // !_SFXINPUTDEVICE_H_

#include "platform/threads/threadPool.h"

#include <speex/speex.h>

class VoipEvent;
class GameConnection;

class VoipClient : public NetObject
{
protected:
   typedef NetObject Parent;

   struct CompressJobResult : public ThreadPool::WorkItem
   {
      VoipEvent* mEvent;
      GameConnection *mConn;
      CompressJobResult(VoipEvent *ev, GameConnection *conn) : mEvent(ev), mConn(conn) {}

      virtual void execute();
   };

   struct DeCompressJobResult : public ThreadPool::WorkItem
   {
      SFXInputDevice* mDev;
      GameConnection *mConn;
      DeCompressJobResult(SFXInputDevice *ev, GameConnection *conn) : mDev(ev), mConn(conn) {}

      virtual void execute();
   };

   struct CompressJob : public ThreadPool::WorkItem
   {
      void *mSpeexEncoder;
      U32 mFrameSize;
      SpeexBits mEncoderBits;
      SFXInputDevice *mClientDev;
      GameConnection *mConnection;

      CompressJob(void* spxEncoder, U32 frSize, SpeexBits encBits, SFXInputDevice *clientDev, GameConnection *conn)
         : mSpeexEncoder(spxEncoder), mFrameSize(frSize), mEncoderBits(encBits), mClientDev(clientDev), mConnection(conn) {}

   protected:
      virtual void execute()
      {
         rawCompress(mSpeexEncoder, mFrameSize, mEncoderBits, mClientDev, mConnection);
      }

      void rawCompress(void* spxEncoder, U32 frSize, SpeexBits encBits, SFXInputDevice *clientDev, GameConnection *conn);
   };

   void voiceCompress(void* spxEncoder, U32 frSize, SpeexBits encBits, SFXInputDevice *clientDev, GameConnection *conn)
   {
      ThreadPool* pThreadPool = &ThreadPool::GLOBAL();
      ThreadSafeRef<CompressJob> item(new CompressJob(spxEncoder, frSize, encBits, clientDev, conn));
      pThreadPool->queueWorkItem(item);
   }

   struct DeCompressJob : public ThreadPool::WorkItem
   {
      void *mSpeexDecoder;
      U32 mFrames;
      U32 mSampleRate;
      U32 mLength;
      SpeexBits mDecoderBits;
      SFXInputDevice *mClientDev;
      GameConnection *mConn;
      const char *mData;

      DeCompressJob(void* spxDecoder, U32 frames, U32 smpRate, U32 length, SpeexBits decBits, SFXInputDevice *clientDev, GameConnection *conn, const char *data)
         : mSpeexDecoder(spxDecoder), mFrames(frames), mSampleRate(smpRate), mLength(length), mDecoderBits(decBits), mClientDev(clientDev), mConn(conn), mData(data) {}

   protected:
      virtual void execute()
      {
         rawDeCompress(mSpeexDecoder, mFrames, mSampleRate,mLength, mDecoderBits, mClientDev,mConn, mData);
      }

      void rawDeCompress(void *spxDecoder, U32 frames, U32 smpRate, U32 length, SpeexBits decBits, SFXInputDevice *clientDev, GameConnection *conn, const char *data);
   };

   void voiceDeCompress(void *spxDecoder, U32 frames, U32 smpRate, U32 length, SpeexBits decBits, SFXInputDevice *clientDev, GameConnection *conn, const char *data)
   {
      ThreadPool* pThreadPool = &ThreadPool::GLOBAL();
      ThreadSafeRef<DeCompressJob> item(new DeCompressJob(spxDecoder, frames, smpRate, length, decBits, clientDev, conn, data));
      pThreadPool->queueWorkItem(item);
   }

   GameConnection *mConnection;
   // probably not a good idea to host voip on a server that can have 126 people.... 
   SpeexBits decoderBits;
   SpeexBits encoderBits;
   U32 frameSize;
   U32 sampleRate;

   void *speexDecoder;
   void *speexEncoder;

   SFXInputDevice *mClientDev;

public:

   DECLARE_CONOBJECT(VoipClient);
   VoipClient();
   ~VoipClient();

   void startRecordingVoip();

   void stopRecordingVoip();

   void setClient(GameConnection *connection) { mConnection = connection; }

   void clientWriteVoip();
   void clientReadVoip(const char *data, U32 frames, U32 length);


};


#endif // _VOIPCLIENT_H_
