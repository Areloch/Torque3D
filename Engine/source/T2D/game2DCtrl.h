#ifndef _GAME2DCTRL_H_
#define _GAME2DCTRL_H_

#ifndef _GAME_H_
#include "app/game.h"
#endif
#ifndef _GUITSCONTROL_H_
#include "gui/3d/guiTSControl.h"
#endif

class GameBase;

class Game2DCtrl : public GuiTSCtrl
{
private:
   typedef GuiTSCtrl Parent;

   void makeScriptCall(const char *func, const GuiEvent &evt)  const;

public:

   Game2DCtrl();

   DECLARE_CONOBJECT(Game2DCtrl);
   DECLARE_DESCRIPTION("A control that renders a 2D view from the current control object.");

   bool processCameraQuery(CameraQuery *query);
   void renderWorld(const RectI &updateRect);

   // GuiControl
   virtual void onMouseDown(const GuiEvent &evt);
   virtual void onRightMouseDown(const GuiEvent &evt);
   virtual void onMiddleMouseDown(const GuiEvent &evt);

   virtual void onMouseUp(const GuiEvent &evt);
   virtual void onRightMouseUp(const GuiEvent &evt);
   virtual void onMiddleMouseUp(const GuiEvent &evt);

   void onMouseMove(const GuiEvent &evt);
   void onRender(Point2I offset, const RectI &updateRect);

   virtual bool onAdd();

};

#endif // !_GAME2DCTRL_H_
