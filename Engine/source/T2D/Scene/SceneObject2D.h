#ifndef _SCENEOBJECT2D_H_
#define _SCENEOBJECT2D_H_

#ifndef _SCENE2D_H_
#include "T2D/Scene/Scene2D.h"
#endif // !_SCENE2D_H_

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

#ifndef _PROCESSLIST_H_
#include "T3D/gameBase/processList.h"
#endif

///-----------------------------------------------------------------------------

typedef VectorPtr<b2FixtureDef*> typeCollisionFixtureDefVector;
typedef VectorPtr<b2Fixture*> typeCollisionFixtureVector;

extern EnumTable bodyTypeTable;
extern EnumTable srcBlendFactorTable;
extern EnumTable dstBlendFactorTable;

class SceneObject2D : public NetObject, public ProcessObject
{
public:
   typedef NetObject Parent;

   /// Networking dirty mask.
   enum SceneObject2DMasks
   {
      InitialUpdateMask = BIT(0),
      ScaleMask = BIT(1),
      FlagMask = BIT(2),
      MountedMask = BIT(3),
      NextFreeMask = BIT(4)
   };

protected:

   SimObjectPtr< SceneObject2D > mAfterObject;

public:
   SceneObject2D();
   virtual ~SceneObject2D();

   // ProcessObject,
   ProcessList* getProcessList() const;
   virtual void processAfter(ProcessObject *obj);
   virtual void clearProcessAfter();
   virtual ProcessObject* getAfterObject() const { return mAfterObject; }
   virtual void setProcessTick(bool t);

   // NetObject.
   virtual U32 packUpdate(NetConnection* conn, U32 mask, BitStream* stream);
   virtual void unpackUpdate(NetConnection* conn, BitStream* stream);
   virtual void onCameraScopeQuery(NetConnection* connection, CameraScopeQuery* query);

   // SimObject.
   virtual bool onAdd();
   virtual void onRemove();
   virtual void onDeleteNotify(SimObject *object);
   virtual void inspectPostApply();
   virtual bool writeField(StringTableEntry fieldName, const char* value);

   static void initPersistFields();


   DECLARE_CONOBJECT(SceneObject2D);
};

#endif // !_SCENEOBJECT2D_H_



