#include "SubScene.h"

#include "gameMode.h"
#include "console/script.h"
#include "scene/sceneRenderState.h"
#include "renderInstance/renderPassManager.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxTransformSaver.h"
#include "gui/editor/inspector/group.h"

IMPLEMENT_CO_NETOBJECT_V1(SubScene);

SubScene::SubScene() :
   mLevelAssetId(StringTable->EmptyString()),
   mGameModesNames(StringTable->EmptyString()),
   mScopeDistance(-1),
   mUnloadTimeoutMs(5000),
   mLoaded(false)
{
   mNetFlags.set(Ghostable | ScopeAlways);

   mTypeMask |= TriggerObjectType;
}

SubScene::~SubScene()
{
}

bool SubScene::onAdd()
{
   if (!Parent::onAdd())
     return false;

    return true;
}

void SubScene::onRemove()
{
    if (isClientObject())
      removeFromScene();

    Parent::onRemove();
}

void SubScene::initPersistFields()
{
   addGroup("SubScene");
   INITPERSISTFIELD_LEVELASSET(Level, SubScene, "The level asset to load.");
   addField("gameModes", TypeGameModeList, Offset(mGameModesNames, SubScene), "The game modes that this subscene is associated with.");
   //addField("ScopeDistance", TypeF32, Offset(mScopeDistance, SubScene), "Distance that clients within have the objects inside the SceneGroup scoped to them. Set to -1 for no scoping behavior");
   endGroup("SubScene");

   Parent::initPersistFields();
}

void SubScene::addObject(SimObject* object)
{
   SceneObject::addObject(object);
}

void SubScene::removeObject(SimObject* object)
{
   SceneObject::removeObject(object);
}

void SubScene::onInspect(GuiInspector* inspector)
{
   //Parent::onInspect();
   
   /*if (isClientObject())
   {
           // Load the level asset.
      if (mLevelAssetId != StringTable->insert(""))
      {
         mLevelAsset = AssetDatabase::loadAsset<LevelAsset>(mLevelAssetId);
         if (mLevelAsset)
         {
            // Add the level asset to the scene.
            mLevelAsset->addToScene();
         }
      }
   }*/

   //Put the GameObject group before everything that'd be gameobject-effecting, for orginazational purposes
   GuiInspectorGroup* subsceneGrp = inspector->findExistentGroup(StringTable->insert("SubScene"));
   if (!subsceneGrp)
      return;

   GuiControl* stack = dynamic_cast<GuiControl*>(subsceneGrp->findObjectByInternalName(StringTable->insert("Stack")));

   GuiInspectorField* fieldGui = subsceneGrp->createInspectorField();
   fieldGui->init(inspector, subsceneGrp);

   fieldGui->setSpecialEditField(true);
   fieldGui->setTargetObject(this);

   StringTableEntry fldnm = StringTable->insert("SaveSubScene");

   fieldGui->setSpecialEditVariableName(fldnm);

   fieldGui->setInspectorField(NULL, fldnm);
   fieldGui->setDocs("");

   stack->addObject(fieldGui);

   GuiButtonCtrl* saveButton = new GuiButtonCtrl();
   saveButton->registerObject();
   saveButton->setDataField(StringTable->insert("profile"), NULL, "ToolsGuiButtonProfile");
   saveButton->setText("Save SubScene");
   saveButton->resize(Point2I::Zero, fieldGui->getExtent());
   saveButton->setHorizSizing(GuiControl::horizResizeWidth);
   saveButton->setVertSizing(GuiControl::vertResizeHeight);

   char szBuffer[512];
   dSprintf(szBuffer, 512, "%d.save();", this->getId());
   saveButton->setConsoleCommand(szBuffer);

   fieldGui->addObject(saveButton);
}

bool SubScene::testBox(const Box3F& testBox)
{
   return getWorldBox().isOverlapped(testBox);
   //F32 distance = (getPosition() - testBox.getCenter()).len();

   //return (distance <= mScopeDistance);
}

