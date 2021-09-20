#include "T2D/Math2D/Vector2.h"


IMPLEMENT_STRUCT(Vector2, Vector2, ,
   "")
END_IMPLEMENT_STRUCT;

ConsoleType(Vector2, TypeVector2, Vector2,"")

ConsoleGetType(TypeVector2)
{
   Vector2 *pt = (Vector2*)dptr;
   static const U32 bufSize = 256;
   char* returnBuffer = Con::getReturnBuffer(bufSize);
   dSprintf(returnBuffer, bufSize, "%d %d", pt->x, pt->y);
   return returnBuffer;
}

ConsoleSetType(TypeVector2)
{
   if (argc == 1)
      dSscanf(argv[0], "%g %g", &((Vector2 *)dptr)->x, &((Vector2 *)dptr)->y);
   else if (argc == 2)
      *((Vector2 *)dptr) = Vector2(dAtof(argv[0]), dAtof(argv[1]));
   else
      Con::printf("Vector2 must be set as { x, y } or \"x y\"");
}
