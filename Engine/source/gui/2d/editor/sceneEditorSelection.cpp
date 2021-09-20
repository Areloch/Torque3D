#include "gui/2d/editor/sceneEditorSelection.h"
#include "gui/2d/editor/sceneEditor.h"
#include "T2D/Scene/SceneObject2D.h"

IMPLEMENT_CONOBJECT(SceneEditorSelection);

SceneEditorSelection::SceneEditorSelection()
   :  mCentroidValid(false),
      mAutoSelect(false),
      mPrevCentroid(0.0f, 0.0f)
{
   setCanSave(false);
   setEditorOnly(true);
}

SceneEditorSelection::~SceneEditorSelection()
{
}

void SceneEditorSelection::initPersistFields()
{
   Parent::initPersistFields();
}

void SceneEditorSelection::addObject(SimObject* obj)
{
   /// straight up copy from worldeditorselection and i aint even ashamed.
   if (objInSet(obj))
      return;

   if (isLocked())
      return;

   if (obj == this)
      return;

   SceneEditorSelection* sel = dynamic_cast<SceneEditorSelection*>(obj);
   if (sel && !sel->objInSet(this))
      return;

   for (SimGroup* grp = getGroup(); grp != NULL; grp = grp->getGroup())
      if (obj == grp)
         return;

   invalidateCentroid();

   Parent::addObject(obj);

   return;
}

void SceneEditorSelection::removeObject(SimObject * obj)
{
   if (!objInSet(obj))
      return;

   // Refuse to remove object if this selection is locked.

   if (isLocked())
      return;

   invalidateCentroid();

   Parent::removeObject(obj);

   return;
}

void SceneEditorSelection::setCanSave(bool value)
{
   if (getCanSave() == value)
      return;

   Parent::setCanSave( value );

   if (getCanSave())
      for (iterator itr = begin(); itr != end(); ++itr)
         (*itr)->getOrCreatePersistentId();

}

bool SceneEditorSelection::objInSet(SimObject * obj)
{
   if (!mIsResolvingPIDs)
      resolvePIDs();

   lock();

   bool res = false;

   for (iterator itr = begin(); itr != end(); ++itr)
   {
      if (obj == *itr)
      {
         res = true;
         break;
      }

      SceneEditorSelection* sel = dynamic_cast<SceneEditorSelection*>(*itr);
      if (sel && sel->objInSet(obj))
      {
         res = true;
         break;
      }

   }

   unlock();

   return res;

}

const Vector2& SceneEditorSelection::getCentroid()
{
   updateCentroid();
   return (mCentroid);
}

const BoxVec2& SceneEditorSelection::getBoxBounds()
{
   updateCentroid();
   return (mBoxBounds);
}

Vector2 SceneEditorSelection::getBoxBottomCenter()
{
   updateCentroid();

   Vector2 bottomCenter = mBoxCentroid;
   bottomCenter.y -= mBoxBounds.len_y() * 0.5f;

   return bottomCenter;
}

Vector2 SceneEditorSelection::getBoxTopCenter()
{
   updateCentroid();

   Vector2 topCenter = mBoxCentroid;
   topCenter.y += mBoxBounds.len_y() * 0.5f;

   return topCenter;
}

Vector2 SceneEditorSelection::getBoxLeftCenter()
{
   return Vector2();
}

Vector2 SceneEditorSelection::getBoxRightCenter()
{
   return Vector2();
}

void SceneEditorSelection::updateCentroid()
{
   if (mCentroidValid)
      return;

   resolvePIDs();

   mCentroidValid = true;

   mCentroid.set(0.0f, 0.0f);
   mBoxCentroid = mCentroid;

   mBoxBounds.minExtents.set(1e10, 1e10);
   mBoxBounds.maxExtents.set(-1e10, -1e10);

   for (SimSet::iterator itr = begin(); itr != end(); ++itr)
   {
      SceneObject2D* obj = dynamic_cast<SceneObject2D*>(*itr);
      if (!obj)
         continue;

      const MatrixF& mat = obj->getTransform();
      Point3F wPos = mat.getPosition();
      Vector2 pos(wPos.x, wPos.y);

      mCentroid += pos;

      const BoxVec2& bounds = obj->getWorldBox();
      mBoxBounds.minExtents.setMin(bounds.minExtents);
      mBoxBounds.maxExtents.setMax(bounds.maxExtents);

   }

   mCentroid /= (F32)size();
   mBoxCentroid = mBoxBounds.getCenter();

}

