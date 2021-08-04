#ifndef _GUISPRITECTRL_H_
#define _GUISPRITECTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

#ifndef _SPRITE_ASSET_H_
#include "T2D/assets/SpriteAsset.h"
#endif // !_SPRITE_ASSET_H_

class GuiSpriteCtrl : public GuiControl
{
public:
   typedef GuiControl Parent;

private:
   bool mFlipX;
   bool mFlipY;

protected:
   U32 mFrame;
   AssetPtr<SpriteAsset> mSpriteAsset;
   StringTableEntry mSpriteAssetId;

   bool setSpriteAsset(const StringTableEntry spriteAssetId);
   bool setFrame(const U32 frame);

public:
   GFXTexHandle txr;

   GuiSpriteCtrl();
   static void initPersistFields();

   static bool _setSpriteAsset(void *obj, const char* index, const char* data);
   static bool _setFieldFrame(void *obj, const char* index, const char* data);

   // GuiControl.
   bool onWake();
   void onSleep();
   void inspectPostApply();
   void onRender(Point2I offset, const RectI &updateRect);

   DECLARE_CONOBJECT(GuiSpriteCtrl);
   DECLARE_CATEGORY("Gui 2D");

};

#endif // !_GUISPRITECTRL_H_
