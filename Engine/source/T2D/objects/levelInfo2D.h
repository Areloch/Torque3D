#ifndef _LEVELINFO2D_H_
#define _LEVELINFO2D_H_

#ifndef _NETOBJECT_H_
#include "sim/netObject.h"
#endif

#ifndef _COLOR_H_
#include "core/color.h"
#endif

#ifndef _SFXCOMMON_H_
#include "sfx/sfxCommon.h"
#endif

#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif

class SFXAmbience;
class SFXSoundscape;

class LevelInfo2D : public NetObject
{
   typedef NetObject Parent;

private:

   Point2F  mCameraSize;
   ColorI   mCanvasClearColor;

   /// here just in case
   bool mAdvancedLightmapSupport;

   /// Global ambient sound space properties.
   SFXAmbience*      mSoundAmbience;
   SFXDistanceModel  mSoundDistanceModel;
   SFXSoundscape*    mSoundscape;

   void _updateSceneGraph2D();
   void _onLMActivate(const char *lm, bool enable);

public:

   LevelInfo2D();
   virtual ~LevelInfo2D();

   virtual bool onAdd();
   virtual void onRemove();
   virtual void inspectPostApply();

   static void initPersistFields();

   enum NetMaskBits
   {
      UpdateMask = BIT(0)
   };

   virtual U32 packUpdate(NetConnection *conn, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *conn, BitStream *stream);

   DECLARE_CONOBJECT(LevelInfo2D);

   virtual SFXAmbience* getSoundAmbience() const { return mSoundAmbience; }

};

#endif // !_LEVELINFO2D_H_
