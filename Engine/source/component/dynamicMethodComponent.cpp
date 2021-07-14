#include "platform/platform.h"
#include "console/engineAPI.h"
#include "dynamicMethodComponent.h"

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(DynamicMethodComponent);

//-----------------------------------------------------------------------------

const char * DynamicMethodComponent::_callMethod(U32 argc, const char * argv[], bool callThis)
{
   SimObject *pThis = dynamic_cast<SimObject *>(this);
   AssertFatal(pThis, "DynamicConsoleMethodComponent::callMethod : this should always exist!");

   const char *cbName = StringTable->insert(argv[0]);

   if (getComponentCount() > 0)
   {
      VectorPtr<SimComponent *>&componentList = lockComponentList();
      for (SimComponentItr nItr = (componentList.end() - 1); nItr >= componentList.begin(); nItr--)
      {
         argv[0] = cbName;

         SimComponent *pComponent = (*nItr);
         AssertFatal(pComponent, "DynamicMethodComponent::callMethod - NULL component in list!");

         DynamicMethodComponent *pThisComponent = dynamic_cast<DynamicMethodComponent*>(pComponent);
         AssertFatal(pThisComponent, "DynamicMethodComponent::callMethod - Non DynamicMethodComponent component attempting to callback!");

         if (pComponent->isEnabled())
            Con::execute(pThisComponent, argc, argv);

      }
      unlockComponentList();
   }

   const char* result = "";
   if (callThis)
      result = Con::execute(pThis, argc, argv, true);

   return result;
}

//-----------------------------------------------------------------------------

const char * DynamicMethodComponent::callMethod(S32 argc, const char * methodName, ...)
{
   const char *argv[128];
   methodName = StringTable->insert(methodName);

   argc++;

   va_list args;
   va_start(args, methodName);
   for (S32 i = 0; i < argc; i++)
      argv[i + 2] = va_arg(args, const char *);
   va_end(args);

   argv[0] = methodName;
   argv[1] = methodName;
   argv[2] = methodName;

   return callMethodArgList(argc, argv);
}

//-----------------------------------------------------------------------------

const char * DynamicMethodComponent::callMethodArgList(U32 argc, const char * argv[], bool callThis)
{
   return _callMethod(argc, argv, callThis);
}

//-----------------------------------------------------------------------------

bool DynamicMethodComponent::handlesConsoleMethod(const char * fname, S32 * routingId)
{
   if (isMethod(fname))
   {
      *routingId = -1;
      return true;
   }

   S32 nI = 0;
   VectorPtr<SimComponent*> &componentList = lockComponentList();
   for (SimComponentItr nItr = componentList.begin(); nItr != componentList.end(); nItr++, nI++)
   {
      SimObject *pComponent = dynamic_cast<SimObject*>(*nItr);
      if (pComponent != NULL && pComponent->isMethod(fname))
      {
         *routingId = -2;
         unlockComponentList();
         return true;
      }
   }
   unlockComponentList();

   return false;
}

//-----------------------------------------------------------------------------

const char * DynamicMethodComponent::callOnBehaviors(U32 argc, const char * argv[])
{
   SimObject *pThis;
   pThis = dynamic_cast<SimObject *>(this);
   AssertFatal(pThis, "DynamicMethodComponent::callOnBehaviors : this should always exist!");

   const char* result = "";
   bool handled = false;

   if (getComponentCount() > 0)
   {
      VectorPtr<SimComponent *>&componentList = lockComponentList();
      for (SimComponentItr nItr = (componentList.end() - 1); nItr >= componentList.begin(); nItr--)
      {
         argv[0] = StringTable->insert(argv[0]);

         SimComponent *pComponent = (*nItr);
         AssertFatal(pComponent, "DynamicMethodComponent::callOnBehaviors - NULL component in list!");

         handled = pComponent->callMethodOnComponents(argc, argv, &result);
         if (handled)
            break;
      }
      unlockComponentList();
   }

   if (!handled)
   {
      result = "ERR_CALL_NOT_HANDLED";
   }

   return result;
}

//-----------------------------------------------------------------------------

DefineEngineStringlyVariadicMethod(DynamicMethodComponent, callOnBehaviors, const char*, 3,64,
   "( string method, string args... ) Dynamically call a method on an object.\n")
{
   argv[1] = argv[2];
   return object->callOnBehaviors(argc - 1, (const char**)argv + 1);
}
