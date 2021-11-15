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

#include <opus.h>

class VoipEvent;
class GameConnection;

#define FRAME_SIZE 160
#define MAX_FRAME_SIZE 6*160

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
      void *mOpusEncoder;
      U32 mFrameSize;
      SFXInputDevice *mClientDev;
      GameConnection *mConnection;

      CompressJob(void* opsEncoder, U32 frSize, SFXInputDevice *clientDev, GameConnection *conn)
         : mOpusEncoder(opsEncoder), mFrameSize(frSize), mClientDev(clientDev), mConnection(conn) {}

   protected:
      virtual void execute()
      {
         rawCompress(mOpusEncoder, mFrameSize, mClientDev, mConnection);
      }

      void rawCompress(void* opsEncoder, U32 frSize, SFXInputDevice *clientDev, GameConnection *conn);
   };

   void voiceCompress(void* opsEncoder, U32 frSize, SFXInputDevice *clientDev, GameConnection *conn)
   {
      ThreadPool* pThreadPool = &ThreadPool::GLOBAL();
      ThreadSafeRef<CompressJob> item(new CompressJob(opsEncoder, frSize, clientDev, conn));
      pThreadPool->queueWorkItem(item);
   }

   struct DeCompressJob : public ThreadPool::WorkItem
   {
      void *mOpusDecoder;
      U32 mFrames;
      U32 mSampleRate;
      U32 mLength;
      SFXInputDevice *mClientDev;
      GameConnection *mConn;
      U8 *mData;

      DeCompressJob(void* opsDecoder, U32 frames, U32 smpRate, U32 length, SFXInputDevice *clientDev, GameConnection *conn, U8 *data)
         : mOpusDecoder(opsDecoder), mFrames(frames), mSampleRate(smpRate), mLength(length), mClientDev(clientDev), mConn(conn), mData(data) {}

   protected:
      virtual void execute()
      {
         rawDeCompress(mOpusDecoder, mFrames, mSampleRate,mLength, mClientDev,mConn, mData);
      }

      void rawDeCompress(void *opsDecoder, U32 frames, U32 smpRate, U32 length, SFXInputDevice *clientDev, GameConnection *conn, U8 *data);
   };

   void voiceDeCompress(void *opsDecoder, U32 frames, U32 smpRate, U32 length,  SFXInputDevice *clientDev, GameConnection *conn, U8 *data)
   {
      ThreadPool* pThreadPool = &ThreadPool::GLOBAL();
      ThreadSafeRef<DeCompressJob> item(new DeCompressJob(opsDecoder, frames, smpRate, length, clientDev, conn, data));
      pThreadPool->queueWorkItem(item);
   }

   GameConnection *mConnection;
   U32 frameSize;
   U32 sampleRate;

   void *opusDecoder;
   void *opusEncoder;

   SFXInputDevice *mClientDev;

public:

   DECLARE_CONOBJECT(VoipClient);
   VoipClient();
   ~VoipClient();

   void startRecordingVoip();

   void stopRecordingVoip();

   void setClient(GameConnection *connection) { mConnection = connection; }

   void clientWriteVoip();
   void clientReadVoip(U8 *data, U32 frames, U32 length);

};


#endif // _VOIPCLIENT_H_
