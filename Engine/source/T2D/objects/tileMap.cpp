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

//---------------------------------------------
// TileLayer
//---------------------------------------------

IMPLEMENT_CONOBJECT(TileLayer);

//---------------------------------------------

TileLayer::TileLayer()
   :  mTileMap(NULL),
      mSortLayer(0),
      mId(-1)
{
}

TileLayer::TileLayer(TileMap* map, U32 id)
   :  mTileMap(map),
      mSortLayer(0),
      mId(id)
{
}

TileLayer::~TileLayer()
{
}

bool TileLayer::onAdd()
{
   if (!Parent::onAdd())
      return false;

   mObjBox = mTileMap->mObjBox;

   mpBodyDef.type = b2_staticBody;

   addToScene();

   return true;
}

void TileLayer::onRemove()
{

   Parent::onRemove();
}

void TileLayer::initPersistFields()
{
   addProtectedField("spriteAsset", TypeSpriteAssetId, Offset(mSpriteAssetId, TileLayer), &_setSpriteAsset, &defaultProtectedGetFn,
      "Add a sprite asset.");

   addField("SortLayer", TypeS32, Offset(mSortLayer, TileLayer),
      "Set sorting for this layer (if 0 age of layer is used).");

}

void TileLayer::inspectPostApply()
{
   Parent::inspectPostApply();
}

U32 TileLayer::packUpdate(NetConnection * conn, U32 mask, BitStream * stream)
{
   U32 retMask = Parent::packUpdate(conn, mask, stream);



   return retMask;
}

void TileLayer::unpackUpdate(NetConnection * conn, BitStream * stream)
{
   Parent::unpackUpdate(conn, stream);



}

bool TileLayer::_setSpriteAsset(void * obj, const char * index, const char * data)
{
   TileLayer* so = static_cast<TileLayer*>(obj);

   so->mSpriteAssetId = StringTable->insert(data);

   return so->setSpriteAsset(so->mSpriteAssetId);
}

bool TileLayer::setSpriteAsset(const StringTableEntry spriteAssetId)
{
   if (!SpriteAsset::getAssetById(spriteAssetId, &mSpriteAsset))
   {
      Con::warnf("Error sprite asset id '%s' not found.", spriteAssetId);
      return false;
   }

   txr.set(mSpriteAsset->getSpriteFileName(), &GFXStaticTextureSRGBProfile, avar("%s() - txr (line %d)", __FUNCTION__, __LINE__));

   return true;

}

