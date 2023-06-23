// GUITemplateCtrl.cpp

#include "guiTemplate.h"

IMPLEMENT_CONOBJECT(GUITemplateCtrl);

GUITemplateCtrl::GUITemplateCtrl() : mHideChildren(true)
{
   mTemplateAssetId = StringTable->EmptyString();
}

GUITemplateCtrl::~GUITemplateCtrl()
{
   // Cleanup resources, if any
}

void GUITemplateCtrl::initPersistFields()
{
   Parent::initPersistFields();

   addProtectedField("TemplateAsset", TypeGUITemplateAssetId, Offset(mTemplateAssetId, GUITemplateCtrl),
      _setTemplateData, &defaultProtectedGetFn, "A GUITemplateCtrl Asset file used for populating this template object");

   //addField("hideChildren", TypeBool, Offset(mHideChildren, GUITemplateCtrl));
}

// SimObject
bool GUITemplateCtrl::onAdd()
{
   if (!Parent::onAdd())
      return false;

   _loadTemplateData();

   if (getClassName() == StringTable->EmptyString() && mTemplateAsset)
      mClassName = mTemplateAsset->getAssetName();

   return true;
}

void GUITemplateCtrl::onRemove()
{
   Parent::onRemove();

   if (mTemplateData.size())
      for (U32 i = 0; i < mTemplateData.size(); i++)
         mTemplateData[i]->safeDeleteObject();
}

void GUITemplateCtrl::onEditorEnable()
{

}
void GUITemplateCtrl::onEditorDisable()
{

}
void GUITemplateCtrl::inspectPostApply()
{

}

void GUITemplateCtrl::onInspect(GuiInspector* inspector)
{
   Parent::onInspect(inspector);

   //iterate over children to get their special onInspect invokes when we inspect this template object
   for (U32 i = 0; i < mTemplateData.size(); i++)
   {
      if (mTemplateData[i].isValid())
      {
         mTemplateData[i]->onInspect(inspector);

         ConsoleValue args[2];
         args[0].setInt(inspector->getId());
         args[1].setInt(getId());

         mTemplateData[i]->callOnChildren("onInspect", 2, args);
      }
   }
}

void GUITemplateCtrl::_loadTemplateData(bool addFileNotify)
{
   //AssertFatal(isServerObject(), "Prefab-bad");

   if (getTemplate() == StringTable->EmptyString() || !mTemplateAsset)
   {
      Con::errorf("GUITemplateCtrl::_loadTemplateData() - template asset invalid!");
      return;
   }

   if (mTemplateData.size())
      for(U32 i=0; i < mTemplateData.size(); i++)
         mTemplateData[i]->safeDeleteObject();

   mTemplateData = mTemplateAsset->instantiateTemplate();
   if (!mTemplateData.size())
   {
      Con::errorf("GUITemplateCtrl::_loadTemplateData() - attempted to instantiate template controls, but failed!");
      return;
   }

   /*if (addFileNotify)
   {
      Torque::FS::AddChangeNotification(mFilename, this, &Prefab::_onFileChanged);
   }*/
}

void GUITemplateCtrl::onRender(Point2I offset, const RectI& updateRect)
{
   if (mTemplateData.size())
   {
      for(U32 i= 0; i < mTemplateData.size(); i++)
         mTemplateData[i]->onRender(offset, updateRect);
   }

   Parent::onRender(offset, updateRect);
}

bool GUITemplateCtrl::resize(const Point2I& newPosition, const Point2I& newExtent)
{
   if (mTemplateData.size())
   {
      for (U32 i = 0; i < mTemplateData.size(); i++)
         mTemplateData[i]->resize(newPosition, newExtent);
   }

   if (!Parent::resize(newPosition, newExtent))
      return false;

   return true;
}

GuiControl* GUITemplateCtrl::findHitControl(const Point2I& pt, S32 initialLayer)
{
   //Only do pass-through if we're not in the gui, otherwise we wanna keep the interactions
   //filtered to the template object
   GuiControl* guiEditorCtrl;
   if (Sim::findObject("GuiEditorGui", guiEditorCtrl))
   {
      if (getRoot()->getContentControl()->getId() == guiEditorCtrl->getId())
      {
         return Parent::findHitControl(pt, initialLayer);
      }
   }

   if (mTemplateData.size())
   {
      for (U32 i = 0; i < mTemplateData.size(); i++)
      {
         GuiControl* hitCtrl = mTemplateData[i]->findHitControl(pt, initialLayer);
         if (hitCtrl != nullptr)
            return hitCtrl;
      }
   }

   return Parent::findHitControl(pt, initialLayer);
}

//
DefineEngineMethod(GUITemplateCtrl, getChildObjects, const char*, (), ,
   "Instantiates the template object and returns the SimObjectId of the copy.\n"
   "@return SimObjectId of the instantiated copy.")
{
   Vector<SimObjectPtr<GuiControl>> temp = object->getTemplateData();

   char* returnBuffer = Con::getReturnBuffer(1024);
   

   String ids;
   for (U32 i = 0; i < temp.size(); i++)
   {
      ids += temp[i]->getIdString();
      if (i < temp.size() - 1)
         ids += " ";
   }

   dSprintf(returnBuffer, 1024, "%s", ids.c_str());

   return returnBuffer;
}
