#include "T3D/gameBase/voipEvent.h"
#include "T3D/gameBase/gameConnection.h"

IMPLEMENT_CO_NETEVENT_V1(VoipEvent);

VoipEvent::VoipEvent()
{
   mSourceId = -1;
   mGuaranteeType = Guaranteed;
   dMemset(mOutData, 0, 1024);
   mFrames = 0;
   mLength = 0;
   //Con::printf("Should not be used! create with VoipEvent(const char* data, U32 frames, U32 length)!");
}

VoipEvent::VoipEvent(const char* data, U32 frames, U32 length)
{
   mSourceId = -1;
   mGuaranteeType = Guaranteed;
   dMemset(mOutData, 0, length);
   for (U32 i = 0; i < length; i++)
      mOutData[i] = data[i];
   mFrames = frames;
   mLength = length;

}

void VoipEvent::pack(NetConnection *conn, BitStream *bstream)
{
   bstream->writeInt(mLength, 32);
   bstream->writeInt(mFrames, 32);
   bstream->writeLongString(mLength, mOutData);
}

void VoipEvent::write(NetConnection *conn, BitStream *bstream)
{
   /// ordinary pack for now
   pack(conn, bstream);
}

void VoipEvent::unpack(NetConnection *conn, BitStream *bstream)
{
   mLength = bstream->readInt(32);
   mFrames = bstream->readInt(32);
   dMemset(mOutData, 0, mLength);
   bstream->readLongString(mLength, mOutData);

}

void VoipEvent::process(NetConnection *conn)
{
   if (conn->isConnectionToServer())
   {
      GameConnection* gc = (GameConnection*)conn;
      gc->getVoipClient()->clientReadVoip(mOutData, mFrames, mLength);
   }
   else
   {
      SimGroup* client = Sim::getClientGroup();
      for (SimGroup::iterator itr = client->begin(); itr != client->end(); itr++)
      {
         GameConnection* gc = dynamic_cast<GameConnection*>(*itr);

         if (gc && gc->isListening())
         {
            //if (mSourceId == gc->getId())
            //   continue;

            gc->postNetEvent(new VoipEvent(mOutData, mFrames, mLength));
         }
      }
   }

}
