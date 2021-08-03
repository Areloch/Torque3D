#include "platform/platform.h"
#include "console/console.h"

#include "T2D/Scene/Scene2D.h"
#include "sim/netConnection.h"
#include "lighting/lightManager.h"
#include "renderInstance/renderPassManager.h"
#include "scene/sceneManager.h"

#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxDebugEvent.h"

MODULE_BEGIN(Scene_2D)

MODULE_INIT_AFTER(Sim)
MODULE_SHUTDOWN_BEFORE(Sim)

MODULE_INIT
{

   Con::addVariable("$Scene2D::renderBoundingRects", TypeBool, &Scene2D::smRenderBoundingRects,
      "If true, the bounding boxes of objects will be displayed.\n\n"
      "@ingroup Rendering");

}

MODULE_SHUTDOWN
{
}

MODULE_END;

bool Scene2D::smRenderBoundingRects;

Scene2D* gClientScene2DGraph;

Scene2D::Scene2D() :
   mpWorld(NULL),
   mpWorldGravity(0.0f, -20.0f),
   mVelocityIterations(8),
   mPositionIterations(3),
   mCameraSize(16.0f, 0.0f),
   mSceneTime(0.0f),
   mAmbientColor(1.0, 1.0, 1.0, 1.0),
   mScenePause(false)
{

   VECTOR_SET_ASSOCIATION(mObjectList);
   VECTOR_SET_ASSOCIATION(mRenderedObjectList);
   VECTOR_SET_ASSOCIATION(mServerObjectList);

}

Scene2D::~Scene2D()
{
}

void Scene2D::initPersistFields()
{
   // parent first
   Parent::initPersistFields();

   addProtectedField("Gravity", TypeVector2, Offset(mpWorldGravity, Scene2D), &defaultProtectedSetFn, &defaultProtectedGetFn,
      "Gravity force for this scene.");

   addField("VelocityIterations", TypeS32, Offset(mVelocityIterations, Scene2D),
      "Velocity iterations for Box2D in this scene.");

   addField("PositionIterations", TypeS32, Offset(mPositionIterations, Scene2D),
      "Position iterations for Box2D in this scene.");

   addField("AmbientLightColor", TypeColorF, Offset(mAmbientColor, Scene2D),
      "Scene ambient light color.");

}

bool Scene2D::ShouldCollide(b2Fixture * pFixtureA, b2Fixture * pFixtureB)
{
   /// only scene objects can collide, all objects in a scene are sceneobjects
   SceneObject2D* pSceneObjectA = static_cast<SceneObject2D*>(pFixtureA->GetBody()->GetUserData().pointer);
   SceneObject2D* pSceneObjectB = static_cast<SceneObject2D*>(pFixtureB->GetBody()->GetUserData().pointer);

   if (pSceneObjectA->mCollisionSuppress || pSceneObjectB->mCollisionSuppress)
      return false;

   if ((pSceneObjectA->mCollisionMask & pSceneObjectB->mCollisionMask) != 0)
      return true;

   if ((pSceneObjectB->mCollisionMask & pSceneObjectA->mCollisionMask) != 0)
      return true;

   return false;
}

bool Scene2D::onAdd()
{
   // Parent first
   if(!Parent::onAdd())
      return false;

   gClientScene2DGraph = this;

   // Box2D 2.4.1 world with gravity
   // and set up our listeners.
   mpWorld = new b2World((b2Vec2)mpWorldGravity);
   mpWorld->SetContactFilter(this);
   mpWorld->SetContactListener(this);
   mpWorld->SetDestructionListener(this);

   // Start ticking
   setProcessTicks(true);

   // Do you really need a comment here?
   return true;

}

void Scene2D::onRemove()
{
   setProcessTicks(false);

   while (mObjectList.size() > 0)
   {
      SceneObject2D* obj = mObjectList[0];
      removeObjectFromScene(obj);
   }

   delete mpWorld;
   mpWorld = NULL;

   Parent::onRemove();
}

void Scene2D::onDeleteNotify(SimObject * object)
{

   Parent::onDeleteNotify(object);
}


