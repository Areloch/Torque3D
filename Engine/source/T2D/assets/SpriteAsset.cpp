#include "T2D/assets/SpriteAsset.h"

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

#include "gfx/gfxStringEnumTranslate.h"

// Debug Profiling.
#include "platform/profiler.h"

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(SpriteAsset);
ConsoleType(SpriteAssetPtr, TypeSpriteAssetPtr, String, ASSET_ID_FIELD_PREFIX)

ConsoleGetType(TypeSpriteAssetPtr)
{
   // Fetch asset Id.
   return *((StringTableEntry*)dptr);
}

ConsoleSetType(TypeSpriteAssetPtr)
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
   Con::warnf("(TypeSpriteAssetPtr) - Cannot set multiple args to a single asset.");
}

//-----------------------------------------------------------------------------

SpriteAsset::SpriteAsset() :
      AssetBase(),
      mSprite(nullptr),
      mCellRowOrder(true),
      mCellOffsetX(0),
      mCellOffsetY(0),
      mCellStrideX(0),
      mCellStrideY(0),
      mCellCountX(0),
      mCellCountY(0),
      mCellWidth(0),
      mCellHeight(0),
      mIsValidSprite(false)

{
   // Set Vector Associations.
   VECTOR_SET_ASSOCIATION(mFrames);
   VECTOR_SET_ASSOCIATION(mExplicitFrames);
}

SpriteAsset::~SpriteAsset()
{
}

void SpriteAsset::initPersistFields()
{
   // Parent
   Parent::initPersistFields();

   addProtectedField("ImageFile", TypeAssetLooseFilePath, Offset(mSpriteFileName, SpriteAsset), &setSpriteFileName, &getSpriteFileName, &defaultProtectedWriteFn,
      "Path to the sprite.");

   addField("CellRowOrder", TypeBool, Offset(mCellRowOrder, SpriteAsset), "");
   addField("CellOffsetX", TypeS32, Offset(mCellOffsetX, SpriteAsset), "");
   addField("CellOffsetY", TypeS32, Offset(mCellOffsetY, SpriteAsset), "");
   addField("CellStrideX", TypeS32, Offset(mCellStrideX, SpriteAsset), "");
   addField("CellStrideY", TypeS32, Offset(mCellStrideY, SpriteAsset), "");
   addField("CellCountX", TypeS32, Offset(mCellCountX, SpriteAsset), "");
   addField("CellCountY", TypeS32, Offset(mCellCountY, SpriteAsset), "");
   addField("CellWidth", TypeS32, Offset(mCellWidth, SpriteAsset), "");
   addField("CellHeight", TypeS32, Offset(mCellHeight, SpriteAsset), "");

}

bool SpriteAsset::getAssetByFilename(StringTableEntry fileName, AssetPtr<SpriteAsset>* spriteAsset)
{
   AssetQuery query;
   S32 foundAssetcount = AssetDatabase.findAssetLooseFile(&query, fileName);
   if (foundAssetcount == 0)
   {
      Con::warnf("Could not find asset %s", fileName);
      //That didn't work, so fail out
      return false;
   }
   else
   {
      //acquire and bind the asset, and return it out
      spriteAsset->setAssetId(query.mAssetList[0]);
      return true;
   }
}

void SpriteAsset::copyTo(SimObject* object)
{
   Parent::copyTo(object);
}

void SpriteAsset::loadSprite()
{
   SAFE_DELETE(mSprite);

   if (mSpriteFileName)
   {
      if (!Platform::isFile(mSpriteFileName))
      {
         Con::errorf("SpriteAsset::initializeAsset: Attempted to load file %s but it was not valid!", mSpriteFileName);
         return;
      }

      mSprite.set(mSpriteFileName, &GFXStaticTextureSRGBProfile, avar("%s() - mSprite (line %d)", __FUNCTION__, __LINE__));
      if (mSprite)
      {
         mIsValidSprite = true;
         return;
      }
   }

   mIsValidSprite = false;

}

void SpriteAsset::initializeAsset()
{
   mSpriteFileName = expandAssetFilePath(mSpriteFileName);

   loadSprite();

   calculateSprite();

}

void SpriteAsset::onAssetRefresh()
{
   setSpriteFileName(mSpriteFileName);

   calculateSprite();
}

void SpriteAsset::setSpriteFileName(const char* pScriptFile)
{
   AssertFatal(pScriptFile != NULL, "Cannot use a null sprite file.");

   mSpriteFileName = StringTable->insert(pScriptFile);

}

void SpriteAsset::setCellRowOrder(const bool cellRowOrder)
{
   // Ignore no change.
   if (cellRowOrder == mCellRowOrder)
      return;

   // Update.
   mCellRowOrder = cellRowOrder;

   // Refresh the asset.
   refreshAsset();
}

void SpriteAsset::setCellOffsetX(const S32 cellOffsetX)
{
   // Ignore no change.
   if (cellOffsetX == mCellOffsetX)
      return;

   // Valid?
   if (cellOffsetX < 0)
   {
      // No, so warn.
      Con::warnf("Invalid CELL offset X '%d'.", cellOffsetX);
      return;
   }

   // Update.
   mCellOffsetX = cellOffsetX;

   // Refresh the asset.
   refreshAsset();
}

