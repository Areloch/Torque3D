#include "platform/platform.h"
#include "console/sim.h"
#include "console/engineAPI.h"
#include "console/consoleTypes.h"
#include "component/simComponent.h"
#include "core/stream/stream.h"

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(SimComponent);

//-----------------------------------------------------------------------------

bool SimComponent::onComponentAdd(SimComponent *target)
{
   Con::executef(this, 2, "onComponentAdd", Con::getIntArg(target->getId()));
   return true;
}

//-----------------------------------------------------------------------------

void SimComponent::onComponentRemove(SimComponent *target)
{
   Con::executef(this, 2, "onComponentRemove", Con::getIntArg(target->getId()));
}

//-----------------------------------------------------------------------------

SimComponent::SimComponent()
   : mOwner(NULL)
{
   mComponentList.clear();
   mMutex = Mutex::createMutex();

   mEnabled = true;
}

//-----------------------------------------------------------------------------

SimComponent::~SimComponent()
{
   Mutex::destroyMutex(mMutex);
   mMutex = NULL;
}

//-----------------------------------------------------------------------------

bool SimComponent::onAdd()
{
   if (!Parent::onAdd())
      return false;

   if (!_registerComponents(this))
      return false;

   return true;
}

//-----------------------------------------------------------------------------

void SimComponent::onRemove()
{
   _unregisterComponents();

   // Delete all components
   VectorPtr<SimComponent *>&componentList = lockComponentList();
   while (componentList.size() > 0)
   {
      SimComponent *c = componentList[0];
      componentList.erase(componentList.begin());

      if (c->isProperlyAdded())
         c->deleteObject();
      else if (!c->isRemoved() && !c->isDeleted())
         delete c;
      // else, something else is deleting this, don't mess with it
   }
   unlockComponentList();

   Parent::onRemove();
}

//-----------------------------------------------------------------------------

