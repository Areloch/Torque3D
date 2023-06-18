#pragma once

// GUITemplateCtrl.h

#ifndef _GUI_TEMPLATE_H_
#define _GUI_TEMPLATE_H_

#include "gui/core/guiControl.h"
#include "T3D/assets/GUITemplateAsset.h"

class GUITemplateCtrl : public GuiControl
{
   typedef GuiControl Parent;

private:
   bool  mHideChildren;

   StringTableEntry mTemplateAssetId;
   AssetPtr<GUITemplateAsset> mTemplateAsset;

   SimObjectPtr<GuiControl> mTemplateData;

public:
   GUITemplateCtrl();
   virtual ~GUITemplateCtrl();

   DECLARE_CONOBJECT(GUITemplateCtrl);

   static void initPersistFields();

   // SimObject
   virtual bool onAdd();
   virtual void onRemove();
   virtual void onEditorEnable();
   virtual void onEditorDisable();
   virtual void inspectPostApply();

   void onRender(Point2I offset, const RectI& updateRect);

   bool resize(const Point2I& newPosition, const Point2I& newExtent) override;

   GuiControl* findHitControl(const Point2I& pt, S32 initialLayer);

   SimObjectPtr<GuiControl> getTemplateData() { return mTemplateData; }

protected:
   //DECLARE_CALLBACK(void, onLoad, (SimGroup* children));
   void _onFileChanged(const Torque::Path& path);

   const AssetPtr<GUITemplateAsset>& getTemplateAsset() const { return mTemplateAsset; }
   void setTemplateAsset(const AssetPtr<GUITemplateAsset>& _in) { mTemplateAsset = _in; }

   bool _setTemplate(StringTableEntry _in)
   {
      if (mTemplateAssetId != _in)
      {
         if (_in == NULL || _in == StringTable->EmptyString())
         {
            mTemplateAssetId = StringTable->EmptyString(); 
            mTemplateAsset = NULL; 
            return true; 
         }
            
         if (AssetDatabase.isDeclaredAsset(_in))
         {
            mTemplateAssetId = _in;
            mTemplateAsset = mTemplateAssetId;;
         }
         else
         {
            mTemplateAssetId = StringTable->EmptyString(); 
            mTemplateAsset = NULL; 
         }
      }
      if (getTemplate() != StringTable->EmptyString() && mTemplateAsset.notNull())
      {
         _loadTemplateData();
      }

      if (getTemplate() == StringTable->EmptyString())
         return true; 
               
      if (mTemplateAsset.notNull() && mTemplateAsset->getStatus() != SoundAsset::Ok)
      {
         Con::errorf("%s(%s)::_set%s() - gui template asset failure\"%s\" due to [%s]", macroText(className), getName(), macroText(name), _in, GUITemplateAsset::getAssetErrstrn(mTemplateAsset->getStatus()).c_str());
         return false; 
      }
      return true; 
   }
      
   const StringTableEntry getTemplate() const
   {
      if (mTemplateAssetId != StringTable->EmptyString())
         return mTemplateAssetId; 
      else
         return StringTable->EmptyString(); 
   }

   static bool _setTemplateData(void* obj, const char* index, const char* data)
   {
      GUITemplateCtrl* object = static_cast<GUITemplateCtrl*>(obj); 
      return object->_setTemplate(StringTable->insert(data)); 
   }

   void _loadTemplateData(bool addFileNotify = false);

   void onInspect(GuiInspector* inspector);

public:
   //virtual void getUtilizedAssets(Vector<StringTableEntry>* usedAssetsList);
};

#endif // _GUI_TEMPLATE_H_
