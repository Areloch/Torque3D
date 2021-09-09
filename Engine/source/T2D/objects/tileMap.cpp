#include "platform/platform.h"

#include "T2D/objects/tileMap.h"
#include "math/mathIO.h"
#include "renderInstance/renderPassManager.h"
#include "scene/sceneRenderState.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDrawUtil.h"
#include "core/stream/bitStream.h"
#include "T3D/gameBase/gameConnection.h"
#include "gui/editor/inspector/group.h"
#include "console/typeValidators.h"

//------------------------------------------------------

IMPLEMENT_CONOBJECT(TileMap);

//------------------------------------------------------

S32 QSORT_CALLBACK TileMap::sortTileLayer(const void * a, const void * b)
{
   TileLayer* tileLayerA = *((TileLayer**)a);
   TileLayer* tileLayerB = *((TileLayer**)b);

   const S32 sortA   = tileLayerA->mSortLayer;
   const S32 sortB   = tileLayerB->mSortLayer;
   const S32 ageA    = tileLayerA->mLayerId;
   const S32 ageB    = tileLayerB->mLayerId;

   /// sort by internal layer sort, or age.
   return sortA < sortB ? 1 : sortA > sortB ? -1 : ageA - ageB;
}

TileMap::TileMap()
   :  mNodeCountX(10),
      mNodeCountY(10),
      mNodeSize(1.0f),
      mLayerCount(1)
{
   VECTOR_SET_ASSOCIATION(mLayers);
   mCurLayerId = 0;
}

TileMap::~TileMap()
{
}

bool TileMap::onAdd()
{
   if(!Parent::onAdd())
      return false;

   resetWorldBox();

   mTileMapHeight = mNodeCountY * mNodeSize;
   mTileMapWidth = mNodeCountX * mNodeSize;

   mpBodyDef.type = b2_staticBody;

   mObjBox = BoxVec2( Vector2(-(mTileMapWidth * 0.5), -(mTileMapHeight * 0.5)),
                      Vector2( (mTileMapWidth * 0.5),  (mTileMapHeight * 0.5)));

   addToScene();

   return true;
}

void TileMap::onRemove()
{
   removeFromScene();

   Parent::onRemove();
}

void TileMap::initPersistFields()
{
   Parent::initPersistFields();

   addField("CellCountX", TypeS32, Offset(mNodeCountX, TileMap),
      "Number of cells in X");

   addField("CellCountY", TypeS32, Offset(mNodeCountY, TileMap),
      "Number of cells in Y");

   addField("CellSize", TypeF32, Offset(mNodeSize, TileMap),
      "Size of Cell");

   addField("TileLayers", TypeS32, Offset(mLayerCount, TileMap),
      "Number of Tile Layers", AbstractClassRep::FieldFlags::FIELD_HideInInspectors);

   /// empty group to add stuff to.
   addGroup("TileLayers");
   /// layer specifics go here :) 
   endGroup("TileLayers");
}

void TileMap::onInspect(GuiInspector* inspector)
{
   /// find group.
   GuiInspectorGroup* layerGrp = inspector->findExistentGroup(StringTable->insert("TileLayers"));
   if (!layerGrp)
      return;

   GuiControl* stack = dynamic_cast<GuiControl*>(layerGrp->findObjectByInternalName(StringTable->insert("Stack")));

   GuiButtonCtrl* addLyrBttn = new GuiButtonCtrl();
   addLyrBttn->setField("text", "Add Layer");
   char szBuffer[512];
   dSprintf(szBuffer, 512, "%d.addNewLayer();", getId());
   addLyrBttn->setField("Command", szBuffer);
   addLyrBttn->setDataField(StringTable->insert("Profile"), NULL, "ToolsGuiButtonProfile");
   addLyrBttn->setDataField(StringTable->insert("tooltipprofile"), NULL, "ToolsGuiToolTipProfile");
   addLyrBttn->setDataField(StringTable->insert("hovertime"), NULL, "1000");
   if(addLyrBttn->registerObject())
      stack->addObject(addLyrBttn);

   char layerName[128];

   for (U32 i = 0; i < mLayers.size(); i++)
   {
      StringTableEntry spriteId = mLayers[i]->getSpriteAssetId();

      dSprintf(layerName, 128, "Layer%d", i);
      GuiInspectorField* fieldGui = layerGrp->constructField(TypeSpriteAssetId);
      fieldGui->init(inspector, layerGrp);
      fieldGui->setTargetObject(this);
      StringTableEntry fldNm = StringTable->insert(layerName);

      fieldGui->setInspectorField(NULL, fldNm);
      fieldGui->setDocs("Sets the spritesheet for this layer.");
      if (fieldGui->registerObject())
      {
         fieldGui->setValue(spriteId);
         stack->addObject(fieldGui);
      }
      else
      {
         SAFE_DELETE(fieldGui);
      }
   }

}

