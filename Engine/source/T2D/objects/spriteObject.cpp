#include "platform/platform.h"
#include "T2D/objects/spriteObject.h"
#include "renderInstance/renderPassManager.h"
#include "scene/sceneRenderState.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/primBuilder.h"

IMPLEMENT_CONOBJECT(SpriteObject);

SpriteObject::SpriteObject()
   :  mFlipX(false),
      mFlipY(false)
{
   mSize.set(1.0f, 1.0f);
}

SpriteObject::~SpriteObject()
{

}

bool SpriteObject::_setSpriteAsset(void * obj, const char * index, const char * data)
{
   SpriteObject* so = static_cast<SpriteObject*>(obj);

   so->mSpriteAssetId = StringTable->insert(data);

   return so->setSpriteAsset(so->mSpriteAssetId);

}

bool SpriteObject::_setFieldFrame(void * obj, const char * index, const char * data)
{
   SpriteObject* so = static_cast<SpriteObject*>(obj);

   so->mFrame = dAtoi(data);

   return so->setFrame(so->mFrame);
}

bool SpriteObject::setSpriteAsset(const StringTableEntry spriteAssetId)
{
   if (!SpriteAsset::getAssetById(spriteAssetId, &mSpriteAsset))
   {
      Con::warnf("Error sprite asset id '%s' not found.", spriteAssetId);
      return false;
   }

   return true;

}

bool SpriteObject::setFrame(const U32 frame)
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

void SpriteObject::initPersistFields()
{
   Parent::initPersistFields();

   addProtectedField("SpriteAsset", TypeSpriteAssetId, Offset(mSpriteAssetId, SpriteObject),&_setSpriteAsset,&defaultProtectedGetFn,
      "Add a sprite asset.");

   addProtectedField("SpriteFrame", TypeS32, Offset(mFrame, SpriteObject),&_setFieldFrame, &defaultProtectedGetFn,
      "Set frame for this sprite to render.");

   addField("FlipX", TypeBool, Offset(mFlipX, SpriteObject),
      "");

   addField("FlipY", TypeBool, Offset(mFlipY, SpriteObject),
      "");

}

bool SpriteObject::onAdd()
{
   if(!Parent::onAdd())
      return false;

   F32 width = mSize.x * 0.5f;
   F32 height = mSize.y * 0.5f;

   mObjBox = BoxVec2(Vector2(-width, -height), Vector2(width, height));

   addToScene();

   return true;

}

void SpriteObject::onRemove()
{
   removeFromScene();

   Parent::onRemove();
}

void SpriteObject::prepRenderImage(SceneRenderState* state)
{
   if (mSpriteAsset == NULL)
      return;

   

   SpriteAsset::FrameArea::TexelArea texelArea = mSpriteAsset->getSpriteFrameArea(mFrame).mTexelArea;

   GFXTexHandle tx = mSpriteAsset->getSprite(GFXStaticTextureSRGBProfile);

   texelArea.setFlip(mFlipX, mFlipY);

   const Vector2& texLower = texelArea.mTexelLower;
   const Vector2& texUpper = texelArea.mTexelUpper;

}
