#ifndef _TILEMAP_H_
#define _TILEMAP_H_

#ifndef _SPRITEOBJECT2D_H_
#include "T2D/Scene/SceneObject2D.h"
#endif // !_SPRITEOBJECT2D_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#ifndef _SPRITE_ASSET_H_
#include "T2D/assets/SpriteAsset.h"
#endif // !_SPRITE_ASSET_H_

class TileLayer;

class TileMap : public SceneObject2D
{
   typedef SceneObject2D Parent;

private:

   S32 mNodeCountX;
   S32 mNodeCountY;
   /// Tiles should be uniform size, don't like it? you tile wrong.
   F32 mNodeSize;
   S32 mLayerCount;

   static S32 QSORT_CALLBACK sortTileLayer(const void* a, const void* b);
   GFXStateBlockRef  nsb;

public:

   typedef Vector<TileLayer*> typeTileLayers;
   typeTileLayers mTileLayerList;

   TileMap();
   virtual ~TileMap();
   /// sim
   virtual bool onAdd();
   virtual void onRemove();

   static void initPersistFields();
   virtual void inspectPostApply();
   void validate();

   /// NetObject
   U32 packUpdate(NetConnection* conn, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection* conn, BitStream* stream);

   virtual void processTick();
   virtual void interpolateTick(F32 dt);

   /// rendering
   void prepRenderImage(SceneCameraState* cam);

   DECLARE_CONOBJECT(TileMap);

};

class TileLayer : public SceneObject2D
{
   typedef SceneObject2D Parent;
private:
   TileMap* mTileMap;
   S32      mId;
   S32      mSortLayer;

protected:
   AssetPtr<SpriteAsset> mSpriteAsset;
   StringTableEntry mSpriteAssetId;
   bool setSpriteAsset(const StringTableEntry spriteAssetId);

public:
   GFXTexHandle txr;

   TileLayer();
   TileLayer(TileMap* map, U32 id);
   virtual ~TileLayer();

   virtual bool onAdd();
   virtual void onRemove();

   static void initPersistFields();
   virtual void inspectPostApply();

   /// NetObject
   /// we will let you pack your own updates for now.
   U32 packUpdate(NetConnection* conn, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection* conn, BitStream* stream);

   void createLayout();
   void sortLayer();
   /// don't let this object update itself.
   void renderLayer();

   /// protected setters.
   static bool _setSpriteAsset(void *obj, const char* index, const char* data);

   /// setters
   void setSortLayer(U32 sort) { mSortLayer = sort; }

   /// getters
   S32 getId() { return mId; }
   S32 getSortLayer() { return mSortLayer; }

   DECLARE_CONOBJECT(TileLayer);
};

#endif // !_TILEMAP_H_