void SubScene::write(Stream& stream, U32 tabStop, U32 flags)
{
   MutexHandle handle;
   handle.lock(mMutex);

   // export selected only?
   if ((flags & SelectedOnly) && !isSelected())
   {
      for (U32 i = 0; i < size(); i++)
         (*this)[i]->write(stream, tabStop, flags);

      return;

   }

   stream.writeTabs(tabStop);
   char buffer[2048];
   const U32 bufferWriteLen = dSprintf(buffer, sizeof(buffer), "new %s(%s) {\r\n", getClassName(), getName() && !(flags & NoName) ? getName() : "");
   stream.write(bufferWriteLen, buffer);
   writeFields(stream, tabStop + 1);

   //The only meaningful difference between this and simSet for writing is we skip the children, since they're just the levelAsset contents

   stream.writeTabs(tabStop);
   stream.write(4, "};\r\n");
}

void SubScene::processTick(const Move* move)
{
   //if (mIsEqual(mScopeDistance, -1))
   //   return;

   //Only the server controls the scoping behavior
   //if (!isServerObject())
   //   return;

   //nothing for us to check for?
   //if (!mLoaded || mStartUnloadTimerMS == -1)
   //   return;

   //if (Sim::getCurrentTime() - mStartUnloadTimerMS > mUnloadTimeoutMs)
   //   _closeFile(true);
}

void SubScene::_onFileChanged(const Torque::Path& path)
{
   if(mLevelAsset.isNull() || Torque::Path(mLevelAsset->getLevelPath()) != path)
      return;

   AssertFatal(path == mLevelAsset->getLevelPath(), "Prefab::_onFileChanged - path does not match filename.");

   _closeFile(false);
   _loadFile(false);
   setMaskBits(U32_MAX);
}

void SubScene::_closeFile(bool removeFileNotify)
{
   AssertFatal(isServerObject(), "Trying to close out a subscene file on the client is bad!");

   U32 count = size();

   for (SimSetIterator itr(this); *itr; ++itr)
   {
      SimObject* child = dynamic_cast<SimObject*>(*itr);

      if (child)
         child->safeDeleteObject();
   }

   if (removeFileNotify && mLevelAsset.notNull() && mLevelAsset->getLevelPath() != StringTable->EmptyString())
   {
      Torque::FS::RemoveChangeNotification(mLevelAsset->getLevelPath(), this, &SubScene::_onFileChanged);
   }

   mGameModesList.clear();

   // Back to a default bounding box size.
   mObjBox.set(Point3F(-0.5f, -0.5f, -0.5f), Point3F(0.5f, 0.5f, 0.5f));
   resetWorldBox();
}

void SubScene::_loadFile(bool addFileNotify)
{
   AssertFatal(isServerObject(), "Trying to load a SubScene file on the client is bad!");

   if(mLevelAsset.isNull() || mLevelAsset->getLevelPath() == StringTable->EmptyString())
      return;

   String evalCmd = String::ToString("exec(\"%s\");", mLevelAsset->getLevelPath());

   String instantGroup = Con::getVariable("InstantGroup");
   Con::setIntVariable("InstantGroup", this->getId());
   Con::evaluate((const char*)evalCmd.c_str(), false, mLevelAsset->getLevelPath());
   Con::setVariable("InstantGroup", instantGroup.c_str());

   if (addFileNotify)
      Torque::FS::AddChangeNotification(mLevelAsset->getLevelPath(), this, &SubScene::_onFileChanged);

   

   /*if (!foundObjects.empty())
   {
      mWorldBox = Box3F::Invalid;

      for (S32 i = 0; i < foundObjects.size(); i++)
      {
         SceneObject* child = foundObjects[i];
         mChildMap.insert(child->getId(), Transform(child->getTransform(), child->getScale()));
         smChildToPrefabMap.insert(child->getId(), getId());

         _updateChildTransform(child);

         mWorldBox.intersect(child->getWorldBox());
      }

      resetObjectBox();
   }*/

   //sPrefabFileStack.pop_back();

   //onLoad_callback(mChildGroup);
}