void SpriteAsset::setCellOffsetY(const S32 cellOffsetY)
{
   // Ignore no change.
   if (cellOffsetY == mCellOffsetY)
      return;

   // Valid?
   if (cellOffsetY < 0)
   {
      // No, so warn.
      Con::warnf("Invalid CELL offset Y '%d'.", cellOffsetY);
      return;
   }

   // Update.
   mCellOffsetY = cellOffsetY;

   // Refresh the asset.
   refreshAsset();
}


void SpriteAsset::setCellStrideX(const S32 cellStrideX)
{
   // Ignore no change.
   if (cellStrideX == mCellStrideX)
      return;

   // Valid?
   if (cellStrideX < 0)
   {
      // No, so warn.
      Con::warnf("Invalid CELL stride X '%d'.", cellStrideX);
      return;
   }

   // Update.
   mCellStrideX = cellStrideX;

   // Refresh the asset.
   refreshAsset();
}

void SpriteAsset::setCellStrideY(const S32 cellStrideY)
{
   // Ignore no change.
   if (cellStrideY == mCellStrideY)
      return;

   // Valid?
   if (cellStrideY < 0)
   {
      // No, so warn.
      Con::warnf("Invalid CELL stride Y '%d'.", cellStrideY);
      return;
   }

   // Update.
   mCellStrideY = cellStrideY;

   // Refresh the asset.
   refreshAsset();
}

void SpriteAsset::setCellCountX(const S32 cellCountX)
{
   // Ignore no change.
   if (cellCountX == mCellCountX)
      return;

   // Valid?
   if (cellCountX < 0)
   {
      // No, so warn.
      Con::warnf("Invalid CELL count X '%d'.", cellCountX);
      return;
   }

   // Update.
   mCellCountX = cellCountX;

   // Refresh the asset.
   refreshAsset();
}

void SpriteAsset::setCellCountY(const S32 cellCountY)
{
   // Ignore no change.
   if (cellCountY == mCellCountY)
      return;

   // Valid?
   if (cellCountY < 0)
   {
      // No, so warn.
      Con::warnf("Invalid CELL count Y '%d'.", cellCountY);
      return;
   }

   // Update.
   mCellCountY = cellCountY;

   // Refresh the asset.
   refreshAsset();
}

void SpriteAsset::setCellWidth(const S32 cellWidth)
{
   // Ignore no change.
   if (cellWidth == mCellWidth)
      return;

   // Valid?
   if (cellWidth < 0)
   {
      // No, so warn.
      Con::warnf("Invalid cell width %d.", cellWidth);
      return;
   }

   // Update.
   mCellWidth = cellWidth;

   // Refresh the asset.
   refreshAsset();
}

void SpriteAsset::setCellHeight(const S32 cellHeight)
{
   if (cellHeight == mCellHeight)
      return;

   if (cellHeight < 0)
   {
      Con::warnf("Invalid cell height %d.", cellHeight);
      return;
   }

   mCellHeight = cellHeight;

   refreshAsset();

}

GFXTexHandle SpriteAsset::getSprite(GFXTextureProfile reqProfile)
{
   if (mResourceMap.contains(reqProfile))
   {
      return mResourceMap.find(reqProfile)->value;
   }
   else
   {
      GFXTexHandle tempSprite;
      tempSprite.set(mSpriteFileName, &reqProfile, avar("%s() - mSprite (line %d)", __FUNCTION__, __LINE__));
      return tempSprite;
   }

   return nullptr;

}

const char* SpriteAsset::getSpriteInfo()
{
   if (mIsValidSprite)
   {
      static const U32 bufSize = 2048;
      char* returnBuffer = Con::getReturnBuffer(bufSize);
      dSprintf(returnBuffer, bufSize, "%s %d %d %d", GFXStringTextureFormat[mSprite.getFormat()], mSprite.getHeight(), mSprite.getWidth(), mSprite.getDepth());

      return returnBuffer;
   }

   return "";

}

void SpriteAsset::calculateSprite()
{
   /// Debug profile
   PROFILE_SCOPE(SpriteAsset_CalculateSprite);

   mFrames.clear();

   calculateImplicit();

   PROFILE_END();

}

