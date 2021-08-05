#include "platform/platform.h"
#include "console/console.h"
#include "math/mathIO.h"
#include "core/stream/bitStream.h"
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

Scene2D * Scene2D::smRootScene = nullptr;
Vector<Scene2D*> Scene2D::smScene2DList;

IMPLEMENT_CO_NETOBJECT_V1(Scene2D);

Scene2D::Scene2D() :
   mpWorld(NULL),
   mpWorldGravity(0.0f, -20.0f),
   mVelocityIterations(8),
   mPositionIterations(3),
   mCameraSize(16.0f, 9.0f),
   mSceneTime(0.0f),
   mAmbientColor(0.0, 0.0, 0.0, 1.0),
   mScenePause(false),
   mScene2DId(-1)
{

   VECTOR_SET_ASSOCIATION(mObjectList);
   VECTOR_SET_ASSOCIATION(mRenderedObjectList);
   VECTOR_SET_ASSOCIATION(mServerObjectList);

   mGameModeName = StringTable->EmptyString();
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

   addGroup("Gameplay");
   addField("gameModeName", TypeString, Offset(mGameModeName, Scene2D), "The name of the gamemode that this scene utilizes");
   endGroup("Gameplay");

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

   smScene2DList.push_back(this);
   mScene2DId = smScene2DList.size() - 1;

   /// this could end up being useful for streaming scenes.
   if (smRootScene == nullptr)
   {
      //we're the first scene, so we're the root. woo!
      smRootScene = this;
      gClientScene2DGraph = this;
   }
   else
   {
      mIsSubScene = true;
      smRootScene->mSubScenes.push_back(this);
   }
   

   // Box2D 2.4.1 world with gravity
   // and set up our listeners.
   mpWorld = new b2World(mpWorldGravity);
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

   smScene2DList.remove(this);
   mScene2DId = -1;

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
   mObjectList.push_back(obj);

   return obj->onScene2DAdd();
}

void Scene2D::removeObjectFromScene(SceneObject2D* obj)
{
   obj->onSceneRemove();

   mObjectList.remove(obj);

   obj->mpScene = NULL;
}

void Scene2D::processTick()
{

   /// keep track of total time.
   mSceneTime += TickSec;

   /// step the physics
   mpWorld->Step(TickSec, mVelocityIterations, mPositionIterations);

   /// update sceneobjects
   for (S32 i = 0; i < mObjectList.size(); ++i)
   {
      mObjectList[i]->processTick();
   }

}

void Scene2D::BeginContact(b2Contact * pContact)
{
}

void Scene2D::EndContact(b2Contact * pContact)
{
}

void Scene2D::interpolateTick(F32 delta)
{

   /// update sceneobjects
   for (S32 i = 0; i < mObjectList.size(); ++i)
   {
      mObjectList[i]->interpolateTick(delta);
   }

}

void Scene2D::sceneRender2D()
{

   SceneCameraState cameraState = SceneCameraState::fromGFX();

   sceneRender2D(&cameraState);

}

void Scene2D::sceneRender2D(SceneCameraState* renderState)
{
   /// 2D needs to register its own lights on
   /// a per frame basis.
   ///LIGHTMGR->registerGlobalLight(light info, object);

   GFX->setGlobalAmbientColor(mAmbientColor);

   for (S32 i = 0; i < mObjectList.size(); ++i)
   {
      //Box3F box = renderState->getCullingFrustum().getBounds();

      SceneObject2D* obj = mObjectList[i];
      obj->prepRenderImage(renderState);

   }

   getPostRenderSignal().trigger(NULL, NULL);

   /// 2d lights should not effect 3d scenes
   //PROFILE_START(Scene2D_unregisterLights);
   //   LIGHTMGR->unregisterAllLights();
   //PROFILE_END();
}

U32 Scene2D::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   stream->write(mpWorldGravity.x);
   stream->write(mpWorldGravity.y);

   return retMask;

}

void Scene2D::unpackUpdate(NetConnection *conn, BitStream *stream)
{
   Parent::unpackUpdate(conn, stream);

   stream->read(&mpWorldGravity.x);
   stream->read(&mpWorldGravity.y);

}

void Scene2D::scopeScene(CameraScopeQuery* query, NetConnection* netConnection)
{
   for (U32 i = 0; i < mObjectList.size(); i++)
   {
      if (mObjectList[i]->isClientObject())
         netConnection->objectInScope(mRenderedObjectList[i]);
      else if (mObjectList[i]->isScopeable())
         netConnection->objectInScope(mObjectList[i]);
         
   }
}


DefineEngineFunction(getScene2D, Scene2D*, (U32 sceneId), (0),
   "Get the root Scene object that is loaded.\n"
   "@return The id of the Root Scene. Will be 0 if no root scene is loaded")
{
   if (Scene2D::smScene2DList.empty() || sceneId >= Scene2D::smScene2DList.size())
      return nullptr;

   return Scene2D::smScene2DList[sceneId];
}

DefineEngineFunction(getScene2DCount, S32, (), ,
   "Get the number of active Scene objects that are loaded.\n"
   "@return The number of active scenes")
{
   return Scene2D::smScene2DList.size();
}


