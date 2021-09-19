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
      mSpriteAssetId(StringTable->EmptyString()),
      mAnimAssetId(StringTable->EmptyString())
{
   mNetFlags.set(Ghostable | ScopeAlways);
   mFrame = 0;
   mSize.set(1.0f, 1.0f);
   mDelta.pos = Point2F(0, 0);
   mDelta.posVec = Point2F(0, 0);
   mDelta.warpOffset = Point3F::Zero;
   mDelta.warpTicks = mDelta.warpCount = 0;
   mDelta.dt = 1;
   dMemset(&mDelta, 0, sizeof(StateDelta));
   mAnimFrame = 0;
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

bool SpriteObject::setSpriteAsset(const StringTableEntry spriteAssetId)
{
   if (!SpriteAsset::getAssetById(spriteAssetId, &mSpriteAsset))
   {
      Con::warnf("Error sprite asset id '%s' not found.", spriteAssetId);
      return false;
   }

   /// wheres the money lebowski.
   txr.set(mSpriteAsset->getSpriteFileName(), &GFXStaticTextureSRGBProfile, avar("%s() - txr (line %d)", __FUNCTION__, __LINE__));

   setMaskBits(AssetUpdateMask);

   return true;
}

bool SpriteObject::_setFieldFrame(void * obj, const char * index, const char * data)
{
   SpriteObject* so = static_cast<SpriteObject*>(obj);

   so->mFrame = dAtoi(data);

   return so->setFrame(so->mFrame);
}

bool SpriteObject::setFrame(const S32 frame)
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

bool SpriteObject::_setAnimationAsset(void * obj, const char * index, const char * data)
{
   SpriteObject* so = static_cast<SpriteObject*>(obj);

   so->mAnimAssetId = StringTable->insert(data);

   return so->setAnimationAsset(so->mAnimAssetId);
}

bool SpriteObject::setAnimationAsset(const StringTableEntry animAssetId)
{
   if (!AnimationAsset::getAssetById(animAssetId, &mAnimAsset))
   {
      Con::warnf("Error Animation asset id '%s' not found.", animAssetId);
      return false;
   }

   mAnimController.setAnimation(animAssetId);

   /// get our texture.
   txr.set(mAnimAsset->getSpriteAsset()->getSpriteFileName(), &GFXStaticTextureSRGBProfile, avar("%s() - txr (line %d)", __FUNCTION__, __LINE__));

   setMaskBits(AssetUpdateMask);
   return true;
}

void SpriteObject::setAnimationFrame(const S32 frame)
{

   if (mAnimAsset == NULL)
   {
      Con::warnf("SpriteObject::setAnimationFrame() - cannot set frame without asset.");
      return;
   }

   if (frame >= mAnimAsset->getAnimationLength())
   {
      Con::warnf("SpriteObject::setAnimationFrame() - Invalid Frame #%d.", frame);
   }

   mAnimFrame = frame;
   setMaskBits(AnimFrameUpdateMask);
}

void SpriteObject::initPersistFields()
{
   Parent::initPersistFields();

   addProtectedField("SpriteAsset", TypeSpriteAssetId, Offset(mSpriteAssetId, SpriteObject),&_setSpriteAsset,&defaultProtectedGetFn,
      "Add a sprite asset.");

   addProtectedField("AnimationAsset", TypeAnimationAssetId, Offset(mAnimAssetId, SpriteObject), &_setAnimationAsset, &defaultProtectedGetFn,
      "Add an animation asset.");

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

   if (mpBody)
   {
      b2PolygonShape defBox;
      defBox.SetAsBox(width, height);
      // create fixture, and set default density.
      mpBody->CreateFixture(&defBox, 1.0f);
   }

   if (mSpriteAssetId != StringTable->EmptyString())
   {
      if(!setSpriteAsset(mSpriteAssetId))
         return false;
   }

   if (mAnimAssetId != StringTable->EmptyString())
   {
      if (!setAnimationAsset(mAnimAssetId))
         return false;
   }

   return true;

}

void SpriteObject::onRemove()
{
   removeFromScene();

   Parent::onRemove();
}

void SpriteObject::setTransform(const MatrixF & mat)
{
   Parent::setTransform(mat);

   setMaskBits(TransformMask);
}

U32 SpriteObject::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   if (stream->writeFlag(mask & TransformMask))
   {
      mathWrite(*stream, mPosition);
      stream->write(mAng);
   }

   if (stream->writeFlag(mask & ScaleMask))
   {
         mathWrite(*stream, mObjScale);
   }

   if (stream->writeFlag(mask & AssetUpdateMask))
   {
      mathWrite(*stream, mSize);
      stream->writeString(mSpriteAsset.getAssetId());
      stream->writeString(mAnimAsset.getAssetId());
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
      Point2F pos;
      F32 ang;
      mathRead(*stream, &pos);
      stream->read(&ang);

      // Determin number of ticks to warp based on the average
      // of the client and server velocities.
      mDelta.warpOffset.x = pos.x - mDelta.pos.x;
      mDelta.warpOffset.y = pos.y - mDelta.pos.y;
      mDelta.warpOffset.z = ang - mDelta.ang; //purely for convienience

      //Approximate of velocity since we have none here
      F32 vel = (pos.len() + mDelta.pos.len()) * 0.5f * TickSec;
      F32 dt = (vel > 0.00001f) ? mDelta.warpOffset.len() / vel : 3; //figure our dt. Max number of warpTicks is 3 here. Probably best as a static var
      mDelta.warpTicks = (S32)((dt > 0.5)? getMax(mFloor(dt + 0.5f), 1.0f): 0.0f); //0.5 is our minimum warptick value, used to wheed out if we need to bother warping or not

      if (mDelta.warpTicks)
      {
         // Setup the warp to start on the next tick, only the
         // object's position is warped.
         if (mDelta.warpTicks > 3)
            mDelta.warpTicks = 3;
         mDelta.warpOffset /= (F32)mDelta.warpTicks;
      }
      else
      {
         // Going to skip the warp, server and client are real close.
         // Adjust the frame interpolation to move smoothly to the
         // new position within the current tick.
         Point2F cp = mDelta.pos + mDelta.posVec * mDelta.dt;
         Vector2 vec = mDelta.pos - cp;
         F32 vl = vec.len();
         if (vl) {
            F32 s = mDelta.posVec.len() / vl;
            mDelta.posVec = (cp - pos) * s;
         }
			mDelta.pos = pos;
      }
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

      /// set sprite asset.
      char buffer[256];
      stream->readString(buffer);
      mSpriteAssetId = StringTable->insert(buffer);
      if(mSpriteAssetId != StringTable->EmptyString())
         setSpriteAsset(mSpriteAssetId);

      /// set animation asset.
      char buffer2[256];
      stream->readString(buffer2);
      mAnimAssetId = StringTable->insert(buffer2);
      if(mAnimAssetId != StringTable->EmptyString())
         setAnimationAsset(mAnimAssetId);

   }

   if (stream->readFlag()) // update frame flag
   {
      stream->read(&mFrame);
      Con::printf("Unpack frame: %d", mFrame);
   }

   mFlipX = stream->readFlag();
   mFlipY = stream->readFlag();

}

