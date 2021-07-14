#include "behaviorComponent.h"
#include "platform/platform.h"
#include "component/simComponent.h"
#include "console/engineAPI.h"
#include "console/consoleTypes.h"
#include "console/consoleInternal.h"
#include "core/stream/stream.h"
#include "core/frameAllocator.h"

//-----------------------------------------------------------------------------
// Behavior Template
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(BehaviorTemplate);

//-----------------------------------------------------------------------------

BehaviorTemplate::BehaviorTemplate() :
   mFriendlyName(StringTable->EmptyString()),
   mDescription(StringTable->EmptyString()),
   mBehaviorType(StringTable->EmptyString())
{
}

//-----------------------------------------------------------------------------

bool BehaviorTemplate::onAdd()
{
   if (!Parent::onAdd())
      return false;

   Sim::gBehaviorSet->addObject(this);

   return true;
}

//-----------------------------------------------------------------------------

void BehaviorTemplate::onRemove()
{
   Sim::gBehaviorSet->removeObject(this);

   Parent::onRemove();
}

//-----------------------------------------------------------------------------

void BehaviorTemplate::initPersistFields()
{
   addGroup("Behavior");
   addField("friendlyName", TypeCaseString, Offset(mFriendlyName, BehaviorTemplate), "Human friendly name of this behavior");
   addProtectedField("description", TypeCaseString, Offset(mDescription, BehaviorTemplate), &setDescription, &getDescription, "The description of this behavior.\n");
   addField("behaviorType", TypeString, Offset(mBehaviorType, BehaviorTemplate), "Organizational keyword");
   endGroup("Behavior");

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

BehaviorInstance* BehaviorTemplate::createInstance(void)
{
   // Create behavior instance.
   BehaviorInstance* pBehavior = new BehaviorInstance(this);

   // Register object.
   if (pBehavior->registerObject())
      return pBehavior;

   // Registration failed so delete behavior.
   delete pBehavior;
   return NULL;
}

//-----------------------------------------------------------------------------

bool BehaviorTemplate::addBehaviorField(const char* name, const char* description, const char* type, const char* defaultValue, const char* userData)
{
   // Does the behavior already have the field?
   if (hasBehaviorField(name))
   {
      // Yes, so warn.
      Con::warnf("Behavior field named '%s' on template '%s' already exists.", name, mFriendlyName);
      return false;
   }

   // Create field.
   BehaviorField field(name, description, type, defaultValue, userData);

   // Use field.
   mFields.push_back(field);

   return true;
}

//-----------------------------------------------------------------------------

bool BehaviorTemplate::addBehaviorOutput(const char* name, const char* label, const char* description)
{
   // Does the behavior already have the output?
   if (hasBehaviorOutput(name))
   {
      // Yes, so warn.
      Con::warnf("Behavior output named '%s' on template '%s' already exists.", name, mFriendlyName);
      return false;
   }

   // Create output.
   BehaviorPortOutput output(name, label, description);

   // Use port.
   mPortOutputs.push_back(output);

   return true;
}

//-----------------------------------------------------------------------------

bool BehaviorTemplate::addBehaviorInput(const char* name, const char* label, const char* description)
{
   // Does the behavior already have the input?
   if (hasBehaviorInput(name))
   {
      // Yes, so warn.
      Con::warnf("Behavior input named '%s' on template '%s' already exists.", name, mFriendlyName);
      return false;
   }

   // Create input.
   BehaviorPortInput input(name, label, description);

   // Use port.
   mPortInputs.push_back(input);

   return true;
}

DefineEngineMethod(BehaviorTemplate, createInstance, S32, (), ,
   "Creates a behavior instance from this template")
{
   BehaviorInstance* inst = object->createInstance();
   return inst ? inst->getId() : -1;
}

DefineEngineStringlyVariadicMethod(BehaviorTemplate, addBehvaiorFeld, bool, 5, 7,
   "( string fieldName, desc, type, [def Value, userdata] ).\n")
{
   const char *defValue = NULL;
   if (argc > 5)
      defValue = argv[5];

   const char *typeInfo = NULL;
   if (argc > 6)
      typeInfo = argv[6];

   return object->addBehaviorField(argv[2], argv[3], argv[4], defValue, typeInfo);
}

DefineEngineMethod(BehaviorTemplate, getBehaviorFieldCount, S32, (), ,
   "Return number of fields in this Template.\n")
{
   return object->getBehaviorFieldCount();
}

DefineEngineMethod(BehaviorTemplate, getBehaviorField, const char*, (U32 id), ,
   "Returns the field for this index.\n")
{
   BehaviorTemplate::BehaviorField* pField = object->getBehaviorField(id);
   if (!pField)
   {
      Con::warnf("getBehaviorField() - index out of range");
      return StringTable->EmptyString();
   }

   char* buff;
   dSprintf(buff, 1024, "%s %s %s", pField->mName, pField->mType, pField->mDefaultValue);
   return buff;
}

DefineEngineMethod(BehaviorTemplate, getBehaviorFieldUserData, const char*, (U32 id), ,
   "Return user data for particular field.\n")
{
   BehaviorTemplate::BehaviorField* pField = object->getBehaviorField(id);
   if (!pField)
   {
      Con::warnf("getBehaviorField() - index out of range");
      return StringTable->EmptyString();
   }

   return pField->mUserData;
}

DefineEngineMethod(BehaviorTemplate, getBehaviorFieldDescription, const char*, (U32 id), ,
   "Return description for particular field.\n")
{
   BehaviorTemplate::BehaviorField* pField = object->getBehaviorField(id);
   if (!pField)
   {
      Con::warnf("getBehaviorField() - index out of range");
      return StringTable->EmptyString();
   }

   return pField->mDescription ? pField->mDescription : "no description set" ;
}

DefineEngineStringlyVariadicMethod(BehaviorTemplate, addBehaviorOutput, bool, 5, 5,
   "(outputName, label, description).\n")
{

   return object->addBehaviorOutput(argv[2], argv[3], argv[4]);
}

DefineEngineMethod(BehaviorTemplate, getBehaviorOutputCount, S32, (), ,
   "Return number of outputs in this Template.\n")
{
   return object->getBehaviorOutputCount();
}

DefineEngineMethod(BehaviorTemplate, getBehaviorOutput, const char*, (U32 id), ,
   "Returns the output for this index.\n")
{
   BehaviorTemplate::BehaviorPortOutput* pOut = object->getBehaviourOutput(id);
   if (!pOut)
   {
      Con::warnf("getBehaviorOutput() - index out of range");
      return StringTable->EmptyString();
   }

   char* buff;
   dSprintf(buff, 1024, "%s %s %s", pOut->mName, pOut->mLabel, pOut->mDescription);
   return buff;
}

DefineEngineMethod(BehaviorTemplate, hasBehaviorOutput, bool, (const char* name), ,
   "Check for output of (name).\n")
{
   return object->hasBehaviorOutput(name);
}

DefineEngineStringlyVariadicMethod(BehaviorTemplate, addBehaviorInput, bool, 5, 5,
   "(inputName, label, description).\n")
{

   return object->addBehaviorInput(argv[2], argv[3], argv[4]);
}

DefineEngineMethod(BehaviorTemplate, getBehaviorInputCount, S32, (), ,
   "Return number of inputs in this Template.\n")
{
   return object->getBehaviorInputCount();
}

DefineEngineMethod(BehaviorTemplate, getBehaviorInput, const char*, (U32 id), ,
   "Returns the input for this index.\n")
{
   BehaviorTemplate::BehaviorPortInput* pIn = object->getBehaviourInput(id);
   if (!pIn)
   {
      Con::warnf("getBehaviorInput() - index out of range");
      return StringTable->EmptyString();
   }

   char* buff;
   dSprintf(buff, 1024, "%s %s %s", pIn->mName, pIn->mLabel, pIn->mDescription);
   return buff;
}

DefineEngineMethod(BehaviorTemplate, hasBehaviorInput, bool, (const char* name), ,
   "Check for input of (name).\n")
{
   return object->hasBehaviorInput(name);
}

//-----------------------------------------------------------------------------
// Behavior Component
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(BehaviorComponent);

//-----------------------------------------------------------------------------

static StringTableEntry behaviorIdFieldName = StringTable->insert("Id");
static StringTableEntry behaviorNodeName = StringTable->insert("Behaviors");
static StringTableEntry behaviorConnectionTypeName = StringTable->insert("Connection");
static StringTableEntry behaviorTemplateAssetName = StringTable->insert("Asset");

//-----------------------------------------------------------------------------

BehaviorComponent::BehaviorComponent() :
   mMasterBehaviorId(1),
   mpBehaviorFieldNames(NULL)
{
   SIMSET_SET_ASSOCIATION(mBehaviors);
}

//-----------------------------------------------------------------------------

bool BehaviorComponent::onAdd()
{
   if (!Parent::onAdd())
      return false;

   return true;
}

//-----------------------------------------------------------------------------

void BehaviorComponent::onRemove()
{
   // Remove all behaviors and notify.
   clearBehaviors();

   // Call parent.
   Parent::onRemove();
}

//-----------------------------------------------------------------------------

void BehaviorComponent::onDeleteNotify(SimObject *object)
{
   // Cast to a behavior instance.
   BehaviorInstance* pInstance = dynamic_cast<BehaviorInstance*>(object);

   // Ignore if not appropriate.
   if (pInstance == NULL)
      return;

   // Is the behavior instance owned by this component?
   if (pInstance->getBehaviorOwner() == this)
   {
      // Yes, so remove.
      removeBehavior(pInstance, false);
   }

   // Destroy any input connections to the instance.
   destroyBehaviorInputConnections(pInstance);
}

//-----------------------------------------------------------------------------

void BehaviorComponent::copyTo(SimObject* obj)
{
   // Call parent.
   Parent::copyTo(obj);

   // Fetch object.
   BehaviorComponent* pObject = dynamic_cast<BehaviorComponent*>(obj);

   // Sanity!
   AssertFatal(pObject != NULL, "BehaviorComponent::copyTo() - Object is not the correct type.");

   // Clear behaviors.
   pObject->clearBehaviors();

   // Behaviors
   U32 behaviorCount = getBehaviorCount();

   // Finish if no behaviors.
   if (behaviorCount == 0)
      return;

   // Initialize a clone map.
   typedef HashMap<BehaviorInstance*, BehaviorInstance*> typeBehaviorCloneHash;
   typeBehaviorCloneHash behaviorInstanceCloneMap;

   // Iterate behaviors.
   for (U32 index = 0; index < behaviorCount; ++index)
   {
      // Clone the behavior instance.
      BehaviorInstance* pFromInstance = getBehavior(index);
      BehaviorTemplate* pFromTemplate = pFromInstance->getTemplate();
      BehaviorInstance* pToInstance = pFromTemplate->createInstance();

      // Assign dynamic fields from behavior instance.
      pToInstance->assignDynamicFieldsFrom(pFromInstance);

      // Add the behavior instance.
      pObject->addBehavior(pToInstance);

      // Add to the clone map.
      behaviorInstanceCloneMap.insert(pFromInstance, pToInstance);
   }

   // Iterate instance connections.
   for (typeInstanceConnectionHash::iterator instanceItr = mBehaviorConnections.begin(); instanceItr != mBehaviorConnections.end(); ++instanceItr)
   {
      // Fetch output name connection(s).
      typeOutputNameConnectionHash* pOutputNameConnection = instanceItr->value;

      // Iterate output name connections.
      for (typeOutputNameConnectionHash::iterator outputItr = pOutputNameConnection->begin(); outputItr != pOutputNameConnection->end(); ++outputItr)
      {
         // Fetch port connection(s).
         typePortConnectionVector* pPortConnections = outputItr->value;

         // Iterate input connections.
         for (typePortConnectionVector::iterator connectionItr = pPortConnections->begin(); connectionItr != pPortConnections->end(); ++connectionItr)
         {
            // Fetch connection.
            BehaviorPortConnection* pConnection = connectionItr;

            // Find behavior instance mappings.
            typeBehaviorCloneHash::iterator toOutputItr = behaviorInstanceCloneMap.find(pConnection->mOutputInstance);
            typeBehaviorCloneHash::iterator toInputItr = behaviorInstanceCloneMap.find(pConnection->mInputInstance);

            // Sanity!
            AssertFatal(toOutputItr != behaviorInstanceCloneMap.end(), "Failed to find output behavior instance mapping during copy.");
            AssertFatal(toInputItr != behaviorInstanceCloneMap.end(), "Failed to find input behavior instance mapping during copy.");

            // Fetch behavior instance mappings.
            BehaviorInstance* pToInstanceOutput = toOutputItr->value;
            BehaviorInstance* pToInstanceInput = toInputItr->value;

            // Make cloned connection.
            pObject->connect(pToInstanceOutput, pToInstanceInput, pConnection->mOutputName, pConnection->mInputName);
         }
      }
   }
}

//-----------------------------------------------------------------------------

bool BehaviorComponent::addBehavior(BehaviorInstance* bi)
{
   if (bi == NULL || !bi->isProperlyAdded())
      return false;

   // Store behavior.
   mBehaviors.pushObject(bi);

   // Notify if the behavior instance is destroyed.
   deleteNotify(bi);

   // Set the behavior owner.
   bi->setBehaviorOwner(this);

   // Allocate a behavior Id.
   bi->setBehaviorId(mMasterBehaviorId++);

   if (bi->isMethod("onBehaviorAdd"))
      Con::executef(bi, "onBehaviorAdd");

   return true;
}

//-----------------------------------------------------------------------------

bool BehaviorComponent::removeBehavior(BehaviorInstance *bi, bool deleteBehavior)
{
   for (SimSet::iterator itr = mBehaviors.begin(); itr != mBehaviors.end(); ++itr)
   {
      if (*itr == bi)
      {
         mBehaviors.removeObject(*itr);

         // Perform callback if allowed.
         if (bi->isProperlyAdded() && bi->isMethod("onBehaviorRemove"))
            Con::executef(bi, "onBehaviorRemove");

         // Destroy any output connections.
         destroyBehaviorOutputConnections(bi);

         if (deleteBehavior && bi->isProperlyAdded())
         {
            bi->deleteObject();
         }
         else
         {
            bi->setBehaviorOwner(NULL);
            bi->setBehaviorId(0);

            // Remove delete notification.
            clearNotify(bi);
         }

         return true;
      }
   }

   return false;
}

//-----------------------------------------------------------------------------

void BehaviorComponent::clearBehaviors()
{
   while (mBehaviors.size() > 0)
   {
      BehaviorInstance *bi = dynamic_cast<BehaviorInstance *>(mBehaviors.first());
      removeBehavior(bi);
   }
}

//-----------------------------------------------------------------------------

BehaviorInstance *BehaviorComponent::getBehavior(StringTableEntry behaviorTemplateName)
{
   for (SimSet::iterator itr = mBehaviors.begin(); itr != mBehaviors.end(); ++itr)
   {
      // Fetch behavior.
      BehaviorInstance* pBehaviorInstance = dynamic_cast<BehaviorInstance*>(*itr);

      if (!pBehaviorInstance || pBehaviorInstance->getTemplateName() != behaviorTemplateName)
         continue;

      return pBehaviorInstance;
   }

   return NULL;
}

//-----------------------------------------------------------------------------

bool BehaviorComponent::reOrder(BehaviorInstance *obj, U32 desiredIndex /* = 0 */)
{
   if (desiredIndex > (U32)mBehaviors.size())
      return false;

   SimObject *target = mBehaviors.at(desiredIndex);
   return mBehaviors.reOrder(obj, target);
}

//-----------------------------------------------------------------------------

void BehaviorComponent::destroyBehaviorOutputConnections(BehaviorInstance* pOutputBehavior)
{
   // Sanity!
   AssertFatal(pOutputBehavior != NULL, "Output behavior cannot be NULL.");

   // Is the output behavior owned by this behavior component?
   if (pOutputBehavior->getBehaviorOwner() != this)
   {
      // No, so warn.
      Con::warnf(
         "Could not destroy output behavior connection for behavior '%s' as the output behavior is not owned by this component.",
         pOutputBehavior->getTemplateName()
      );
      return;
   }

   // Find behavior instance connections.
   typeInstanceConnectionHash::iterator instanceItr = mBehaviorConnections.find(pOutputBehavior->getId());

   // Finish if there are no outbound connections for this output behavior.
   if (instanceItr == mBehaviorConnections.end())
      return;

   // Fetch output name hash.
   typeOutputNameConnectionHash* pOutputNameHash = instanceItr->value;

   // Iterate all outputs.
   for (typeOutputNameConnectionHash::iterator outputItr = pOutputNameHash->begin(); outputItr != pOutputNameHash->end(); ++outputItr)
   {
      // Fetch port connection(s).
      typePortConnectionVector* pPortConnections = outputItr->value;

      // Destroy port connections.
      delete pPortConnections;
   }

   // Destroy outputs.
   delete pOutputNameHash;

   // Remove connection.
   mBehaviorConnections.erase(pOutputBehavior->getId());
}

//-----------------------------------------------------------------------------

void BehaviorComponent::destroyBehaviorInputConnections(BehaviorInstance* pInputBehavior)
{
   // Sanity!
   AssertFatal(pInputBehavior != NULL, "Input behavior cannot be NULL.");

   // Iterate connections.
   for (typeInstanceConnectionHash::iterator instanceItr = mBehaviorConnections.begin(); instanceItr != mBehaviorConnections.end(); ++instanceItr)
   {
      // Fetch output name hash.
      typeOutputNameConnectionHash* pOutputNameHash = instanceItr->value;

      // Iterate all outputs.
      for (typeOutputNameConnectionHash::iterator outputItr = pOutputNameHash->begin(); outputItr != pOutputNameHash->end(); ++outputItr)
      {
         // Fetch port connection(s).
         typePortConnectionVector* pPortConnections = outputItr->value;

         bool connectionFound;
         do
         {
            // Flag connection as 'not found' initially.
            connectionFound = false;

            // Look for an existing connection to the specified input instance.
            for (typePortConnectionVector::iterator connectionItr = pPortConnections->begin(); connectionItr != pPortConnections->end(); ++connectionItr)
            {
               // Is this the input behavior?
               if (connectionItr->mInputInstance == pInputBehavior)
               {
                  // Yes, so destroy it.
                  pPortConnections->erase_fast(connectionItr);

                  // Flag connection as 'found'.
                  connectionFound = true;
                  break;
               }
            }

         } while (connectionFound);
      }
   }
}

//-----------------------------------------------------------------------------

BehaviorInstance* BehaviorComponent::getBehaviorByInstanceId(const U32 behaviorId)
{
   for (SimSet::iterator instanceItr = mBehaviors.begin(); instanceItr != mBehaviors.end(); ++instanceItr)
   {
      // Fetch behavior instance.
      BehaviorInstance* pInstance = static_cast<BehaviorInstance*>(*instanceItr);

      // Return instance if it has the same behavior Id.
      if (pInstance->getBehaviorId() == behaviorId)
         return pInstance;
   }

   // Not found.
   return NULL;
}

//-----------------------------------------------------------------------------

bool BehaviorComponent::connect(BehaviorInstance* pOutputBehavior, BehaviorInstance* pInputBehavior, StringTableEntry pOutputName, StringTableEntry pInputName)
{
   // Sanity!
   AssertFatal(pOutputBehavior != NULL, "Output behavior cannot be NULL.");
   AssertFatal(pInputBehavior != NULL, "Input behavior cannot be NULL.");
   AssertFatal(pOutputName != NULL, "Output name cannot be NULL.");
   AssertFatal(pInputName != NULL, "Input name cannot be NULL.");

   // Is the output behavior owned by this behavior component?
   if (pOutputBehavior->getBehaviorOwner() != this)
   {
      // No, so warn.
      Con::warnf(
         "Could not connect output '%s' on behavior '%s' to input '%s' on behavior '%s' as the output behavior is not owned by this component.",
         pOutputName,
         pOutputBehavior->getTemplateName(),
         pInputName,
         pInputBehavior->getTemplateName()
      );
      return false;
   }

   // Is the input behavior owned by this behavior component?
   if (pInputBehavior->getBehaviorOwner() != this)
   {
      // No, so warn.
      Con::warnf(
         "Could not connect output '%s' on behavior '%s' to input '%s' on behavior '%s' as the input behavior is not owned by this component.",
         pOutputName,
         pOutputBehavior->getTemplateName(),
         pInputName,
         pInputBehavior->getTemplateName()
      );
      return false;
   }

   // Does the output behavior have the specified output?
   if (!pOutputBehavior->getTemplate()->hasBehaviorOutput(pOutputName))
   {
      // No, so warn.
      Con::warnf(
         "Could not connect output '%s' on behavior '%s' to input '%s' on behavior '%s' as the output behavior does not have such an output.",
         pOutputName,
         pOutputBehavior->getTemplateName(),
         pInputName,
         pInputBehavior->getTemplateName()
      );
      return false;
   }

   // Does the input behavior have the specified input?
   if (!pInputBehavior->getTemplate()->hasBehaviorInput(pInputName))
   {
      // No, so warn.
      Con::warnf(
         "Could not connect output '%s' on behavior '%s' to input '%s' on behavior '%s' as the input behavior does not have such an input.",
         pOutputName,
         pOutputBehavior->getTemplateName(),
         pInputName,
         pInputBehavior->getTemplateName()
      );
      return false;
   }

   // Find behavior instance connections.
   typeInstanceConnectionHash::iterator instanceItr = mBehaviorConnections.find(pOutputBehavior->getId());

   // Are there currently any outbound connections for this output instance?
   if (instanceItr == mBehaviorConnections.end())
   {
      // No, so create an entry for this instance.
      typeOutputNameConnectionHash* pOutputHash = new typeOutputNameConnectionHash();

      // Insert new output hash.
      instanceItr = mBehaviorConnections.insert(pOutputBehavior->getId(), pOutputHash);
   }

   // Fetch output name hash.
   typeOutputNameConnectionHash* pOutputNameHash = instanceItr->value;

   // Find instance output connection.
   typeOutputNameConnectionHash::iterator outputItr = pOutputNameHash->find(pOutputName);

   // Are there currently any outbound connections for this specific output?
   if (outputItr == pOutputNameHash->end())
   {
      // No, so create an entry for this output.
      typePortConnectionVector* pPortConnections = new typePortConnectionVector();

      // Insert new port connections.
      outputItr = pOutputNameHash->insert(pOutputName, pPortConnections);
   }

   // Fetch port connection(s).
   typePortConnectionVector* pPortConnections = outputItr->value;

   // Look for an identical connection.
   for (typePortConnectionVector::iterator connectionItr = pPortConnections->begin(); connectionItr != pPortConnections->end(); ++connectionItr)
   {
      // Is this an identical connection?
      if (connectionItr->mInputInstance == pInputBehavior && connectionItr->mInputName == pInputName)
      {
         // Yes, so warn.
         Con::warnf(
            "Could not connect output '%s' on behavior '%s' to input '%s' on behavior '%s' as the connection already exists.",
            pOutputName,
            pOutputBehavior->getTemplateName(),
            pInputName,
            pInputBehavior->getTemplateName()
         );

         return false;
      }
   }

   // Populate port connection.
   BehaviorPortConnection portConnection(pOutputBehavior, pInputBehavior, pOutputName, pInputName);

   // Add port connection.
   pPortConnections->push_back(portConnection);

   // Notify if the input behavior instance is destroyed.
   deleteNotify(pInputBehavior);

   return true;
}

//-----------------------------------------------------------------------------

bool BehaviorComponent::disconnect(BehaviorInstance* pOutputBehavior, BehaviorInstance* pInputBehavior, StringTableEntry pOutputName, StringTableEntry pInputName)
{
   // Sanity!
   AssertFatal(pOutputBehavior != NULL, "Output behavior cannot be NULL.");
   AssertFatal(pInputBehavior != NULL, "Input behavior cannot be NULL.");
   AssertFatal(pOutputName != NULL, "Output name cannot be NULL.");
   AssertFatal(pInputName != NULL, "Input name cannot be NULL.");

   // Is the output behavior owned by this behavior component?
   if (pOutputBehavior->getBehaviorOwner() != this)
   {
      // No, so warn.
      Con::warnf(
         "Could not disconnect output '%s' on behavior '%s' from input '%s' on behavior '%s' as the output behavior is not owned by this component.",
         pOutputName,
         pOutputBehavior->getTemplateName(),
         pInputName,
         pInputBehavior->getTemplateName()
      );
      return false;
   }

   // Is the input behavior owned by this behavior component?
   if (pInputBehavior->getBehaviorOwner() != this)
   {
      // No, so warn.
      Con::warnf(
         "Could not disconnect output '%s' on behavior '%s' from input '%s' on behavior '%s' as the input behavior is not owned by this component.",
         pOutputName,
         pOutputBehavior->getTemplateName(),
         pInputName,
         pInputBehavior->getTemplateName()
      );
      return false;
   }

   // Does the output behavior have the specified output?
   if (!pOutputBehavior->getTemplate()->hasBehaviorOutput(pOutputName))
   {
      // No, so warn.
      Con::warnf(
         "Could not disconnect output '%s' on behavior '%s' to input '%s' on behavior '%s' as the output behavior does not have such an output.",
         pOutputName,
         pOutputBehavior->getTemplateName(),
         pInputName,
         pInputBehavior->getTemplateName()
      );
      return false;
   }

   // Does the input behavior have the specified input?
   if (!pInputBehavior->getTemplate()->hasBehaviorInput(pInputName))
   {
      // No, so warn.
      Con::warnf(
         "Could not disconnect output '%s' on behavior '%s' to input '%s' on behavior '%s' as the input behavior does not have such an input.",
         pOutputName,
         pOutputBehavior->getTemplateName(),
         pInputName,
         pInputBehavior->getTemplateName()
      );
      return false;
   }

   // Find behavior instance connections.
   typeInstanceConnectionHash::iterator instanceItr = mBehaviorConnections.find(pOutputBehavior->getId());

   // Are there currently any outbound connections for this output instance?
   if (instanceItr == mBehaviorConnections.end())
   {
      // No, so warn.
      Con::warnf(
         "Could not disconnect output '%s' on behavior '%s' from input '%s' on behavior '%s' as the behavior does not have any connections.",
         pOutputName,
         pOutputBehavior->getTemplateName(),
         pInputName,
         pInputBehavior->getTemplateName()
      );
      return false;
   }

   // Fetch output name hash.
   typeOutputNameConnectionHash* pOutputNameHash = instanceItr->value;

   // Find instance output connection.
   typeOutputNameConnectionHash::iterator outputItr = pOutputNameHash->find(pOutputName);

   // Are there currently any outbound connections for this specific output?
   if (outputItr == pOutputNameHash->end())
   {
      // No, so warn.
      Con::warnf(
         "Could not disconnect output '%s' on behavior '%s' from input '%s' on behavior '%s' as the specified output does not have any connections.",
         pOutputName,
         pOutputBehavior->getTemplateName(),
         pInputName,
         pInputBehavior->getTemplateName()
      );
      return false;
   }

   // Fetch port connection(s).
   typePortConnectionVector* pPortConnections = outputItr->value;

   // Look for an existing connection to the specified input.
   for (typePortConnectionVector::iterator connectionItr = pPortConnections->begin(); connectionItr != pPortConnections->end(); ++connectionItr)
   {
      // Is this the requested disconnection?
      if (connectionItr->mInputInstance == pInputBehavior && connectionItr->mInputName == pInputName)
      {
         // Yes, so remove connection.
         pPortConnections->erase_fast(connectionItr);

         return true;
      }
   }

   // Not found so warn.
   Con::warnf(
      "Could not disconnect output '%s' on behavior '%s' from input '%s' on behavior '%s' as the connection does not exist.",
      pOutputName,
      pOutputBehavior->getTemplateName(),
      pInputName,
      pInputBehavior->getTemplateName()
   );

   return false;
}

//-----------------------------------------------------------------------------

bool BehaviorComponent::raise(BehaviorInstance* pOutputBehavior, StringTableEntry pOutputName)
{
   // Sanity!
   AssertFatal(pOutputBehavior != NULL, "Output behavior cannot be NULL.");
   AssertFatal(pOutputBehavior->isProperlyAdded(), "Output behavior is not registered.");
   AssertFatal(pOutputName != NULL, "Output name cannot be NULL.");

   // Is the output behavior owned by this behavior component?
   if (pOutputBehavior->getBehaviorOwner() != this)
   {
      // No, so warn.
      Con::warnf(
         "Could not raise output '%s' on behavior '%s' as the output behavior is not owned by this component.",
         pOutputName,
         pOutputBehavior->getTemplateName()
      );
      return false;
   }

   // Does the behavior have the specified output?
   if (!pOutputBehavior->getTemplate()->hasBehaviorOutput(pOutputName))
   {
      // No, so warn.
      Con::warnf(
         "Could not raise output '%s' on behavior '%s' as the behavior does not have such an output.",
         pOutputName,
         pOutputBehavior->getTemplateName()
      );
      return false;
   }

   // Execute a callback for the output.
   // NOTE: This callback should not delete behaviors otherwise strange things can happen!
   Con::executef(this, pOutputName, pOutputBehavior->getIdString());

   // Find behavior instance connections.
   typeInstanceConnectionHash::iterator instanceItr = mBehaviorConnections.find(pOutputBehavior->getId());

   // Finish if there are no outbound connections for this output behavior.
   if (instanceItr == mBehaviorConnections.end())
      return true;

   // Find instance output connection.
   typeOutputNameConnectionHash::iterator outputItr = instanceItr->value->find(pOutputName);

   // Finish if there are no outbound connections for this output.
   if (outputItr == instanceItr->value->end())
      return true;

   // Fetch port connection(s).
   typePortConnectionVector* pPortConnections = outputItr->value;

   // Process output connection(s).
   for (typePortConnectionVector::iterator connectionItr = pPortConnections->begin(); connectionItr != pPortConnections->end(); ++connectionItr)
   {
      // Fetch input behavior.
      BehaviorInstance* pInputBehavior = connectionItr->mInputInstance;

      // Fetch input name.
      StringTableEntry pInputName = connectionItr->mInputName;

#ifdef TORQUE_DEBUG

      // Sanity!
      AssertFatal(pInputBehavior->isProperlyAdded(), "Input behavior is not registered.");

      // Does the behavior have the specified input?
      if (!pInputBehavior->getTemplate()->hasBehaviorInput(pInputName))
      {
         // No, so warn.
         Con::warnf(
            "Could not raise input '%s' on behavior '%s' as the behavior does not have such an input.",
            pInputName,
            pInputBehavior->getTemplateName()
         );
         return false;
      }
#endif
      // Execute a callback for the input.
      // NOTE: This callback should not delete behaviors otherwise strange things can happen!
      Con::executef(pInputBehavior, pInputName, pOutputBehavior->getIdString(), pOutputName);
   }

   return true;
}

//-----------------------------------------------------------------------------

U32 BehaviorComponent::getBehaviorConnectionCount(BehaviorInstance* pOutputBehavior, StringTableEntry pOutputName)
{
   // Sanity!
   AssertFatal(pOutputBehavior != NULL, "Output behavior cannot be NULL.");
   AssertFatal(pOutputName != NULL, "Output name cannot be NULL.");

   // Is the output behavior owned by this behavior component?
   if (pOutputBehavior->getBehaviorOwner() != this)
   {
      // No, so warn.
      Con::warnf(
         "Could not get behavior connection count on output '%s' on behavior '%s' as the output behavior is not owned by this component.",
         pOutputName,
         pOutputBehavior->getTemplateName()
      );
      return 0;
   }

   // Does the behavior have the specified output?
   if (!pOutputBehavior->getTemplate()->hasBehaviorOutput(pOutputName))
   {
      // No, so warn.
      Con::warnf(
         "Could not get behavior connection count for output '%s' on behavior '%s' as the behavior does not have such an output.",
         pOutputName,
         pOutputBehavior->getTemplateName()
      );
      return 0;
   }

   // Find behavior instance connections.
   typeInstanceConnectionHash::iterator instanceItr = mBehaviorConnections.find(pOutputBehavior->getId());

   // Finish if there are no outbound connections for this output behavior.
   if (instanceItr == mBehaviorConnections.end())
      return 0;

   // Find instance output connection.
   typeOutputNameConnectionHash::iterator outputItr = instanceItr->value->find(pOutputName);

   // Finish if there are no outbound connections for this output.
   if (outputItr == instanceItr->value->end())
      return 0;

   // Fetch port connection(s).
   typePortConnectionVector* pPortConnections = outputItr->value;

   // Return number of connections.
   return pPortConnections->size();
}

//-----------------------------------------------------------------------------

const BehaviorComponent::BehaviorPortConnection* BehaviorComponent::getBehaviorConnection(BehaviorInstance* pOutputBehavior, StringTableEntry pOutputName, const U32 connectionIndex)
{
   // Sanity!
   AssertFatal(pOutputBehavior != NULL, "Output behavior cannot be NULL.");
   AssertFatal(pOutputName != NULL, "Output name cannot be NULL.");

   // Fetch behavior connection count.
   const U32 behaviorConnectionCount = getBehaviorConnectionCount(pOutputBehavior, pOutputName);

   // Finish if there are no connections.
   if (behaviorConnectionCount == 0)
      return NULL;

   // Is the connection index valid?
   if (connectionIndex >= behaviorConnectionCount)
   {
      // No, so warn.
      Con::warnf(
         "Could not get behavior the behavior connection index '%d' on output '%s' on behavior '%s' as the output behavior only has '%d' connections",
         connectionIndex,
         pOutputName,
         pOutputBehavior->getTemplateName(),
         behaviorConnectionCount
      );
      return NULL;
   }

   // Fetch behavior connections.
   const typePortConnectionVector* pConnections = getBehaviorConnections(pOutputBehavior, pOutputName);

   // Fetch behavior connection.
   const BehaviorComponent::BehaviorPortConnection* pBehaviorConnection = &((*pConnections)[connectionIndex]);

   // Return behavior connection.
   return pBehaviorConnection;
}

//-----------------------------------------------------------------------------

const BehaviorComponent::typePortConnectionVector* BehaviorComponent::getBehaviorConnections(BehaviorInstance* pOutputBehavior, StringTableEntry pOutputName)
{
   // Sanity!
   AssertFatal(pOutputBehavior != NULL, "Output behavior cannot be NULL.");
   AssertFatal(pOutputName != NULL, "Output name cannot be NULL.");

   // Is the output behavior owned by this behavior component?
   if (pOutputBehavior->getBehaviorOwner() != this)
   {
      // No, so warn.
      Con::warnf(
         "Could not get behavior connections on output '%s' on behavior '%s' as the output behavior is not owned by this component.",
         pOutputName,
         pOutputBehavior->getTemplateName()
      );
      return NULL;
   }

   // Does the behavior have the specified output?
   if (!pOutputBehavior->getTemplate()->hasBehaviorOutput(pOutputName))
   {
      // No, so warn.
      Con::warnf(
         "Could not get behavior connections for output '%s' on behavior '%s' as the behavior does not have such an output.",
         pOutputName,
         pOutputBehavior->getTemplateName()
      );
      return NULL;
   }

   // Find behavior instance connections.
   typeInstanceConnectionHash::iterator instanceItr = mBehaviorConnections.find(pOutputBehavior->getId());

   // Finish if there are no outbound connections for this output behavior.
   if (instanceItr == mBehaviorConnections.end())
      return NULL;

   // Find instance output connection.
   typeOutputNameConnectionHash::iterator outputItr = instanceItr->value->find(pOutputName);

   // Finish if there are no outbound connections for this output.
   if (outputItr == instanceItr->value->end())
      return NULL;

   // Fetch port connection(s).
   typePortConnectionVector* pPortConnections = outputItr->value;

   // Return number of connections.
   return pPortConnections;
}

//-----------------------------------------------------------------------------

void BehaviorComponent::write(Stream &stream, U32 tabStop, U32 flags /* = 0 */)
{
   // Export selected only?
   if ((flags & SelectedOnly) && !isSelected())
   {
      return;
   }

   if (mBehaviors.size() == 0)
   {
      Parent::write(stream, tabStop, flags);
      return;
   }

   // The work we want to perform here is in the Taml callback.
   onTamlPreWrite();

   // Write object.
   Parent::write(stream, tabStop, flags);

   // The work we want to perform here is in the Taml callback.
   onTamlPostWrite();
}

//-----------------------------------------------------------------------------

bool BehaviorComponent::handlesConsoleMethod(const char *fname, S32 *routingId)
{

   if (dStricmp(fname, "delete") == 0)
      return Parent::handlesConsoleMethod(fname, routingId);

   for (SimSet::iterator nItr = mBehaviors.begin(); nItr != mBehaviors.end(); nItr++)
   {
      SimObject *pComponent = dynamic_cast<SimObject *>(*nItr);
      if (pComponent != NULL && pComponent->isMethod(fname))
      {
         *routingId = -2; // -2 denotes method on component
         return true;
      }
   }

   // Let parent handle it
   return Parent::handlesConsoleMethod(fname, routingId);
}

//-----------------------------------------------------------------------------

// Needed to be able to directly call execute on a Namespace::Entry
extern ExprEvalState gEvalState;

const char *BehaviorComponent::callOnBehaviors(U32 argc, const char *argv[])
{
   if (mBehaviors.empty())
      return Parent::callOnBehaviors(argc, argv);

   // Copy the arguments to avoid weird clobbery situations.
   FrameTemp<char *> argPtrs(argc);

   U32 strdupWatermark = FrameAllocator::getWaterMark();
   for (U32 i = 0; i < argc; i++)
   {
      argPtrs[i] = static_cast<char *>(FrameAllocator::alloc(dStrlen(argv[i]) + 1));
      dStrcpy(argPtrs[i], argv[i], dStrlen(argv[i]) + 1 );
   }

   // Walk backwards through the list just as with components
   const char* result = "";
   bool handled = false;
   for (SimSet::iterator i = (mBehaviors.end() - 1); i >= mBehaviors.begin(); i--)
   {
      BehaviorInstance *pBehavior = dynamic_cast<BehaviorInstance *>(*i);
      AssertFatal(pBehavior, "BehaviorComponent::callOnBehaviors - Bad behavior instance in list.");
      AssertFatal(pBehavior->getId() > 0, "Invalid id for behavior component");

      // Use the BehaviorInstance's namespace
      Namespace *pNamespace = pBehavior->getNamespace();
      if (!pNamespace)
         continue;

      // Lookup the Callback Namespace entry and then splice callback
      const char *cbName = StringTable->insert(argv[0]);
      Namespace::Entry *pNSEntry = pNamespace->lookup(cbName);
      if (pNSEntry)
      {
         // Set %this to our BehaviorInstance's Object ID
         argPtrs[1] = const_cast<char *>(pBehavior->getIdString());

         // Change the Current Console object, execute, restore Object
         SimObject *save = gEvalState.thisObject;
         gEvalState.thisObject = pBehavior;

         result = pNSEntry->execute(argc, (ConsoleValueRef*)(~argPtrs), &gEvalState);

         gEvalState.thisObject = save;
         handled = true;
         break;
      }
   }

   // If this wasn't handled by a behavior above then pass along to the parent DynamicConsoleMethodComponent
   // to deal with it.  If the parent cannot handle the message it will return an error string.
   if (!handled)
   {
      result = Parent::callOnBehaviors(argc, argv);
   }

   // Clean up.
   FrameAllocator::setWaterMark(strdupWatermark);

   return result;
}

//-----------------------------------------------------------------------------

const char *BehaviorComponent::_callMethod(U32 argc, const char *argv[], bool callThis /* = true  */)
{
   if (mBehaviors.empty())
      return Parent::_callMethod(argc, argv, callThis);

   // Copy the arguments to avoid weird clobbery situations.
   FrameTemp<char *> argPtrs(argc);

   U32 strdupWatermark = FrameAllocator::getWaterMark();
   for (U32 i = 0; i < argc; i++)
   {
      argPtrs[i] = reinterpret_cast<char *>(FrameAllocator::alloc(dStrlen(argv[i]) + 1));
      dStrcpy(argPtrs[i], argv[i], dStrlen(argv[i]) + 1);
   }

   for (SimSet::iterator i = mBehaviors.begin(); i != mBehaviors.end(); i++)
   {
      BehaviorInstance *pBehavior = dynamic_cast<BehaviorInstance *>(*i);
      AssertFatal(pBehavior, "BehaviorComponent::_callMethod - Bad behavior instance in list.");
      AssertFatal(pBehavior->getId() > 0, "Invalid id for behavior component");

      // Use the BehaviorInstance's namespace
      Namespace *pNamespace = pBehavior->getNamespace();
      if (!pNamespace)
         continue;

      // Lookup the Callback Namespace entry and then splice callback
      const char *cbName = StringTable->insert(argv[0]);
      Namespace::Entry *pNSEntry = pNamespace->lookup(cbName);
      if (pNSEntry)
      {
         // Set %this to our BehaviorInstance's Object ID
         argPtrs[1] = const_cast<char *>(pBehavior->getIdString());

         // Change the Current Console object, execute, restore Object
         SimObject *save = gEvalState.thisObject;
         gEvalState.thisObject = pBehavior;

         pNSEntry->execute(argc, (ConsoleValueRef*)(~argPtrs), &gEvalState);

         gEvalState.thisObject = save;
      }
   }

   // Pass this up to the parent since a BehaviorComponent is still a DynamicConsoleMethodComponent
   // it needs to be able to contain other components and behave properly
   const char* fnRet = Parent::_callMethod(argc, argv, callThis);

   // Clean up.
   FrameAllocator::setWaterMark(strdupWatermark);

   return fnRet;
}

//-----------------------------------------------------------------------------

DefineEngineMethod(BehaviorComponent, addBehavior, bool, (BehaviorInstance* bi), ,
   "Add (BehaviorInstance) to component.\n")
{
   return object->addBehavior(bi);
}

DefineEngineMethod(BehaviorComponent, removeBehavior, bool, (BehaviorInstance* bi), ,
   "Remove (BehaviorInstance) from component.\n")
{
   return object->removeBehavior(bi);
}

DefineEngineMethod(BehaviorComponent, clearBehaviors, void, (), ,
   "Clear behaviors from this component.\n")
{
   object->clearBehaviors();
}

DefineEngineMethod(BehaviorComponent, getBehaviorCount, S32,(),,
   "Return behavior count for this component.\n")
{
   return object->getBehaviorCount();
}


DefineEngineMethod(BehaviorComponent, getBehavior, S32, (StringTableEntry btName), ,
   "Return the BehaviorInstance ID with (name).\n")
{
   BehaviorInstance* pBinst = object->getBehavior(btName);

   return pBinst ? pBinst->getId() : -1;
}

DefineEngineMethod(BehaviorComponent, getBehaviorByID, S32, (U32 id), ,
   "Return the BehaviorInstance ID by (ID).\n")
{
   BehaviorInstance* pBinst = object->getBehavior(id);

   return pBinst ? pBinst->getId() : -1;
}


DefineEngineMethod(BehaviorComponent, connect, bool, (U32 outputId, U32 inputId, StringTableEntry outputName, StringTableEntry inputName), ,
   "Connects (outputName) on Behavior(outputID) to Behavior(inputId) (inputname).\n")
{
   BehaviorInstance* pOutBeh = dynamic_cast<BehaviorInstance*>(Sim::findObject(outputId));
   if (!pOutBeh)
   {
      Con::warnf("connect - Could not find output behavior id");
      return false;
   }
   BehaviorInstance* pInBeh = dynamic_cast<BehaviorInstance*>(Sim::findObject(inputId));
   if (!pInBeh)
   {
      Con::warnf("connect - Could not find input behavior id");
      return false;
   }

   return object->connect(pOutBeh, pInBeh, outputName, inputName);

}

DefineEngineMethod(BehaviorComponent, disconnect, bool, (U32 outputId, U32 inputId, StringTableEntry outputName, StringTableEntry inputName), ,
   "Disconnects (outputName) on Behavior(outputID) from Behavior(inputId) (inputname).\n")
{
   BehaviorInstance* pOutBeh = dynamic_cast<BehaviorInstance*>(Sim::findObject(outputId));
   if (!pOutBeh)
   {
      Con::warnf("disconnect - Could not find output behavior id");
      return false;
   }
   BehaviorInstance* pInBeh = dynamic_cast<BehaviorInstance*>(Sim::findObject(inputId));
   if (!pInBeh)
   {
      Con::warnf("disconnect - Could not find input behavior id");
      return false;
   }

   return object->disconnect(pOutBeh, pInBeh, outputName, inputName);

}

DefineEngineMethod(BehaviorComponent, raise, bool, (U32 outputId, StringTableEntry outputName, U32 time), ,
   "Raise an event on a behavior(outputID) output(outputName) at an optional[time].\n")
{
   BehaviorInstance* pOutBeh = dynamic_cast<BehaviorInstance*>(Sim::findObject(outputId));
   if (!pOutBeh)
   {
      Con::warnf("raise - Could not find output behavior id");
      return false;
   }

   U32 timeDelt = 0;

   if (time)
      timeDelt = time;

   BehaviorComponentRaiseEvent* pEven = new BehaviorComponentRaiseEvent(pOutBeh, outputName);
   Sim::postEvent(object, pEven, Sim::getCurrentTime() + timeDelt);

   return true;

}


DefineEngineMethod(BehaviorComponent, getBehaviorConnectionCount, S32, (U32 outputId, StringTableEntry outputName), ,
   "Returns output count for behavior(outputID) output(outputName).\n")
{
   BehaviorInstance* pOutBeh = dynamic_cast<BehaviorInstance*>(Sim::findObject(outputId));
   if (!pOutBeh)
   {
      Con::warnf("getBehaviorConnectionCount - Could not find output behavior id");
      return -1;
   }

   return object->getBehaviorConnectionCount(pOutBeh,outputName);

}

DefineEngineMethod(BehaviorComponent, getBehaviorConnection, const char*, (U32 outputId, StringTableEntry outputName, U32 id), ,
   "Returns output count for behavior(outputID) output(outputName).\n")
{
   BehaviorInstance* pOutBeh = dynamic_cast<BehaviorInstance*>(Sim::findObject(outputId));
   if (!pOutBeh)
   {
      Con::warnf("getBehaviorConnection - Could not find output behavior id");
      return StringTable->EmptyString();
   }

   const BehaviorComponent::BehaviorPortConnection* pBehConn = object->getBehaviorConnection(pOutBeh, outputName, id);

   if (!pBehConn)
   {
      Con::warnf("getBehaviorConnection - Could not find connection id");
      return StringTable->EmptyString();
   }

   char* buff;
   dSprintf(buff, 1024, "%d %d %s %s",
      pBehConn->mOutputInstance->getId(),
      pBehConn->mInputInstance->getId(),
      pBehConn->mOutputName,
      pBehConn->mInputName);

   return buff;

}


//-----------------------------------------------------------------------------
// Behavior Instance
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(BehaviorInstance);

//-----------------------------------------------------------------------------

BehaviorInstance::BehaviorInstance(BehaviorTemplate* pTemplate) :
   mTemplate(pTemplate),
   mBehaviorOwner(NULL),
   mBehaviorId(0)
{
   if (pTemplate != NULL)
   {
      // Fetch field prototype count.
      const U32 fieldCount = pTemplate->getBehaviorFieldCount();

      // Set field prototypes.
      for (U32 index = 0; index < fieldCount; ++index)
      {
         // Fetch fields.
         BehaviorTemplate::BehaviorField* pField = pTemplate->getBehaviorField(index);

         // Set cloned field.
         setDataField(pField->mName, NULL, pField->mDefaultValue);
      }
   }
}

//-----------------------------------------------------------------------------

bool BehaviorInstance::onAdd()
{
   if (!Parent::onAdd())
      return false;

   // Store this object's namespace
   mNameSpace = Namespace::global()->find(getTemplateName());

   return true;
}

//-----------------------------------------------------------------------------

void BehaviorInstance::onRemove()
{
   Parent::onRemove();
}

//-----------------------------------------------------------------------------

void BehaviorInstance::initPersistFields()
{
   addGroup("Behavior");
   addField("template", TypeSimObjectName, Offset(mTemplate, BehaviorInstance), "Template this instance was created from.");
   addProtectedField("Owner", TypeSimObjectPtr, Offset(mBehaviorOwner, BehaviorInstance), &setOwner, &defaultProtectedGetFn, "Behavior component owner.");
   endGroup("Behavior");

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

const char* BehaviorInstance::getTemplateName(void)
{
   return mTemplate ? mTemplate->getName() : NULL;
}

// Get template.
const char* BehaviorInstance::getTemplate(void* obj, const char* data)
{
   return static_cast<BehaviorInstance*>(obj)->getTemplate()->getIdString();
}

DefineEngineMethod(BehaviorInstance, getTemplateName, const char*, (), ,
   "Returns template name this BehaviorInstance is using.\n")
{
   const char* nName = object->getTemplateName();
   return nName ? nName : StringTable->EmptyString();
}

DefineEngineFunction(copyBehaviorToComponent, bool, (U32 bId, U32 cId), ,
   "Copy BehaviorInstance(bId) to component(cId)")
{
   BehaviorInstance* pBeh = dynamic_cast<BehaviorInstance*>(Sim::findObject(bId));

   if (!pBeh)
   {
      Con::errorf("copyBehaviorToComponent - cannot find behavior id");
      return false;
   }
   SimComponent* pCom = dynamic_cast<SimComponent*>(Sim::findObject(cId));
   if (!pCom)
   {
      Con::errorf("copyBehaviorToComponent - cannot find component id");
      return false;
   }

   BehaviorTemplate* pTemp = pBeh->getTemplate();

   U32 fieldCount = pTemp->getBehaviorFieldCount();
   const char* pFieldVal = NULL;

   BehaviorTemplate::BehaviorField* pField = NULL;
   for (U32 i = 0; i < fieldCount; ++i)
   {
      pField = pTemp->getBehaviorField(i);
      pFieldVal = pBeh->getDataField(pField->mName, NULL);

      pCom->setDataField(pField->mName, NULL, pFieldVal);
   }

   return true;

}


