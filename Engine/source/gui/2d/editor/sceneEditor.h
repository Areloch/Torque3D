#ifndef _SCENEEDITOR_H_
#define _SCENEEDITOR_H_

#ifndef _EDITTS2DCTRL_H_
#include "gui/2d/editor/editTS2Dctrl.h"
#endif

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif

#ifndef _TSIGNAL_H_
#include "core/util/tSignal.h"
#endif

#ifndef _CONSOLE_SIMOBJECTMEMENTO_H_
#include "console/simObjectMemento.h"
#endif

#ifndef _UNDO_H_
#include "util/undo.h"
#endif

#ifndef _DYNAMIC_CONSOLETYPES_H_
#include "console/dynamicTypes.h"
#endif

class SceneObject2D;

class SceneEditor : public EditTS2DCtrl
{
   typedef EditTS2DCtrl Parent;

public:
   SceneEditor();
   ~SceneEditor();

   /// SimObject
   virtual bool onAdd();

   static void initPersistFields();

   /// EditTS2DCtrl
   void on2DMouseUp(const Gui2DMouseEvent &event);
   void on2DMouseDown(const Gui2DMouseEvent &event);
   void on2DMouseMove(const Gui2DMouseEvent &event);
   void on2DMouseDragged(const Gui2DMouseEvent &event);
   void on2DMouseEnter(const Gui2DMouseEvent &event);
   void on2DMouseLeave(const Gui2DMouseEvent &event);
   void on2DRightMouseDown(const Gui2DMouseEvent &event);
   void on2DRightMouseUp(const Gui2DMouseEvent &event);
   void on2DRightMouseDragged(const Gui2DMouseEvent &event);
   void on2DMouseWheelUp(const Gui2DMouseEvent &event) {};
   void on2DMouseWheelDown(const Gui2DMouseEvent &event) {};
   void updateGuiInfo();

   DECLARE_CONOBJECT(SceneEditor);

};

#endif // !_SCENEEDITOR_H_
