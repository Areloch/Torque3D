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
#ifndef IMAGE_ASSET_H
#define IMAGE_ASSET_H

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
#ifndef _ASSET_PTR_H_
#include "assets/assetPtr.h"
#endif 

#include "gfx/bitmap/gBitmap.h"
#include "gfx/gfxTextureHandle.h"

#include "gui/editor/guiInspectorTypes.h"

#include <string>

//-----------------------------------------------------------------------------
class ImageAsset : public AssetBase
{
   typedef AssetBase Parent;

public:
   /// The different types of image use cases
   enum ImageTypes
   {
      Albedo = 0,
      Normal = 1,
      ORMConfig = 2,
      GUI = 3,
      Roughness = 4,
      AO = 5,
      Metalness = 6,
      Glow = 7,
      Particle = 8,
      Decal = 9,
      Cubemap = 10,
      ImageTypeCount = 11
   };

protected:
   StringTableEntry mImageFileName;
   StringTableEntry mImagePath;

   GBitmap* mBitmap;
   //GFXTexHandle mTexture;

   bool mIsValidImage;
   bool mUseMips;
   bool mIsHDRImage;

   ImageTypes mImageType;

   HashMap<GFXTextureProfile*, GFXTexHandle> mResourceMap;

public:
   ImageAsset();
   virtual ~ImageAsset();

   /// Engine.
   static void initPersistFields();
   virtual void copyTo(SimObject* object);

   /// Declare Console Object.
   DECLARE_CONOBJECT(ImageAsset);

   void                    setImageFileName(StringTableEntry pScriptFile);
   inline StringTableEntry getImageFileName(void) const { return mImageFileName; };

   inline StringTableEntry getImagePath(void) const { return mImagePath; };

   bool isValid() { return mIsValidImage; }

   const GBitmap& getImage();
   GFXTexHandle getTexture(GFXTextureProfile* requestedProfile);

   StringTableEntry getImageInfo();

   static StringTableEntry getImageTypeNameFromType(ImageTypes type);
   static ImageTypes getImageTypeFromName(StringTableEntry name);

   void setImageType(ImageTypes type) { mImageType = type; }

   static bool getAssetByFilename(StringTableEntry fileName, AssetPtr<ImageAsset>* imageAsset);
   static StringTableEntry getAssetIdByFilename(StringTableEntry fileName);
   static U32 getAssetById(StringTableEntry assetId, AssetPtr<ImageAsset>* imageAsset);
   static U32 getAssetById(String assetId, AssetPtr<ImageAsset>* imageAsset) { return getAssetById(assetId.c_str(), imageAsset); };

protected:
   virtual void            initializeAsset(void);
   virtual void            onAssetRefresh(void);

   static bool setImageFileName(void* obj, StringTableEntry index, StringTableEntry data) { static_cast<ImageAsset*>(obj)->setImageFileName(data); return false; }
   static StringTableEntry getImageFileName(void* obj, StringTableEntry data) { return static_cast<ImageAsset*>(obj)->getImageFileName(); }

   void loadImage();
};

DefineConsoleType(TypeImageAssetPtr, ImageAsset)
DefineConsoleType(TypeImageAssetId, String)

typedef ImageAsset::ImageTypes ImageAssetType;
DefineEnumType(ImageAssetType);

class GuiInspectorTypeImageAssetPtr : public GuiInspectorTypeFileName
{
   typedef GuiInspectorTypeFileName Parent;
public:

   GuiBitmapButtonCtrl* mImageEdButton;

   DECLARE_CONOBJECT(GuiInspectorTypeImageAssetPtr);
   static void consoleInit();

   virtual GuiControl* constructEditControl();
   virtual bool updateRects();
   bool renderTooltip(const Point2I& hoverPos, const Point2I& cursorPos, const char* tipText = NULL);
};

class GuiInspectorTypeImageAssetId : public GuiInspectorTypeImageAssetPtr
{
   typedef GuiInspectorTypeImageAssetPtr Parent;
public:

   DECLARE_CONOBJECT(GuiInspectorTypeImageAssetId);
   static void consoleInit();
};

#pragma region Singular Asset Macros

//Singular assets
/// <Summary>
/// Declares an image asset
/// This establishes the assetId, asset and legacy filepath fields, along with supplemental getter and setter functions
/// </Summary>
#define DECLARE_IMAGEASSET(className, name, profile) public: \
   GFXTexHandle m##name = NULL;\
   FileName m##name##Filename = String::EmptyString; \
   StringTableEntry m##name##AssetId = StringTable->EmptyString();\
   AssetPtr<ImageAsset>  m##name##Asset = NULL;\
   GFXTextureProfile* m##name##Profile = &profile;\
