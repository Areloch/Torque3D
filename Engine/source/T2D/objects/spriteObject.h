#ifndef _SPRITE_OBJECT_H_
#define _SPRITE_OBJECT_H_

#ifndef _SPRITEOBJECT2D_H_
#include "T2D/Scene/SceneObject2D.h"
#endif // !_SPRITEOBJECT2D_H_

#ifndef _SCENERENDERSTATE_H_
#include "scene/sceneRenderState.h"
#endif // !_SCENERENDERSTATE_H_

#ifndef _ANIMATION_CONTROLLER_H_
#include "T2D/core/AnimationController.h"
#endif // !_ANIMATION_CONTROLLER_H_


#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#ifndef _ANIMATION_ASSET_H_
#include "T2D/assets/AnimationAsset.h"
#endif

#ifndef _SPRITE_ASSET_H_
#include "T2D/assets/SpriteAsset.h"
#endif // !_SPRITE_ASSET_H_


class SpriteObject : public SceneObject2D
{
   typedef SceneObject2D Parent;

   enum
   {
      TransformMask        = Parent::NextFreeMask << 0,
      AssetUpdateMask      = Parent::NextFreeMask << 1,
      FrameUpdateMask      = Parent::NextFreeMask << 2,
      AnimUpdateMask       = Parent::NextFreeMask << 3,
      AnimFrameUpdateMask  = Parent::NextFreeMask << 4,
      NextFreeMask         = Parent::NextFreeMask << 5
   };

   // Client interpolation data
   struct StateDelta {

      Point2F pos;
      Vector2 posVec;
      F32 ang;
      F32 angVec;
      // Warp data
      S32 warpTicks;                ///< Number of ticks to warp
      S32 warpCount;                ///< Current pos in warp
      Point3F warpOffset; 
      F32 dt;
   };
   StateDelta mDelta;

private:

   bool mFlipX;
   bool mFlipY;
   Vector2 mSize;
   GFXStateBlockRef  nsb;

protected:

   S32 mFrame;
   AssetPtr<SpriteAsset> mSpriteAsset;
   StringTableEntry mSpriteAssetId;
   bool setSpriteAsset(const StringTableEntry spriteAssetId);
   bool setFrame(const S32 frame);

   AssetPtr<AnimationAsset> mAnimAsset;
   StringTableEntry mAnimAssetId;
   bool setAnimationAsset(const StringTableEntry animAssetId);
   void setAnimationFrame(const S32 frame);
   S32 mAnimFrame;

   AnimationController mAnimController;

public:

   GFXTexHandle txr;

   SpriteObject();
   virtual ~SpriteObject();

   static bool _setSpriteAsset(void *obj, const char* index, const char* data);
   static bool _setFieldFrame(void *obj, const char* index, const char* data);
   static bool _setAnimationAsset(void *obj, const char* index, const char* data);
   
   static void initPersistFields();
   virtual void inspectPostApply();

   virtual void interpolateTick(F32 delta);
   virtual void processTick();

   /// sim
   virtual bool onAdd();
   virtual void onRemove();

   virtual void setTransform(const MatrixF& mat);
   /// rendering
   void prepRenderImage(SceneCameraState* cam);

   /// NetObject
   U32 packUpdate(NetConnection* conn, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection* conn, BitStream* stream);

   DECLARE_CONOBJECT(SpriteObject);
   

};

#endif // !_SPRITE_OBJECT_H_
