#ifndef _SCENE2D_H_
#define _SCENE2D_H_

#ifndef _MMATH_H_
#include "math/mMath.h"
#endif

#ifndef _VECTOR2_H_
#include "T2D/Math2D/Vector2.h"
#endif

#ifndef _SCENEOBJECT2D_H_
#include "T2D/Scene/SceneObject2D.h"
#endif // !_SCENEOBJECT2D_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _COLOR_H_
#include "core/color.h"
#endif

#ifndef _BEHAVIORCOMPONENT_H_
#include "component/behaviors/behaviorComponent.h"
#endif

#ifndef _TSIGNAL_H_
#include "core/util/tSignal.h"
#endif

#ifndef _INTERPOLATEDCHANGEPROPERTY_H_
#include "util/interpolatedChangeProperty.h"
#endif

#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif

#ifndef _FOGSTRUCTS_H_
#include "scene/fogStructs.h"
#endif

#ifndef _NETOBJECT_H_
#include "sim/netObject.h"
#endif

#ifndef _SCENERENDERSTATE_H_
#include "scene/sceneRenderState.h"
#endif // !_SCENERENDERSTATE_H_

#ifndef _SCENECAMERASTATE_H_
#include "scene/sceneCameraState.h"
#endif // !_SCENECAMERASTATE_H_

#ifndef _ITICKABLE_H_
#include "core/iTickable.h"
#endif

#ifndef BOX2D_H
#include "Box2D/Box2D.h"
#endif


///-----------------------------------------------------------------------------

class SceneObject2D;
class Game2DCtrl;
class NetConnection;
class RenderPassManager;
class LightManager;
class SceneRenderState;
class SceneCameraState;

///-----------------------------------------------------------------------------

struct TickContact
{
   TickContact()
   {
      initialize(NULL, NULL, NULL, NULL, NULL);
   }

   void initialize(
      b2Contact*      pContact,
      SceneObject2D*  pSceneObjectA,
      SceneObject2D*  pSceneObjectB,
      b2Fixture*      pFixtureA,
      b2Fixture*      pFixtureB)
   {
      mpContact      = pContact;
      mpSceneObjectA = pSceneObjectA;
      mpSceneObjectB = pSceneObjectB;
      mpFixtureA     = pFixtureA;
      mpFixtureB     = pFixtureB;

      // Get world manifold. 
      if (mpContact != NULL)
      {
         mPointCount = pContact->GetManifold()->pointCount;
         mpContact->GetWorldManifold(&mWorldManifold);
      }
      else
      {
         mPointCount = 0;
      }

      // Reset impulses.
      for (U32 i = 0; i < b2_maxManifoldPoints; i++)
      {
         mNormalImpulses[i] = 0;
         mTangentImpulses[i] = 0;
      }
   }

   inline SceneObject2D* getCollideWith(SceneObject2D* pMe) const
   {
      return pMe == mpSceneObjectA ? mpSceneObjectB : mpSceneObjectA;
   }

   inline b2Fixture* getCollideWithFixture(b2Fixture* pMe) const
   {
      return pMe == mpFixtureA ? mpFixtureB : mpFixtureA;
   }

   b2Contact*      mpContact;
   SceneObject2D*  mpSceneObjectA;
   SceneObject2D*  mpSceneObjectB;
   b2Fixture*      mpFixtureA;
   b2Fixture*      mpFixtureB;
   U32             mPointCount;
   b2WorldManifold mWorldManifold;
   F32             mNormalImpulses[b2_maxManifoldPoints];
   F32             mTangentImpulses[b2_maxManifoldPoints];
};

///-----------------------------------------------------------------------------

class Scene2D :
   public NetObject,
   public b2ContactListener,
   public b2DestructionListener,
   public b2ContactFilter,
   public virtual ITickable
{
   typedef NetObject Parent;

   bool mIsSubScene;

   S32 mScene2DId;

protected:

   static Scene2D* smRootScene;
   SceneRenderState* mCurrentRenderState;
   typedef InterpolatedChangeProperty< LinearColorF > AmbientLightInterpolator;
   LightManager* mLightManager;
   AmbientLightInterpolator mAmbientLightColor;
   Point2F mCameraSize;

   MatrixSet *mMatrixSet;

private:

   b2World*          mpWorld;
   Vector2           mpWorldGravity;
   S32               mVelocityIterations;
   S32               mPositionIterations;
   b2BlockAllocator  mBlockAllocator;
   F32               mSceneTime;
   bool              mScenePause;
   LinearColorF      mAmbientColor;

public:

   typedef Signal< void(Scene2D*, const SceneRenderState*) > RenderSignal;
   typedef Vector<SceneObject2D*> SceneObjectList;

   SceneObjectList   mObjectList;
   SceneObjectList   mRenderedObjectList;
   SceneObjectList   mServerObjectList;

   static bool smRenderBoundingRects;

   static RenderSignal& getPreRenderSignal()
   {
      static RenderSignal theSignal;
      return theSignal;
   }

   static RenderSignal& getPostRenderSignal()
   {
      static RenderSignal theSignal;
      return theSignal;
   }

   Scene2D();
   ~Scene2D();

   RenderPassManager * getDefaultRenderPass() const;

   /// SimObject
   virtual bool   onAdd();

   

   bool addObjectToScene(SceneObject2D * obj);
   void removeObjectFromScene(SceneObject2D * obj);

   virtual void   onRemove();
   virtual void   onDeleteNotify(SimObject* object);
   static void    initPersistFields();
   Point2F getCameraSize() { return mCameraSize; }
   void setCameraSize(Point2F camSize) { mCameraSize = camSize; }

   /// Destruction Listener
   virtual void   SayGoodbye(b2Joint* pJoint) {}
   virtual void   SayGoodbye(b2Fixture* pFixture) {}

   /// contact filter callback.
   /// makes sense the scene handling these too.
   virtual bool ShouldCollide(b2Fixture* pFixtureA, b2Fixture* pFixtureB);

   /// Contact processing Box2D
   //virtual void   PreSolve(b2Contact* pContact, const b2Manifold* pOldManifold);
   //virtual void   PostSolve(b2Contact* pContact, const b2ContactImpulse* pImpulse);
   virtual void   BeginContact(b2Contact* pContact);
   virtual void   EndContact(b2Contact* pContact);

   /// scene ticking
   virtual void   interpolateTick(F32 delta);
   virtual void   processTick();
   virtual void   advanceTime(F32 timeDelta) {};

   /// scene render
   void           sceneRender2D();
   void           sceneRender2D(SceneCameraState * renderState);

   ///Networking
   U32            packUpdate(NetConnection *conn, U32 mask, BitStream *stream);
   void           unpackUpdate(NetConnection *conn, BitStream *stream);

   DECLARE_CONOBJECT(Scene2D);
   /// scope scene
   void scopeScene(CameraScopeQuery* query, NetConnection* netConnection);

   inline b2World*         getWorld(void) const { return mpWorld; }

   static Scene2D *getRootScene2D()
   {
      if (Scene2D::smScene2DList.empty())
         return nullptr;

      return Scene2D::smScene2DList[0];
   }

   static Vector<Scene2D*> smScene2DList;

   
};

extern Scene2D* gClientScene2DGraph;


#endif // !_SCENE2D_H_
