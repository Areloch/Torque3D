#include "platform/platform.h"
#include "T2D/objects/tileMap.h"
#include "T2D/Scene/SceneObject2D.h"
#include "math/mathIO.h"
#include "renderInstance/renderPassManager.h"
#include "scene/sceneRenderState.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDrawUtil.h"
#include "core/stream/bitStream.h"
#include "T3D/gameBase/gameConnection.h"
#include "gui/editor/inspector/group.h"
#include "console/typeValidators.h"

//---------------------------------------------
// TileMap
//---------------------------------------------

IMPLEMENT_CO_NETOBJECT_V1(TileMap);

//---------------------------------------------

TileMap::TileMap()
   :  mNodeCountX(10),
      mNodeCountY(10),
      mNodeSize(1.0f),
      mLayerCount(0)
{

   mNetFlags.set(Ghostable | ScopeAlways);

   VECTOR_SET_ASSOCIATION(mTileLayerList);

   mTileLayerId = 0;

}

TileMap::~TileMap()
{

}

bool TileMap::onAdd()
{
   if (!Parent::onAdd())
      return false;

   F32 width   = (mNodeCountX * mNodeSize) * 0.5f;
   F32 height  = (mNodeCountY * mNodeSize) * 0.5f;

   mObjBox = BoxVec2(Vector2(-width, -height), Vector2(width, height));

   addToScene();

   return true;
}

void TileMap::onRemove()
{
   removeFromScene();
   Parent::onRemove();
}

void TileMap::createLayer()
{
   TileLayer* layer = new TileLayer(mTileLayerId);

   layer->createTiles(mNodeCountX, mNodeCountY, mNodeSize);

   mTileLayerList.push_back(layer);
   mTileLayerId++;

}