void SimComponent::initPersistFields()
{
   addProtectedField("Enabled", TypeBool, Offset(mEnabled, SimComponent), &setEnabled, &getEnabled, &writeEnabled, "");
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool SimComponent::_registerComponents(SimComponent *owner)
{
   // This method will return true if the object contains no components. See the
   // documentation for SimComponent::onComponentRegister for more information
   // on this behavior.
   bool ret = true;

   // If this doesn't contain components, don't even lock the list.
   if (hasComponents())
   {
      VectorPtr<SimComponent *> &components = lockComponentList();
      for (SimComponentItr i = components.begin(); i != components.end(); i++)
      {
         if (!(*i)->onComponentRegister(owner))
         {
            ret = false;
            break;
         }

         AssertFatal((*i)->mOwner == owner, "Component failed to call parent onComponentRegister!");

         // Recurse
         if (!(*i)->_registerComponents(owner))
         {
            ret = false;
            break;
         }
      }

      unlockComponentList();
   }

   return ret;
}

//-----------------------------------------------------------------------------

void SimComponent::_unregisterComponents()
{
   if (!hasComponents())
      return;

   VectorPtr<SimComponent *> &components = lockComponentList();
   for (SimComponentItr i = components.begin(); i != components.end(); i++)
   {
      (*i)->onComponentUnRegister();

      AssertFatal((*i)->mOwner == NULL, "Component failed to call parent onUnRegister");

      // Recurse
      (*i)->_unregisterComponents();
   }

   unlockComponentList();
}

//-----------------------------------------------------------------------------

bool SimComponent::processArguments(S32 argc, const char **argv)
{
   for (S32 i = 0; i < argc; i++)
   {
      SimComponent *obj = dynamic_cast<SimComponent*> (Sim::findObject(argv[i]));
      if (obj)
         addComponent(obj);
      else
         Con::printf("SimComponent::processArguments - Invalid Component Object \"%s\"", argv[i]);
   }
   return true;
}

//-----------------------------------------------------------------------------

bool SimComponent::addComponent(SimComponent *component)
{
   AssertFatal(dynamic_cast<SimObject*>(component), "SimComponent - Cannot add non SimObject derived components!");

   MutexHandle mh;
   if (mh.lock(mMutex, true))
   {
      for (SimComponentItr nItr = mComponentList.begin(); nItr != mComponentList.end(); nItr++)
      {
         SimComponent *pComponent = dynamic_cast<SimComponent*>(*nItr);
         AssertFatal(pComponent, "SimComponent::addComponent - NULL component in list!");
         if (pComponent == component)
            return true;
      }

      if (component->onComponentAdd(this))
      {
         component->mOwner = this;
         mComponentList.push_back(component);
         return true;
      }
   }

   return false;
}

//-----------------------------------------------------------------------------

bool SimComponent::removeComponent(SimComponent *component)
{
   MutexHandle mh;
   if (mh.lock(mMutex, true))
   {
      for (SimComponentItr nItr = mComponentList.begin(); nItr != mComponentList.end(); nItr++)
      {
         SimComponent *pComponent = dynamic_cast<SimComponent*>(*nItr);
         AssertFatal(pComponent, "SimComponent::removeComponent - NULL component in list!");
         if (pComponent == component)
         {
            AssertFatal(component->mOwner == this, "Somehow we contain a component who doesn't think we are it's owner.");
            (*nItr)->onComponentRemove(this);
            component->mOwner = NULL;
            mComponentList.erase(nItr);
            return true;
         }
      }
   }
   return false;
}

//-----------------------------------------------------------------------------

bool SimComponent::writeField(StringTableEntry fieldname, const char* value)
{
   if (!Parent::writeField(fieldname, value))
      return false;

   if (fieldname == StringTable->insert("owner"))
      return false;

   return true;
}

//-----------------------------------------------------------------------------

void SimComponent::write(Stream &stream, U32 tabStop, U32 flags /* = 0 */)
{
#if 1
   Parent::write(stream, tabStop, flags);
#else
   MutexHandle handle;
   handle.lock(mMutex); // When this goes out of scope, it will unlock it

   // export selected only?
   if ((flags & SelectedOnly) && !isSelected())
   {
      for (U32 i = 0; i < mComponentList.size(); i++)
         mComponentList[i]->write(stream, tabStop, flags);

      return;
   }

   stream.writeTabs(tabStop);
   char buffer[1024];
   dSprintf(buffer, sizeof(buffer), "new %s(%s) {\r\n", getClassName(), getName() ? getName() : "");
   stream.write(dStrlen(buffer), buffer);
   writeFields(stream, tabStop + 1);

   if (mComponentList.size())
   {
      stream.write(2, "\r\n");

      stream.writeTabs(tabStop + 1);
      stream.writeLine((U8 *)"// Note: This is a list of behaviors, not arbitrary SimObjects as in a SimGroup or SimSet.\r\n");

      for (U32 i = 0; i < mComponentList.size(); i++)
         mComponentList[i]->write(stream, tabStop + 1, flags);
   }

   stream.writeTabs(tabStop);
   stream.write(4, "};\r\n");
#endif
}

//-----------------------------------------------------------------------------

bool SimComponent::callMethodOnComponents(U32 argc, const char* argv[], const char** result)
{
   const char *cbName = StringTable->insert(argv[0]);

   if (isEnabled())
   {
      if (isMethod(cbName))
      {
         // This component can handle the given method
         *result = Con::execute(this, argc, argv, true);
         return true;
      }
      else if (getComponentCount() > 0)
      {
         // Need to try the component's children
         bool handled = false;
         VectorPtr<SimComponent *>&componentList = lockComponentList();
         for (SimComponentItr nItr = (componentList.end() - 1); nItr >= componentList.begin(); nItr--)
         {
            argv[0] = cbName;

            SimComponent *pComponent = (*nItr);
            AssertFatal(pComponent, "SimComponent::callMethodOnComponents - NULL component in list!");

            // Call on children
            handled = pComponent->callMethodOnComponents(argc, argv, result);
            if (handled)
               break;
         }

         unlockComponentList();

         if (handled)
            return true;
      }
   }

   return false;
}

//-----------------------------------------------------------------------------

DefineEngineMethod(SimComponent, addComponent, void, (SimComponent* sCom), ,
   "Adds a component.\n")
{
   if (!sCom)
   {
      Con::errorf("SimComponent::addComponent() needs to be a component.");
      return;
   }

   object->addComponent(sCom);

}

DefineEngineMethod(SimComponent, removeComponent, void, (SimComponent* sCom), ,
   "Removes a component.\n")
{
   if (!sCom)
   {
      Con::errorf("SimComponent::addComponent() needs to be a component.");
      return;
   }

   if (object->removeComponent(sCom))
   {
      return;
   }
   else
   {
      Con::errorf("SimComponent::removeComponent() not found.");
      return;
   }

}

DefineEngineMethod(SimComponent, getComponentCount, U32, (), ,
   "Get the component count.\n")
{
   return object->getComponentCount();
}

DefineEngineMethod(SimComponent, getComponent, S32, (S32 id), ,
   "Get component by id.\n")
{
   if (id < 0 || id >= object->getComponentCount())
   {
      Con::errorf("SimComponent::getComponent - Invalid Index %d", id);
      return 0;
   }

   SimComponent* comp = object->getComponent(id);
   return (comp) ? comp->getId() : -1;
}

DefineEngineMethod(SimComponent, setEnabled, void, (bool enabled), (false),
   "Set component to enabled.\n")
{
   object->setEnabled(enabled);
}

DefineEngineMethod(SimComponent, isEnabled, bool, (),,
   "Returns true if the component is enabled.\n")
{
   return object->isEnabled();
}

