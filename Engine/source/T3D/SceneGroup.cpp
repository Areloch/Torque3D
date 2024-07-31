#include "SceneGroup.h"

#include "gameBase/gameConnection.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxTransformSaver.h"
#include "gui/worldEditor/editor.h"
#include "math/mathIO.h"
#include "physics/physicsShape.h"
#include "renderInstance/renderPassManager.h"
#include "scene/sceneRenderState.h"

IMPLEMENT_CO_NETOBJECT_V1(SceneGroup);

ConsoleDocClass(SceneGroup,
   "@brief A collection of arbitrary objects which can be allocated and manipulated as a group.\n\n"

   "%SceneGroup always points to a (.SceneGroup) file which defines its objects. In "
   "fact more than one %SceneGroup can reference this file and both will update "
   "if the file is modified.\n\n"

   "%SceneGroup is a very simple object and only exists on the server. When it is "
   "created it allocates children objects by reading the (.SceneGroup) file like "
   "a list of instructions.  It then sets their transform relative to the %SceneGroup "
   "and Torque networking handles the rest by ghosting the new objects to clients. "
   "%SceneGroup itself is not ghosted.\n\n"

   "@ingroup enviroMisc"
);

SceneGroup::SceneGroup()
{
   // Not ghosted unless we're editing
   mNetFlags.clear(Ghostable | ScopeAlways);

   mTypeMask |= ZoneObjectType;
}

SceneGroup::~SceneGroup()
{
}

void SceneGroup::initPersistFields()
{
   docsURL;

   Parent::initPersistFields();
}

bool SceneGroup::onAdd()
{
   if (!Parent::onAdd())
      return false;

   mObjBox.set(Point3F(-0.5f, -0.5f, -0.5f),
      Point3F(0.5f, 0.5f, 0.5f));

   resetWorldBox();

   // Not added to the scene unless we are editing.
   if (gEditingMission)
      onEditorEnable();

   return true;
}

void SceneGroup::onRemove()
{
   removeFromScene();
   Parent::onRemove();
}

void SceneGroup::onEditorEnable()
{
   if (isClientObject())
      return;

   // Just in case we are already in the scene, lets not cause an assert.   
   if (mContainer != NULL)
      return;

   // Enable ghosting so we can see this on the client.
   mNetFlags.set(Ghostable);
   setScopeAlways();
   addToScene();

   Parent::onEditorEnable();
}

void SceneGroup::onEditorDisable()
{
   if (isClientObject())
      return;

   // Just in case we are not in the scene, lets not cause an assert.   
   if (mContainer == NULL)
      return;

   // Do not need this on the client if we are not editing.
   removeFromScene();
   mNetFlags.clear(Ghostable);
   clearScopeAlways();

   Parent::onEditorDisable();
}

void SceneGroup::inspectPostApply()
{
   Parent::inspectPostApply();
}

void SceneGroup::setTransform(const MatrixF& mat)
{
   //transform difference
   MatrixF oldTransform = getTransform();

   Parent::setTransform(mat);

   // Calculate the delta transformation
   MatrixF deltaTransform;
   oldTransform.inverse();
   deltaTransform.mul(oldTransform, getTransform());

   if (isServerObject())
   {
      setMaskBits(TransformMask);

      // Update all child transforms
      for (SimSetIterator itr(this); *itr; ++itr)
      {
         SceneObject* child = dynamic_cast<SceneObject*>(*itr);
         if (child)
         {
            MatrixF childTransform = child->getTransform();
            MatrixF relativeTransform;
            relativeTransform.mul(deltaTransform, childTransform);
            child->setTransform(relativeTransform);
         }
      }
   }
}

