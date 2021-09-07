#include "T2D/core/AnimationController.h"

#include "T2D/assets/AnimationAsset.h"

AnimationController::AnimationController()
   : mAnimationAsset(NULL)
{
}



AnimationController::~AnimationController()
{
   /// make sure values are reset
   reset();
}

void AnimationController::reset()
{
   /// reset EVERYTHING!
   mCurFrame            = 0;
   mCurTime             = 0.0f;
   mPauseTime           = 0.0f;
   mReverse             = false;
   mAnimationFinished   = false;
}

void AnimationController::setAnimation(const StringTableEntry animAssetId)
{
   if (!AnimationAsset::getAssetById(animAssetId,&mAnimationAsset))
   {
      Con::warnf("Error: AnimationController.setAnimation() animation asset '%s' not found.", animAssetId);
      return;
   }

   setAnimationDefaults();

}

void AnimationController::setAnimationDefaults()
{
   if (mAnimationAsset == NULL)
      return;

   mTotalFrames = (mAnimationAsset->getAnimationLength() - 1);
   mFPS         = mAnimationAsset->getAnimationFPS();
   mFrameTime   = 1000 / mFPS;
   mLoop        = mAnimationAsset->getAnimationLoop();
   mBounce      = mAnimationAsset->getAnimationBounce();

   /// this should go without saying but unfortunately it needs to be said =/
   if (mBounce)
      mLoop = false;

   /// reset state
   reset();

}

bool AnimationController::setAnimationFrame(const U32 frame)
{
   /// this would be the one that is done through network to catchup
   if (mAnimationAsset == NULL)
   {
      Con::warnf("AnimationController.setAnimationFrame() - no animation asset.");
      return false;
   }

   if (frame < 0 || frame > mTotalFrames)
   {
      Con::warnf("AnimationController.setAnimationFrame() - Frame out of range");
      return false;
   }

   /// cur frame calc from cur time
   mCurTime = frame * mFrameTime;
   /// show curframe incase tick
   update(0.0f);

   return true;

}

void AnimationController::update(F32 dt)
{
   if (mAnimationAsset == NULL)
      Con::warnf("AnimationController update, no animation asset.");

   if (mAnimationFinished)
      return;

   /// add time
   mCurTime += dt;

   /// if cur time is more than single frame time change up.
   if (mCurTime > mFrameTime)
   {
      /// what change in frame do you want
      S32 frame = mCurTime / mFrameTime;

      if (mReverse)
      {
         mCurFrame -= frame;

         if (mCurFrame < 0)
         {
            if (mBounce)
            {
               mCurFrame = 0;
               mAnimationFinished = false;
               mReverse = false;
            }

            if (mLoop)
            {
               mCurFrame = mTotalFrames;
               mAnimationFinished = false;
            }

            if (!mLoop && !mBounce)
            {
               mCurFrame = 0;
               mAnimationFinished = true;
            }

         }
      }
      else
      {
         mCurFrame += frame;

         if (mCurFrame > mTotalFrames)
         {
            if (mBounce)
            {
               mCurFrame = mTotalFrames;
               mAnimationFinished = false;
               mReverse = true;
            }

            if (mLoop)
            {
               mCurFrame = 0;
               mAnimationFinished = false;
            }

            if(!mLoop && !mBounce)
            {
               mCurFrame = mTotalFrames;
               mAnimationFinished = true;
            }
         }
      }
      /// reset time for next frame.
      mCurTime = 0.0f;

   }

}

SpriteAsset::FrameArea AnimationController::getCurrentAnimationFrame()
{
   U32 texFrame = mAnimationAsset->getAnimationFrame(mCurFrame);

   return mAnimationAsset->getActiveFrameArea(texFrame);
}
