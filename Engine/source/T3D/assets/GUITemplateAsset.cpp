//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "GUITemplateAsset.h"

#ifndef _ASSET_MANAGER_H_
#include "assets/assetManager.h"
#endif

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

#ifndef _TAML_
#include "persistence/taml/taml.h"
#endif

#ifndef _ASSET_PTR_H_
#include "assets/assetPtr.h"
#endif

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GUITemplateAsset);

ConsoleType(GUITemplateAssetPtr, TypeGUITemplateAssetPtr, GUITemplateAsset, ASSET_ID_FIELD_PREFIX)

//-----------------------------------------------------------------------------

ConsoleGetType(TypeGUITemplateAssetPtr)
{
   // Fetch asset Id.
   return (*((AssetPtr<GUITemplateAsset>*)dptr)).getAssetId();
}

//-----------------------------------------------------------------------------

ConsoleSetType(TypeGUITemplateAssetPtr)
{
   // Was a single argument specified?
   if (argc == 1)
   {
      // Yes, so fetch field value.
      const char* pFieldValue = argv[0];

      // Fetch asset pointer.
      AssetPtr<GUITemplateAsset>* pAssetPtr = dynamic_cast<AssetPtr<GUITemplateAsset>*>((AssetPtrBase*)(dptr));

      // Is the asset pointer the correct type?
      if (pAssetPtr == NULL)
      {
         // No, so fail.
         //Con::warnf("(TypeGUITemplateAssetPtr) - Failed to set asset Id '%d'.", pFieldValue);
         return;
      }

      // Set asset.
      pAssetPtr->setAssetId(pFieldValue);

      return;
   }

   // Warn.
   Con::warnf("(TypeGUITemplateAssetPtr) - Cannot set multiple args to a single asset.");
}


ConsoleType(assetIdString, TypeGUITemplateAssetId, const char*, ASSET_ID_FIELD_PREFIX)

ConsoleGetType(TypeGUITemplateAssetId)
{
   // Fetch asset Id.
   return *((const char**)(dptr));
}

ConsoleSetType(TypeGUITemplateAssetId)
{
   // Was a single argument specified?
   if (argc == 1)
   {
      // Yes, so fetch field value.
      const char* pFieldValue = argv[0];

      // Fetch asset Id.
      StringTableEntry* assetId = (StringTableEntry*)(dptr);

      // Update asset value.
      *assetId = StringTable->insert(pFieldValue);

      return;
   }

   // Warn.
   Con::warnf("(TypeGUITemplateAssetId) - Cannot set multiple args to a single asset.");
}
//-----------------------------------------------------------------------------

GUITemplateAsset::GUITemplateAsset()
{
   mTemplateData = nullptr;
}

//-----------------------------------------------------------------------------

GUITemplateAsset::~GUITemplateAsset()
{
   if(!mTemplateData.isNull())
      mTemplateData->safeDeleteObject();
}

//-----------------------------------------------------------------------------

void GUITemplateAsset::initPersistFields()
{
   docsURL;
   // Call parent.
   Parent::initPersistFields();
}

void GUITemplateAsset::initializeAsset()
{
   // Call parent.
   Parent::initializeAsset();

   loadGUiTemplate();
}

void GUITemplateAsset::onAssetRefresh()
{
   loadGUiTemplate();
}

//------------------------------------------------------------------------------

void GUITemplateAsset::loadGUiTemplate()
{
   if (mTemplateData.isValid())
   {
      mTemplateData->safeDeleteObject();
   }

   if (size() != 0)
   {
      for (U32 i = 0; i < size(); i++)
      {
         mTemplateData = dynamic_cast<GuiControl*>(getObject(i));
         if (mTemplateData)
         {
            mLoadedState = Ok;
            return;
         }
      }
   }

   mLoadedState = Failed;
}

//------------------------------------------------------------------------------

void GUITemplateAsset::copyTo(SimObject* object)
{
   // Call to parent.
   Parent::copyTo(object);
}

SimObjectPtr<GuiControl> GUITemplateAsset::instantiateTemplate()
{
   if (!mTemplateData)
      return nullptr;

   GuiControl* controls = dynamic_cast<GuiControl*>(mTemplateData->deepClone());

   return controls;
}

Point2I GUITemplateAsset::getExtents()
{
   if (!mTemplateData)
      return Point2I::Zero;

   Point2I extents = Point2I::Zero;
   for (U32 i = 0; i < mTemplateData->size(); i++)
   {
      GuiControl* control = dynamic_cast<GuiControl*>(mTemplateData->at(i));
      if (control)
      {
         Point2I ctrlExt = control->getExtent();

         if(ctrlExt.x > extents.x)
            extents.x = ctrlExt.x;
         if(ctrlExt.y > extents.y)
            extents.y = ctrlExt.y;
      }
   }

   return extents;
}

