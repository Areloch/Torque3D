#include "platform/platform.h"
#include "T2D/Scene/Scene2D.h"
#include "math/mMath.h"
#include "sim/netConnection.h"
#include "lighting/lightManager.h"
#include "renderInstance/renderPassManager.h"
#include "scene/sceneManager.h"
#include "console/console.h"
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

Scene2D::Scene2D() :
   mIsClient(false),
   mpWorld(NULL),
   mpWorldGravity(0.0f, -20.0f),
   mVelocityIterations(8),
   mPositionIterations(3),
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

bool Scene2D::onAdd()
{
   // Parent first
   if(!Parent::onAdd())
      return false;

   if (this->isClientObject())
   {
      mIsClient = true;
      gClientScene2DGraph = this;
   }
   else
   {
      mIsClient = false;
      gServerScene2DGraph = this;
   }

   // Box2D (LiquidFun) world with grav
   mpWorld = new b2World(mpWorldGravity);

   // Start ticking
   setProcessTicks(true);

   // Do you really need a comment here?
   return true;

}

bool Scene2D::addObjectToScene(SceneObject2D* obj)
{
   obj->mpScene = this;

   /// takes on the functionality of the scenecontainer
   mObjectList.push_back(obj);

   return obj->onScene2DAdd();
}

void Scene2D::removeObjectFromScene(SceneObject2D* obj)
{
   obj->onSceneRemove();

   if (mIsClient)
      mObjectList.remove(obj);
   else
      mServerObjectList.remove(obj);

   obj->mpScene = NULL;
}

void Scene2D::processTick()
{

   PROFILE_SCOPE(Scene2D_ProcessTick);

   mSceneTime += TickSec;

   /// step the physics
   mpWorld->Step(TickSec, mVelocityIterations, mPositionIterations);

   /// update sceneobjects
   for (S32 i = 0; i < mObjectList.size(); ++i)
   {
      mObjectList[i]->processTick();
   }


   PROFILE_END();

}

void Scene2D::interpolateTick(F32 delta)
{
   if (mScenePause)
      return;

   PROFILE_SCOPE(Scene2D_InterpolateTick);

   /// update sceneobjects
   for (S32 i = 0; i < mObjectList.size(); ++i)
   {
      mObjectList[i]->interpolateTick(delta);
   }

   PROFILE_END();

}

void Scene2D::sceneRender2D()
{
   SceneCameraState cameraState = SceneCameraState::fromGFX();

   SceneManager *temp = new SceneManager(true);

   SceneRenderState renderState(NULL, SPT_Diffuse, cameraState);

   sceneRender2D(&renderState);

}

void Scene2D::sceneRender2D(SceneRenderState* renderState)
{
   PROFILE_START(Scene2D_registerLights);
   LIGHTMGR->registerGlobalLights(&renderState->getCullingFrustum(), false, true);
   PROFILE_END();

   renderState->setAmbientLightColor(mAmbientColor);

   PROFILE_START(Scene2D_preRenderSignal);
      mCurrentRenderState = renderState;
      getPreRenderSignal().trigger(this, renderState);
      mCurrentRenderState = NULL;
   PROFILE_END();

   for (S32 i = 0; i < mObjectList.size(); ++i)
   {
      mObjectList[i]->prepRenderImage(renderState);
   }

   if (smRenderBoundingRects)
   {

      for (S32 i = 0; i < mObjectList.size(); ++i)
      {
         SceneObject2D* obj = mObjectList[i];

         const BoxVec2 worldBox = obj->getWorldBox();

         RectF rect((Point2F&)worldBox.minExtents, (Point2F&)worldBox.maxExtents);

         GFX->getDrawUtil()->drawRect(rect, ColorI::WHITE);

      }

   }

   PROFILE_START(Scene2DRender_postRenderSignal);
      mCurrentRenderState = renderState;
      getPostRenderSignal().trigger(this, renderState);
      mCurrentRenderState = NULL;
   PROFILE_END();

   PROFILE_START(Scene2D_unregisterLights);
      LIGHTMGR->unregisterAllLights();
   PROFILE_END();
}

void Scene2D::scopeScene(CameraScopeQuery* query, NetConnection* netConnection)
{
   if (mIsClient)
   {
      for (U32 i = 0; i < mRenderedObjectList.size(); i++)
      {
         netConnection->objectInScope(mRenderedObjectList[i]);
      }
   }
   else
   {
      for (U32 i = 0; i < mObjectList.size(); i++)
      {
         SceneObject2D* obj = mObjectList[i];

         if(obj->isScopeable())
            netConnection->objectInScope(obj);

      }
   }
}
