#ifndef _SCENEOBJECT2D_H_
#define _SCENEOBJECT2D_H_

#ifndef _SCENE2D_H_
#include "T2D/Scene/Scene2D.h"
#endif // !_SCENE2D_H_

#ifndef _SCENERENDERSTATE_H_
#include "scene/sceneRenderState.h"
#endif // !_SCENERENDERSTATE_H_

#ifndef _NETOBJECT_H_
#include "sim/netObject.h"
#endif

#ifndef _COLOR_H_
#include "core/color.h"
#endif

#ifndef _BITSET_H_
#include "core/bitSet.h"
#endif

#ifndef _GAME2DCTRL_H_
#include "T2D/game2DCtrl.h"
#endif // !_GAME2DCTRL_H_

#ifndef _ASSET_PTR_H_
#include "assets/assetPtr.h"
#endif

#ifndef _BEHAVIORCOMPONENT_H_
#include "component/behaviors/behaviorComponent.h"
#endif


#ifndef _PROCESSLIST_H_
#include "T3D/gameBase/processList.h"
#endif

///-----------------------------------------------------------------------------

typedef VectorPtr<b2FixtureDef*> typeCollisionFixtureDefVector;
typedef VectorPtr<b2Fixture*> typeCollisionFixtureVector;

extern EnumTable bodyTypeTable;

class SceneObject2D : public BehaviorComponent
{
   typedef BehaviorComponent Parent;

public:
   friend class Scene2D;

   /// Networking dirty mask.
   enum SceneObject2DMasks
   {
      InitialUpdateMask = BIT(0),
      ScaleMask = BIT(1),
      FlagMask = BIT(2),
      MoveMask = BIT(3),
      NextFreeMask = BIT(4)
   };

   enum SceneObject2DFlags
   {
      RenderEnabledFlag = BIT(0),
      NextFreeFlag = BIT(1)
   };

protected:

   BitSet32 mObjectFlags;

   virtual U32 getObjectFlagMax() const { return NextFreeFlag - 1; }

public:

   /// these need to be accessed by other classes
   /// Lifetime.
   Scene2D*                mpScene;
   F32                     mLifetime;
   bool                    mLifetimeActive;

   /// Collision handling
   b2Body*                 mpBody;
   b2BodyDef               mpBodyDef;
   bool                    mCollisionSuppress;
   bool                    mCollisionOneWay;
   U32                     mCollisionMask;

   Vector2                 mPosition;
   F32                     mAng;

   ///
   MatrixF                 mObjToWorld;
   MatrixF                 mWorldToObj;
   MatrixF                 mRenderObjToWorld;
   MatrixF                 mRenderWorldToObj;
   Vector2                 mObjScale;
   BoxVec2                 mObjBox;
   BoxVec2                 mWorldBox;
   BoxVec2                 mRenderWorldBox;

   /// Scene layers.
   U32                     mSceneLayer;
   U32                     mSceneLayerMask;
   F32                     mSceneLayerDepth;

   SceneObject2D();
   virtual ~SceneObject2D();

   void addToScene();
   void removeFromScene();

   Scene2D* getScene() const { return mpScene; }

   virtual bool onScene2DAdd();
   virtual void onSceneRemove();

   // NetObject.
   virtual U32 packUpdate(NetConnection* conn, U32 mask, BitStream* stream);
   virtual void unpackUpdate(NetConnection* conn, BitStream* stream);

   virtual void onCameraScopeQuery(NetConnection* connection, CameraScopeQuery* query);

   // SimObject.
   virtual bool onAdd();
   virtual void onRemove();

   DECLARE_CONOBJECT(SceneObject2D);

   virtual void onDeleteNotify(SimObject *object);
   virtual void inspectPostApply();
   virtual bool writeField(StringTableEntry fieldName, const char* value);

   /// scene ticking
   /// updates sent from scene.
   virtual void   interpolateTick(F32 delta);
   virtual void   processTick();
   virtual void   advanceTime(F32 timeDelta) {};

   virtual void prepRenderImage(SceneCameraState* cam) {}

   static void initPersistFields();

   const BoxVec2& getObjBox() const { return mObjBox; }
   const BoxVec2& getWorldBox() const { return mWorldBox; }
   virtual const MatrixF& getTransform() const { return mObjToWorld; }
   const MatrixF& getRenderTransform() const { return mRenderObjToWorld; }

   void resetWorldBox();
   void resetRenderWorldBox();
   void resetObjectBox();

   void setPosition(const Vector2 &pos);
   void setAngle(const F32 &ang);
   void setScale(const Vector2 &scale);

   void setTransform(const MatrixF& mat);
   void setRenderTransform(const MatrixF & mat);
   
};

#endif // !_SCENEOBJECT2D_H_



