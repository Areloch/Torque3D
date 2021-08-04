#include "platform/platform.h"
#include "gui/2d/guiSpriteCtrl.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"

IMPLEMENT_CONOBJECT(GuiSpriteCtrl);

ConsoleDocClass(GuiSpriteCtrl,
   "@brief A gui control that is used to display a sprite.\n\n"

   "The image is stretched to the constraints of the control by default. However, the control can also\n"

   "@tsexample\n"
   "// Create a tiling GuiSpriteCtrl \n"
   "%bitmapCtrl = new GuiSpriteCtrl()\n"
   "{\n"
   "   SpriteAsset = \"Module:SpriteAsset\";\n"
   "   SpriteFrame = \"1\";\n"
   "};\n"
   "@endtsexample\n\n"

   "@ingroup GuiControls"
);

bool GuiSpriteCtrl::_setSpriteAsset(void * obj, const char * index, const char * data)
{
   GuiSpriteCtrl* so = static_cast<GuiSpriteCtrl*>(obj);

   so->mSpriteAssetId = StringTable->insert(data);

   return so->setSpriteAsset(so->mSpriteAssetId);

}

bool GuiSpriteCtrl::_setFieldFrame(void * obj, const char * index, const char * data)
{
   GuiSpriteCtrl* so = static_cast<GuiSpriteCtrl*>(obj);

   so->mFrame = dAtoi(data);

   return so->setFrame(so->mFrame);
}

bool GuiSpriteCtrl::onWake()
{
   if(!Parent::onWake())
      return false;

   setActive(true);
   if(setSpriteAsset(mSpriteAssetId))
      setFrame(mFrame);

   return true;
}

void GuiSpriteCtrl::onSleep()
{
   Parent::onSleep();
}

void GuiSpriteCtrl::inspectPostApply()
{
   Parent::inspectPostApply();

   /// this is just to set it to some sort of size.
   /// this will need changed to a per sprite frame basis.
   if (txr)
   {
      if ((getExtent().x == 0) && (getExtent().y == 0) && txr)
      {
         setExtent(txr->getWidth(), txr->getHeight());
      }
   }
}

bool GuiSpriteCtrl::setSpriteAsset(const StringTableEntry spriteAssetId)
{
   if (!SpriteAsset::getAssetById(spriteAssetId, &mSpriteAsset))
   {
      Con::warnf("Error sprite asset id '%s' not found.", spriteAssetId);
      return false;
   }

   txr.set(mSpriteAsset->getSpriteFileName(),&GFXDefaultGUIProfile,avar("%s() - txr (line %d)", __FUNCTION__, __LINE__));

   return true;

}

bool GuiSpriteCtrl::setFrame(const U32 frame)
{
   if (mSpriteAsset == NULL)
   {
      Con::warnf("SpriteObject::setFrame() - cannot set frame without asset.");
      return false;
   }

   if (frame >= mSpriteAsset->getFrameCount())
   {
      Con::warnf("SpriteObject::setFrame() - Invalid Frame #%d.", frame);
   }

   mFrame = frame;

   return true;

}

GuiSpriteCtrl::GuiSpriteCtrl()
   :  mFlipX(false),
      mFlipY(false),
      mSpriteAssetId(StringTable->EmptyString())
{
   mFrame = 0;
}

void GuiSpriteCtrl::initPersistFields()
{
   

   addProtectedField("SpriteAsset", TypeSpriteAssetId, Offset(mSpriteAssetId, GuiSpriteCtrl), &_setSpriteAsset, &defaultProtectedGetFn,
      "Add a sprite asset.");

   addProtectedField("SpriteFrame", TypeS32, Offset(mFrame, GuiSpriteCtrl), &_setFieldFrame, &defaultProtectedGetFn,
      "Set frame for this sprite to render.");

   addField("FlipX", TypeBool, Offset(mFlipX, GuiSpriteCtrl),
      "");

   addField("FlipY", TypeBool, Offset(mFlipY, GuiSpriteCtrl),
      "");

   Parent::initPersistFields();

}

void GuiSpriteCtrl::onRender(Point2I offset, const RectI & updateRect)
{
   
   if (txr)
   {
      GFX->getDrawUtil()->clearBitmapModulation();

      SpriteAsset::FrameArea pxArea = mSpriteAsset->getSpriteFrameArea(mFrame);

      RectI srcRegion(pxArea.mPixelArea.mPixelOffset, Point2I(pxArea.mPixelArea.mPixelWidth, pxArea.mPixelArea.mPixelHeight));
      RectI dstRegion(offset, getBounds().extent);

      /// never wrap sprite.... may change for certain situations
      GFX->getDrawUtil()->drawBitmapStretchSR(txr, dstRegion, srcRegion, GFXBitmapFlip_None, GFXTextureFilterLinear, false);

   }

   renderChildControls(offset, updateRect);

}

