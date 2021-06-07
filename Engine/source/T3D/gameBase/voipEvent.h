#ifndef _VOIPEVENT_H_
#define _VOIPEVENT_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#ifndef _GAMECONNECTION_H_
#include "T3D/gameBase/gameConnection.h"
#endif

#ifndef _BITSTREAM_H_
#include "core/stream/bitStream.h"
#endif

#ifndef _VOIPCLIENT_H_
#include "T3D/gameBase/VoipClient.h"
#endif


class VoipEvent : public NetEvent
{
public:

   typedef NetEvent Parent;

protected:

   char *mData;
   U32 mFrames;
   U32 mLength;

public:

   VoipEvent();
   VoipEvent(char *data, U32 frames, U32 length);
   void pack(NetConnection *conn, BitStream *bstream);
   void write(NetConnection *conn, BitStream *bstream);
   void unpack(NetConnection *conn, BitStream *bstream);
   void process(NetConnection*);

   DECLARE_CONOBJECT(VoipEvent);

};


#endif // !_VOIPEVENT_H_