void SpriteObject::processTick()
{
   if (!mAnimAsset == NULL)
   {
      mAnimController.update(TickMs);
   }

   const b2Vec2 pos = mpBody->GetPosition();
   F32 ang = mpBody->GetAngle();

   const Vector2 prePos = Vector2(mObjToWorld.getPosition().x, mObjToWorld.getPosition().y);

   // Warp to catch up to server
   if (mDelta.warpTicks > 0) 
   {
      mDelta.warpCount--;

      //Set new pos
      mDelta.pos.x = prePos.x;
      mDelta.pos.y = prePos.y;
      mDelta.pos += Point2F(mDelta.warpOffset.x, mDelta.warpOffset.y);

      mAng = mDelta.angVec;

      //Backstepping
      mDelta.pos.x = -mDelta.warpOffset.x;
      mDelta.pos.y = -mDelta.warpOffset.y;
   } 
   else
   {
      //Save current state
      mDelta.posVec = prePos;

      //Update whatever positional/angle stuff as part of the tick so we have our new data
      //Set the new state
      //Wrap up interpolation info
      mDelta.pos.x = pos.x;
      mDelta.pos.y = pos.y;
      mDelta.posVec.x -= pos.x;
      mDelta.posVec.y -= pos.y;
      mDelta.ang = ang;
   }

}

void SpriteObject::interpolateTick(F32 dt)
{
   Parent::interpolateTick(dt);

   mDelta.dt = dt;
   mPosition = mDelta.pos + mDelta.posVec * dt;
   mAng = mDelta.ang + mDelta.angVec * dt;
   MatrixF mat;
   mat.set(EulerF(0, 0, mDelta.ang), Point3F(mDelta.pos.x, mDelta.pos.y, 0.0f));
   setTransform(mat);
}

void SpriteObject::prepRenderImage(SceneCameraState* state)
{
   SpriteAsset::FrameArea::TexelArea texelArea;

   /// if sprite is not null, get it.
   if (!mSpriteAsset == NULL)
      texelArea = mSpriteAsset->getSpriteFrameArea(mFrame).mTexelArea;

   /// if there is an animation, overwrite the sprite.
   if (!mAnimAsset == NULL)
      texelArea = mAnimController.getCurrentAnimationFrame().mTexelArea;

   texelArea.setFlip(mFlipX, mFlipY);

   const F32 texLowerX = texelArea.mTexelLower.x;
   const F32 texLowerY = texelArea.mTexelLower.y;
   const F32 texUpperX = texelArea.mTexelUpper.x;
   const F32 texUpperY = texelArea.mTexelUpper.y;

   if (!nsb)
   {
      GFXStateBlockDesc desc;
      desc.setCullMode(GFXCullNone);
      desc.setZReadWrite(true, false);
      desc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
      desc.setColorWrites(true, true, true, false);
      desc.samplersDefined = true;
      desc.samplers[0] = GFXSamplerStateDesc::getClampPoint();
      nsb = GFX->createStateBlock(desc);
   }

   GFXVertexBufferHandle<GFXVertexPCT> verts(GFX, 4, GFXBufferTypeVolatile);
   verts.lock();

   F32 width = mSize.x * 0.5f * mObjScale.x;
   F32 height = mSize.y * 0.5f * mObjScale.y;

   verts[0].point.set(-width, height, 0.0f);
   verts[1].point.set(width, height, 0.0f);
   verts[2].point.set(-width, -height, 0.0f);
   verts[3].point.set(width, -height, 0.0f);

   verts[0].texCoord.set(texLowerX, texLowerY);
   verts[1].texCoord.set(texUpperX, texLowerY);
   verts[2].texCoord.set(texLowerX, texUpperY);
   verts[3].texCoord.set(texUpperX, texUpperY);

   verts.unlock();

   GFX->setVertexBuffer(verts);

   MatrixF mat = getRenderTransform();
   Point3F scale(mObjScale.x, mObjScale.y, 0.0f);
   mat.scale(scale);

   GFX->pushWorldMatrix();

   GFX->multWorld(mat);
  
   GFX->setStateBlock(nsb);

   GFX->setTexture(0, txr);
   GFX->setupGenericShaders(GFXDevice::GSTexture);
   GFX->drawPrimitive(GFXTriangleStrip, 0, 2);

   GFX->popWorldMatrix();

}