void TileMap::onDynamicModified(const char* slotName, const char* newValue)
{
   if (FindMatch::isMatch("Layer*", slotName, false))
   {
      S32 slot = -1;
      String outStr(String::GetTrailingNumber(slotName, slot));
      if (slot == -1)
         return;

      StringTableEntry sprAssetId = newValue;
      if (sprAssetId == StringTable->EmptyString())
         return;

      if (!mLayers[slot]->setSpriteAsset(sprAssetId))
      {
         Con::warnf("cannot find sprite asset for layer %d .", slot);
      }
   }

   Parent::onDynamicModified(slotName, newValue);

}

void TileMap::inspectPostApply()
{
   Parent::inspectPostApply();
   validate();

   /// no need to sort if only 1 layer exists.
   if(mLayerCount > 1)
      dQsort(mLayers.address(), mLayers.size(), sizeof(TileLayer*), sortTileLayer);
}

void TileMap::validate()
{
   /// validate inspector fields here.
   /// minimum of 1 layer
   if (mLayerCount < 1)
      mLayerCount = 1;

   /// minimum of 1 x cell
   if (mNodeCountX < 1)
      mNodeCountX = 1;

   /// minimum of 1 y cell
   if (mNodeCountY < 1)
      mNodeCountY = 1;

   /// node size must always be positive.
   if (mNodeSize < 0.0f)
      mNodeSize = 0.1f;

   ///we have no max here this isn't the 3d end ;)
}

void TileMap::addLayer(U32 num)
{
   for (U32 i = 0; i < num; i++)
   {
      TileLayer* lyr = new TileLayer(this, mCurLayerId, mNodeCountX, mNodeCountY);
      mLayers.push_back(lyr);
      mCurLayerId++;
   }
}

void TileMap::setLayerMasks(U32 id, U32 mask)
{
   setMaskBits(LayerUpdateMask);

   mLayers[id]->updateState = TileLayer::Updating;
   mLayers[id]->updateNetMaskBits |= mask;
}

void TileMap::setTileObjectFrame(U32 layerId, U32 objId, U32 frame)
{
   if (layerId > mLayers.size() || layerId < 0)
   {
      Con::warnf("SetTileFrame - Layer out of range.");
      return;
   }

   mLayers[layerId]->setTileFrame(objId, frame);

}

U32 TileMap::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   if (stream->writeFlag(mask & LayerUpdateMask))
   {
      U32 updateLayerCount = 0;
      for (U32 i = 0; i < mLayers.size(); i++)
      {
         if (mLayers[i]->updateState == TileLayer::Updating)
            updateLayerCount++;
      }
      /// soft limit to 255. Wil probably update to more later.
      stream->writeInt(updateLayerCount, 8);

      for (U32 i = 0; i < mLayers.size(); i++)
      {
         if (mLayers[i]->updateState == TileLayer::Updating)
         {
            stream->writeInt(i, 8);
            mLayers[i]->updateNetMaskBits = mLayers[i]->packUpdate(conn, mLayers[i]->updateNetMaskBits, stream);
         }
      }
   }

   return retMask;
}

