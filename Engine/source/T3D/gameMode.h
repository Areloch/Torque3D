#pragma once
#ifndef GAME_MODE_H
#define GAME_MODE_H

#ifdef TORQUE_TOOLS
#ifndef _GUI_INSPECTOR_TYPES_H_
#include "gui/editor/guiInspectorTypes.h"
#endif
#endif

class GameMode : public SimObject
{
   typedef SimObject Parent;
private:
   StringTableEntry mGameModeName;

   bool mIsActive;

public:

   GameMode();
   ~GameMode() = default;
   static void initPersistFields();
   bool onAdd() override;
   void onRemove() override;

   bool isActive() { return mIsActive; }
   void setActive(const bool& active);

   DECLARE_CONOBJECT(GameMode);

   static void findGameModes(const char* gameModeList, Vector<GameMode*>* outGameModes);

   DECLARE_CALLBACK(void, onActivated, ());
   DECLARE_CALLBACK(void, onDeactivated, ());
   DECLARE_CALLBACK(void, onSceneLoaded, ());
   DECLARE_CALLBACK(void, onSceneUnloaded, ());
   DECLARE_CALLBACK(void, onSubsceneLoaded, ());
   DECLARE_CALLBACK(void, onSubsceneUnloaded, ());
};

DefineConsoleType(TypeGameModeList, String)

#ifdef TORQUE_TOOLS
class GuiInspectorTypeGameModeListHelper;

class GuiInspectorTypeGameModeList : public GuiInspectorField
{
   typedef GuiInspectorField Parent;
public:

   GuiInspectorTypeGameModeListHelper* mHelper;
   GuiRolloutCtrl* mRollout;
   GuiDynamicCtrlArrayControl* mArrayCtrl;
   Vector<GuiInspectorField*> mChildren;

   GuiBitmapButtonCtrl* mButton;
   RectI mButtonRect;

   DECLARE_CONOBJECT(GuiInspectorTypeGameModeList);

   GuiInspectorTypeGameModeList();

   // ConsoleObject
   bool onAdd() override;
   static void consoleInit();

   // GuiInspectorField
   bool resize(const Point2I& newPosition, const Point2I& newExtent) override;
   void childResized(GuiControl* child) override;
   bool updateRects() override;
   void updateData() override;
   StringTableEntry getValue() override;
   void setValue(StringTableEntry value) override;
};

class GuiInspectorTypeGameModeListHelper : public GuiInspectorField
{
   typedef GuiInspectorField Parent;

public:

   GuiInspectorTypeGameModeListHelper();

   DECLARE_CONOBJECT(GuiInspectorTypeGameModeListHelper);

   GuiBitmapButtonCtrl* mButton;
   GuiRolloutCtrl* mParentRollout;
   GuiInspectorTypeGameModeList* mParentField;
   RectI mButtonRect;

   //-----------------------------------------------------------------------------
   // Override able methods for custom edit fields
   //-----------------------------------------------------------------------------
   GuiControl* constructEditControl() override;
   bool               resize(const Point2I& newPosition, const Point2I& newExtent) override;
   bool               updateRects() override;
   void               setValue(StringTableEntry value) override;
};
#endif
#endif
