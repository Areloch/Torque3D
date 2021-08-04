#ifndef _SIMCOMPONENT_H_
#define _SIMCOMPONENT_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif // !_PLATFORM_H_

#ifndef _SIMBASE_H_
#include "console/sim.h"
#endif

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#ifndef _NETOBJECT_H_
#include "sim/netObject.h"
#endif

#ifndef _STREAM_H_
#include "core/stream/stream.h"
#endif // !_STREAM_H_

class SimComponent : public SimObject
{
   typedef SimObject Parent;

private:
   VectorPtr<SimComponent*> mComponentList;
   void *mMutex;

   SimObjectPtr<SimComponent> mOwner;

   bool _registerComponents(SimComponent *owner);
   void _unregisterComponents();

protected:
   bool mEnabled;

   SimComponent *_getOwner() { return mOwner; }

   typedef VectorPtr<SimComponent*>::iterator SimComponentItr;
   VectorPtr<SimComponent*> &lockComponentList()
   {
      Mutex::lockMutex(mMutex);
      return mComponentList;
   };

   void unlockComponentList()
   {
      Mutex::unlockMutex(mMutex);
   }

   virtual bool onComponentRegister(SimComponent *owner)
   {
      mOwner = owner;
      return true;
   }

   virtual void onComponentUnRegister()
   {
      mOwner = NULL;
   }

public:

   SimComponent();
   virtual ~SimComponent();

   virtual bool onAdd();
   virtual void onRemove();
   static void  initPersistFields();
   virtual bool processArguments(S32 argc, const char **argv);

   virtual bool addComponent(SimComponent * component);
   virtual bool removeComponent(SimComponent * component);
   virtual bool clearComponents() { mComponentList.clear(); return true; };
   virtual bool onComponentAdd(SimComponent *target);
   virtual void onComponentRemove(SimComponent *target);

   void setEnabled(const bool enabled) { mEnabled = enabled; }
   inline bool getEnabled() { return mEnabled; }
   bool isEnabled() const { return mEnabled; }

   virtual void onUpdate(void) {}
   virtual void onAddToScene(void) {}
   virtual void onRemoveFromScene(void) {}

   inline U32 getComponentCount() { return mComponentList.size(); }
   inline SimComponent *getComponent(const U32 index) { return mComponentList[index]; }

   bool callMethodOnComponents(U32 argc, const char * argv[], const char ** result);

   bool hasComponents() const { return (mComponentList.size() > 0); };
   const SimComponent *getOwner() const { return mOwner; };
   inline virtual StringTableEntry  getComponentName() { return StringTable->insert(getClassName()); };

   ///init fields
   static bool setEnabled(void *obj, const char *array, const char *data) { static_cast<SimComponent*>(obj)->setEnabled(dAtob(data)); return false; };
   static const char* getEnabled(void* obj, const char* data) { return Con::getBoolArg(static_cast<SimComponent*>(obj)->getEnabled()); }
   static bool writeEnabled(void* obj, StringTableEntry pFieldName) { return static_cast<SimComponent*>(obj)->mEnabled == false; }

   void write(Stream & stream, U32 tabStop, U32 flags);
   bool writeField(StringTableEntry fieldname, const char * value);

   DECLARE_CONOBJECT(SimComponent);
   DECLARE_DESCRIPTION("Base class for all other components.");

};

#endif // !_SIMCOMPONENT_H_
