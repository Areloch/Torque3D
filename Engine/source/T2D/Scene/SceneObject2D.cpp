#include "platform/platform.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "T2D/Scene/SceneObject2D.h"
#include "scene/sceneRenderState.h"
#include "core/stream/bitStream.h"
#include "sim/netConnection.h"
#include "T3D/gameBase/gameConnection.h"
#include "scene/sceneRenderState.h"

#include "T2D/Scene/Scene2D.h"

IMPLEMENT_CONOBJECT(SceneObject2D);

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

   mNetFlags.set(Ghostable | ScopeAlways);

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
   mpBodyDef.enabled = true;
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

SceneObject2D::~SceneObject2D()
{

   if(mpScene)
      removeFromScene();
      
}

bool SceneObject2D::onAdd()
{
   if (!Parent::onAdd())
      return false;

   mWorldToObj = mObjToWorld;
   mWorldToObj.affineInverse();
   resetWorldBox();

   setRenderTransform(mObjToWorld);

   return true;
}

void SceneObject2D::onRemove()
{
   
   if(mpScene)
      removeFromScene();

   Parent::onRemove();

}

void SceneObject2D::addToScene()
{
   if (mpScene)
      return;

   gClientScene2DGraph->addObjectToScene(this);
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

   mpBody->GetUserData().pointer = this;

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
}

void SceneObject2D::setRenderTransform(const MatrixF& mat)
{
   mRenderObjToWorld = mRenderWorldToObj = mat;
   mRenderWorldToObj.affineInverse();

   resetRenderWorldBox();
}

void SceneObject2D::onDeleteNotify(SimObject * object)
{
   Parent::onDeleteNotify(object);
}

void SceneObject2D::inspectPostApply()
{
   Parent::inspectPostApply();
}

bool SceneObject2D::writeField(StringTableEntry fieldName, const char * value)
{
   if(!Parent::writeField(fieldName, value))
      return false;

   return true;
}

void SceneObject2D::interpolateTick(F32 delta)
{
   
}

void SceneObject2D::processTick()
{

}

void SceneObject2D::initPersistFields()
{
   Parent::initPersistFields();
}

void SceneObject2D::writePacketData(GameConnection*, BitStream*)
{
}

void SceneObject2D::readPacketData(GameConnection*, BitStream*)
{
}


U32 SceneObject2D::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   if (stream->writeFlag(mask & FlagMask))
      stream->writeRangedU32((U32)mObjectFlags, 0, getObjectFlagMax());

   return retMask;

}

void SceneObject2D::unpackUpdate(NetConnection *conn, BitStream *stream)
{
   Parent::unpackUpdate(conn, stream);

   if (stream->readFlag())
      mObjectFlags = stream->readRangedU32(0, getObjectFlagMax());

}

void SceneObject2D::onCameraScopeQuery(NetConnection * connection, CameraScopeQuery * query)
{

   GameConnection* conn = dynamic_cast<GameConnection*> (connection);

   if (this->isScopeable())
      conn->objectInScope(this);

   mpScene->scopeScene(query, conn);

}
