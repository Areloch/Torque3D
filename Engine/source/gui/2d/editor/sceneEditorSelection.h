#ifndef _SCENEEDITORSELECTION_H_
#define _SCENEEDITORSELECTION_H_

#ifndef _SIMPERSISTSET_H_
#include "console/simPersistSet.h"
#endif

#ifndef _VECTOR2_H_
#include "T2D/Math2D/Vector2.h"
#endif // !_VECTOR2_H_


#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

class SceneObject2D;

class SceneEditorSelection : public SimPersistSet
{
public:
   typedef SimPersistSet Parent;

private:
   Vector2 mCentroid;
   BoxVec2 mBoxBounds;
   Vector2 mBoxCentroid;
   bool mCentroidValid;

   bool mAutoSelect;
   Vector2 mPrevCentroid;
   void updateCentroid();

public:
   SceneEditorSelection();
   ~SceneEditorSelection();

   static void initPersistFields();

   // SimSet.
   virtual void addObject(SimObject* obj);
   virtual void removeObject(SimObject* obj);
   virtual void setCanSave(bool value);

   /// return true if the scene object 2d is in the selection
   bool objInSet(SimObject* obj);

   const Vector2& getCentroid();
   const BoxVec2& getBoxBounds();

   /// helpful box positions
   Vector2 getBoxBottomCenter();
   Vector2 getBoxTopCenter();
   Vector2 getBoxLeftCenter();
   Vector2 getBoxRightCenter();

   void storeCurrentCentroid() { mPrevCentroid = getCentroid(); }
   bool hasCentroidChanged() { return (mPrevCentroid != getCentroid()); }
   void setAutoSelect(bool b) { mAutoSelect = b; }
   void invalidateCentroid() { mCentroidValid = false; }

   void offset(const Vector2& delta, F32 gridSnap = 0.0f);
   void setPosition(const Vector2& pos);
   F32 _snapFloat(const F32 &val, const F32 &snap) const;
   void setCentroidPosition(bool useBoxCenter, const Vector2& pos);
   void rotate(const EulerF &ang, const Vector2 &pos);

   DECLARE_CONOBJECT(SceneEditorSelection);
   DECLARE_CATEGORY("Editor World");
   DECLARE_DESCRIPTION("A selection for the 2d scene editor");

};

#endif // !_SCENEEDITORSELECTION_H_
