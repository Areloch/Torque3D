#include "platform/platform.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "T2D/Scene/SceneObject2D.h"
#include "core/stream/bitStream.h"
#include "sim/netConnection.h"
#include "T3D/gameBase/gameConnection.h"
#include "scene/sceneRenderState.h"

#include "T2D/Scene/Scene2D.h"

SceneObject2D::SceneObject2D() :
   mpScene(NULL),
   mpBody(NULL),
   mLifetime(0.0f),
   mSceneLayer(0),
   mSceneLayerMask(BIT(mSceneLayer)),
   mSceneLayerDepth(0.0f),
   mCollisionMask(MASK_ALL),
   mCollisionSuppress(false),
   mCollisionOneWay(false)
{

   mpBodyDef.userData = this;
   mpBodyDef.position.Set(0.0f, 0.0f);
   mpBodyDef.angle = 0.0f;
   mpBodyDef.linearVelocity.Set(0.0f, 0.0f);
   mpBodyDef.angularVelocity = 0.0f;
   mpBodyDef.linearDamping = 0.0f;
   mpBodyDef.angularDamping = 0.0f;
   mpBodyDef.allowSleep = true;
   mpBodyDef.awake = true;
   mpBodyDef.fixedRotation = false;
   mpBodyDef.bullet = false;
   mpBodyDef.type = b2_dynamicBody;
   mpBodyDef.active = true;
   mpBodyDef.gravityScale = 1.0f;

   mObjScale.set(1, 1);
   mObjToWorld.identity();
   mWorldToObj.identity();

   mPosition.set(0.0f, 0.0f);
   mAng = 0.0f;

   mRenderObjToWorld.identity();
   mRenderWorldToObj.identity();

   mObjBox = BoxVec2(Vector2(0.0f, 0.0f), Vector2(0.0f, 0.0f));
   mWorldBox = BoxVec2(Vector2(0.0f, 0.0f), Vector2(0.0f, 0.0f));
}

bool SceneObject2D::onAdd()
{
   if (!Parent::onAdd())
      return false;

   mWorldToObj = mObjToWorld;
   mWorldToObj.affineInverse();
   resetWorldBox();

   setRenderTransform(mObjToWorld);

}

void SceneObject2D::addToScene()
{
   if (mpScene)
      return;

   if (isClientObject())
      gClientScene2DGraph->addObjectToScene(this);
   else
      gServerScene2DGraph->addObjectToScene(this);
}

void SceneObject2D::removeFromScene()
{
   if (!mpScene)
      return;

   mpScene->removeObjectFromScene(this);
}

bool SceneObject2D::onScene2DAdd()
{
   if(!mpScene)
      return false;

   mpBody = mpScene->getWorld()->CreateBody(&mpBodyDef);

   return true;

}

void SceneObject2D::onSceneRemove()
{
   // Destroy the physics body.
   mpScene->getWorld()->DestroyBody(mpBody);
   mpBody = NULL;
}

void SceneObject2D::resetWorldBox()
{

   mWorldBox = mObjBox;
   mWorldBox.minExtents.convolve(mObjScale);
   mWorldBox.maxExtents.convolve(mObjScale);

   Box3F mWorldBox3D(Point3F(mWorldBox.minExtents.x,mWorldBox.minExtents.y,0.0f),
                     Point3F(mWorldBox.maxExtents.x, mWorldBox.maxExtents.y, 0.0f));

   mObjToWorld.mul(mWorldBox3D);

}

void SceneObject2D::resetRenderWorldBox()
{
   mRenderWorldBox = mObjBox;
   mRenderWorldBox.minExtents.convolve(mObjScale);
   mRenderWorldBox.maxExtents.convolve(mObjScale);

   Box3F mWorldBox3D(Point3F(mWorldBox.minExtents.x, mWorldBox.minExtents.y, 0.0f),
      Point3F(mWorldBox.maxExtents.x, mWorldBox.maxExtents.y, 0.0f));

   mRenderObjToWorld.mul(mWorldBox3D);
}

void SceneObject2D::resetObjectBox()
{
   mObjBox = mWorldBox;
   Vector2 objScale(mObjScale);
   objScale.setMax(Vector2((F32)POINT_EPSILON, (F32)POINT_EPSILON));
   mObjBox.minExtents.convolve(objScale);
   mObjBox.maxExtents.convolve(objScale);

}

void SceneObject2D::setPosition(const Vector2 &pos)
{
   mPosition = pos;
   MatrixF xfm = mObjToWorld;
   xfm.setColumn(3, Point3F(mPosition.x, mPosition.y, 0.0f));
   setTransform(xfm);
}

void SceneObject2D::setAngle(const F32 &ang)
{
   mAng = ang;
   MatrixF tXfm = mObjToWorld;
   tXfm.set(EulerF(0.0f, 0.0f, mAng), mObjToWorld.getPosition());
   setTransform(tXfm);
}

void SceneObject2D::setScale(const Vector2 &scale)
{
   if (mObjScale.isEqual(scale))
      return;

   mObjScale = scale;

   setTransform(mObjToWorld);

   setMaskBits(ScaleMask);
}

void SceneObject2D::setTransform(const MatrixF& mat)
{
   mObjToWorld = mWorldToObj = mat;
   mWorldToObj.affineInverse();

   resetWorldBox();

   setRenderTransform(mat);
   setMaskBits(MoveMask);
}

void SceneObject2D::setRenderTransform(const MatrixF& mat)
{
   mRenderObjToWorld = mRenderWorldToObj = mat;
   mRenderWorldToObj.affineInverse();

   resetRenderWorldBox();
}

bool SceneObject2D::writeField(StringTableEntry fieldName, const char * value)
{
   if(!Parent::writeField(fieldName, value))
      return false;

   return true;
}

void SceneObject2D::interpolateTick(F32 delta)
{
   if (delta < 1.0f)
   {
      /// render pos
      Vector2 pos = mPosition;
      /// body pos
      Vector2 bPos = mpBody->GetPosition();

      Vector2 posDelta = bPos - pos;

      posDelta *= delta;

      /// setRender positon.
      setPosition(bPos - posDelta);

      F32 bAng = mpBody->GetAngle();

      F32 rel = bAng - mAng;

      if (rel > M_PI_F)
         rel -= M_2PI_F;
      else if (rel < -M_PI_F)
         rel += M_2PI_F;

      /// set render angle
      setAngle(bAng-(rel * delta));
   }
}

void SceneObject2D::processTick()
{

}

U32 SceneObject2D::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   if (stream->writeFlag(mask & MoveMask))
   {
      Point3F pos;
      getTransform().getColumn(3, &pos);
      stream->writeCompressedPoint(pos);
      stream->writeFloat(mAng / M_2PI_F, 7);
   }

   return retMask;

}

void SceneObject2D::unpackUpdate(NetConnection *conn, BitStream *stream)
{
   Parent::unpackUpdate(conn, stream);

   ///MoveMask
   if (stream->readFlag())
   {
      Point3F pos, rot;
      stream->readCompressedPoint(&pos);
      rot.z = stream->readFloat(7) * M_2PI_F;

      setPosition(Vector2(pos.x, pos.y));
      setAngle(rot.z);
   }

}

void SceneObject2D::onCameraScopeQuery(NetConnection * connection, CameraScopeQuery * query)
{

   GameConnection* conn = dynamic_cast<GameConnection*> (connection);

   if (this->isScopeable())
      connection->objectInScope(this);

   if (mpScene->isClientScene())
      mpScene->scopeScene(query, connection);
   else
      gServerScene2DGraph->scopeScene(query, connection);

}
