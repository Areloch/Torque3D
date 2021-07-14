#ifndef _SCENE2D_H_
#define _SCENE2D_H_

#ifndef _MMATH_H_
#include "math/mMath.h"
#endif

#ifndef _VECTOR2_H_
#include "T2D/Math2D/Vector2.h"
#endif

#ifndef _NETOBJECT_H_
#include "sim/netObject.h"
#endif

#ifndef _ITICKABLE_H_
#include "core/iTickable.h"
#endif

///-----------------------------------------------------------------------------

class SceneObject2D;
class Game2DCtrl;

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
   public virtual ITickable
{
   typedef NetObject Parent;

   bool mIsSubScene;

protected:

   static Scene2D* smRootScene;

public:
   Scene2D();
   ~Scene2D();

   /// SimObject
   virtual bool   onAdd();
   virtual void   onRemove();
   virtual void   onDeleteNotify(SimObject* object);
   static void    initPersistFields();

   /// scene ticking
   virtual void   interpolateTick(F32 delta);
   virtual void   processTick();
   virtual void   advanceTime(F32 timeDelta);

   /// Contact processing.
   virtual void   PreSolve(b2Contact* pContact, const b2Manifold* pOldManifold);
   virtual void   PostSolve(b2Contact* pContact, const b2ContactImpulse* pImpulse);
   virtual void   BeginContact(b2Contact* pContact);
   virtual void   EndContact(b2Contact* pContact);

   ///Networking
   U32            packUpdate(NetConnection *conn, U32 mask, BitStream *stream);
   void           unpackUpdate(NetConnection *conn, BitStream *stream);
   
};

#endif // !_SCENE2D_H_
