#ifndef _EDITTS2DCTRL_H_
#define _EDITTS2DCTRL_H_

#ifndef _GUITS2DCONTROL_H_
#include "gui/2d/guiTS2DControl.h"
#endif // !_GUITS2DCONTROL_H_

#ifndef _GIZMO_H_
#include "gui/worldEditor/gizmo.h"
#endif

#ifndef _GFX_GFXDRAWER_H_
#include "gfx/gfxDrawUtil.h"
#endif

class Gizmo;
class EditManager;
struct ObjectRenderInst;
class SceneRenderState;
class BaseMatInstance;

class EditTS2DCtrl : public GuiTS2DCtrl
{
   typedef GuiTS2DCtrl Parent;

protected:

   // GuiControl
   /*virtual void getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent);
   virtual void onMouseUp(const GuiEvent & event);
   virtual void onMouseDown(const GuiEvent & event);
   virtual void onMouseMove(const GuiEvent & event);
   virtual void onMouseDragged(const GuiEvent & event);
   virtual void onMouseEnter(const GuiEvent & event);
   virtual void onMouseLeave(const GuiEvent & event);
   virtual void onRightMouseDown(const GuiEvent & event);
   virtual void onRightMouseUp(const GuiEvent & event);
   virtual void onRightMouseDragged(const GuiEvent & event);
   virtual void onMiddleMouseDown(const GuiEvent & event);
   virtual void onMiddleMouseUp(const GuiEvent & event);
   virtual void onMiddleMouseDragged(const GuiEvent & event);
   virtual bool onInputEvent(const InputEventInfo & event);
   virtual bool onMouseWheelUp(const GuiEvent &event);
   virtual bool onMouseWheelDown(const GuiEvent &event);

   virtual void updateGuiInfo() {};
   virtual void renderScene(const RectI &) {};
   virtual void renderCameraAxis();
   virtual void renderGrid();

   // GuiTS2DCtrl
   void renderWorld(const RectI & updateRect);
   void _renderScene(ObjectRenderInst*, SceneRenderState *state, BaseMatInstance*);

   bool              mLeftMouseDown;
   bool              mRightMouseDown;
   bool              mMiddleMouseDown;
   bool              mMiddleMouseTriggered;
   bool              mMouseLeft;

   SimObjectPtr<Gizmo> mGizmo;
   GizmoProfile *mGizmoProfile;

   // grid drawing
   GFXVertexBufferHandle<GFXVertexPCT> mDots;
   GFXStateBlockRef mDotSB;

public:

   EditTS2DCtrl();
   ~EditTS2DCtrl();

   // SimObject
   bool onAdd();
   void onRemove();

   static void initPersistFields();
   static void consoleInit();

   static Point3F    smCamPos;
   static MatrixF    smCamMatrix;

   virtual bool getCameraTransform(MatrixF* cameraMatrix);
   bool processCameraQuery(CameraQuery * query);

   // guiControl
   virtual void onRender(Point2I offset, const RectI &updateRect);

   virtual bool resize(const Point2I& newPosition, const Point2I& newExtent);

   DECLARE_CONOBJECT(EditTS2DCtrl);
   DECLARE_CATEGORY("Gui Editor");*/

};

#endif // !_EDITTS2DCTRL_H_
