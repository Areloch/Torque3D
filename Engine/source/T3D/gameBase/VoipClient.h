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

#ifndef _SFXSYSTEM_H_
#include "sfx/sfxSystem.h"
#endif // !_SFXSYSTEM_H_

#ifndef _SFXINPUTDEVICE_H_
#include "sfx/sfxInputDevice.h"
#endif // !_SFXINPUTDEVICE_H_

#include <speex/speex.h>

class GameConnection;

class VoipClient : public NetObject
{
protected:
   typedef NetObject Parent;

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