void SpriteAsset::calculateImplicit()
{
   PROFILE_SCOPE(SpriteAsset_CalculateImplicit);

   /// calculate texel scale.
   const F32 texWScale = 1.0f / (F32)mSprite->getWidth();
   const F32 texHScale = 1.0f / (F32)mSprite->getHeight();

   /// original bitmap dimension
   const U32 spriteWidth = mSprite->getBitmapWidth();
   const U32 spriteHeight = mSprite->getBitmapHeight();

   /// default frame.
   FrameArea frameArea(0, 0, spriteWidth, spriteHeight, texWScale, texHScale);
   mFrames.push_back(frameArea);

   /// we don't have cells get out.
   if (mCellCountX < 1 || mCellCountY < 1)
      return;

   if (mCellWidth < 1 || mCellWidth > spriteWidth)
   {
      Con::warnf("SpriteAsset::calculateSprite() - Invalid cell width of %d", mCellWidth);
      return;
   }

   if (mCellHeight < 1 || mCellHeight > spriteHeight)
   {
      Con::warnf("SpriteAsset::calculateSprite() - Invalid cell height of %d", mCellHeight);
      return;
   }

   // The Cell Offset X needs to be within the image.
   if (mCellOffsetX < 0 || mCellOffsetX >= spriteWidth)
   {
      // Warn.
      Con::warnf("SpriteAsset::calculateSprite() - Invalid Cell OffsetX of %d.", mCellOffsetX);
      return;
   }

   // The Cell Offset Y needs to be within the image.
   if (mCellOffsetY < 0 || mCellOffsetY >= spriteHeight)
   {
      // Warn.
      Con::warnf("SpriteAsset::calculateSprite() - Invalid Cell OffsetY of %d.", mCellOffsetY);
      return;
   }

   // Are we using Cell-StrideX?
   S32 cellStepX;
   if (mCellStrideX != 0)
   {
      // Yes, so set stepX to be StrideX.
      cellStepX = mCellStrideX;
   }
   else
   {
      // No, so set stepY to be Cell Width.
      cellStepX = mCellWidth;
   }

   // Are we using Cell-StrideY?
   S32 cellStepY;
   if (mCellStrideY != 0)
   {
      // Yes, so set stepY to be StrideY.
      cellStepY = mCellStrideY;
   }
   else
   {
      // No, so set stepY to be Cell Height.
      cellStepY = mCellHeight;
   }

   // Calculate Final Cell Position X.
   S32 cellFinalPositionX = mCellOffsetX + ((mCellCountX - ((cellStepX < 0) ? 1 : 0))*cellStepX);
   // Off Left?
   if (cellFinalPositionX < 0)
   {
      // Warn.
      Con::warnf("ImageAsset::calculateImage() - Invalid Cell OffsetX(%d)/Width(%d)/CountX(%d); off image left-hand-side.", mCellOffsetX, mCellWidth, mCellCountX);
      return;
   }
   // Off Right?
   else if (cellFinalPositionX > spriteWidth)
   {
      // Warn.
      Con::warnf("ImageAsset::calculateImage() - Invalid Cell OffsetX(%d)/Width(%d)/CountX(%d); off image right-hand-side.", mCellOffsetX, mCellWidth, mCellCountX);
      return;
   }

   // Calculate Final Cell Position Y.
   S32 cellFinalPositionY = mCellOffsetY + ((mCellCountY - ((cellStepY < 0) ? 1 : 0))*cellStepY);
   // Off Top?
   if (cellFinalPositionY < 0)
   {
      // Warn.
      Con::warnf("ImageAsset::calculateImage() - Invalid Cell OffsetY(%d)/Height(%d)/CountY(%d); off image top-side.", mCellOffsetY, mCellHeight, mCellCountY);
      return;
   }
   // Off Bottom?
   else if (cellFinalPositionY > spriteHeight)
   {
      // Warn.
      Con::warnf("ImageAsset::calculateImage() - Invalid Cell OffsetY(%d)/Height(%d)/CountY(%d); off image bottom-side.", mCellOffsetY, mCellHeight, mCellCountY);
      return;
   }

   // Clear default frame.
   mFrames.clear();

   // Cell Row Order?
   if (mCellRowOrder)
   {
      // Yes, so RowRow Order.
      for (S32 y = 0, cellPositionY = mCellOffsetY; y < mCellCountY; y++, cellPositionY += cellStepY)
      {
         for (S32 x = 0, cellPositionX = mCellOffsetX; x < mCellCountX; x++, cellPositionX += cellStepX)
         {
            // Set frame area.
            frameArea.setArea(cellPositionX, cellPositionY, mCellWidth, mCellHeight, texWScale, texHScale);

            // Store fame.
            mFrames.push_back(frameArea);
         }
      }

      return;
   }

   // No, so Column Order.
   for (S32 x = 0, cellPositionX = mCellOffsetX; x < mCellCountX; x++, cellPositionX += cellStepX)
   {
      for (S32 y = 0, cellPositionY = mCellOffsetY; y < mCellCountY; y++, cellPositionY += cellStepY)
      {
         // Set frame area.
         frameArea.setArea(cellPositionX, cellPositionY, mCellWidth, mCellHeight, texWScale, texHScale);

         // Store fame.
         mFrames.push_back(frameArea);
      }
   }

   PROFILE_END();
}

DefineEngineMethod(SpriteAsset, getSpriteFileName, const char*, (), ,
   "Creates an instance of the given GameObject given the asset definition.\n"
   "@return The GameObject entity created from the asset.")
{
   return object->getSpriteFileName();
}

DefineEngineMethod(SpriteAsset, getSpriteInfo, const char*, (), ,
   "Creates an instance of the given GameObject given the asset definition.\n"
   "@return The GameObject entity created from the asset.")
{
   return object->getSpriteInfo();
}

