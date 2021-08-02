#ifndef _SPRITE_OBJECT_H_
#define _SPRITE_OBJECT_H_

#ifndef _SPRITEOBJECT2D_H_
#include "T2D/Scene/SceneObject2D.h"
#endif // !_SPRITEOBJECT2D_H_

#ifndef _SCENERENDERSTATE_H_
#include "scene/sceneRenderState.h"
#endif // !_SCENERENDERSTATE_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#ifndef _SPRITE_ASSET_H_
#include "T2D/assets/SpriteAsset.h"
#endif // !_SPRITE_ASSET_H_


class SpriteObject : public SceneObject2D
{
   typedef SceneObject2D Parent;

private:

   bool mFlipX;
   bool mFlipY;
   Vector2 mSize;

protected:

   U32 mFrame;
   AssetPtr<SpriteAsset> mSpriteAsset;
   StringTableEntry mSpriteAssetId;
   bool setSpriteAsset(const StringTableEntry spriteAssetId);
   bool setFrame(const U32 frame);

public:
   SpriteObject();
   virtual ~SpriteObject();

   DECLARE_CONOBJECT(SpriteObject);

   static bool _setSpriteAsset(void *obj, const char* index, const char* data);
   static bool _setFieldFrame(void *obj, const char* index, const char* data);
   
   static void initPersistFields();

   /// sim
   virtual bool onAdd();
   virtual void onRemove();

   virtual void prepRenderImage(SceneRenderState *state);

};

#endif // !_SPRITE_OBJECT_H_
