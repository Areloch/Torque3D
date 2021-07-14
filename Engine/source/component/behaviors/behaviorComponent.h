#ifndef _BEHAVIORCOMPONENT_H_
#define _BEHAVIORCOMPONENT_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tvector.h"
#endif

#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif

#ifndef _SIMCOMPONENT_H_
#include "component/simComponent.h"
#endif

#ifndef _DYNAMICMETHODCOMPONENT_H_
#include "component/dynamicMethodComponent.h"
#endif // !_DYNAMICMETHODCOMPONENT_H_

//forward decl
class BehaviorInstance;

class BehaviorTemplate : public SimObject
{
   typedef SimObject Parent;

public:
   struct BehaviorField
   {
      BehaviorField(const char* name, const char* description, const char* type, const char* defaultValue, const char* userData)
      {
         mName = name ? StringTable->insert(name) : StringTable->EmptyString;;
         mDescription = description ? StringTable->insert(description) : StringTable->EmptyString;
         mType = type ? StringTable->insert(type) : StringTable->EmptyString;
         mDefaultValue = defaultValue ? StringTable->insert(defaultValue) : StringTable->EmptyString;
         mUserData = userData ? StringTable->insert(userData) : StringTable->EmptyString;
      }

      StringTableEntry mName;
      StringTableEntry mDescription;
      StringTableEntry mType;
      StringTableEntry mUserData;
      StringTableEntry mDefaultValue;
   };

   /// Behavior port common functionality.
   struct BehaviorPort
   {
      BehaviorPort(const char* name, const char* label, const char* description)
      {
         mName = name ? StringTable->insert(name) : StringTable->EmptyString;
         mLabel = label ? StringTable->insert(label) : StringTable->EmptyString;
         mDescription = description ? StringTable->insert(description) : StringTable->EmptyString;
      }

      StringTableEntry mName;
      StringTableEntry mLabel;
      StringTableEntry mDescription;
   };

   /// A behavior port that accepts input.
   struct BehaviorPortInput : public BehaviorPort
   {
      BehaviorPortInput(const char* name, const char* label, const char* description) :
         BehaviorPort(name, label, description)
      {
      }
   };

   /// A behavior port that raises an output.
   struct BehaviorPortOutput : public BehaviorPort
   {
      BehaviorPortOutput(const char* name, const char* label, const char* description) :
         BehaviorPort(name, label, description)
      {
      }
   };

public:
   BehaviorTemplate();
   virtual ~BehaviorTemplate() {}

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   /// Create a BehaviorInstance from this template
   BehaviorInstance* createInstance(void);

   /// Template.
   inline StringTableEntry getFriendlyName(void) const { return mFriendlyName; }
   inline StringTableEntry getDescription(void) const { return mDescription; }
   inline StringTableEntry getBehaviorType(void) const { return mBehaviorType; }

   /// Fields.
   bool addBehaviorField(const char* fieldName, const char* description, const char* type, const char* defaultValue = NULL, const char* userData = NULL);
   inline U32 getBehaviorFieldCount(void) const { return mFields.size(); };
   inline BehaviorField* getBehaviorField(const U32 index) { return index < (U32)mFields.size() ? &mFields[index] : NULL; }
   inline BehaviorField* getBehaviorField(const char* fieldName)
   {
      StringTableEntry name = StringTable->insert(fieldName);
      for (Vector<BehaviorField>::iterator itr = mFields.begin(); itr != mFields.end(); ++itr)
      {
         // Check if found.
         if (name == itr->mName)
            return itr;
      }
      return NULL;
   }
   inline bool hasBehaviorField(const char* fieldName)
   {
      StringTableEntry name = StringTable->insert(fieldName);
      for (Vector<BehaviorField>::iterator itr = mFields.begin(); itr != mFields.end(); ++itr)
      {
         // Check if found.
         if (name == itr->mName)
            return true;
      }
      return false;
   }
   /// Outputs.
   bool addBehaviorOutput(const char* portName, const char* label, const char* description);
   inline U32 getBehaviorOutputCount(void) const { return mPortOutputs.size(); }
   inline BehaviorPortOutput* getBehaviourOutput(const U32 index) { return index < (U32)mPortOutputs.size() ? &mPortOutputs[index] : NULL; }
   inline bool hasBehaviorOutput(const char* portName)
   {
      StringTableEntry name = StringTable->insert(portName);
      for (Vector<BehaviorPortOutput>::iterator itr = mPortOutputs.begin(); itr != mPortOutputs.end(); ++itr)
      {
         // Check if found.
         if (name == itr->mName)
            return true;
      }
      return false;
   }

