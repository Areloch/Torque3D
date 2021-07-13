#include "T2D/game2DCtrl.h"
#include "console/consoleTypes.h"
#include "T3D/gameBase/gameBase.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/gameFunctions.h"
#include "console/engineAPI.h"

#include "game2DCtrl_ScriptBinding.h"

//----------------------------------------------------------------------------
// Class: Game2DCtrl
//----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(Game2DCtrl);

// See Torque manual (.CHM) for more information
ConsoleDocClass(Game2DCtrl,
   "@brief The main 2D viewport for a Torque 2D game.\n\n"
   "@ingroup Gui2D\n");

void Game2DCtrl::makeScriptCall(const char * func, const GuiEvent & evt) const
{
   // write screen position
   char *sp = Con::getArgBuffer(32);
   dSprintf(sp, 32, "%d %d", evt.mousePoint.x, evt.mousePoint.y);

   // write world position
   char *wp = Con::getArgBuffer(32);
   Point3F camPos;
   mLastCameraQuery.cameraMatrix.getColumn(3, &camPos);
   dSprintf(wp, 32, "%g %g %g", camPos.x, camPos.y, camPos.z);

   // write click vector
   char *vec = Con::getArgBuffer(32);
   Point3F fp(evt.mousePoint.x, evt.mousePoint.y, 1.0);
   Point3F ray;
   unproject(fp, &ray);
   ray -= camPos;
   ray.normalizeSafe();
   dSprintf(vec, 32, "%g %g %g", ray.x, ray.y, ray.z);

   Con::executef((SimObject*)this, func, sp, wp, vec);
}

Game2DCtrl::Game2DCtrl()
{
}

bool Game2DCtrl::onAdd()
{
   if (!Parent::onAdd())
      return false;

   return true;
}


bool Game2DCtrl::processCameraQuery(CameraQuery * query)
{
   GameUpdateCameraFov();
   return GameProcessCameraQuery(query);
}

void Game2DCtrl::renderWorld(const RectI & updateRect)
{
   GameRenderWorld();
}

void Game2DCtrl::onMouseDown(const GuiEvent &evt)
{
   Parent::onMouseDown(evt);
   if (isMethod("onMouseDown"))
      makeScriptCall("onMouseDown", evt);
}

void Game2DCtrl::onRightMouseDown(const GuiEvent &evt)
{
   Parent::onRightMouseDown(evt);
   if (isMethod("onRightMouseDown"))
      makeScriptCall("onRightMouseDown", evt);
}

void Game2DCtrl::onMiddleMouseDown(const GuiEvent &evt)
{
   Parent::onMiddleMouseDown(evt);
   if (isMethod("onMiddleMouseDown"))
      makeScriptCall("onMiddleMouseDown", evt);
}

void Game2DCtrl::onMouseUp(const GuiEvent &evt)
{
   Parent::onMouseUp(evt);
   if (isMethod("onMouseUp"))
      makeScriptCall("onMouseUp", evt);
}

void Game2DCtrl::onRightMouseUp(const GuiEvent &evt)
{
   Parent::onRightMouseUp(evt);
   if (isMethod("onRightMouseUp"))
      makeScriptCall("onRightMouseUp", evt);
}

void Game2DCtrl::onMiddleMouseUp(const GuiEvent &evt)
{
   Parent::onMiddleMouseUp(evt);
   if (isMethod("onMiddleMouseUp"))
      makeScriptCall("onMiddleMouseUp", evt);
}

void Game2DCtrl::onMouseMove(const GuiEvent &evt)
{
   MatrixF mat;
   Point3F vel;
   if (GameGetCameraTransform(&mat, &vel))
   {
      Point3F pos;
      mat.getColumn(3, &pos);
      Point3F screenPoint((F32)evt.mousePoint.x, (F32)evt.mousePoint.y, -1.0f);
      Point3F worldPoint;
      if (unproject(screenPoint, &worldPoint)) {
         Point3F vec = worldPoint - pos;
         vec.normalizeSafe();
      }
   }
}

void Game2DCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   // check if should bother with a render
   GameConnection * con = GameConnection::getConnectionToServer();
   bool skipRender = !con || (con->getWhiteOut() >= 1.f) || (con->getDamageFlash() >= 1.f) || (con->getBlackOut() >= 1.f);

   if (!skipRender || true)
      Parent::onRender(offset, updateRect);
}

