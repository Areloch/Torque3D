#include "platform/platform.h"
#include "T2D/objects/spriteObject.h"
#include "T2D/Scene/SceneObject2D.h"
#include "math/mathIO.h"
#include "renderInstance/renderPassManager.h"
#include "scene/sceneRenderState.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDrawUtil.h"
#include "core/stream/bitStream.h"
#include "T3D/gameBase/gameConnection.h"

IMPLEMENT_CO_NETOBJECT_V1(SpriteObject);

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

   txr.set(mSpriteAsset->getSpriteFileName(), &GFXDefaultGUIProfile, avar("%s() - txr (line %d)", __FUNCTION__, __LINE__));

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

   setMaskBits(AssetUpdateMask);

   return true;

}

void SpriteObject::onRemove()
{
   removeFromScene();

   Parent::onRemove();
}

U32 SpriteObject::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   if (stream->writeFlag(mask & TransformMask))
   {
      mathWrite(*stream, getTransform());
   }

   if (stream->writeFlag(mask & ScaleMask))
   {
         mathWrite(*stream, mObjScale);
   }

   if (stream->writeFlag(mask & AssetUpdateMask))
   {
      mathWrite(*stream, mSize);
      stream->writeString(mSpriteAsset.getAssetId());
   }

   if (stream->writeFlag(mask & FrameUpdateMask))
   {
      stream->write(mFrame);
   }
   stream->writeFlag(mFlipX);
   stream->writeFlag(mFlipY);

   return retMask;
}

void SpriteObject::unpackUpdate(NetConnection *conn, BitStream *stream)
{

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

void SpriteObject::prepRenderImage(SceneCameraState* state)
{

   SpriteAsset::FrameArea texelArea = mSpriteAsset->getSpriteFrameArea(mFrame);

   texelArea.mTexelArea.setFlip(mFlipX, mFlipY);

   const Vector2& texLower = texelArea.mTexelArea.mTexelLower;
   const Vector2& texUpper = texelArea.mTexelArea.mTexelUpper;
   Con::printf("TexLower.x: %d, TexLower.y: %d, TexUpper.x: %d, TexUpper.y: %d",
      texLower.x,
      texLower.y,
      texUpper.x,
      texUpper.y);

   GFXStateBlockDesc desc;
   desc.setCullMode(GFXCullNone);
   desc.setZReadWrite(true,false);
   desc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
   desc.setColorWrites(true, true, true, true);
   desc.samplersDefined = true;
   desc.samplers[0] = GFXSamplerStateDesc::getClampLinear();
   desc.samplers[0].minFilter = GFXTextureFilterPoint;
   desc.samplers[0].mipFilter = GFXTextureFilterPoint;
   desc.samplers[0].magFilter = GFXTextureFilterPoint;
   nsb = GFX->createStateBlock(desc);

   GFXVertexBufferHandle<GFXVertexPCT> verts(GFX, 4, GFXBufferTypeVolatile);
   verts.lock();

   F32 width = mSize.x * 0.5f * mObjScale.x;
   F32 height = mSize.y * 0.5f * mObjScale.y;

   verts[0].point.set(-width, height, 0.0f);
   verts[1].point.set(width, height, 0.0f);
   verts[2].point.set(-width, -height, 0.0f);
   verts[3].point.set(width, -height, 0.0f);

   verts[0].color = verts[1].color = verts[2].color = verts[3].color = GFXVertexColor(ColorI(255,255,255,255));
   
   verts[0].texCoord.set(texLower.x, texUpper.y);
   verts[1].texCoord.set(texUpper.x, texUpper.y);
   verts[2].texCoord.set(texLower.x, texLower.y);
   verts[3].texCoord.set(texUpper.x, texLower.y);

   verts.unlock();

   GFX->setVertexBuffer(verts);

   MatrixF mat = getRenderTransform();
   Point3F scale(mObjScale.x, mObjScale.y, 0.0f);
   mat.scale(scale);

   GFX->pushWorldMatrix();

   GFX->multWorld(mat);
  
   GFX->setStateBlock(nsb);

   GFX->setTexture(0, txr);
   GFX->setupGenericShaders(GFXDevice::GSAddColorTexture);
   GFX->drawPrimitive(GFXTriangleStrip, 0, 2);

   GFX->popWorldMatrix();

}