   /// Inputs.
   bool addBehaviorInput(const char* portName, const char* label, const char* description);
   inline U32 getBehaviorInputCount(void) const { return mPortInputs.size(); }
   inline BehaviorPortInput* getBehaviourInput(const U32 index) { return index < (U32)mPortInputs.size() ? &mPortInputs[index] : NULL; }
   inline bool hasBehaviorInput(const char* portName)
   {
      StringTableEntry name = StringTable->insert(portName);
      for (Vector<BehaviorPortInput>::iterator itr = mPortInputs.begin(); itr != mPortInputs.end(); ++itr)
      {
         // Check if found.
         if (name == itr->mName)
            return true;
      }
      return false;
   }

   DECLARE_CONOBJECT(BehaviorTemplate);
   DECLARE_DESCRIPTION("Template to be used by behavior instances.")

protected:
   StringTableEntry mFriendlyName;
   StringTableEntry mDescription;
   StringTableEntry mBehaviorType;

   Vector<BehaviorField> mFields;
   Vector<BehaviorPortInput> mPortInputs;
   Vector<BehaviorPortOutput> mPortOutputs;

   static bool setDescription(void *obj, const char *array, const char *data) { static_cast<BehaviorTemplate *>(obj)->mDescription = data ? StringTable->insert(data) : StringTable->EmptyString; return false; }
   static const char* getDescription(void *obj, const char *data) { return static_cast<BehaviorTemplate *>(obj)->getDescription(); }

};

class BehaviorComponent : public DynamicMethodComponent
{
   typedef DynamicMethodComponent Parent;

private:
   /// Component Behaviors
   SimSet  mBehaviors;

   /// Master behavior Id.
   U32 mMasterBehaviorId;

   Vector<StringTableEntry>* mpBehaviorFieldNames;
public:
   /// A behavior port connection.
   struct BehaviorPortConnection
   {
      BehaviorPortConnection(
         BehaviorInstance* pOutputBehavior,
         BehaviorInstance* pInputBehavior,
         StringTableEntry pOutputName,
         StringTableEntry pInputName)
      {
         mOutputInstance = pOutputBehavior;
         mInputInstance = pInputBehavior;
         mOutputName = pOutputName;
         mInputName = pInputName;
      }

      BehaviorInstance*   mOutputInstance;
      BehaviorInstance*   mInputInstance;
      StringTableEntry    mOutputName;
      StringTableEntry    mInputName;
   };

   /// Behavior connection map.
   /// NOTE: This configuration provides more efficient raising of outputs as opposed to general administration.
   typedef Vector<BehaviorPortConnection> typePortConnectionVector;
   typedef HashMap<StringTableEntry, typePortConnectionVector*> typeOutputNameConnectionHash;
   typedef HashMap<SimObjectId, typeOutputNameConnectionHash*> typeInstanceConnectionHash;
   typeInstanceConnectionHash mBehaviorConnections;

protected:
   virtual const char* _callMethod(U32 argc, const char *argv[], bool callThis = true);

private:
   void destroyBehaviorOutputConnections(BehaviorInstance* pOutputBehavior);
   void destroyBehaviorInputConnections(BehaviorInstance* pInputBehavior);


public:
   BehaviorComponent();
   virtual ~BehaviorComponent() {}

   /// SimObject overrides
   virtual bool onAdd();
   virtual void onRemove();
   virtual void onDeleteNotify(SimObject *object);
   virtual void copyTo(SimObject* object);

   /// Behavior interface.
   BehaviorInstance* getBehaviorByInstanceId(const U32 behaviorId);
   virtual bool addBehavior(BehaviorInstance *bi);
   virtual bool removeBehavior(BehaviorInstance *bi, bool deleteBehavior = true);
   virtual void clearBehaviors();
   virtual U32 getBehaviorCount() const { return mBehaviors.size(); }
   virtual const SimSet &getBehaviors() const { return mBehaviors; }
   virtual BehaviorInstance *getBehavior(StringTableEntry behaviorTemplateName);
   virtual BehaviorInstance *getBehavior(const U32 index) { return index < (U32)mBehaviors.size() ? reinterpret_cast<BehaviorInstance *>(mBehaviors[index]) : NULL; }
   virtual bool reOrder(BehaviorInstance *obj, U32 desiredIndex);