void SceneGroup::setRenderTransform(const MatrixF& mat)
{
   //transform difference
   MatrixF oldTransform = getRenderTransform();

   Parent::setRenderTransform(mat);

   // Calculate the delta transformation
   MatrixF deltaTransform;
   oldTransform.inverse();
   deltaTransform.mul(oldTransform, getRenderTransform());

   // Update all child transforms
   for (SimSetIterator itr(this); *itr; ++itr)
   {
      SceneObject* child = dynamic_cast<SceneObject*>(*itr);
      if (child)
      {
         MatrixF childTransform = child->getRenderTransform();
         MatrixF relativeTransform;
         relativeTransform.mul(deltaTransform, childTransform);
         child->setRenderTransform(relativeTransform);
      }
   }
}

void SceneGroup::addObject(SimObject* object)
{
   Parent::addObject(object);

   // Recalculate the bounding box from scratch (simpler but potentially costly)
   recalculateBoundingBox();
}

void SceneGroup::removeObject(SimObject* object)
{
   Parent::removeObject(object);

   // Recalculate the bounding box from scratch (simpler but potentially costly)
   recalculateBoundingBox();
}

void SceneGroup::recalculateBoundingBox()
{
   // Reset the bounding box
   Box3F bounds;

   bounds.minExtents.set(1e10, 1e10, 1e10);
   bounds.maxExtents.set(-1e10, -1e10, -1e10);

   // Extend the bounding box to include each child's bounding box
   for (SimSetIterator itr(this); *itr; ++itr)
   {
      SceneObject* child = dynamic_cast<SceneObject*>(*itr);
      if (child)
      {
         const Box3F& childBox = child->getWorldBox();

         bounds.minExtents.setMin(childBox.minExtents);
         bounds.maxExtents.setMax(childBox.maxExtents);
      }
   }

   mWorldBox = bounds;
   resetObjectBox();
   setMaskBits(TransformMask);
}

U32 SceneGroup::packUpdate(NetConnection* conn, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   mathWrite(*stream, mObjBox);

   if (stream->writeFlag(mask & TransformMask))
   {
      mathWrite(*stream, getTransform());
      mathWrite(*stream, getScale());
   }

   return retMask;
}

void SceneGroup::unpackUpdate(NetConnection* conn, BitStream* stream)
{
   Parent::unpackUpdate(conn, stream);

   mathRead(*stream, &mObjBox);
   resetWorldBox();

   // TransformMask
   if (stream->readFlag())
   {
      mathRead(*stream, &mObjToWorld);
      mathRead(*stream, &mObjScale);

      setTransform(mObjToWorld);
   }
}

bool SceneGroup::buildPolyList(PolyListContext context, AbstractPolyList* polyList, const Box3F& box, const SphereF& sphere)
{
   Vector<SceneObject*> foundObjects;
   if (empty())
   {
      Con::warnf("SceneGroup::buildPolyList() - SceneGroup %s is empty!", getName());
      return false;
   }
   findObjectByType(foundObjects);

   for (S32 i = 0; i < foundObjects.size(); i++)
   {
      foundObjects[i]->buildPolyList(context, polyList, box, sphere);
   }

   return true;
}

bool SceneGroup::buildExportPolyList(ColladaUtils::ExportData* exportData, const Box3F& box, const SphereF& sphere)
{
   Vector<SceneObject*> foundObjects;
   findObjectByType(foundObjects);

   for (S32 i = 0; i < foundObjects.size(); i++)
   {
      foundObjects[i]->buildExportPolyList(exportData, box, sphere);
   }

   return true;
}

void SceneGroup::getUtilizedAssets(Vector<StringTableEntry>* usedAssetsList)
{
   //if (empty())
      return;

   Vector<SceneObject*> foundObjects;
   findObjectByType(foundObjects);

   for (S32 i = 0; i < foundObjects.size(); i++)
   {
      SceneObject* child = foundObjects[i];

      child->getUtilizedAssets(usedAssetsList);
   }
}

DefineEngineMethod(SceneGroup, recalculateBounds, void, (), ,
   "Recalculates the SceneGroups' bounds and centerpoint.\n")
{
   object->recalculateBoundingBox();
}
