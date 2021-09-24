#include "platform/platform.h"
#include "T3D/physics/stock/stockWorld.h"

#include "T3D/physics/physicsUserData.h"
#include "core/stream/bitStream.h"
#include "platform/profiler.h"
#include "sim/netConnection.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "scene/sceneRenderState.h"
#include "T3D/gameBase/gameProcess.h"
#include "T3D/physics/stock/stockBody.h"

static U32 sCollisionMoveMask = TerrainObjectType |
                                 WaterObjectType |
                                 PlayerObjectType |
                                 StaticShapeObjectType |
                                 VehicleObjectType |
                                 PhysicalZoneObjectType |
                                 PathShapeObjectType;

StockWorld::StockWorld()
   :  mProcessList(NULL),
      mIsSimulating(false),
      mErrorReport(false),
      mTickCount(0),
      mIsEnabled(false),
      mEditorTimeScale(1.0f)
{
}

StockWorld::~StockWorld()
{
}

void StockWorld::addBody(PhysicsBody* body)
{
   /// for now just fire everything in here.
   mNonStaticBodies.push_back(body);

   S32 workingSetEntry = findWorkingSet(body);
   if (workingSetEntry == -1)
   {
      UpdateWorkingSet workSet;
      workSet.mBody = static_cast<StockBody*>(body);
      
      workSet.mConvexTester.init(workSet.mBody->getObject());
      Box3F objBox = workSet.mBody->getObject()->getObjBox();
      objBox.getCenter(&workSet.mConvexTester.mCenter);
      workSet.mConvexTester.mSize.x = objBox.len_x() / 2.0;
      workSet.mConvexTester.mSize.y = objBox.len_y() / 2.0;
      workSet.mConvexTester.mSize.z = objBox.len_z() / 2.0;
      workSet.mWorkingQueryBox.minExtents.set(-1e9f, -1e9f, -1e9f);
      workSet.mWorkingQueryBox.maxExtents.set(-1e9f, -1e9f, -1e9f);

      mWorkingSets.push_back(workSet);
   }
}

bool StockWorld::initWorld(bool isServer, ProcessList *processList)
{
   /// just cos.
   if (!processList)
      return false;

   mIsServer = isServer;

   mProcessList = processList;
   mProcessList->preTickSignal().notify(this, &StockWorld::getPhysicsResults );
   mProcessList->postTickSignal().notify(this, &StockWorld::tickPhysics, 1000.0f);

   return true;
}

void StockWorld::_destroy()
{
   // Release the tick processing signals.
   if (mProcessList)
   {
      mProcessList->preTickSignal().remove(this, &StockWorld::getPhysicsResults);
      mProcessList->postTickSignal().remove(this, &StockWorld::tickPhysics);
      mProcessList = NULL;
   }
}

void StockWorld::tickPhysics(U32 elapsedMs)
{
   if (!mIsEnabled)
      return;

   AssertFatal(!mIsSimulating, "StockWorld::tickPhysics() - Already Simulating!");
   AssertFatal(elapsedMs != 0 &&
      (elapsedMs % TickMs) == 0, "StockWorld::tickPhysics() - Got bad elapsed time!");

   PROFILE_SCOPE(StockWorld_tickPhysics);

   const F32 elapsedSec = (F32)elapsedMs * 0.001f;

   stepWorld(elapsedSec * mEditorTimeScale, smPhysicsMaxSubSteps, smPhysicsStepTime);

   mIsSimulating = true;

}

void StockWorld::clearForces()
{
   for (U32 i = 0; i < mNonStaticBodies.size(); i++)
   {
      StockBody* body = static_cast<StockBody*>(mNonStaticBodies[i]);
      if (!body->isDynamic())
         continue;

      body->clearForces();

   }
}

void StockWorld::applyGravity()
{
   for (U32 i = 0; i < mNonStaticBodies.size(); i++)
   {
      StockBody* body = static_cast<StockBody*>(mNonStaticBodies[i]);
      if (!body->isDynamic())
         continue;

      body->applyForce(mGravity);

   }
}

void StockWorld::stepWorld(F32 elapsed, U32 steps, F32 stepTime)
{
   /// clear forces.
   clearForces();
   /// apply world gravity once.
   applyGravity();
   F32 adjTime = elapsed / steps;

   for (U32 i = 0; i < steps; i++)
   {
      for (U32 j = 0; j < mWorkingSets.size(); j++)
      {
         if (!mWorkingSets[j].mBody->isDynamic())
            continue;

         updateWorkingCollisionSet(&mWorkingSets[j]);

         mWorkingSets[j].mBody->updatePos(&mWorkingSets[j], adjTime);
      }

      /*for (U32 j = 0; j < mNonStaticBodies.size(); j++)
      {
         StockBody* body = static_cast<StockBody*>(mNonStaticBodies[j]);
         if (!body->isDynamic())
            continue;

         /// collision filtering handled by body flags.
         //body->updateWorkingCollisionSet();
         //body->updateForces(elapsed);
         body->updatePos(adjTime);
      }*/
   }
}

