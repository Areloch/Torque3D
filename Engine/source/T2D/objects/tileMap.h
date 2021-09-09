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

   enum
   {
      LayerUpdateMask   = Parent::NextFreeMask << 0,
      NextFreeMask      = Parent::NextFreeMask << 1
   };

private:

   S32 mNodeCountX;
   S32 mNodeCountY;
   /// Tiles should be uniform size, don't like it? you tile wrong.
   F32 mNodeSize;
   S32 mLayerCount;

   static S32 QSORT_CALLBACK sortTileLayer(const void* a, const void* b);

public:

   S32 mCurLayerId;

   class TileLayer
   {
      TileMap* oTileMap;
   private:
      AssetPtr<SpriteAsset>   mSpriteAsset;
      StringTableEntry        mSpriteAssetId;
      
   public:
      enum NetMaskBits
      {
         InitialUpdateMask = TileMap::NextFreeMask << 0,
         SpriteUpdateMask  = TileMap::NextFreeMask << 1,
         LayerObjectMask   = TileMap::NextFreeMask << 2,
         NextFreeMask      = TileMap::NextFreeMask << 3
      };

      class TileObject
      {
         TileLayer*  oTileLayer;

      private:
         U32         mFrame;
         S32         mObjId;
         bool        mActive;
         bool        mPhysics;
         Vector2     mPos;

      public:

         enum NetMaskBits
         {
            InitialUpdateMask = TileLayer::NextFreeMask << 0,
            ObjectUpdateMask = TileLayer::NextFreeMask << 1,
            NextFreeMask = TileMap::NextFreeMask << 2
         };

         TileObject();
         TileObject(TileLayer* lyr, S32 id, Vector2 pos);
         ~TileObject();

         void setFrame(U32 frame);

         /// Net update overrides
         enum UpdateState
         {
            None,
            Updating
         };
         UpdateState             updateState;
         U32                     updateNetMaskBits;

         void setObjectMaskBits(U32 mask);
         U32 packUpdate(NetConnection* conn, U32 mask, BitStream* stream);
         void unpackUpdate(NetConnection* conn, BitStream* stream);

      };

   private:
      typedef Vector<TileObject*> tileObjVector;
      tileObjVector mTileObjs;

   public:

      S32                     mLayerId;
      S32                     mSortLayer;
      S32                     mNodeObjCountX;
      S32                     mNodeObjCountY;
      GFXTexHandle            layerTex;
      S32                     mCurObjId;

      TileLayer();
      TileLayer(TileMap* tileMap, S32 id, S32 countX, S32 countY);
      ~TileLayer();

      StringTableEntry getSpriteAssetId() { return mSpriteAssetId; }
      bool setSpriteAsset(StringTableEntry spriteId);
      bool validateFrame(U32 frame);
      void createLayout();

      /// Net update overrides
      enum UpdateState
      {
         None,
         Updating
      };

      UpdateState             updateState;

      U32                     updateNetMaskBits;
      void setLayerMaskBits(U32 mask);
      void setObjectMask(S32 id, U32 mask);
      void setTileFrame(U32 id, U32 frame);
      U32 packUpdate(NetConnection* conn, U32 mask, BitStream* stream);
      void unpackUpdate(NetConnection* conn, BitStream* stream);


   };
private:
   typedef Vector<TileLayer*> tileLayerVector;
   tileLayerVector mLayers;

public:
   
   F32 mTileMapWidth;
   F32 mTileMapHeight;

   TileMap();
   virtual ~TileMap();
   /// sim
   virtual bool onAdd();
   virtual void onRemove();

   static void initPersistFields();
   virtual void onInspect(GuiInspector* inspector);
   virtual void onDynamicModified(const char * slotName, const char * newValue);
   virtual void inspectPostApply();
   void validate();

   void addLayer(U32 num);

   void setLayerMasks(U32 id, U32 mask);

   void setTileObjectFrame(U32 layerId, U32 objId, U32 frame);

   /// NetObject
   U32 packUpdate(NetConnection* conn, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection* conn, BitStream* stream);

   virtual void processTick();
   virtual void interpolateTick(F32 dt);

   /// rendering
   void prepRenderImage(SceneCameraState* cam);

   DECLARE_CONOBJECT(TileMap);

};

#endif // !_TILEMAP_H_