bool Scene2D::addObjectToScene(SceneObject2D* obj)
{
   obj->mpScene = this;

   /// takes on the functionality of the scenecontainer
   if (obj->isClientObject())
      mObjectList.push_back(obj);
   else
      mServerObjectList.push_back(obj);

   return obj->onScene2DAdd();
}

void Scene2D::removeObjectFromScene(SceneObject2D* obj)
{
   obj->onSceneRemove();

   if (obj->isClientObject())
      mObjectList.remove(obj);
   else
      mServerObjectList.remove(obj);

   obj->mpScene = NULL;
}

void Scene2D::processTick()
{

   PROFILE_SCOPE(Scene2D_ProcessTick);

   /// keep track of total time.
   mSceneTime += TickSec;

   /// step the physics
   mpWorld->Step(TickSec, mVelocityIterations, mPositionIterations);

   /// update sceneobjects
   for (S32 i = 0; i < mObjectList.size(); ++i)
   {
      if(mObjectList[i]->isEnabled())
         mObjectList[i]->processTick();
   }

   PROFILE_END();

}

void Scene2D::BeginContact(b2Contact * pContact)
{
}

void Scene2D::EndContact(b2Contact * pContact)
{
}

void Scene2D::interpolateTick(F32 delta)
{

   PROFILE_SCOPE(Scene2D_InterpolateTick);

   /// update sceneobjects
   for (S32 i = 0; i < mObjectList.size(); ++i)
   {
      if (mObjectList[i]->isEnabled())
         mObjectList[i]->interpolateTick(delta);
   }

   PROFILE_END();

}

void Scene2D::sceneRender2D()
{
   SceneCameraState cameraState = SceneCameraState::fromGFX();

   SceneRenderState renderState(NULL, SPT_Diffuse, cameraState);

   sceneRender2D(&renderState);

}

void Scene2D::sceneRender2D(SceneRenderState* renderState)
{
   /// 2D needs to register its own lights on
   /// a per frame basis.
   ///LIGHTMGR->registerGlobalLight(light info, object);

   renderState->setAmbientLightColor(mAmbientColor);

   PROFILE_START(Scene2D_preRenderSignal);
      mCurrentRenderState = renderState;
      getPreRenderSignal().trigger(this, renderState);
      mCurrentRenderState = NULL;
   PROFILE_END();

   for (U32 layer = MAX_LAYERS_SUPPORTED - 1; layer >= 0; layer--)
   {
      for (S32 i = 0; i < mObjectList.size(); ++i)
      {
         Box3F box = renderState->getCullingFrustum().getBounds();
         SceneObject2D* obj = mObjectList[i];
         if(obj->mSceneLayer == layer)
            obj->prepRenderImage(renderState);

      }
   }

   if (smRenderBoundingRects)
   {

      for (S32 i = 0; i < mObjectList.size(); ++i)
      {
         SceneObject2D* obj = mObjectList[i];

         const BoxVec2 worldBox = obj->getWorldBox();

         Point2F min(worldBox.minExtents.x, worldBox.minExtents.y);

         RectF rect(Point2F(worldBox.minExtents.x, worldBox.minExtents.y), Point2F(worldBox.getExtents().x, worldBox.getExtents().y));

         GFX->getDrawUtil()->drawRect(rect, ColorI::WHITE);

      }

   }

   PROFILE_START(Scene2DRender_postRenderSignal);
      mCurrentRenderState = renderState;
      getPostRenderSignal().trigger(this, renderState);
      mCurrentRenderState = NULL;
   PROFILE_END();

   /// 2d lights should not effect 3d scenes
   PROFILE_START(Scene2D_unregisterLights);
      LIGHTMGR->unregisterAllLights();
   PROFILE_END();
}

void Scene2D::scopeScene(CameraScopeQuery* query, NetConnection* netConnection)
{
   for (U32 i = 0; i < mRenderedObjectList.size(); i++)
   {
      netConnection->objectInScope(mRenderedObjectList[i]);
   }
   for (U32 i = 0; i < mServerObjectList.size(); i++)
   {
      SceneObject2D* obj = mServerObjectList[i];

      if(obj->isScopeable())
         netConnection->objectInScope(obj);

   }
}
