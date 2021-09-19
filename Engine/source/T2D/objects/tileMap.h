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

   struct TileObject
   {
      S32 mId;
      S32 mFrame;
      Point2F mPos;
      TileObject()
         :  mId(-1),
            mFrame(-1)
      {}

      TileObject(S32 id, Point2F pos)
         :  mId(id),
            mFrame(-1),
            mPos(pos)
      {}
      ~TileObject() {}

      void setFrame(S32 frame) { mFrame = frame; }
      S32 getFrame() { return mFrame; }
      Point2F getPos() { return mPos; }
   };

   typedef Vector<TileObject*> typeTileObject;

   struct TileLayer
   {
      S32 mId;
      AssetPtr<SpriteAsset> mSpriteAsset;
      StringTableEntry mSpriteAssetId;
      S32 mObjId;
      typeTileObject mTileObjectList;
      typeTileObject mActiveList;
      GFXTexHandle txr;
      F32 nodeSize;

      TileLayer()
         : mId(-1)
      {
         VECTOR_SET_ASSOCIATION(mTileObjectList);
         mObjId = 0;
         nodeSize = 1.0f;
      }
      TileLayer(S32 id)
         : mId(id)
      {
         mObjId = 0;
      }

      ~TileLayer();

      bool setSpriteAsset(StringTableEntry spriteAssetId);
      StringTableEntry getSpriteAssetId() { return mSpriteAssetId; }

      void createTiles(S32 tileX, S32 tileY, F32 tileSize);
      bool validateFrame(S32 id, S32 frame);
      void activeCount();
      void renderLayer();
   };

   typedef Vector<TileLayer*> typeTileLayer;
   typeTileLayer mTileLayerList;

   S32 mTileLayerId;

   TileMap();
   virtual ~TileMap();
   /// sim
   virtual bool onAdd();
   virtual void onRemove();

   static void initPersistFields();
   virtual void onInspect(GuiInspector * inspector);
   virtual void onDynamicModified(const char * slotName, const char * newValue);
   void setTileFrame(S32 layerId, S32 tileId, S32 frame);
   //virtual void inspectPostApply();
   //void validate();

   void createLayer();
   void createSpecifiedLayer(U32 slotNum, StringTableEntry spriteAssetId);

   /// NetObject
   U32 packUpdate(NetConnection* conn, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection* conn, BitStream* stream);

   //virtual void processTick();
   //virtual void interpolateTick(F32 dt);

   /// rendering
   void prepRenderImage(SceneCameraState* cam);

   DECLARE_CONOBJECT(TileMap);

};

#endif // !_TILEMAP_H_
