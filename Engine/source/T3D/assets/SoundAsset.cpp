//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef SOUND_ASSET_H
#include "SoundAsset.h"
#endif

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

// Debug Profiling.
#include "platform/profiler.h"

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(SoundAsset);

ConsoleType(SoundAssetPtr, TypeSoundAssetPtr, const char*, ASSET_ID_FIELD_PREFIX)

//-----------------------------------------------------------------------------

ConsoleGetType(TypeSoundAssetPtr)
{
   // Fetch asset Id.
   return *((const char**)(dptr));
}

//-----------------------------------------------------------------------------

ConsoleSetType(TypeSoundAssetPtr)
{
   // Was a single argument specified?
   if (argc == 1)
   {
      // Yes, so fetch field value.
      *((const char**)dptr) = StringTable->insert(argv[0]);

      return;
   }

   // Warn.
   Con::warnf("(TypeSoundAssetPtr) - Cannot set multiple args to a single asset.");
}

//-----------------------------------------------------------------------------

ConsoleType(assetIdString, TypeSoundAssetId, const char*, ASSET_ID_FIELD_PREFIX)

ConsoleGetType(TypeSoundAssetId)
{
   // Fetch asset Id.
   return *((const char**)(dptr));
}

ConsoleSetType(TypeSoundAssetId)
{
   // Was a single argument specified?
   if (argc == 1)
   {
      // Yes, so fetch field value.
      *((const char**)dptr) = StringTable->insert(argv[0]);

      return;
   }

   // Warn.
   Con::warnf("(TypeAssetId) - Cannot set multiple args to a single asset.");
}

//-----------------------------------------------------------------------------

SoundAsset::SoundAsset()
{
   mSoundFile = StringTable->EmptyString();
   mSoundPath = StringTable->EmptyString();
   mSubtitleString = StringTable->EmptyString();

   mLoadedState = AssetErrCode::NotLoaded;

   // SFX description inits
   // reverb is useless here, reverb is inacted on listener.
   mPitchAdjust = 1;
   mVolumeAdjust = 1;
   mIs3D = true;
   mLoop = false;
   mIsStreaming = false;
   mUseHardware = false;
   mMinDistance = 1;
   mMaxDistance = 100;
   mConeInsideAngle = 360;
   mConeOutsideAngle = 360;
   mConeOutsideVolume = 1;
   mRolloffFactor = -1.0f;
   mScatterDistance = Point3F(0.f, 0.f, 0.f);
   mPriority = 1.0f;

}

//-----------------------------------------------------------------------------

SoundAsset::~SoundAsset()
{
}

//-----------------------------------------------------------------------------

void SoundAsset::initPersistFields()
{
   // Call parent.
   Parent::initPersistFields();

   addProtectedField("soundFile", TypeAssetLooseFilePath, Offset(mSoundFile, SoundAsset),
      &setSoundFile, &getSoundFile, "Path to the sound file.");

   addField("pitchAdjust", TypeF32, Offset(mPitchAdjust, SoundAsset), "Adjustment of the pitch value 1 is default.");
   addField("volumeAdjust", TypeF32, Offset(mVolumeAdjust, SoundAsset), "Adjustment to the volume.");
   addField("is3D", TypeBool, Offset(mVolumeAdjust, SoundAsset), "Set this sound to 3D.");
   addField("isLooping", TypeBool, Offset(mLoop, SoundAsset), "Does this sound loop.");
   addField("useHardware", TypeBool, Offset(mUseHardware, SoundAsset), "Use hardware mixing for this sound.");
   // if streaming, a default packet size should be chosen for all sounds.
   addField("isStreaming", TypeBool, Offset(mIsStreaming, SoundAsset), "Use streaming.");
   //....why?
   addField("minDistance", TypeF32, Offset(mMinDistance, SoundAsset), "Minimum distance for sound.");
   // more like it.
   addField("maxDistance", TypeF32, Offset(mMaxDistance, SoundAsset), "Max distance for sound.");
   addField("scatterDistance", TypePoint3F, Offset(mScatterDistance, SoundAsset), "Randomization to the spacial position of the sound.");
   addField("coneInsideAngle", TypeS32, Offset(mConeInsideAngle, SoundAsset), "Cone inside angle.");
   addField("coneOutsideAngle", TypeS32, Offset(mConeOutsideAngle, SoundAsset), "Cone outside angle.");
   addField("coneOutsideVolume", TypeS32, Offset(mConeOutsideVolume, SoundAsset), "Cone outside volume.");
   addField("rolloffFactor", TypeF32, Offset(mRolloffFactor, SoundAsset), "Rolloff factor.");

}

//------------------------------------------------------------------------------

void SoundAsset::copyTo(SimObject* object)
{
   // Call to parent.
   Parent::copyTo(object);
}

void SoundAsset::initializeAsset(void)
{
   Parent::initializeAsset();

   if (mSoundFile == StringTable->EmptyString())
      return;

   ResourceManager::get().getChangedSignal.notify(this, &SoundAsset::_onResourceChanged);

   //Ensure our path is expando'd if it isn't already
   if (!Platform::isFullPath(mSoundPath))
      mSoundPath = getOwned() ? expandAssetFilePath(mSoundFile) : mSoundPath;

   mSoundPath = expandAssetFilePath(mSoundPath);

   loadSound();
}

void SoundAsset::_onResourceChanged(const Torque::Path &path)
{
   if (path != Torque::Path(mSoundPath))
      return;

   refreshAsset();

   loadSound();
}

void SoundAsset::onAssetRefresh(void)
{
   if (mSoundFile == StringTable->EmptyString())
      return;

   //Update
   if (!Platform::isFullPath(mSoundFile))
      mSoundPath = getOwned() ? expandAssetFilePath(mSoundFile) : mSoundPath;

   loadSound();
}

bool SoundAsset::loadSound()
{
   mSoundResource = ResourceManager::get().load(mSoundPath);

   if (!mSoundResource)
   {
      Con::errorf("SoundAsset::loadShape : failed to load sound file %s (%s)!", getAssetName(), mSoundPath);
      mLoadedState = BadFileReference;
      return false; //if it failed to load, bail out
   }

   mChangeSignal.trigger();
   mLoadedState = Ok;
   return true;
}

void SoundAsset::setSoundFile(const char* pSoundFile)
{
   // Sanity!
   AssertFatal(pSoundFile != NULL, "Cannot use a NULL sound file.");

   // Fetch sound file.
   pSoundFile = StringTable->insert(pSoundFile);

   // Ignore no change,
   if (pSoundFile == mSoundFile)
      return;

   // Update.
   mSoundFile = pSoundFile;

   // Refresh the asset.
   refreshAsset();
}

DefineEngineMethod(SoundAsset, getSoundPath, const char*, (), , "")
{
   return object->getSoundPath();
}
