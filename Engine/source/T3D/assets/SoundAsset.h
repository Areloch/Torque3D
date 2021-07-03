#pragma once
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
#define SOUND_ASSET_H

#ifndef _ASSET_BASE_H_
#include "assets/assetBase.h"
#endif

#ifndef _ASSET_DEFINITION_H_
#include "assets/assetDefinition.h"
#endif

#ifndef _STRINGUNIT_H_
#include "string/stringUnit.h"
#endif

#ifndef _ASSET_FIELD_TYPES_H_
#include "assets/assetFieldTypes.h"
#endif

#include "gui/editor/guiInspectorTypes.h"

#ifndef _BITSTREAM_H_
#include "core/stream/bitStream.h"
#endif

#ifndef _SFXRESOURCE_H_
#include "sfx/sfxResource.h"
#endif

#ifndef _SFXDESCRIPTION_H_
#include "sfx/sfxDescription.h"
#endif // !_SFXDESCRIPTION_H_

#ifndef _SFXPROFILE_H_
#include "sfx/sfxProfile.h"
#endif // !_SFXPROFILE_H_

class SFXResource;

//-----------------------------------------------------------------------------
class SoundAsset : public AssetBase
{
   typedef AssetBase Parent;

protected:
   StringTableEntry        mSoundFile;
   StringTableEntry        mSoundPath;
   SFXProfile              mSFXProfile;
   SFXDescription          mProfileDesc;
   // subtitles
   StringTableEntry        mSubtitleString;
   bool                    mPreload;

   /*These will be needed in the refactor!
   Resource<SFXResource>   mSoundResource;
   

   // SFXDesctriptions, some off these will be removed
   F32                     mPitchAdjust;
   F32                     mVolumeAdjust;
   bool                    mIs3D;
   bool                    mLoop;
   bool                    mIsStreaming;
   bool                    mUseHardware;

   F32                     mMinDistance;
   F32                     mMaxDistance;
   U32                     mConeInsideAngle;
   U32                     mConeOutsideAngle;
   F32                     mConeOutsideVolume;
   F32                     mRolloffFactor;
   Point3F                 mScatterDistance;
   F32                     mPriority;
   */

   typedef Signal<void()> SoundAssetChanged;
   SoundAssetChanged mChangeSignal;

public:
   SoundAsset();
   virtual ~SoundAsset();

   /// Engine.
   static void initPersistFields();
   virtual void copyTo(SimObject* object);

   //SFXResource* getSound() { return mSoundResource; }
   Resource<SFXResource> getSoundResource() { return mSFXProfile.getResource(); }

   /// Declare Console Object.
   DECLARE_CONOBJECT(SoundAsset);

   void                    setSoundFile(const char* pSoundFile);
   bool loadSound();
   inline StringTableEntry getSoundFile(void) const { return mSoundFile; };
   inline StringTableEntry getSoundPath(void) const { return mSoundPath; };
   SFXProfile* getSfxProfile() { return &mSFXProfile; }
   SFXDescription* getSfxDescription() { return &mProfileDesc; }

   bool isLoop() { return mProfileDesc.mIsLooping; }
   bool is3D() { return mProfileDesc.mIs3D; }


protected:
   virtual void            initializeAsset(void);
   void _onResourceChanged(const Torque::Path & path);
   virtual void            onAssetRefresh(void);

   static bool setSoundFile(void *obj, const char *index, const char *data) { static_cast<SoundAsset*>(obj)->setSoundFile(data); return false; }
   static const char* getSoundFile(void* obj, const char* data) { return static_cast<SoundAsset*>(obj)->getSoundFile(); }
};

DefineConsoleType(TypeSoundAssetPtr, SoundAsset)
DefineConsoleType(TypeSoundAssetId, String)

//-----------------------------------------------------------------------------
// TypeAssetId GuiInspectorField Class
//-----------------------------------------------------------------------------
class GuiInspectorTypeSoundAssetPtr : public GuiInspectorTypeFileName
{
   typedef GuiInspectorTypeFileName Parent;
public:

   GuiBitmapButtonCtrl* mSoundButton;

   DECLARE_CONOBJECT(GuiInspectorTypeSoundAssetPtr);
   static void consoleInit();

   virtual GuiControl* constructEditControl();
   virtual bool updateRects();
};

class GuiInspectorTypeSoundAssetId : public GuiInspectorTypeSoundAssetPtr
{
   typedef GuiInspectorTypeSoundAssetPtr Parent;
public:

   DECLARE_CONOBJECT(GuiInspectorTypeSoundAssetId);
   static void consoleInit();
};

#pragma region Singular Asset Macros

//Singular assets
/// <Summary>
/// Declares a sound asset
/// This establishes the assetId, asset and legacy filepath fields, along with supplemental getter and setter functions
/// </Summary>
#define DECLARE_SOUNDASSET(className, name, profile) public: \
   FileName m##name##Filename = String::EmptyString; \
   StringTableEntry m##name##AssetId = StringTable->EmptyString();\
   AssetPtr<SoundAsset> m##name##Asset = NULL;\
   SFXProfile* m##name##Profile = &profile;\