void StockWorld::getPhysicsResults()
{
   if (!mIsSimulating)
      return;

   PROFILE_SCOPE(StockWorld_GetPhysicsResults);

   mIsSimulating = false;
   mTickCount++;
}

void StockWorld::setEnabled(bool enabled)
{
   mIsEnabled = enabled;

   if (!mIsEnabled)
      getPhysicsResults();
}

PhysicsBody* StockWorld::castRay(const Point3F& start, const Point3F& end, U32 bodyTypes)
{
   /*for (U32 i = 0; i < mNonStaticBodies.size(); i++)
   {
      mNonStaticBodies[i]->cas
   }*/
   return NULL;
}

//
S32 StockWorld::findWorkingSet(PhysicsBody* body)
{
   for (U32 i = 0; i < mWorkingSets.size(); i++)
   {
      if (mWorkingSets[i].mBody == body)
         return i;
   }

   return -1;
}

void StockWorld::updateWorkingCollisionSet(UpdateWorkingSet* set)
{
   StockBody* body = set->mBody;
   StockCollision* colShape = static_cast<StockCollision*>(body->getColShape());
   SceneObject* mUserObject = body->getObject();

   // First, we need to adjust our velocity for possible acceleration.  It is assumed
   // that we will never accelerate more than 20 m/s for gravity, plus 10 m/s for
   // jetting, and an equivalent 10 m/s for jumping.  We also assume that the
   // working list is updated on a Tick basis, which means we only expand our
   // box by the possible movement in that tick.
   Point3F scaledVelocity = body->getLinVelocity() * TickSec;
   F32 len = scaledVelocity.len();
   F32 newLen = len + (10.0f * TickSec);


   // Check to see if it is actually necessary to construct the new working list,
   //  or if we can use the cached version from the last query.  We use the x
   //  component of the min member of the mWorkingQueryBox, which is lame, but
   //  it works ok.
   set->needsUpdate = false;
   /// predicted transform
   //F32 len = (mLinVelocity.len() + 50) * TickSec;
   MatrixF transform;
   body->getTransform(&transform);
   transform.setPosition(transform.getPosition() + body->getLinVelocity() * TickSec);
   //Box3F convexBox = mColShape->getConvexList()->getBoundingBox(transform, mUserData.getObject()->getScale());
   /// make convex box from shapes AABB.
   Box3F convexBox = body->getAABB();
   /// move it to the predicted position.
   convexBox.setCenter(transform.getPosition());
   F32 l = (newLen * 1.1f) + 0.1f;  // from Convex::updateWorkingList
   const Point3F  lPoint(l, l, l);
   convexBox.minExtents -= lPoint;
   convexBox.maxExtents += lPoint;

   // Check containment
   if (set->mWorkingQueryBox.minExtents.x != -1e9f)
   {
      if (set->mWorkingQueryBox.isContained(convexBox) == false)
         // Needed region is outside the cached region.  Update it.
         set->needsUpdate = true;
   }
   else
   {
      // Must update
      set->needsUpdate = true;
   }
   // Actually perform the query, if necessary
   if (set->needsUpdate == true)
   {
      const Point3F  twolPoint(2.0f * l, 2.0f * l, 2.0f * l);
      set->mWorkingQueryBox = convexBox;
      set->mWorkingQueryBox.minExtents -= twolPoint;
      set->mWorkingQueryBox.maxExtents += twolPoint;

      if (mUserObject)
         mUserObject->disableCollision();

      //We temporarily disable the collisions of anything mounted to us so we don't accidentally walk into things we've attached to us
      for (SceneObject* ptr = mUserObject->getMountList(); ptr; ptr = ptr->getMountLink())
      {
         ptr->disableCollision();
      }

      set->mConvexTester.updateWorkingList(set->mWorkingQueryBox, sCollisionMoveMask);

      //And now re-enable the collisions of the mounted things
      for (SceneObject* ptr = mUserObject->getMountList(); ptr; ptr = ptr->getMountLink())
      {
         ptr->enableCollision();
      }

      if (mUserObject)
         mUserObject->enableCollision();
   }
}