void TileMap::unpackUpdate(NetConnection *conn, BitStream *stream)
{
   Parent::unpackUpdate(conn, stream);

   if (stream->readFlag()) /// layerupdatemask
   {
      /// soft limit at 255 layers. This could be expanded.
      U32 updatingLayers = stream->readInt(8);

      for (U32 i = 0; i < updatingLayers; i++)
      {
         U32 id = stream->readInt(8);
         mLayers[id]->unpackUpdate(conn, stream);
      }

   }

}

void TileMap::processTick()
{
}

void TileMap::interpolateTick(F32 dt)
{
   Parent::interpolateTick(dt);
}

void TileMap::prepRenderImage(SceneCameraState* cam)
{
}

//--------------------------------------------------------------
// TileLayer
//--------------------------------------------------------------

TileMap::TileLayer::TileLayer()
   :  oTileMap(NULL),
      mSpriteAssetId(StringTable->EmptyString()),
      mLayerId(-1),
      mNodeObjCountX(1),
      mNodeObjCountY(1),
      mSortLayer(0)
{
   VECTOR_SET_ASSOCIATION(mTileObjs);
   mCurObjId = 0;
}

TileMap::TileLayer::TileLayer(TileMap* tileMap, S32 id, S32 countX, S32 countY)
   :  oTileMap(tileMap),
      mSpriteAssetId(StringTable->EmptyString()),
      mLayerId(id),
      mNodeObjCountX(countX),
      mNodeObjCountY(countY),
      mSortLayer(0)
{
   createLayout();
}

TileMap::TileLayer::~TileLayer()
{
}

bool TileMap::TileLayer::setSpriteAsset(StringTableEntry spriteId)
{
   if (!SpriteAsset::getAssetById(spriteId, &mSpriteAsset))
   {
      Con::warnf("Error sprite asset id '%s' not found.", spriteId);
      return false;
   }

   layerTex.set(mSpriteAsset->getSpriteFileName(), &GFXStaticTextureSRGBProfile, avar("%s() - txr (line %d)", __FUNCTION__, __LINE__));

   setLayerMaskBits(SpriteUpdateMask);
   return true;
}

bool TileMap::TileLayer::validateFrame(U32 frame)
{
   if (frame >= mSpriteAsset->getFrameCount())
   {
      Con::warnf("TileObject() - Frame #%d out of range.", frame);
      return false;
   }

   return true;

}

void TileMap::TileLayer::createLayout()
{
   /// offset from position.
   Point3F pos = oTileMap->getTransform().getPosition();
   F32 offSet = oTileMap->mNodeSize * 0.5f;

   /// start top left offset by the halfsize.
   Vector2 start(pos.x - ((mNodeObjCountX * oTileMap->mNodeSize) * 0.5f) + offSet,
                 pos.y + ((mNodeObjCountY * oTileMap->mNodeSize) * 0.5f) - offSet);

   for (U32 y = 0; y < mNodeObjCountY; y++)
   {
      /// this should be 0 on the first one.
      F32 yPos = y * oTileMap->mNodeSize;

      for (U32 x = 0; x < mNodeObjCountX; x++)
      {
         /// same as y this should be zero.
         F32 xPos = x * oTileMap->mNodeSize;
         /// place the tile object in the correct location.
         TileObject* obj = new TileObject(this,mCurObjId,Vector2(start.x + xPos, start.y + yPos));
         mTileObjs.push_back(obj);
         mCurObjId++;

      }
   }
}

void TileMap::TileLayer::setLayerMaskBits(U32 mask)
{
   AssertFatal(mask != 0, "Invalid Mask.");

   oTileMap->setLayerMasks(mLayerId, mask);
}

void TileMap::TileLayer::setObjectMask(S32 id,U32 mask)
{
   mTileObjs[id]->updateState = TileObject::Updating;
   mTileObjs[id]->updateNetMaskBits |= mask;
   setLayerMaskBits(LayerObjectMask);
}