public: \
   const StringTableEntry get##name##File() const { return StringTable->insert(m##name##Filename.c_str()); }\
   void set##name##File(const FileName &_in) { m##name##Filename = _in;}\
   const AssetPtr<ImageAsset> & get##name##Asset() const { return m##name##Asset; }\
   void set##name##Asset(const AssetPtr<ImageAsset> &_in) { m##name##Asset = _in;}\
   \
   bool _set##name(StringTableEntry _in)\
   {\
      if (_in == StringTable->EmptyString())\
      {\
         m##name##Filename = String::EmptyString;\
         m##name##AssetId = StringTable->EmptyString();\
         m##name##Asset = NULL;\
         m##name.free();\
         m##name = NULL;\
         return true;\
      }\
      \
      if (AssetDatabase.isDeclaredAsset(_in))\
      {\
         m##name##AssetId = _in;\
         \
         U32 assetState = ImageAsset::getAssetById(m##name##AssetId, &m##name##Asset);\
         \
         if (ImageAsset::Ok == assetState)\
         {\
            m##name##Filename = StringTable->EmptyString();\
         }\
         else\
         {\
            m##name##Filename = _in;\
            m##name##Asset = NULL;\
         }\
      }\
      else\
      {\
         Torque::Path imagePath = _in;\
         if (imagePath.getExtension() == String::EmptyString)\
         {\
            if (Platform::isFile(imagePath.getFullPath() + ".png"))\
               imagePath.setExtension("png");\
            else if (Platform::isFile(imagePath.getFullPath() + ".dds"))\
               imagePath.setExtension("dds");\
            else if (Platform::isFile(imagePath.getFullPath() + ".jpg"))\
               imagePath.setExtension("jpg");\
         }\
         if (ImageAsset::getAssetByFilename(imagePath.getFullPath(), &m##name##Asset))\
         {\
            m##name##AssetId = m##name##Asset.getAssetId();\
            \
            if (ImageAsset::Ok == m##name##Asset->getStatus())\
               m##name##Filename = StringTable->EmptyString();\
         }\
         else\
         {\
            m##name##Filename = _in;\
            m##name##AssetId = StringTable->EmptyString();\
            m##name##Asset = NULL;\
         }\
      }\
      if (get##name() != StringTable->EmptyString() && !m##name##Filename.equal("texhandle", String::NoCase))\
      {\
         m##name.set(get##name(), m##name##Profile, avar("%s() - mTextureObject (line %d)", __FUNCTION__, __LINE__));\
         return true;\
      }\
      return false;\
   }\
   \
   const StringTableEntry get##name() const\
   {\
      if (m##name##Asset && (m##name##Asset->getImageFileName() != StringTable->EmptyString()))\
         return  Platform::makeRelativePathName(m##name##Asset->getImagePath(), Platform::getMainDotCsDir());\
      else if (m##name##Filename.isNotEmpty())\
         return StringTable->insert(Platform::makeRelativePathName(m##name##Filename.c_str(), Platform::getMainDotCsDir()));\
      else\
         return StringTable->EmptyString();\
   }\
   GFXTexHandle get##name##Resource() \
   {\
      return m##name;\
   }

#define DECLARE_IMAGEASSET_SETGET(className, name)\
   static bool _set##name##Data(void* obj, const char* index, const char* data)\
   {\
      bool ret = false;\
      className* object = static_cast<className*>(obj);\
      ret = object->_set##name(StringTable->insert(data));\
      return ret;\
   }

#define DECLARE_IMAGEASSET_NET_SETGET(className, name, bitmask)\
   static bool _set##name##Data(void* obj, const char* index, const char* data)\
   {\
      bool ret = false;\
      className* object = static_cast<className*>(obj);\
      ret = object->_set##name(StringTable->insert(data));\
      if(ret)\
         object->setMaskBits(bitmask);\
      return ret;\
   }

#define DEF_IMAGEASSET_BINDS(className,name)\
DefineEngineMethod(className, get##name, const char*, (), , "get name")\
{\
   return object->get##name(); \
}\
DefineEngineMethod(className, get##name##Asset, const char*, (), , assetText(name, asset reference))\
{\
   return object->m##name##AssetId; \
}\
DefineEngineMethod(className, set##name, bool, (const char* map), , assetText(name,assignment. first tries asset then flat file.))\
{\
    return object->_set##name(StringTable->insert(map));\
}

#define INIT_IMAGEASSET(name) \
   m##name##Filename = String::EmptyString; \
   m##name##AssetId = StringTable->EmptyString(); \
   m##name##Asset = NULL;

#define INITPERSISTFIELD_IMAGEASSET(name, consoleClass, docs) \
   addProtectedField(#name, TypeImageFilename, Offset(m##name##Filename, consoleClass), _set##name##Data, &defaultProtectedGetFn,assetDoc(name, docs)); \
   addProtectedField(assetText(name, Asset), TypeImageAssetId, Offset(m##name##AssetId, consoleClass), _set##name##Data, &defaultProtectedGetFn, assetDoc(name, asset docs.));

#define CLONE_IMAGEASSET(name) \
   m##name##Filename = other.m##name##Filename;\
   m##name##AssetId = other.m##name##AssetId;\
   m##name##Asset = other.m##name##Asset;

#define AUTOCONVERT_IMAGEASSET(name)\
if (m##name##Filename != String::EmptyString)\
{\
   PersistenceManager* persistMgr;\
   if (!Sim::findObject("ImageAssetValidator", persistMgr))\
      Con::errorf("ImageAssetValidator not found!");\
   \
   if (persistMgr && m##name##Filename != String::EmptyString && m####name##AssetId == StringTable->EmptyString())\
   {\
      persistMgr->setDirty(this);\
   }\
   if (m##name##Filename != String::EmptyString)\
   {\
      Torque::Path imagePath = m##name##Filename;\
      if (imagePath.getPath() == String::EmptyString)\
      {\
         String subPath = Torque::Path(getFilename()).getPath();\
         imagePath.setPath(subPath);\
      }\
      \
      if (imagePath.getExtension() == String::EmptyString)\
      {\
         if (Platform::isFile(imagePath.getFullPath() + ".png"))\
            imagePath.setExtension("png");\
         else if (Platform::isFile(imagePath.getFullPath() + ".dds"))\
            imagePath.setExtension("dds");\
         else if (Platform::isFile(imagePath.getFullPath() + ".jpg"))\
            imagePath.setExtension("jpg");\
      }\
      \
      m##name##AssetId = ImageAsset::getAssetIdByFilename(imagePath.getFullPath());\
   }\
}

#define LOAD_IMAGEASSET(name)\
if (m##name##AssetId != StringTable->EmptyString())\
{\
   S32 assetState = ImageAsset::getAssetById(m##name##AssetId, &m##name##Asset);\
   if (assetState == ImageAsset::Ok )\
   {\
      m##name##Filename = StringTable->EmptyString();\
   }\
   else Con::warnf("Warning: %s::LOAD_IMAGEASSET(%s)-%s", mClassName, m##name##AssetId, ImageAsset::getAssetErrstrn(assetState).c_str());\
}

#define PACK_IMAGEASSET(netconn, name)\
   if (stream->writeFlag(m##name##Asset.notNull()))\
   {\
      NetStringHandle assetIdStr = m##name##Asset.getAssetId();\
      netconn->packNetStringHandleU(stream, assetIdStr);\
   }\
   else\
      stream->writeString(m##name##Filename);

#define UNPACK_IMAGEASSET(netconn, name)\
   if (stream->readFlag())\
   {\
      m##name##AssetId = StringTable->insert(netconn->unpackNetStringHandleU(stream).getString());\
      _set##name(m##name##AssetId);\
   }\
   else\
      m##name##Filename = stream->readSTString();

#pragma endregion

#pragma region Arrayed Asset Macros

//Arrayed Assets
#define DECLARE_IMAGEASSET_ARRAY(className, name, profile, max) public: \
   static const U32 sm##name##Count = max;\
   GFXTexHandle m##name[max] = { NULL };\
   FileName m##name##Filename[max] = { String::EmptyString }; \
   StringTableEntry m##name##AssetId[max] = { StringTable->EmptyString() };\
   AssetPtr<ImageAsset>  m##name##Asset[max] = { NULL };\
   GFXTextureProfile * m##name##Profile = &profile;\
public: \
   const StringTableEntry get##name##File(const U32& index) const { return StringTable->insert(m##name##Filename[index].c_str()); }\
   void set##name##File(const FileName &_in, const U32& index) { m##name##Filename[index] = _in;}\
   const AssetPtr<ImageAsset> & get##name##Asset(const U32& index) const { return m##name##Asset[index]; }\
   void set##name##Asset(const AssetPtr<ImageAsset> &_in, const U32& index) { m##name##Asset[index] = _in;}\
   \
   bool _set##name(StringTableEntry _in, const U32& index)\
   {\
      if(index >= sm##name##Count || index < 0)\
         return false;\
      if (_in == StringTable->EmptyString())\
      {\
         m##name##Filename[index] = String::EmptyString;\
         m##name##AssetId[index] = StringTable->EmptyString();\
         m##name##Asset[index] = NULL;\
         m##name[index].free();\
         m##name[index] = NULL;\
         return true;\
      }\
      \
      if (AssetDatabase.isDeclaredAsset(_in))\
      {\
         m##name##AssetId[index] = _in;\
         \
         U32 assetState = ImageAsset::getAssetById(m##name##AssetId[index], &m##name##Asset[index]);\
         \
         if (ImageAsset::Ok == assetState)\
         {\
            m##name##Filename[index] = StringTable->EmptyString();\
         }\
         else\
         {\
            m##name##Filename[index] = _in;\
            m##name##Asset[index] = NULL;\
         }\
      }\
      else\
      {\
         Torque::Path imagePath = _in;\
         if (imagePath.getExtension() == String::EmptyString)\
         {\
            if (Platform::isFile(imagePath.getFullPath() + ".png"))\
               imagePath.setExtension("png");\
            else if (Platform::isFile(imagePath.getFullPath() + ".dds"))\
               imagePath.setExtension("dds");\
            else if (Platform::isFile(imagePath.getFullPath() + ".jpg"))\
               imagePath.setExtension("jpg");\
         }\
         if (ImageAsset::getAssetByFilename(imagePath.getFullPath(), &m##name##Asset[index]))\
         {\
            m##name##AssetId[index] = m##name##Asset[index].getAssetId();\
            \
            if (ImageAsset::Ok == m##name##Asset[index]->getStatus())\
               m##name##Filename[index] = StringTable->EmptyString();\
         }\
         else\
         {\
            m##name##Filename[index] = _in;\
            m##name##AssetId[index] = StringTable->EmptyString();\
            m##name##Asset[index] = NULL;\
         }\
      }\
      if (get##name(index) != StringTable->EmptyString() && !m##name##Filename[index].equal("texhandle", String::NoCase))\
      {\
         m##name[index].set(get##name(index), m##name##Profile, avar("%s() - mTextureObject (line %d)", __FUNCTION__, __LINE__));\
         return true;\
      }\
      return false;\
   }\
   \
   const StringTableEntry get##name(const U32& index) const\
   {\
      if (m##name##Asset[index] && (m##name##Asset[index]->getImageFileName() != StringTable->EmptyString()))\
         return  Platform::makeRelativePathName(m##name##Asset[index]->getImagePath(), Platform::getMainDotCsDir());\
      else if (m##name##Filename[index].isNotEmpty())\
         return StringTable->insert(Platform::makeRelativePathName(m##name##Filename[index].c_str(), Platform::getMainDotCsDir()));\
      else\
         return StringTable->EmptyString();\
   }\
   GFXTexHandle get##name##Resource(const U32& index) \
   {\
      if(index >= sm##name##Count || index < 0)\
         return nullptr;\
      return m##name[index];\
   }

#define DECLARE_IMAGEASSET_ARRAY_SETGET(className, name)\
   static bool _set##name##Filename(void* obj, const char* index, const char* data)\
   {\
      if (!index) return false;\
      U32 idx = dAtoi(index);\
      if (idx >= sm##name##Count)\
         return false;\
      bool ret = false;\
      className* object = static_cast<className*>(obj);\
      ret = object->_set##name(StringTable->insert(data),idx);\
      return ret;\
   }\
   \
   static bool _set##name##Asset(void* obj, const char* index, const char* data)\
   {\
      if (!index) return false;\
      U32 idx = dAtoi(index);\
      if (idx >= sm##name##Count)\
         return false;\
      bool ret = false;\
      className* object = static_cast<className*>(obj);\
      ret = object->_set##name(StringTable->insert(data),idx);\
      return ret;\
   }

#define DECLARE_IMAGEASSET_ARRAY_NET_SETGET(className, name, bitmask)\
   static bool _set##name##Filename(void* obj, const char* index, const char* data)\
   {\
      if (!index) return false;\
      U32 idx = dAtoi(index);\
      if (idx >= max)\
         return false;\
      bool ret = false;\
      className* object = static_cast<className*>(obj);\
      ret = object->_set##name(StringTable->insert(data),idx);\
      if(ret)\
         object->setMaskBits(bitmask);\
      return ret;\
   }\
   \
   static bool _set##name##Asset(void* obj, const char* index, const char* data)\
   {\
      if (!index) return false;\
      U32 idx = dAtoi(index);\
      if (idx >= max)\
         return false;\
      bool ret = false;\
      className* object = static_cast<className*>(obj);\
      ret = object->_set##name(StringTable->insert(data),idx);\
      if(ret)\
         object->setMaskBits(bitmask);\
      return ret;\
   }

#define DEF_IMAGEASSET_ARRAY_BINDS(className,name)\
DefineEngineMethod(className, get##name, const char*, (S32 index), , "get name")\
{\
   return object->get##name(index); \
}\
DefineEngineMethod(className, get##name##Asset, const char*, (S32 index), , assetText(name, asset reference))\
{\
   if(index >= className::sm##name##Count || index < 0)\
      return "";\
   return object->m##name##AssetId[index]; \
}\
DefineEngineMethod(className, set##name, bool, (const char* map, S32 index), , assetText(name,assignment. first tries asset then flat file.))\
{\
    return object->_set##name(StringTable->insert(map), index);\
}

#define INIT_IMAGEASSET_ARRAY(name, index) \
   m##name##Filename[index] = String::EmptyString; \
   m##name##AssetId[index] = StringTable->EmptyString(); \
   m##name##Asset[index] = NULL;


#define INITPERSISTFIELD_IMAGEASSET_ARRAY(name, arraySize, consoleClass, docs) \
   addProtectedField(#name, TypeImageFilename, Offset(m##name##Filename, consoleClass), _set##name##Filename,&defaultProtectedGetFn, arraySize, assetDoc(name, docs)); \
   addProtectedField(assetText(name, Asset), TypeImageAssetId, Offset(m##name##AssetId, consoleClass), consoleClass::_set##name##Asset, &defaultProtectedGetFn, arraySize, assetDoc(name, asset docs.));

#define CLONE_IMAGEASSET_ARRAY(name, index) \
   m##name##Filename[index] = other.m##name##Filename[index];\
   m##name##AssetId[index] = other.m##name##AssetId[index];\
   m##name##Asset[index] = other.m##name##Asset[index];

#define AUTOCONVERT_IMAGEASSET_ARRAY(name, index)\
if (m##name##Filename[index] != String::EmptyString)\
{\
   PersistenceManager* persistMgr;\
   if (!Sim::findObject("ImageAssetValidator", persistMgr))\
      Con::errorf("ImageAssetValidator not found!");\
   \
   if (persistMgr && m##name##Filename[index] != String::EmptyString && m####name##AssetId[index] == StringTable->EmptyString())\
   {\
      persistMgr->setDirty(this);\
   }\
   if (m##name##Filename[index] != String::EmptyString)\
   {\
      Torque::Path imagePath = m##name##Filename[index];\
      if (imagePath.getPath() == String::EmptyString)\
      {\
         String subPath = Torque::Path(getFilename()).getPath();\
         imagePath.setPath(subPath);\
      }\
      \
      if (imagePath.getExtension() == String::EmptyString)\
      {\
         if (Platform::isFile(imagePath.getFullPath() + ".png"))\
            imagePath.setExtension("png");\
         else if (Platform::isFile(imagePath.getFullPath() + ".dds"))\
            imagePath.setExtension("dds");\
         else if (Platform::isFile(imagePath.getFullPath() + ".jpg"))\
            imagePath.setExtension("jpg");\
      }\
      \
      m##name##AssetId[index] = ImageAsset::getAssetIdByFilename(imagePath.getFullPath());\
   }\
}

#define LOAD_IMAGEASSET_ARRAY(name, index)\
if (m##name##AssetId[index] != StringTable->EmptyString())\
{\
   S32 assetState = ImageAsset::getAssetById(m##name##AssetId[index], &m##name##Asset[index]);\
   if (assetState == ImageAsset::Ok )\
   {\
      m##name##Filename[index] = StringTable->EmptyString();\
   }\
   else Con::warnf("Warning: %s::LOAD_IMAGEASSET(%s)-%s", mClassName, m##name##AssetId[index], ImageAsset::getAssetErrstrn(assetState).c_str());\
}

#define PACK_IMAGEASSET_ARRAY(netconn, name, index)\
   if (stream->writeFlag(m##name##Asset[index].notNull()))\
   {\
      NetStringHandle assetIdStr = m##name##Asset[index].getAssetId();\
      netconn->packNetStringHandleU(stream, assetIdStr);\
   }\
   else\
      stream->writeString(m##name##Filename[index]);

#define UNPACK_IMAGEASSET_ARRAY(netconn, name, index)\
   if (stream->readFlag())\
   {\
      m##name##AssetId[index] = StringTable->insert(netconn->unpackNetStringHandleU(stream).getString());\
      _set##name(m##name##AssetId[index], index);\
   }\
   else\
      m##name##Filename[index] = stream->readSTString();

#pragma endregion

#endif

