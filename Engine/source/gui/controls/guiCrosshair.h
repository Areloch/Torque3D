#ifndef _GUICROSSHAIR_H_
#define _GUICROSSHAIR_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif // !_SCENEOBJECT_H_


class GuiCrosshairCtrl : public GuiControl
{
   typedef GuiControl Parent;

public:
   enum CurState
   {
      NORMAL,
      ENEMY,
      FRIEND,
      INTERACT
   };

protected:

   CurState mCurState;

   U32 mCrossWidth;
   U32 mCrossHeight;
   F32 mMinSpread;
   F32 mMaxSpread;
   F32 mSpreadSpeed;
   F32 mSpreadIncMult;
   ColorI mNeutralColor;
   ColorI mEnemyColor;
   ColorI mFriendColor;
   F32 mDist;
   bool mAnimated;
   bool mUseViewDistance;
   bool mUseBitmapCrosshair;
   bool mFpsMode;

   GFXTexHandle mCrossHairSection;
   GFXTexHandle mInteractTexture;

   SceneObject* mSelectedObj;

public:
   Point3F  mPrevCamPos;
   F32      mPrevSpeed;
   F32      mCurrentSpread;

   GuiCrosshairCtrl();
   static void initPersistFields();

   // GuiControl.
   //bool onWake();
   //void onSleep();
   //void inspectPostApply();

   void onRender(Point2I offset, const RectI &updateRect);
   void setMode(const CurState &mode);
   F32 updateOffset(F32 speed);

   DECLARE_CONOBJECT(GuiCrosshairCtrl);
   DECLARE_CATEGORY("Gui Gameplay");
   DECLARE_DESCRIPTION("A control that displaces a crosshair.")
};

typedef GuiCrosshairCtrl::CurState enumCurState;

DefineEnumType(enumCurState);

#endif

static GuiCrosshairCtrl::CurState getModeTypeEnum(const char * pLabel);