DefineEngineMethod(GUITemplateAsset, instantiate, S32, (), ,
   "Instantiates the template object and returns the SimObjectId of the copy.\n"
   "@return SimObjectId of the instantiated copy.")
{
   SimObjectPtr<GuiControl> temp = object->instantiateTemplate();
   if (temp)
      return temp->getId();
   return 0;
}

DefineEngineMethod(GUITemplateAsset, getExtents, Point2F, (), ,
   "Instantiates the template object and returns the SimObjectId of the copy.\n"
   "@return SimObjectId of the instantiated copy.")
{
   Point2I extents = object->getExtents();
   return Point2F(extents.x, extents.y);
}

#ifdef TORQUE_TOOLS
//-----------------------------------------------------------------------------
// GuiInspectorTypeAssetId
//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiInspectorTypeGUITemplateAssetPtr);

ConsoleDocClass(GuiInspectorTypeGUITemplateAssetPtr,
   "@brief Inspector field type for GUI Template Asset Objects\n\n"
   "Editor use only.\n\n"
   "@internal"
);

void GuiInspectorTypeGUITemplateAssetPtr::consoleInit()
{
   Parent::consoleInit();

   ConsoleBaseType::getType(TypeGUITemplateAssetPtr)->setInspectorFieldType("GuiInspectorTypeGUITemplateAssetPtr");
}

GuiControl* GuiInspectorTypeGUITemplateAssetPtr::constructEditControl()
{
   // Create base filename edit controls
   GuiControl* retCtrl = Parent::constructEditControl();
   if (retCtrl == NULL)
      return retCtrl;

   // Change filespec
   char szBuffer[512];
   dSprintf(szBuffer, sizeof(szBuffer), "AssetBrowser.showDialog(\"GUITemplateAsset\", \"AssetBrowser.changeAsset\", %d, %s);",
      mInspector->getIdString(), mCaption);
   mBrowseButton->setField("Command", szBuffer);

   // Create "Open in ShapeEditor" button
   mSMEdButton = new GuiBitmapButtonCtrl();

   dSprintf(szBuffer, sizeof(szBuffer), "echo(\"Game Object Editor not implemented yet!\");", retCtrl->getId());
   mSMEdButton->setField("Command", szBuffer);

   char bitmapName[512] = "ToolsModule:GameTSCtrl_image";
   mSMEdButton->setBitmap(StringTable->insert(bitmapName));

   mSMEdButton->setDataField(StringTable->insert("Profile"), NULL, "GuiButtonProfile");
   mSMEdButton->setDataField(StringTable->insert("tooltipprofile"), NULL, "GuiToolTipProfile");
   mSMEdButton->setDataField(StringTable->insert("hovertime"), NULL, "1000");
   mSMEdButton->setDataField(StringTable->insert("tooltip"), NULL, "Open this file in the GUI Editor");

   mSMEdButton->registerObject();
   addObject(mSMEdButton);

   return retCtrl;
}

bool GuiInspectorTypeGUITemplateAssetPtr::updateRects()
{
   S32 dividerPos, dividerMargin;
   mInspector->getDivider(dividerPos, dividerMargin);
   Point2I fieldExtent = getExtent();
   Point2I fieldPos = getPosition();

   mCaptionRect.set(0, 0, fieldExtent.x - dividerPos - dividerMargin, fieldExtent.y);
   mEditCtrlRect.set(fieldExtent.x - dividerPos + dividerMargin, 1, dividerPos - dividerMargin - 34, fieldExtent.y);

   bool resized = mEdit->resize(mEditCtrlRect.point, mEditCtrlRect.extent);
   if (mBrowseButton != NULL)
   {
      mBrowseRect.set(fieldExtent.x - 32, 2, 14, fieldExtent.y - 4);
      resized |= mBrowseButton->resize(mBrowseRect.point, mBrowseRect.extent);
   }

   if (mSMEdButton != NULL)
   {
      RectI shapeEdRect(fieldExtent.x - 16, 2, 14, fieldExtent.y - 4);
      resized |= mSMEdButton->resize(shapeEdRect.point, shapeEdRect.extent);
   }

   return resized;
}

IMPLEMENT_CONOBJECT(GuiInspectorTypeGUITemplateAssetId);

ConsoleDocClass(GuiInspectorTypeGUITemplateAssetId,
   "@brief Inspector field type for GuiTemplates\n\n"
   "Editor use only.\n\n"
   "@internal"
);

void GuiInspectorTypeGUITemplateAssetId::consoleInit()
{
   Parent::consoleInit();

   ConsoleBaseType::getType(TypeGUITemplateAssetId)->setInspectorFieldType("GuiInspectorTypeGUITemplateAssetId");
}
#endif

