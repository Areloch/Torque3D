#include "platform/platform.h"
#include"gui/2d/editor/editTS2Dctrl.h"

#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "T3D/gameBase/gameConnection.h"
#include "gui/worldEditor/editor.h"
#include "gui/core/guiCanvas.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDebugEvent.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "renderInstance/renderBinManager.h"

#include "T2D/Scene/Scene2D.h"

//------------------------------------------

IMPLEMENT_CONOBJECT(EditTS2DCtrl);
ConsoleDocClass(EditTS2DCtrl,
   "@brief 2D view ctrl used by Scene Editor.\n\n"
   "For editor use only.\n\n"
   "@ingroup Editors\n"
   "@internal"
);

//------------------------------------------

EditTS2DCtrl::EditTS2DCtrl()
{
   mGizmo                  = NULL;
   mGizmoProfile           = NULL;
   mLeftMouseDown          = false;
   mRightMouseDown         = false;
   mMiddleMouseDown        = false;
   mMiddleMouseTriggered   = false;
   mMouseLeft              = false;
   mRenderDots             = true;
   mRenderGrid             = true;
   mBlendSB                = NULL;
   mLastMousePos.set(0, 0);
   mDotColor.set(255, 255, 255, 100);
   mGridSnap.set(1.0f, 1.0f);
   mDotSB = NULL;

}

EditTS2DCtrl::~EditTS2DCtrl()
{
   mBlendSB = NULL;
   mDotSB = NULL;

}

bool EditTS2DCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;

   setModStaticFields(true);

   GFXStateBlockDesc blenddesc;
   blenddesc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
   mBlendSB = GFX->createStateBlock( blenddesc );

   return true;

}

void EditTS2DCtrl::onRemove()
{
   Parent::onRemove();

   if (mGizmo)
      mGizmo->deleteObject();

}

void EditTS2DCtrl::onRender(Point2I offset, const RectI & updateRect)
{
   updateGuiInfo();
   Parent::onRender(offset, updateRect);

}

void EditTS2DCtrl::renderWorld(const RectI & updateRect)
{
   gClientScene2DGraph->sceneRender2D();
}

void EditTS2DCtrl::_renderScene()
{
   if(mRenderGrid)
      renderGrid();

   if (mRenderDots)
      renderDots();

   renderScene(mSaveViewport);

}

void EditTS2DCtrl::renderGrid()
{
}

void EditTS2DCtrl::renderDots()
{
   /// get the camera area.
   RectF area = getCameraArea();
   ///work out max dots.
   U32 maxDot = (U32)(area.extent.x / mGridSnap.x) * (U32)(area.extent.y / mGridSnap.y);
   /// find the end.
   Point2F areaEnd(area.point.x + area.extent.x, area.point.y + area.extent.y);
   area.point.x = mGridSnap.x * ((S32)(area.point.x / mGridSnap.x));
   area.point.y = mGridSnap.y * ((S32)(area.point.y / mGridSnap.y));

   mDots.set(GFX, maxDot, GFXBufferTypeStatic);
   Vector2 curDot;
   U32 nDot = 0;
   mDots.lock();

   /// dots should be the center point of the grid snap.
   for (F32 ix = area.point.x + (mGridSnap.x * 0.5f); ix < areaEnd.x; ix += mGridSnap.x)
   {
      for (F32 iy = area.point.y - (mGridSnap.y * 0.5f); iy < areaEnd.y; iy += mGridSnap.y)
      {
         curDot.set(ix, iy);
         sceneToWindow(curDot, curDot);
         curDot = (Vector2)localToGlobalCoord(Point2I(S32(curDot.x), S32(curDot.y)));
         mDots[nDot].color.set(mDotColor);
         mDots[nDot].point.x = curDot.x;
         mDots[nDot].point.y = curDot.y;
         mDots[nDot].point.z = 0.0f;
         nDot++;

      }
   }
   mDots.unlock();

   if (!mDotSB)
   {
      GFXStateBlockDesc dotdesc;
      dotdesc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
      dotdesc.setCullMode(GFXCullNone);
      mDotSB = GFX->createStateBlock(dotdesc);
   }

   GFX->setStateBlock(mDotSB);

   GFX->setVertexBuffer(mDots);
   GFX->drawPrimitive(GFXPointList, 0, mDots->mNumVerts);

}

void EditTS2DCtrl::renderCameraArea()
{
}


bool EditTS2DCtrl::resize(const Point2I & newPosition, const Point2I & newExtent)
{
   if(!Parent::resize(newPosition, newExtent))
      return false;

   if (isMethod("onResize"))
      Con::executef(this, "onResize", newPosition, newExtent);

   return true;
}


//------------------------------------------
// Handle Input.
//------------------------------------------

void EditTS2DCtrl::make2DMouseEvent(Gui2DMouseEvent& gui2DEvent, const GuiEvent &guiEvent)
{
   (GuiEvent&)(gui2DEvent) = guiEvent;
   gui2DEvent.windowPos = Vector2(guiEvent.mousePoint.x, guiEvent.mousePoint.y);
   windowToScene(gui2DEvent.windowPos, gui2DEvent.scenePos);

}

void EditTS2DCtrl::getCursor(GuiCursor *&cursor, bool &visible, const GuiEvent &event)
{
   make2DMouseEvent(mLastEvent, event);
   get2DCursor(cursor, visible, mLastEvent);

}

