#include "T3D/gameBase/voipEvent.h"
#include "T3D/gameBase/gameConnection.h"

IMPLEMENT_CO_NETEVENT_V1(VoipEvent);

VoipEvent::VoipEvent()
{
   mData = "";
   dStrcpy(mData, "reeeeeeeeee", 1024);
   mFrames = 0;
   mLength = 0;
}

VoipEvent::VoipEvent(char *data, U32 frames, U32 length)
{
   Con::printf("VoipEvent Created: %s Frames:%d Length:%d",data,frames);
   mData = "";
   //dMemset(mData, 0, 1024);
   dStrcpy(mData, data, length);
   mFrames = frames;
   mLength = length;
}

void VoipEvent::pack(NetConnection *conn, BitStream *bstream)
{
   Con::printf("VoipEvent packed");
   bstream->writeInt(mLength, 4);
   bstream->writeInt(mFrames, 4);
   bstream->writeLongString(mLength, mData);
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
   bstream->readLongString(mLength, mData);
}

void VoipEvent::process(NetConnection *conn)
{
   Con::printf("VoipEvent process");
   if (conn->isConnectionToServer())
   {
      GameConnection* gc = (GameConnection*)conn;
      gc->getVoipClient()->clientReadVoip(mData,mFrames,mLength);
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
