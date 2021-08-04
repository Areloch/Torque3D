#include "platform/platform.h"
#include "T2D/objects/spriteObject.h"
#include "T2D/Scene/SceneObject2D.h"
#include "math/mathIO.h"
#include "renderInstance/renderPassManager.h"
#include "scene/sceneRenderState.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/primBuilder.h"
#include "core/stream/bitStream.h"
#include "T3D/gameBase/gameConnection.h"

IMPLEMENT_CONOBJECT(SpriteObject);

SpriteObject::SpriteObject()
   :  mFlipX(false),
      mFlipY(false),
      mSpriteAssetId(StringTable->EmptyString())
{
   mNetFlags.set(Ghostable | ScopeAlways);
   mFrame = 0;
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

   txr = mSpriteAsset->getSprite(GFXStaticTextureProfile);

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

   setMaskBits(FrameUpdateMask);

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

void SpriteObject::inspectPostApply()
{

   Parent::inspectPostApply();

   setMaskBits(AssetUpdateMask | FrameUpdateMask);

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

void SpriteObject::prepRenderImage(SceneCameraState* state)
{
   Con::printf("Sprite prepRenderImage");

   Con::printf("_renderSprite");

   SpriteAsset::FrameArea::TexelArea texelArea = mSpriteAsset->getSpriteFrameArea(mFrame).mTexelArea;

   texelArea.setFlip(mFlipX, mFlipY);

   const Vector2& texLower = texelArea.mTexelLower;
   const Vector2& texUpper = texelArea.mTexelUpper;

   if (nsb.isNull())
   {
      GFXStateBlockDesc desc;
      desc.setCullMode(GFXCullNone);
      desc.alphaTestEnable = true;
      desc.setZReadWrite(true);
      desc.zWriteEnable = false;
      desc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
      nsb = GFX->createStateBlock(desc);
   }

   GFX->setStateBlock(nsb);

   GFXTransformSaver saver;
   GFX->multWorld(getRenderTransform());

   GFX->setTexture(0, txr);

   MatrixF worldmod = GFX->getWorldMatrix();
   MatrixF viewmod = GFX->getViewMatrix();

   Point4F Position;
   MatrixF ModelView;
   ModelView.mul(viewmod, worldmod);
   ModelView.getColumn(3, &Position);
   ModelView.identity();
   ModelView.setColumn(3, Position);

   GFX->setWorldMatrix(ModelView);
   MatrixF ident;
   ident.identity();
   GFX->setViewMatrix(ident);

   F32 width = mSize.x * 0.5f * mObjScale.x;
   F32 height = mSize.y * 0.5f * mObjScale.y;

   Point2F pts[4];
   pts[0].set(width, -height);
   pts[1].set(-width, -height);
   pts[2].set(-width, height);
   pts[3].set(width, height);

   PrimBuild::begin(GFXTriangleStrip, 4);
   {
      PrimBuild::texCoord2f(texLower.x, texLower.y);
      PrimBuild::vertex2fv(pts[1]);
      PrimBuild::texCoord2f(texUpper.x, texLower.y);
      PrimBuild::vertex2fv(pts[0]);
      PrimBuild::texCoord2f(texLower.x, texUpper.y);
      PrimBuild::vertex2fv(pts[2]);
      PrimBuild::texCoord2f(texUpper.x, texUpper.y);
      PrimBuild::vertex2fv(pts[3]);
   }
   PrimBuild::end();


}

U32 SpriteObject::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   Con::printf("sprite object packupdate");

   if (stream->writeFlag(mask & TransformMask))
   {
      Con::printf("sprite packupdate transform");
      mathWrite(*stream, getTransform());
   }

   if (stream->writeFlag(mask & ScaleMask))
   {
      Con::printf("sprite packupdate scale");
         mathWrite(*stream, mObjScale);
   }

   if (stream->writeFlag(mask & AssetUpdateMask))
   {
      Con::printf("sprite packupdate asset");
      mathWrite(*stream, mSize);
      stream->writeString(mSpriteAssetId);
   }

   if (stream->writeFlag(mask & FrameUpdateMask))
   {
      Con::printf("sprite packupdate frame update");
      stream->write(mFrame);
   }
   Con::printf("sprite packupdate flip");
   stream->writeFlag(mFlipX);
   stream->writeFlag(mFlipY);

   Con::printf("sprite packupdate finish");
   return retMask;
}

void SpriteObject::unpackUpdate(NetConnection *conn, BitStream *stream)
{
   Con::printf("sprite unpack");

   Parent::unpackUpdate(conn, stream);

   if (stream->readFlag()) // TransformMask
   {
      MatrixF mat;
      mathRead(*stream, &mat);
      setTransform(mat);
      setRenderTransform(mat);
   }

   if (stream->readFlag()) // ScaleMask
   {
         Point2F scale;
         mathRead(*stream, &scale);
         setScale(scale);
   }

   if (stream->readFlag()) // Asset update Flag
   {
      Point2F size;
      mathRead(*stream, &size);
      mSize = Vector2(size.x, size.y);
      char buffer[256];
      stream->readString(buffer);
      setSpriteAsset(StringTable->insert(buffer));
   }

   if (stream->readFlag()) // update frame flag
   {
      stream->read(&mFrame);
      setFrame(mFrame);
   }

   mFlipX = stream->readFlag();
   mFlipY = stream->readFlag();

}

