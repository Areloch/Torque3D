#ifndef _ANIMATION_CONTROLLER_H_
#define _ANIMATION_CONTROLLER_H_

#ifndef _ANIMATION_ASSET_H_
#include "T2D/assets/AnimationAsset.h"
#endif

#ifndef _SPRITE_ASSET_H_
#include "T2D/assets/SpriteAsset.h"
#endif

#ifndef _ASSET_PTR_H_
#include "assets/assetPtr.h"
#endif


class AnimationController
{
protected:
   AssetPtr<AnimationAsset>   mAnimationAsset;
   S32                        mCurFrame;
   F32                        mCurTime;
   F32                        mPauseTime;
   bool                       mAnimationFinished;
   bool                       mReverse;
   bool                       mBounce;

   S32                        mTotalFrames;
   F32                        mFrameTime;
   S32                        mFPS;
   bool                       mLoop;

public:
   AnimationController();
   virtual ~AnimationController();

   void reset();

   void setAnimation(const StringTableEntry animAssetId);
   void setAnimationDefaults();
   bool setAnimationFrame(const U32 frame);

   S32 getCurrentFrame() { return mCurFrame; }

   void update(F32 dt);

   SpriteAsset::FrameArea getCurrentAnimationFrame();

};

#endif // !_ANIMATION_CONTROLLER_H_