void SubScene::load()
{
   mStartUnloadTimerMS = -1; //reset unload timers

   //no need to load multiple times
   if (mLoaded)
      return;

   Con::printf("SUBSCENE::LOAD() - Loading file!");

   _loadFile(true);
   mLoaded = true;

   GameMode::findGameModes(mGameModesNames, &mGameModesList);

   for (U32 i = 0; i < mGameModesList.size(); i++)
   {
      mGameModesList[i]->onSubsceneLoaded_callback();
   }
}

void SubScene::unload()
{
   if (!mLoaded)
      return;

   Con::printf("SUBSCENE::UNLOAD() - Unloading file!");

   //mStartUnloadTimerMS = Sim::getCurrentTime();
   _closeFile(true);
   mLoaded = false;

   for (U32 i = 0; i < mGameModesList.size(); i++)
   {
      mGameModesList[i]->onSubsceneUnloaded_callback();
   }
}

bool SubScene::save()
{
   //if there's nothing TO save, don't bother
   if (size() == 0)
      return false;

   if (mLevelAsset.isNull())
      return false;

   StringTableEntry levelPath = mLevelAsset->getLevelPath();

   for (SimGroup::iterator itr = begin(); itr != end(); itr++)
   {
      //Just in case there's a valid callback the scene object would like to invoke for saving
      SceneObject* gc = dynamic_cast<SceneObject*>(*itr);
      if (gc)
      {
         gc->onSaving_callback(mLevelAssetId);
      }

      SimObject* sO = static_cast<SimObject*>(*itr);

      if (!sO->save(levelPath))
      {
         Con::errorf("SubScene::save() - error, failed to write object %s to file: %s", sO->getIdString(), levelPath);
         return false;
      }
   }

   //process our gameModeList and write it out to the levelAsset for metadata stashing
   bool saveSuccess = false;

   //Get the level asset
   if (mLevelAsset.isNull())
      return saveSuccess;

   //update the gamemode list as well
   mLevelAsset->setDataField(StringTable->insert("gameModesNames"), NULL, StringTable->insert(mGameModesNames));

   //Finally, save
   saveSuccess = mLevelAsset->saveAsset();

}

void SubScene::prepRenderImage(SceneRenderState* state)
{
   // only render if selected or render flag is set
   if (/*!smRenderTriggers && */!isSelected())
      return;

   ObjectRenderInst* ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind(this, &SubScene::renderObject);
   ri->type = RenderPassManager::RIT_Editor;
   ri->translucentSort = true;
   ri->defaultKey = 1;
   state->getRenderPass()->addInst(ri);
}

void SubScene::renderObject(ObjectRenderInst* ri,
   SceneRenderState* state,
   BaseMatInstance* overrideMat)
{
   if (overrideMat)
      return;

   GFXStateBlockDesc desc;
   desc.setZReadWrite(true, false);
   desc.setBlend(true);

   // Trigger polyhedrons are set up with outward facing normals and CCW ordering
   // so can't enable backface culling.
   desc.setCullMode(GFXCullNone);

   GFXTransformSaver saver;

   MatrixF mat = getRenderTransform();
   mat.scale(getScale());

   GFX->multWorld(mat);

   GFXDrawUtil* drawer = GFX->getDrawUtil();

   drawer->drawCube(desc, getWorldBox(), ColorI(255, 192, 0, 45));

   // Render wireframe.

   desc.setFillModeWireframe();
   drawer->drawCube(desc, getWorldBox(), ColorI::BLACK);
}

DefineEngineMethod(SubScene, save, bool, (),,
   "Save out the subScene.\n")
{
   return object->save();
}


DefineEngineMethod(SubScene, load, void, (), ,
   "Loads the SubScene's level file.\n")
{
   object->load();
}

DefineEngineMethod(SubScene, unload, void, (), ,
   "Unloads the SubScene's level file.\n")
{
   object->unload();
}