   /// Behavior connectivity.
   bool connect(BehaviorInstance* pOutputBehavior, BehaviorInstance* pInputBehavior, StringTableEntry pOutputName, StringTableEntry pInputName);
   bool disconnect(BehaviorInstance* pOutputBehavior, BehaviorInstance* pInputBehavior, StringTableEntry pOutputName, StringTableEntry pInputName);
   bool raise(BehaviorInstance* pOutputBehavior, StringTableEntry pOutputName);
   U32 getBehaviorConnectionCount(BehaviorInstance* pOutputBehavior, StringTableEntry pOutputName);
   const BehaviorPortConnection* getBehaviorConnection(BehaviorInstance* pOutputBehavior, StringTableEntry pOutputName, const U32 connectionIndex);
   const typePortConnectionVector* getBehaviorConnections(BehaviorInstance* pOutputBehavior, StringTableEntry pOutputName);

   /// DynamicConsoleMethodComponent Overrides
   virtual bool handlesConsoleMethod(const char *fname, S32 *routingId);
   virtual const char* callOnBehaviors(U32 argc, const char *argv[]);

   /// SimComponent overrides
   virtual void write(Stream &stream, U32 tabStop, U32 flags = 0);

   DECLARE_CONOBJECT(BehaviorComponent);
   DECLARE_DESCRIPTION("The behavior component.")

};

class BehaviorInstance : public SimObject
{
   typedef SimObject Parent;
public:
   BehaviorInstance(BehaviorTemplate* pTemplate = NULL);
   virtual ~BehaviorInstance() {}

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   inline BehaviorTemplate* getTemplate(void) { return mTemplate; }
   const char* getTemplateName(void);

   inline void setBehaviorOwner(BehaviorComponent* pOwner) { mBehaviorOwner = pOwner; }
   inline BehaviorComponent* getBehaviorOwner(void) const { return mBehaviorOwner ? mBehaviorOwner : NULL; }

   inline void setBehaviorId(const U32 id) { mBehaviorId = id; }
   inline U32 getBehaviorId(void) const { return mBehaviorId; }

   DECLARE_CONOBJECT(BehaviorInstance);
   DECLARE_DESCRIPTION("Instance of a behavior template.")

protected:
   BehaviorTemplate*   mTemplate;
   BehaviorComponent*  mBehaviorOwner;
   U32                 mBehaviorId;

   static bool setOwner(void *obj, const char *array, const char *data) { return true; }

   static const char* getTemplate(void* obj, const char* data);

};

//-----------------------------------------------------------------------------

class BehaviorComponentRaiseEvent : public SimEvent
{
public:
   BehaviorComponentRaiseEvent(BehaviorInstance* pOutputBehavior, StringTableEntry pOutputName)
   {
      // Sanity!
      AssertFatal(pOutputBehavior != NULL, "Output behavior cannot be NULL.");
      AssertFatal(pOutputBehavior->isProperlyAdded(), "Output behavior is not registered.");
      AssertFatal(pOutputName != NULL, "Output name cannot be NULL.");

      mpOutputBehavior = pOutputBehavior;
      mpOutputName = pOutputName;
   }
   virtual  ~BehaviorComponentRaiseEvent() {}

   virtual void process(SimObject *object)
   {
      // Fetch behavior component.
      BehaviorComponent* pBehaviorComponent = dynamic_cast<BehaviorComponent*>(object);

      // Sanity!
      AssertFatal(pBehaviorComponent, "BehaviorComponentRaiseEvent() - Could not process scheduled signal raise as the event was not raised on a behavior component.");

      // Is the output behavior still around?
      if (!mpOutputBehavior)
      {
         // No, so warn.
         Con::warnf("BehaviorComponentRaiseEvent() - Could not raise output '%s' on behavior as the behavior is not longer present.", mpOutputName);
         return;
      }

      // Raise output signal.
      pBehaviorComponent->raise(mpOutputBehavior, mpOutputName);
   }

private:
   SimObjectPtr<BehaviorInstance>  mpOutputBehavior;
   StringTableEntry                mpOutputName;
};

#endif // !_BEHAVIORCOMPONENT_H_
