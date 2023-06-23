#pragma once

#ifndef _ASSET_BASE_H_
#include "assets/assetBase.h"
#endif

#ifndef _ASSET_DEFINITION_H_
#include "assets/assetDefinition.h"
#endif

#ifndef _STRINGUNIT_H_
#include "string/stringUnit.h"
#endif

#ifndef _ASSET_FIELD_TYPES_H_
#include "assets/assetFieldTypes.h"
#endif

#ifndef _ASSET_PTR_H_
#include "assets/assetPtr.h"
#endif

#ifndef _GUI_INSPECTOR_TYPES_H_
#include "gui/editor/guiInspectorTypes.h"
#endif

#include "gui/core/guiControl.h"
#include "assetMacroHelpers.h"
#include <gui/controls/guiBitmapCtrl.h>

//-----------------------------------------------------------------------------
class GUITemplateAsset : public AssetBase
{
   typedef AssetBase Parent;

   Vector<SimObjectPtr<GuiControl>> mTemplateData;

public:
   GUITemplateAsset();
   virtual ~GUITemplateAsset();

   /// Engine.
   static void initPersistFields();
   virtual void copyTo(SimObject* object);

   Vector<SimObjectPtr<GuiControl>> instantiateTemplate();

   Point2I getExtents();

   /// Declare Console Object.
   DECLARE_CONOBJECT(GUITemplateAsset);

   //DECLARE_CALLBACK(void, onInitializeAsset, ());
   //DECLARE_CALLBACK(void, onRefreshAsset, ());
   //DECLARE_CALLBACK(void, onUnloadAsset, ());

protected:
   virtual void            initializeAsset(void);
   virtual void            onAssetRefresh(void);

   void loadGUiTemplate();
};

DefineConsoleType(TypeGUITemplateAssetPtr, GUITemplateAsset)
DefineConsoleType(TypeGUITemplateAssetId, String)

#ifdef TORQUE_TOOLS
//-----------------------------------------------------------------------------
// TypeAssetId GuiInspectorField Class
//-----------------------------------------------------------------------------
class GuiInspectorTypeGUITemplateAssetPtr : public GuiInspectorTypeFileName
{
   typedef GuiInspectorTypeFileName Parent;
public:

   GuiBitmapButtonCtrl* mSMEdButton;

   DECLARE_CONOBJECT(GuiInspectorTypeGUITemplateAssetPtr);
   static void consoleInit();

   virtual GuiControl* constructEditControl();
   virtual bool updateRects();
};

class GuiInspectorTypeGUITemplateAssetId : public GuiInspectorTypeGUITemplateAssetPtr
{
   typedef GuiInspectorTypeGUITemplateAssetPtr Parent;
public:

   DECLARE_CONOBJECT(GuiInspectorTypeGUITemplateAssetId);
   static void consoleInit();
};
#endif
