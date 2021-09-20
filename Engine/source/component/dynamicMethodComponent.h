#ifndef _DYNAMICMETHODCOMPONENT_H_
#define _DYNAMICMETHODCOMPONENT_H_

#ifndef _CONSOLEINTERNAL_H_
#include "console/consoleInternal.h"
#endif // !_CONSOLEINTERNAL_H_

#ifndef _SIMCOMPONENT_H_
#include "component/simComponent.h"
#endif // !_SIMCOMPONENT_H_

class DynamicMethodComponent : public SimComponent
{
   typedef SimComponent Parent;

protected:

   virtual const char* _callMethod(U32 argc, const char *argv[], bool callThis = true);

public:

   const char* callMethod(S32 argc, const char* methodName, ...);

   virtual const char* callMethodArgList(U32 argc, const char *argv[], bool callThis = true);
   virtual bool handlesConsoleMethod(const char * fname, S32 * routingId);
   virtual const char* callOnBehaviors(U32 argc, const char *argv[]);



   DECLARE_CONOBJECT(DynamicMethodComponent);
   DECLARE_DESCRIPTION("The Builds a method for behavior components.");
};


#endif // !_DYNAMICMETHODCOMPONENT_H_