void EditTS2DCtrl::get2DCursor(GuiCursor *&cursor, bool &visible, const Gui2DMouseEvent &event)
{
   TORQUE_UNUSED(event);
   cursor = NULL;
   visible = false;

}

void EditTS2DCtrl::onMouseUp(const GuiEvent & event)
{
   mLeftMouseDown = false;
   make2DMouseEvent(mLastEvent, event);
   on2DMouseUp(mLastEvent);
}

void EditTS2DCtrl::onMouseDown(const GuiEvent & event)
{
   mLeftMouseDown = true;
   make2DMouseEvent(mLastEvent, event);
   on2DMouseDown(mLastEvent);

   setFirstResponder();

}

void EditTS2DCtrl::onMouseMove(const GuiEvent & event)
{
   make2DMouseEvent(mLastEvent, event);
   on2DMouseMove(mLastEvent);

   mLastMousePos = event.mousePoint;
}

void EditTS2DCtrl::onMouseDragged(const GuiEvent & event)
{
   make2DMouseEvent(mLastEvent, event);
   on2DMouseDragged(mLastEvent);
}

void EditTS2DCtrl::onMouseEnter(const GuiEvent & event)
{
   mMouseLeft = false;
   make2DMouseEvent(mLastEvent, event);
   on2DMouseEnter(mLastEvent);
}

void EditTS2DCtrl::onMouseLeave(const GuiEvent & event)
{
   mMouseLeft = true;
   make2DMouseEvent(mLastEvent, event);
   on2DMouseLeave(mLastEvent);
}

void EditTS2DCtrl::onRightMouseDown(const GuiEvent & event)
{
   mRightMouseDown = true;
   make2DMouseEvent(mLastEvent, event);
   on2DRightMouseDown(mLastEvent);
}

void EditTS2DCtrl::onRightMouseUp(const GuiEvent & event)
{
   mRightMouseDown = false;
   make2DMouseEvent(mLastEvent, event);
   on2DRightMouseUp(mLastEvent);
}

void EditTS2DCtrl::onRightMouseDragged(const GuiEvent & event)
{
   make2DMouseEvent(mLastEvent, event);
   on2DRightMouseDragged(mLastEvent);
}

void EditTS2DCtrl::onMiddleMouseDown(const GuiEvent & event)
{
   mMiddleMouseDown = true;
   mMiddleMouseTriggered = false;

   if (!mLeftMouseDown && !mRightMouseDown && mProfile->mCanKeyFocus)
   {
      GuiCanvas *pCanvas = getRoot();
      if (!pCanvas)
         return;

      PlatformWindow *pWindow = static_cast<GuiCanvas*>(getRoot())->getPlatformWindow();
      if (!pWindow)
         return;

      PlatformCursorController *pController = pWindow->getCursorController();
      if (!pController)
         return;

      // ok, gotta disable the mouse
      // script functions are lockMouse(true); Canvas.cursorOff();
      pWindow->setMouseLocked(true);
      pCanvas->setCursorON(false);

      // Trigger 2 is used by the camera
      MoveManager::mTriggerCount[2]++;
      mMiddleMouseTriggered = true;

      setFirstResponder();
   }
   
}

void EditTS2DCtrl::onMiddleMouseUp(const GuiEvent & event)
{
   // Trigger 2 is used by the camera
   if (mMiddleMouseTriggered)
   {
      MoveManager::mTriggerCount[2]++;
      mMiddleMouseTriggered = false;
   }

   mMiddleMouseDown = false;

}

void EditTS2DCtrl::onMiddleMouseDragged(const GuiEvent & event)
{
   mLastMousePos = event.mousePoint;
}

bool EditTS2DCtrl::onMouseWheelUp(const GuiEvent & event)
{
   make2DMouseEvent(mLastEvent, event);
   on2DMouseWheelUp(mLastEvent);

   return false;

}

bool EditTS2DCtrl::onMouseWheelDown(const GuiEvent & event)
{
   make2DMouseEvent(mLastEvent, event);
   on2DMouseWheelDown(mLastEvent);

   return false;

}

bool EditTS2DCtrl::onInputEvent(const InputEventInfo & event)
{
   return false;
}

//------------------------------------------

bool EditTS2DCtrl::processCameraQuery(CameraQuery *query)
{
   /// the same process camera query from game2DCtrl.
   /// editor view might need to be different, but for now
   /// cant think of any reason it should.
   GameConnection* conn = GameConnection::getConnectionToServer();
   if (conn && conn->getControlCameraTransform(0.032f, &query->cameraMatrix))
   {
      query->object = dynamic_cast<GameBase*>(conn->getCameraObject());

      query->eyeTransforms[0] = query->cameraMatrix;
      query->eyeTransforms[1] = query->cameraMatrix;
      query->headMatrix = query->cameraMatrix;

      F32 cameraFov = 0.0f;
      // Use the connection's FOV settings if requried
      if (!conn->getControlCameraFov(&cameraFov))
      {
         return false;
      }

      query->fov = mDegToRad(cameraFov);

      /// change the camera pos to a 2d position
      Point2F pos(query->cameraMatrix.getPosition().x, query->cameraMatrix.getPosition().y);

      /// pass camera size off to query.
      query->mCameraSize = gClientScene2DGraph->getCameraSize();

      /// set camera area 
      query->mCamArea = RectF(pos.x - (query->mCameraSize.x * 0.5f),
         pos.y - (query->mCameraSize.y *0.5f),
         query->mCameraSize.x, query->mCameraSize.y);

      return true;
   }

   return false;
}