void TileMap::createSpecifiedLayer(U32 slotNum, StringTableEntry spriteAssetId)
{
   TileLayer* layer = new TileLayer(slotNum);
   layer->setSpriteAsset(spriteAssetId);

   layer->createTiles(mNodeCountX, mNodeCountY, mNodeSize);

   mTileLayerList.push_back(layer);
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
   if (addLyrBttn->registerObject())
      stack->addObject(addLyrBttn);

   char layerName[128];

   for (U32 i = 0; i < mLayerCount; i++)
   {
      StringTableEntry spriteId = mTileLayerList[i]->getSpriteAssetId();

      dSprintf(layerName, 128, "Layer%d", mTileLayerList[i]->mId);
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


      char tileName[256];
      for (U32 j = 0; j < mTileLayerList[i]->mTileObjectList.size(); j++)
      {
         S32 frame = mTileLayerList[i]->mTileObjectList[j]->getFrame();
         char frameChar[256];
         dSprintf(frameChar, 256, "%d", frame);

         dSprintf(tileName, 256, "Tile_%d_%d", i, j);
         GuiInspectorField* fGui = layerGrp->constructField(TypeS32);
         fGui->init(inspector, layerGrp);
         fGui->setTargetObject(this);
         StringTableEntry fld = StringTable->insert(tileName);

         fGui->setInspectorField(NULL, fld);
         fGui->setDocs("Set the frame for this tile.");
         if (fGui->registerObject())
         {
            fGui->setValue(frameChar);
            stack->addObject(fGui);
         }
         else
         {
            SAFE_DELETE(fGui);
         }

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

      StringTableEntry sprAssetId = StringTable->insert(newValue);
      if (sprAssetId == StringTable->EmptyString())
         return;

      bool found = false;
      for (U32 i = 0; i < mTileLayerList.size(); i++)
      {
         if (mTileLayerList[i]->mId == slot)
         {
            if (!mTileLayerList[i]->setSpriteAsset(sprAssetId))
            {
               Con::warnf("cannot find sprite asset for layer %d .", slot);
            }

            found = true;
         }
      }
      if (!found)
      {
         createSpecifiedLayer(slot, sprAssetId);
      }

   }

   if (FindMatch::isMatch("Tilel*t*", slotName, false))
   {
      S32 layer = -1;
      S32 tile = -1;
      U32 start;
      U32 end;

      String startNum = String::GetFirstNumber(slotName, start, end);
      layer = dAtoi(startNum);
      if (layer == -1)
         return;

      String tileNum = String::GetTrailingNumber(slotName, tile);
      if (tile == -1)
         return;

      S32 newFrame = dAtoi(newValue);

      setTileFrame(layer, tile, newFrame);

   }

   Parent::onDynamicModified(slotName, newValue);

}

U32 TileMap::packUpdate(NetConnection * conn, U32 mask, BitStream * stream)
{
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   stream->write(mNodeCountX);
   stream->write(mNodeCountY);
   stream->write(mNodeSize);
   stream->write(mLayerCount);

   for (U32 i = 0; i < mLayerCount; i++)
   {
      stream->write(mTileLayerList[i]->mId);
      stream->writeString(mTileLayerList[i]->mSpriteAssetId);

      for (U32 j = 0; j < mTileLayerList[i]->mTileObjectList.size(); j++)
      {
         S32 id = mTileLayerList[i]->mTileObjectList[j]->mId;
         S32 frame = mTileLayerList[i]->mTileObjectList[j]->getFrame();
         stream->write(id);
         stream->write(frame);
      }

   }

   return retMask;
}

void TileMap::unpackUpdate(NetConnection * conn, BitStream * stream)
{
   Parent::unpackUpdate(conn, stream);

   stream->read(&mNodeCountX);
   stream->read(&mNodeCountY);
   stream->read(&mNodeSize);
   stream->read(&mLayerCount);

   for (U32 i = 0; i < mLayerCount; i++)
   {
      U32 id;
      stream->read(&id);
      char buffer[256];
      stream->readString(buffer);
      createSpecifiedLayer(id, buffer);

      for (U32 j = 0; j < mTileLayerList[i]->mTileObjectList.size(); j++)
      {
         S32 id;
         id = stream->read(&id);
         S32 frame;
         frame = stream->read(&frame);

         U32 size = mTileLayerList[i]->mTileObjectList.size();
         mTileLayerList[i]->mTileObjectList[id]->setFrame(frame);

      }

   }

}

void TileMap::setTileFrame(S32 layerId, S32 tileId, S32 frame)
{
   if (!mTileLayerList[layerId]->validateFrame(tileId, frame))
   {
      Con::warnf("TileMap - layer %d cannot set tile %d frame %d", layerId, tileId, frame);
   }
}

TileMap::TileLayer::~TileLayer()
{
}

bool TileMap::TileLayer::setSpriteAsset(StringTableEntry spriteAssetId)
{
   if (!SpriteAsset::getAssetById(spriteAssetId, &mSpriteAsset))
   {
      Con::warnf("Error sprite asset id '%s' not found.", spriteAssetId);
      return false;
   }

   mSpriteAssetId = spriteAssetId;

   txr.set(mSpriteAsset->getSpriteFileName(), &GFXStaticTextureSRGBProfile, avar("%s() - txr (line %d)", __FUNCTION__, __LINE__));

   return true;
}

void TileMap::TileLayer::createTiles(S32 tileX, S32 tileY, F32 tileSize)
{
   F32 startX = -((tileSize * tileX) * 0.5f) + (tileSize * 0.5f);
   F32 startY = ((tileSize * tileY)  * 0.5f) - (tileSize * 0.5f);

   for (U32 i = 0; i < tileY; i++)
   {
      F32 offsetY = tileSize * i;
      for (U32 j = 0; j < tileX; j++)
      {
         F32 offsetX = tileSize * j;

         TileObject* tile = new TileObject(mObjId, Point2F(startX + offsetX, startY - offsetY));
         mTileObjectList.push_back(tile);
         mObjId++;
      }
   }

   nodeSize = tileSize;

}

bool TileMap::TileLayer::validateFrame(S32 id, S32 frame)
{
   if (mSpriteAsset == NULL)
   {
      Con::warnf("TileLayer does not have an asset.");
      return false;
   }

   if (id > mTileObjectList.size())
   {
      Con::warnf("TileLayer - TileObject id out of range.");
      return false;
   }

   if (frame == -1)
   {
      mTileObjectList[id]->setFrame(frame);
      return true;
   }

   if (frame >= mSpriteAsset->getFrameCount())
   {
      Con::warnf("Frame for tileObject out of range.");
      return false;
   }

   mTileObjectList[id]->setFrame(frame);
   return true;
}

void TileMap::TileLayer::activeCount()
{
   /// loop through to find active tiles.
   mActiveList.clear();

   for (U32 i = 0; i < mTileObjectList.size(); i++)
   {
      S32 frame = mTileObjectList[i]->getFrame();
      if (frame > -1)
         mActiveList.push_back(mTileObjectList[i]);
   }

}

//-------------------------------------------------
// Render Code
//-------------------------------------------------

void TileMap::TileLayer::renderLayer()
{
   SpriteAsset::FrameArea::TexelArea texelArea;
   activeCount();

   GFX->setTexture(0, txr);

   for (U32 i = 0; i < mActiveList.size(); i++)
   {
      GFXVertexBufferHandle<GFXVertexPCT> verts(GFX, 4, GFXBufferTypeVolatile);
      verts.lock();
      /// setup our frame.
      texelArea = mSpriteAsset->getSpriteFrameArea(mActiveList[i]->getFrame()).mTexelArea;
      const F32 texLowerX = texelArea.mTexelLower.x;
      const F32 texLowerY = texelArea.mTexelLower.y;
      const F32 texUpperX = texelArea.mTexelUpper.x;
      const F32 texUpperY = texelArea.mTexelUpper.y;

      Point2F pos(mActiveList[i]->getPos().x, mActiveList[i]->getPos().y);
      F32 halfNode = nodeSize * 0.5f;

      verts[0].point.set(pos.x - halfNode, pos.y + halfNode, 0.0f);
      verts[1].point.set(pos.x + halfNode, pos.y + halfNode, 0.0f);
      verts[2].point.set(pos.x - halfNode, pos.y - halfNode, 0.0f);
      verts[3].point.set(pos.x + halfNode, pos.y - halfNode, 0.0f);

      /// flip the order because opengl reasons.
      verts[0].texCoord.set(texLowerX, texLowerY);
      verts[1].texCoord.set(texUpperX, texLowerY);
      verts[2].texCoord.set(texLowerX, texUpperY);
      verts[3].texCoord.set(texUpperX, texUpperY);

      verts.unlock();
      GFX->setVertexBuffer(verts);
      GFX->setupGenericShaders(GFXDevice::GSTexture);
      GFX->drawPrimitive(GFXTriangleStrip, 0, 2);
   }

}

void TileMap::prepRenderImage(SceneCameraState * cam)
{
   if (!nsb)
   {
      GFXStateBlockDesc desc;
      desc.setCullMode(GFXCullNone);
      desc.setZReadWrite(true, false);
      desc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
      desc.setColorWrites(true, true, true, false);
      desc.samplersDefined = true;
      desc.samplers[0] = GFXSamplerStateDesc::getClampPoint();

      nsb = GFX->createStateBlock(desc);
   }

   MatrixF mat = getRenderTransform();
   Point3F scale(mObjScale.x, mObjScale.y, 0.0f);
   mat.scale(scale);

   GFX->pushWorldMatrix();
   GFX->multWorld(mat);
   GFX->setStateBlock(nsb);

   for (U32 i = 0; i < mTileLayerList.size(); i++)
   {
      mTileLayerList[i]->renderLayer();
   }

   GFX->popWorldMatrix();

}