public: \
   const StringTableEntry get##name##File() const { return StringTable->insert(m##name##Filename.c_str()); }\
   void set##name##File(const FileName &_in) { m##name##Filename = _in;}\
   const AssetPtr<SoundAsset> & get##name##Asset() const { return m##name##Asset; }\
   void set##name##Asset(const AssetPtr<SoundAsset> &_in) { m##name##Asset = _in;}\
   \
   bool _set##name(StringTableEntry _in)\
   {\
      if (_in == StringTable->EmptyString())\
      {\
         m##name##Name = StringTable->EmptyString();\
         m##name##AssetId = StringTable->EmptyString();\
         m##name##Asset = NULL;\
         m##name = NULL;\
         return true;\
      }\
      \
      if (AssetDatabase.isDeclaredAsset(_in))\
      {\
         m##name##AssetId = _in;\
         \
         U32 assetState = SoundAsset::getAssetById(m##name##AssetId, &m##name##Asset);\
         \
         if (SoundAsset::Ok == assetState)\
         {\
            m##name##Name = StringTable->EmptyString();\
         }\
         else\
         {\
            m##name##Name = _in;\
            m##name##Asset = NULL;\
         }\
      }\
      else\
      {\
         if (SoundAsset::getAssetByFilename(_in, &m##name##Asset))\
         {\
            m##name##AssetId = m##name##Asset.getAssetId();\
            \
            if(SoundAsset::Ok == m##name##Asset->getStatus())\
            {\
               m##name##Filename = String::EmptyString;\
            }\
         }\
         else\
         {\
            m##name##Name = _in;\
            m##name##AssetId = StringTable->EmptyString();\
            m##name##Asset = NULL;\
         }\
      }\
      if (get##name() != StringTable->EmptyString() && m##name##Asset.notNull())\
      {\
         m##name = m##name##Asset->getSoundResource();\
         \
         if (bool(m##name) == NULL)\
         {\
            Con::errorf("%s::_set##name() - %s: Couldn't load sound %s", assetText(className,""), get##name());\
            return false;\
         }\
         return true;\
      }\
      return false;\
   }\
   \
   const StringTableEntry get##name() const\
   {\
      if (m##name##Asset && (m##name##Asset->getSoundPath() != StringTable->EmptyString()))\
         return m##name##Asset->getSoundPath();\
      else if (m##name##Name != StringTable->EmptyString())\
         return StringTable->insert(m##name##Name);\
      else\
         return StringTable->EmptyString();\
   }\
   Resource<SFXResource> get##name##Resource() \
   {\
      return m##name;\
   }

#define DECLARE_SOUNDASSET_SETGET(className, name)\
   static bool _set##name##Data(void* obj, const char* index, const char* data)\
   {\
      bool ret = false;\
      className* object = static_cast<className*>(obj);\
      ret = object->_set##name(StringTable->insert(data));\
      return ret;\
   }

#define DECLARE_SOUNDASSET_NET_SETGET(className, name, bitmask)\
   static bool _set##name##Data(void* obj, const char* index, const char* data)\
   {\
      bool ret = false;\
      className* object = static_cast<className*>(obj);\
      ret = object->_set##name(StringTable->insert(data));\
      if(ret)\
         object->setMaskBits(bitmask);\
      return ret;\
   }

#define DEF_SOUNDASSET_BINDS(className,name)\
DefineEngineMethod(className, get##name, String, (), , "get name")\
{\
   return object->get##name(); \
}\
DefineEngineMethod(className, get##name##Asset, String, (), , assetText(name, asset reference))\
{\
   return object->m##name##AssetId; \
}\
DefineEngineMethod(className, set##name, bool, (const char*  shape), , assetText(name,assignment. first tries asset then flat file.))\
{\
   return object->_set##name(StringTable->insert(shape));\
}

#define INIT_SOUNDASSET(name) \
   m##name##Name = StringTable->EmptyString(); \
   m##name##AssetId = StringTable->EmptyString(); \
   m##name##Asset = NULL; \
   m##name = NULL;\

#define INITPERSISTFIELD_SOUNDASSET(name, consoleClass, docs) \
   addProtectedField(assetText(name, File), TypeSoundFilename, Offset(m##name##Name, consoleClass), _set##name##Data, & defaultProtectedGetFn, assetText(name, docs)); \
   addProtectedField(assetText(name, Asset), TypeSoundAssetId, Offset(m##name##AssetId, consoleClass), _set##name##Data, & defaultProtectedGetFn, assetText(name, asset reference.));

#define CLONE_SOUNDASSET(name) \
   m##name##Name = other.m##name##Name;\
   m##name##AssetId = other.m##name##AssetId;\
   m##name##Asset = other.m##name##Asset;\

#define PACKDATA_SOUNDASSET(name)\
   if (stream->writeFlag(m##name##Asset.notNull()))\
   {\
      stream->writeString(m##name##Asset.getAssetId());\
   }\
   else\
      stream->writeString(m##name##Name);

#define UNPACKDATA_SOUNDASSET(name)\
   if (stream->readFlag())\
   {\
      m##name##AssetId = stream->readSTString();\
   }\
   else\
      m##name##Name = stream->readSTString();

#define PACK_SOUNDASSET(netconn, name)\
   if (stream->writeFlag(m##name##Asset.notNull()))\
   {\
      NetStringHandle assetIdStr = m##name##Asset.getAssetId();\
      netconn->packNetStringHandleU(stream, assetIdStr);\
   }\
   else\
      stream->writeString(m##name##Name);

#define UNPACK_SOUNDASSET(netconn, name)\
   if (stream->readFlag())\
   {\
      m##name##AssetId = StringTable->insert(netconn->unpackNetStringHandleU(stream).getString());\
      _set##name(m##name##AssetId);\
   }\
   else\
      m##name##Name = stream->readSTString();

#pragma endregion

#endif // _ASSET_BASE_H_

