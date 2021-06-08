#include "T3D/gameBase/voipEvent.h"
#include "T3D/gameBase/gameConnection.h"

IMPLEMENT_CO_NETEVENT_V1(VoipEvent);

VoipEvent::VoipEvent()
{
   mSourceId = -1;
   mGuaranteeType = Guaranteed;
   mData = "";
   mFrames = 0;
   mLength = 0;
   Con::printf("Should not be used! create with VoipEvent(const char* data, U32 frames, U32 length)!");
}

VoipEvent::VoipEvent(const char* data, U32 frames, U32 length)
{
   mSourceId = -1;
   mGuaranteeType = Guaranteed;
   Con::printf("Creating VoipEvent: %s Frames:%d Length:%d", data, frames, length);
   //mData = "";
   mData = data;
   mFrames = frames;
   mLength = length;

   Con::printf("Creation finished");
}

void VoipEvent::pack(NetConnection *conn, BitStream *bstream)
{
   Con::printf("VoipEvent packed");
   bstream->writeInt(mLength, 4);
   bstream->writeInt(mFrames, 4);
   bstream->writeLongString(mLength, const_cast<char*>(mData));
}

void VoipEvent::write(NetConnection *conn, BitStream *bstream)
{
   /// ordinary pack for now
   Con::printf("VoipEvent write");
   pack(conn, bstream);
}

void VoipEvent::unpack(NetConnection *conn, BitStream *bstream)
{
   Con::printf("VoipEvent unpack");
   mLength = bstream->readInt(4);
   mFrames = bstream->readInt(4);
   bstream->readLongString(mLength, const_cast<char*>(mData));
}

void VoipEvent::process(NetConnection *conn)
{
   Con::printf("VoipEvent process");
   if (conn->isConnectionToServer())
   {
      GameConnection* gc = (GameConnection*)conn;
      gc->getVoipClient()->clientReadVoip(mData, mFrames, mLength);
   }
   else
   {
      SimGroup* client = Sim::getClientGroup();
      for (SimGroup::iterator itr = client->begin(); itr != client->end(); itr++)
      {
         GameConnection* gc = dynamic_cast<GameConnection*>(*itr);

         if (gc && gc->isListening())
         {
            if (mSourceId == gc->getId())
               continue;

            gc->postNetEvent(new VoipEvent(mData, mFrames, mLength));
         }
      }
   }

}