void TileMap::TileLayer::setTileFrame(U32 id, U32 frame)
{
   if (id > mTileObjs.size() || id < 0)
   {
      Con::warnf("SetTileFrame - Tile does not exist.");
      return;
   }

   mTileObjs[id]->setFrame(frame);
}

U32 TileMap::TileLayer::packUpdate(NetConnection* conn, U32 mask, BitStream* stream)
{
   U32 retMask = 0;

   if (stream->writeFlag(mask & SpriteUpdateMask))
   {
      stream->writeString(mSpriteAsset.getAssetId());
   }

   if(stream->writeFlag(mask & LayerObjectMask)) /// layer object mask
   {
      U32 updateObjCount = 0;
      for (U32 i = 0; i < mTileObjs.size(); i++)
      {
         if (mTileObjs[i]->updateState == TileObject::Updating)
            updateObjCount++;
      }

      stream->writeInt(updateObjCount, 16);

      for (U32 i = 0; i < mTileObjs.size(); i++)
      {
         if (mTileObjs[i]->updateState == TileObject::Updating)
         {
            stream->writeInt(i, 16);
            mTileObjs[i]->updateNetMaskBits = mTileObjs[i]->packUpdate(conn, mTileObjs[i]->updateNetMaskBits, stream);
         }
      }
   }

   return retMask;
}

void TileMap::TileLayer::unpackUpdate(NetConnection* conn, BitStream* stream)
{
   if (stream->readFlag()) /// sprite update mask
   {
      char buffer[256];
      stream->readString(buffer);
      mSpriteAssetId = StringTable->insert(buffer);
      if (mSpriteAssetId != StringTable->EmptyString())
         setSpriteAsset(mSpriteAssetId);
   }

   if (stream->readFlag()) /// layer object mask
   {
      U32 updatingObjects = stream->readInt(16);

      for (U32 i = 0; i < updatingObjects; i++)
      {
         U32 id = stream->readInt(16);
         mTileObjs[id]->unpackUpdate(conn, stream);
      }

   }

}

//--------------------------------------------------------------
// TileObject
//--------------------------------------------------------------

TileMap::TileLayer::TileObject::TileObject()
   :  oTileLayer(NULL),
      mObjId(-1),
      mPos(0.0f,0.0f),
      mFrame(-1),
      mActive(false),
      mPhysics(false)
{
}

TileMap::TileLayer::TileObject::TileObject(TileLayer* lyr, S32 id, Vector2 pos)
   :  oTileLayer(lyr),
      mObjId(id),
      mPos(pos),
      mFrame(-1),
      mActive(false),
      mPhysics(false)
{
   /// keep it deactivated until a frame is applied.
}

TileMap::TileLayer::TileObject::~TileObject()
{
}

void TileMap::TileLayer::TileObject::setFrame(U32 frame)
{
   /// we want to display something check if we can activate
   /// or deactivate if set to -1
   if (frame > -1)
   {
      /// validate the frame before activating.
      if (oTileLayer->validateFrame(frame))
      {
         mFrame = frame;
         mActive = true;
         setObjectMaskBits(ObjectUpdateMask);
      }
      else
         mActive = false;
   }
   else
   {
      mActive = false;
   }
}

void TileMap::TileLayer::TileObject::setObjectMaskBits(U32 mask)
{
   oTileLayer->setObjectMask(mObjId, mask);
}

U32 TileMap::TileLayer::TileObject::packUpdate(NetConnection * conn, U32 mask, BitStream * stream)
{
   U32 retmask = 0;

   if (stream->writeFlag(mask & ObjectUpdateMask))
   {
      stream->write(mFrame);

      stream->writeFlag(mPhysics);
   }

   return retmask;
}

void TileMap::TileLayer::TileObject::unpackUpdate(NetConnection * conn, BitStream * stream)
{

   if (stream->readFlag())
   {
      stream->read(&mFrame);

      mPhysics = stream->readFlag();
   }

}

