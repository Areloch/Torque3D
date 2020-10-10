//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
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
#include "renderProbeMgr.h"
#include "console/consoleTypes.h"
#include "scene/sceneObject.h"
#include "materials/materialManager.h"
#include "scene/sceneRenderState.h"
#include "math/util/sphereMesh.h"
#include "math/util/matrixSet.h"
#include "materials/processedMaterial.h"
#include "renderInstance/renderDeferredMgr.h"
#include "math/mPolyhedron.impl.h"
#include "gfx/gfxTransformSaver.h"
#include "lighting/advanced/advancedLightBinManager.h" //for ssao
#include "gfx/gfxDebugEvent.h"
#include "shaderGen/shaderGenVars.h"
#include "materials/shaderData.h"

#include "gfx/gfxTextureManager.h"
#include "scene/reflectionManager.h"

#include "postFx/postEffect.h"
#include "T3D/lighting/reflectionProbe.h"
#include "T3D/lighting/IBLUtilities.h"

//For our cameraQuery setup
#include "T3D/gameTSCtrl.h"

#include "T3D/Scene.h"

#define TORQUE_GFX_VISUAL_DEBUG //renderdoc debugging

IMPLEMENT_CONOBJECT(RenderProbeMgr);

ConsoleDocClass( RenderProbeMgr, 
   "@brief A render bin which uses object callbacks for rendering.\n\n"
   "This render bin gathers object render instances and calls its delegate "
   "method to perform rendering.  It is used infrequently for specialized "
   "scene objects which perform custom rendering.\n\n"
   "@ingroup RenderBin\n" );

RenderProbeMgr *RenderProbeMgr::smProbeManager = NULL;

bool RenderProbeMgr::smRenderReflectionProbes = true;
F32 RenderProbeMgr::smMaxProbeDrawDistance = 100;

S32 QSORT_CALLBACK AscendingReflectProbeInfluence(const void* a, const void* b)
{
   // Debug Profiling.
   PROFILE_SCOPE(AdvancedLightBinManager_AscendingReflectProbeInfluence);

   // Fetch asset definitions.
   const ProbeRenderInst* pReflectProbeA = (*(ProbeRenderInst**)a);
   const ProbeRenderInst* pReflectProbeB = (*(ProbeRenderInst**)b);
   //sort by score
   return  pReflectProbeA->mScore - pReflectProbeB->mScore;
}

//
//
ProbeRenderInst::ProbeRenderInst() :
   mIsEnabled(true),
   mTransform(true),
   mPriority(1.0f),
   mScore(0.0f),
   mRadius(1.0f),
   mProbeRefOffset(0, 0, 0),
   mProbeRefScale(1,1,1),
   mAtten(0.0),
   mCubemapIndex(0),
   mProbeIdx(0),
   mProbeShapeType(Box)
{
}

ProbeRenderInst::~ProbeRenderInst()
{
}

void ProbeRenderInst::set(const ProbeRenderInst *probeInfo)
{
   mTransform = probeInfo->mTransform;
   mRadius = probeInfo->mRadius;
   mProbeShapeType = probeInfo->mProbeShapeType;
   mBounds = probeInfo->mBounds;
   mScore = probeInfo->mScore;
   mAtten = probeInfo->mAtten;
}

bool ProbeRenderInst::setCubemaps(GFXCubemapHandle prefilter, GFXCubemapHandle irrad)
{
   if (prefilter->isInitialized() && irrad->isInitialized())
   {
      //Update the probe manager with our new texture!
      PROBEMGR->updateProbeTexture(this, prefilter, irrad);

      return true;
   }
   else
   {
      return false;
   }
}

bool ProbeRenderInst::setCubemaps(FileName prefilPath, FileName irradPath)
{
   if (!Platform::isFile(prefilPath) || !Platform::isFile(irradPath))
      return false;

   CubemapData* prefilterMapData = PROBEMGR->getPrefilterMapData();
   prefilterMapData->setCubemapFile(FileName(prefilPath));
   prefilterMapData->updateFaces();

   if (!prefilterMapData->mCubemap->isInitialized())
   {
      Con::errorf("ProbeRenderInst::setCubemaps() - Unable to load baked prefilter map at %s", prefilPath.c_str());
      return false;
   }

   CubemapData* irradMapData = PROBEMGR->getIrradianceMapData();
   irradMapData->setCubemapFile(FileName(irradPath));
   irradMapData->updateFaces();

   if (!irradMapData->mCubemap->isInitialized())
   {
      Con::errorf("ProbeRenderInst::setCubemaps() - Unable to load baked iraddiance map at %s", irradPath.c_str());
      return false;
   }

   return setCubemaps(prefilterMapData->mCubemap, irradMapData->mCubemap);
}

bool ProbeRenderInst::setCubemap(StringTableEntry cubemapName)
{
   String path = Con::getVariable("$pref::ReflectionProbes::CurrentLevelPath", "levels/");

   char prefilterFileName[256];
   dSprintf(prefilterFileName, 256, "%s%s_Prefilter.dds", path.c_str(), cubemapName);

   char irradFileName[256];
   dSprintf(irradFileName, 256, "%s_Irradiance.dds", path.c_str(), cubemapName);

   //Do some trickery here so if we're trying to load a cubemap that doesn't have the valid full names
   if (!Platform::isFile(prefilterFileName) || !Platform::isFile(irradFileName))
   {
      CubemapData* staticCubemap;

      //If we are missing either of the files, just re-run the bake
      Sim::findObject(cubemapName, staticCubemap);

      if (!staticCubemap)
      {
         Con::errorf("ProbeRenderInst::setCubemap() - unable to find static cubemap file!");
         return false;
      }

      if (staticCubemap->mCubemap == nullptr)
      {
         staticCubemap->createMap();
         staticCubemap->updateFaces();
      }

      CubemapData* prefilterMapData = PROBEMGR->getPrefilterMapData();
      CubemapData* irradMapData = PROBEMGR->getIrradianceMapData();

      if (PROBEMGR->getUseHDRCaptures())
      {
         prefilterMapData->mCubemap->initDynamic(PROBEMGR->getPrefilterSize(), GFXFormatR16G16B16A16F);
         prefilterMapData->mCubemap->initDynamic(PROBEMGR->getPrefilterSize(), GFXFormatR16G16B16A16F);
      }
      else
      {
         prefilterMapData->mCubemap->initDynamic(PROBEMGR->getPrefilterSize(), GFXFormatR8G8B8A8);
         prefilterMapData->mCubemap->initDynamic(PROBEMGR->getPrefilterSize(), GFXFormatR8G8B8A8);
      }

      GFXTextureTargetRef renderTarget = GFX->allocRenderToTextureTarget(false);

      IBLUtilities::GenerateIrradianceMap(renderTarget, staticCubemap->mCubemap, irradMapData->mCubemap);
      IBLUtilities::GeneratePrefilterMap(renderTarget, staticCubemap->mCubemap, PROBEMGR->getPrefilterMipLevels(), prefilterMapData->mCubemap);

      IBLUtilities::SaveCubeMap(irradFileName, irradMapData->mCubemap);
      IBLUtilities::SaveCubeMap(prefilterFileName, prefilterMapData->mCubemap);

      return setCubemaps(prefilterMapData->mCubemap, irradMapData->mCubemap);
   }
   else
   {
      return setCubemaps(FileName(prefilterFileName), FileName(irradFileName));
   }
}

//
//
ProbeShaderConstants::ProbeShaderConstants()
   : mInit(false),
   mShader(NULL),
   mProbePositionSC(NULL),
   mProbeRefPosSC(NULL),
   mRefBoxMinSC(NULL),
   mRefBoxMaxSC(NULL),
   mProbeConfigDataSC(NULL),
   mProbeSpecularCubemapSC(NULL),
   mProbeIrradianceCubemapSC(NULL),
   mProbeCountSC(NULL),
   mBRDFTextureMap(NULL),
   mSkylightCubemapIdxSC(NULL),
   mWorldToObjArraySC(NULL)
{
}

ProbeShaderConstants::~ProbeShaderConstants()
{
   if (mShader.isValid())
   {
      mShader->getReloadSignal().remove(this, &ProbeShaderConstants::_onShaderReload);
      mShader = NULL;
   }
}

void ProbeShaderConstants::init(GFXShader* shader)
{
   if (mShader.getPointer() != shader)
   {
      if (mShader.isValid())
         mShader->getReloadSignal().remove(this, &ProbeShaderConstants::_onShaderReload);

      mShader = shader;
      mShader->getReloadSignal().notify(this, &ProbeShaderConstants::_onShaderReload);
   }
   
   //Reflection Probes
   mProbePositionSC = shader->getShaderConstHandle(ShaderGenVars::probePosition);
   mProbeRefPosSC = shader->getShaderConstHandle(ShaderGenVars::probeRefPos);
   mRefBoxMinSC = shader->getShaderConstHandle(ShaderGenVars::refBoxMin);
   mRefBoxMaxSC = shader->getShaderConstHandle(ShaderGenVars::refBoxMax);
   mWorldToObjArraySC = shader->getShaderConstHandle(ShaderGenVars::worldToObjArray);
   mProbeConfigDataSC = shader->getShaderConstHandle(ShaderGenVars::probeConfigData);
   mProbeSpecularCubemapSC = shader->getShaderConstHandle(ShaderGenVars::specularCubemapAR);
   mProbeIrradianceCubemapSC = shader->getShaderConstHandle(ShaderGenVars::irradianceCubemapAR);
   mProbeCountSC = shader->getShaderConstHandle(ShaderGenVars::probeCount);

   mBRDFTextureMap = shader->getShaderConstHandle(ShaderGenVars::BRDFTextureMap);

   mSkylightCubemapIdxSC = shader->getShaderConstHandle(ShaderGenVars::skylightCubemapIdx);

   mInit = true;
}

bool ProbeShaderConstants::isValid()
{
   if (mProbePositionSC->isValid() ||
      mProbeConfigDataSC->isValid() ||
      mRefBoxMinSC->isValid() ||
      mRefBoxMaxSC->isValid() ||
      mProbeSpecularCubemapSC->isValid() ||
      mProbeIrradianceCubemapSC->isValid())
      return true;

   return false;
}

void ProbeShaderConstants::_onShaderReload()
{
   if (mShader.isValid())
      init(mShader);
}

//
//
RenderProbeMgr::RenderProbeMgr()
: RenderBinManager(RenderPassManager::RIT_Probes, 1.0f, 1.0f),
   mLastShader(nullptr),
   mLastConstants(nullptr),
   mHasSkylight(false),
   mSkylightCubemapIdx(-1),
   mCubeMapCount(0),
   mDefaultSkyLight(nullptr),
   mBakeRenderTarget(nullptr)
{
   mEffectiveProbeCount = 0;
   mMipCount = 0;

   mProbeArrayEffect = nullptr;

   smProbeManager = this;

   mCubeMapCount = 0;
   mCubeSlotCount = PROBE_ARRAY_SLOT_BUFFER_SIZE;

   for (U32 i = 0; i < PROBE_MAX_COUNT; i++)
   {
      mCubeMapSlots[i] = false;
   }

   mPrefilterSize = 64;
   mPrefilterMipLevels = mLog2(F32(mPrefilterSize)) + 1;

   mPrefilterMapData = nullptr;
   mIrradianceMapData = nullptr;
}

RenderProbeMgr::RenderProbeMgr(RenderInstType riType, F32 renderOrder, F32 processAddOrder)
 : RenderBinManager(riType, renderOrder, processAddOrder)
{
   mCubeMapCount = 0;
   dMemset(mCubeMapSlots, false, sizeof(mCubeMapSlots));
   mCubeSlotCount = PROBE_ARRAY_SLOT_BUFFER_SIZE;
   mDefaultSkyLight = nullptr;
   mEffectiveProbeCount = 0;
   mHasSkylight = false;
   mSkylightCubemapIdx = -1;
   mLastConstants = nullptr;
   mMipCount = 0;

   mUseHDRCaptures = true;

   mPrefilterSize = 64;
   mPrefilterMipLevels = mLog2(F32(mPrefilterSize)) + 1;

   mPrefilterMapData = nullptr;
   mIrradianceMapData = nullptr;
}

RenderProbeMgr::~RenderProbeMgr()
{
   mLastShader = NULL;
   mLastConstants = NULL;

   for (ProbeConstantMap::Iterator i = mConstantLookup.begin(); i != mConstantLookup.end(); i++)
   {
      if (i->value)
         SAFE_DELETE(i->value);
   }
   mConstantLookup.clear();
}

bool RenderProbeMgr::onAdd()
{
   if (!Parent::onAdd())
      return false;

   mPrefilterMapData = new CubemapData();
   mPrefilterMapData->registerObject();
   mPrefilterMapData->createMap();

   mIrradianceMapData = new CubemapData();
   mIrradianceMapData->registerObject();
   mIrradianceMapData->createMap();

   mIrradianceArray = GFXCubemapArrayHandle(GFX->createCubemapArray());
   mPrefilterArray = GFXCubemapArrayHandle(GFX->createCubemapArray());

   //pre-allocate a few slots
   mIrradianceArray->init(PROBE_ARRAY_SLOT_BUFFER_SIZE, PROBE_IRRAD_SIZE, PROBE_FORMAT);
   mPrefilterArray->init(PROBE_ARRAY_SLOT_BUFFER_SIZE, PROBE_PREFILTER_SIZE, PROBE_FORMAT);
   mCubeSlotCount = PROBE_ARRAY_SLOT_BUFFER_SIZE;

   //create our own default default skylight
   mDefaultSkyLight = new ProbeRenderInst;
   mDefaultSkyLight->mProbeShapeType = ProbeRenderInst::Skylight;
   mDefaultSkyLight->mIsEnabled = false;

   //TODO: fix up the default skylight
   /*String defaultIrradMapPath = GFXTextureManager::getDefaultIrradianceCubemapPath();
   if (!mDefaultSkyLight->mIrradianceCubemap.set(defaultIrradMapPath))
   {
      Con::errorf("RenderProbeMgr::onAdd: Failed to load default irradiance cubemap");
      return false;
   }

   String defaultPrefilterPath = GFXTextureManager::getDefaultPrefilterCubemapPath();
   if (!mDefaultSkyLight->mPrefilterCubemap.set(defaultPrefilterPath))
   {
      Con::errorf("RenderProbeMgr::onAdd: Failed to load default prefilter cubemap");
      return false;
   }*/

   String brdfTexturePath = GFXTextureManager::getBRDFTexturePath();
   if (!mBRDFTexture.set(brdfTexturePath, &GFXTexturePersistentSRGBProfile, "BRDFTexture"))
   {
      Con::errorf("RenderProbeMgr::onAdd: Failed to load BRDF Texture");
      return false;
   } 

   return true;
}

void RenderProbeMgr::onRemove()
{
   mPrefilterMapData->deleteObject();
   mIrradianceMapData->deleteObject();

   if(mCubeReflector.isEnabled())
      mCubeReflector.unregisterReflector();

   Parent::onRemove();
}

void RenderProbeMgr::initPersistFields()
{
   Parent::initPersistFields();
}

void RenderProbeMgr::consoleInit()
{
   Parent::consoleInit();

   // Vars for debug rendering while the RoadEditor is open, only used if smEditorOpen is true.
   Con::addVariable("$pref::maxProbeDrawDistance", TypeF32, &RenderProbeMgr::smMaxProbeDrawDistance, "Max distance for reflection probes to render.\n");
}

void RenderProbeMgr::registerProbe(ProbeRenderInst* newProbe)
{
   //Can't have over the probe limit
   if (mRegisteredProbes.size() + 1 >= PROBE_MAX_COUNT)
      return;

   mRegisteredProbes.push_back(newProbe);

   newProbe->mProbeIdx = mRegisteredProbes.size() - 1;

   const U32 cubeIndex = _findNextEmptyCubeSlot();
   if (cubeIndex == INVALID_CUBE_SLOT)
   {
      Con::warnf("RenderProbeMgr::addProbe: Invalid cubemap slot.");
      return;
   }

   //check if we need to resize the cubemap array
   if (cubeIndex >= mCubeSlotCount)
   {
      //alloc temp array handles
      GFXCubemapArrayHandle irr = GFXCubemapArrayHandle(GFX->createCubemapArray());
      GFXCubemapArrayHandle prefilter = GFXCubemapArrayHandle(GFX->createCubemapArray());

      irr->init(mCubeSlotCount + PROBE_ARRAY_SLOT_BUFFER_SIZE, PROBE_IRRAD_SIZE, PROBE_FORMAT);
      prefilter->init(mCubeSlotCount + PROBE_ARRAY_SLOT_BUFFER_SIZE, PROBE_PREFILTER_SIZE, PROBE_FORMAT);

      mIrradianceArray->copyTo(irr);
      mPrefilterArray->copyTo(prefilter);

      //assign the temp handles to the new ones, this will destroy the old ones as well
      mIrradianceArray = irr;
      mPrefilterArray = prefilter;

      mCubeSlotCount += PROBE_ARRAY_SLOT_BUFFER_SIZE;
   }

   newProbe->mCubemapIndex = cubeIndex;

   //mark cubemap slot as taken
   mCubeMapSlots[cubeIndex] = true;
   mCubeMapCount++;

#ifdef TORQUE_DEBUG
   Con::warnf("RenderProbeMgr::registerProbe: Registered probe %u to cubeIndex %u", newProbe->mProbeIdx, cubeIndex);
#endif
}

void RenderProbeMgr::unregisterProbe(U32 probeIdx)
{
   //Mostly for consolidation, but also lets us sanity check or prep any other data we need for rendering this in one place at time of flagging for render
   if (probeIdx >= mRegisteredProbes.size())
      return;

   if (mRegisteredProbes[probeIdx]->mCubemapIndex == INVALID_CUBE_SLOT)
      return;

   //mark cubemap slot as available now
   mCubeMapSlots[mRegisteredProbes[probeIdx]->mCubemapIndex] = false;
   mCubeMapCount--;

   mRegisteredProbes.erase(probeIdx);

   //recalculate all the probe's indicies just to be sure
   for (U32 i = 0; i < mRegisteredProbes.size(); i++)
   {
      mRegisteredProbes[i]->mProbeIdx = i;
   }
}

void RenderProbeMgr::submitProbe(const ProbeRenderInst& newProbe)
{
   mActiveProbes.push_back(newProbe);
}

//
//
PostEffect* RenderProbeMgr::getProbeArrayEffect()
{
   if (!mProbeArrayEffect)
   {
      mProbeArrayEffect = dynamic_cast<PostEffect*>(Sim::findObject("reflectionProbeArrayPostFX"));

      if (!mProbeArrayEffect)
         return nullptr;
   }
   return mProbeArrayEffect;
}

void RenderProbeMgr::updateProbeTexture(ProbeRenderInst* probeInfo, GFXCubemapHandle prefilterCubemap, GFXCubemapHandle irradianceCubemap)
{
   if (irradianceCubemap.isNull() || !irradianceCubemap->isInitialized())
   {
      Con::errorf("RenderProbeMgr::updateProbeTexture() - tried to update a probe's texture with an invalid or uninitialized irradiance map!");
      return;
   }

   if (prefilterCubemap.isNull() || !prefilterCubemap->isInitialized())
   {
      Con::errorf("RenderProbeMgr::updateProbeTexture() - tried to update a probe's texture with an invalid or uninitialized specular map!");
      return;
   }

   const U32 cubeIndex = probeInfo->mCubemapIndex;
   mIrradianceArray->updateTexture(irradianceCubemap, cubeIndex);
   mPrefilterArray->updateTexture(prefilterCubemap, cubeIndex);

#ifdef TORQUE_DEBUG
   Con::warnf("UpdatedProbeTexture - probeIdx: %u on cubeIndex %u, Irrad validity: %d, Prefilter validity: %d", probeInfo->mProbeIdx, cubeIndex,
      irradianceCubemap->isInitialized(), prefilterCubemap->isInitialized());
#endif
}

void RenderProbeMgr::reloadTextures()
{
   //TODO Fix up probe reloading
   /*U32 probeCount = mRegisteredProbes.size();
   for (U32 i = 0; i < probeCount; i++)
   {
      updateProbeTexture(&mRegisteredProbes[i]);
   }

   mProbesDirty = true;*/
}

void RenderProbeMgr::_setupPerFrameParameters(const SceneRenderState *state)
{
   PROFILE_SCOPE(RenderProbeMgr_SetupPerFrameParameters);

   mProbeData = ProbeDataSet(PROBE_MAX_FRAME);

   getBestProbes(state->getCameraPosition(), &mProbeData);
}

ProbeShaderConstants* RenderProbeMgr::getProbeShaderConstants(GFXShaderConstBuffer* buffer)
{
   if (!buffer)
      return NULL;

   PROFILE_SCOPE(ProbeManager_GetProbeShaderConstants);

   GFXShader* shader = buffer->getShader();

   // Check to see if this is the same shader, we'll get hit repeatedly by
   // the same one due to the render bin loops.
   if (mLastShader.getPointer() != shader)
   {
      ProbeConstantMap::Iterator iter = mConstantLookup.find(shader);
      if (iter != mConstantLookup.end())
      {
         mLastConstants = iter->value;
      }
      else
      {
         ProbeShaderConstants* psc = new ProbeShaderConstants();
         mConstantLookup[shader] = psc;

         mLastConstants = psc;
      }

      // Set our new shader
      mLastShader = shader;
   }

   // Make sure that our current lighting constants are initialized
   if (mLastConstants && !mLastConstants->mInit)
      mLastConstants->init(shader);

   return mLastConstants;
}

void RenderProbeMgr::setupSGData(SceneData& data, const SceneRenderState* state, LightInfo* light)
{
   //ensure they're sorted for forward rendering
   mActiveProbes.sort(_probeScoreCmp);
}

void RenderProbeMgr::_update4ProbeConsts(const SceneData &sgData,
   MatrixSet &matSet,
   ProbeShaderConstants *probeShaderConsts,
   GFXShaderConstBuffer *shaderConsts)
{
   PROFILE_SCOPE(ProbeManager_Update4ProbeConsts);

   // Skip over gathering lights if we don't have to!
   if (probeShaderConsts->isValid())
   {
      PROFILE_SCOPE(ProbeManager_Update4ProbeConsts_setProbes);

      const U32 MAX_FORWARD_PROBES = 4;
      ProbeDataSet probeSet(MAX_FORWARD_PROBES);

      matSet.restoreSceneViewProjection();

      getBestProbes(sgData.objTrans->getPosition(), &probeSet);

      static AlignedArray<Point4F> probePositionAlignedArray(probeSet.maxProbeCount, sizeof(Point4F));
      static AlignedArray<Point4F> refBoxMinAlignedArray(probeSet.maxProbeCount, sizeof(Point4F));
      static AlignedArray<Point4F> refBoxMaxAlignedArray(probeSet.maxProbeCount, sizeof(Point4F));
      static AlignedArray<Point4F> probeRefPositionAlignedArray(probeSet.maxProbeCount, sizeof(Point4F));
      static AlignedArray<Point4F> probeConfigAlignedArray(probeSet.maxProbeCount, sizeof(Point4F));

      for (U32 i = 0; i < probeSet.maxProbeCount; i++)
      {
         probePositionAlignedArray[i] = probeSet.probePositionArray[i];
         probeRefPositionAlignedArray[i] = probeSet.probeRefPositionArray[i];
         refBoxMinAlignedArray[i] = probeSet.refBoxMinArray[i];
         refBoxMaxAlignedArray[i] = probeSet.refBoxMaxArray[i];
         probeConfigAlignedArray[i] = probeSet.probeConfigArray[i];
      }

      shaderConsts->setSafe(probeShaderConsts->mProbeCountSC, (S32)probeSet.effectiveProbeCount);

      shaderConsts->setSafe(probeShaderConsts->mProbePositionSC, probePositionAlignedArray);
      shaderConsts->setSafe(probeShaderConsts->mProbeRefPosSC, probeRefPositionAlignedArray);

      if(probeShaderConsts->isValid())
         shaderConsts->set(probeShaderConsts->mWorldToObjArraySC, probeSet.probeWorldToObjArray.address(), probeSet.effectiveProbeCount, GFXSCT_Float4x4);

      shaderConsts->setSafe(probeShaderConsts->mRefBoxMinSC, refBoxMinAlignedArray);
      shaderConsts->setSafe(probeShaderConsts->mRefBoxMaxSC, refBoxMaxAlignedArray);
      shaderConsts->setSafe(probeShaderConsts->mProbeConfigDataSC, probeConfigAlignedArray);

      shaderConsts->setSafe(probeShaderConsts->mSkylightCubemapIdxSC, (float)probeSet.skyLightIdx);

      if(probeShaderConsts->mBRDFTextureMap->getSamplerRegister() != -1 && mBRDFTexture.isValid())
         GFX->setTexture(probeShaderConsts->mBRDFTextureMap->getSamplerRegister(), mBRDFTexture);

      if(probeShaderConsts->mProbeSpecularCubemapSC->getSamplerRegister() != -1)
         GFX->setCubeArrayTexture(probeShaderConsts->mProbeSpecularCubemapSC->getSamplerRegister(), mPrefilterArray);
      if(probeShaderConsts->mProbeIrradianceCubemapSC->getSamplerRegister() != -1)
         GFX->setCubeArrayTexture(probeShaderConsts->mProbeIrradianceCubemapSC->getSamplerRegister(), mIrradianceArray);
   }
}

S32 QSORT_CALLBACK RenderProbeMgr::_probeScoreCmp(const ProbeRenderInst* a, const  ProbeRenderInst* b)
{
   F32 diff = a->getScore() - b->getScore();
   return diff < 0 ? 1 : diff > 0 ? -1 : 0;
}

void RenderProbeMgr::getBestProbes(const Point3F& objPosition, ProbeDataSet* probeDataSet)
{
   PROFILE_SCOPE(ProbeManager_getBestProbes);

   //Array rendering
   U32 probeCount = mActiveProbes.size();

   Vector<S8> bestPickProbes;
   bestPickProbes.setSize(probeDataSet->maxProbeCount);
   bestPickProbes.fill(-1);

   probeDataSet->effectiveProbeCount = 0;
   for (U32 i = 0; i < probeCount; i++)
   {
      if (probeDataSet->effectiveProbeCount >= probeDataSet->maxProbeCount)
         break;

      const ProbeRenderInst& curEntry = mActiveProbes[i];
      if (!curEntry.mIsEnabled)
         continue;

      if (curEntry.mProbeShapeType != ProbeRenderInst::Skylight)
      {
         bestPickProbes[probeDataSet->effectiveProbeCount] = i;
         probeDataSet->effectiveProbeCount++;
      }
      else
      {
         probeDataSet->skyLightIdx = curEntry.mCubemapIndex;
      }
   }

   //Grab our best probe picks
   for (U32 i = 0; i < bestPickProbes.size(); i++)
   {
      if (bestPickProbes[i] == -1)
         continue;

      const ProbeRenderInst& curEntry = mActiveProbes[bestPickProbes[i]];

      Point3F refPos = curEntry.getPosition() + curEntry.mProbeRefOffset;
      Point3F refBoxMin = refPos - curEntry.mProbeRefScale * curEntry.getTransform().getScale();
      Point3F refBoxMax = refPos + curEntry.mProbeRefScale * curEntry.getTransform().getScale();

      probeDataSet->probeWorldToObjArray[i] = curEntry.getTransform();

      probeDataSet->probePositionArray[i] = curEntry.getPosition();
      probeDataSet->probeRefPositionArray[i] = curEntry.mProbeRefOffset;

      probeDataSet->refBoxMinArray[i] = Point4F(refBoxMin.x, refBoxMin.y, refBoxMin.z, 0);
      probeDataSet->refBoxMaxArray[i] = Point4F(refBoxMax.x, refBoxMax.y, refBoxMax.z, 0);
      probeDataSet->probeConfigArray[i] = Point4F(curEntry.mProbeShapeType,
         curEntry.mRadius,
         curEntry.mAtten,
         curEntry.mCubemapIndex);
   }
}

void RenderProbeMgr::getProbeTextureData(ProbeTextureArrayData* probeTextureSet)
{
   probeTextureSet->BRDFTexture = mBRDFTexture;
   probeTextureSet->prefilterArray = mPrefilterArray;
   probeTextureSet->irradianceArray = mIrradianceArray;
}

void RenderProbeMgr::setProbeInfo(ProcessedMaterial *pmat,
   const Material *mat,
   const SceneData &sgData,
   const SceneRenderState *state,
   U32 pass,
   GFXShaderConstBuffer *shaderConsts)
{

   // Skip this if we're rendering from the deferred bin.
   if (sgData.binType == SceneData::DeferredBin)
      return;

   PROFILE_SCOPE(ProbeManager_setProbeInfo);

   ProbeShaderConstants *psc = getProbeShaderConstants(shaderConsts);

   // NOTE: If you encounter a crash from this point forward
   // while setting a shader constant its probably because the
   // mConstantLookup has bad shaders/constants in it.
   //
   // This is a known crash bug that can occur if materials/shaders
   // are reloaded and the light manager is not reset.
   //
   // We should look to fix this by clearing the table.
   MatrixSet matSet = state->getRenderPass()->getMatrixSet();

   // Update the forward shading light constants.
   _update4ProbeConsts(sgData, matSet, psc, shaderConsts);
}

//-----------------------------------------------------------------------------
// render objects
//-----------------------------------------------------------------------------
void RenderProbeMgr::render( SceneRenderState *state )
{
   if (getProbeArrayEffect() == nullptr)
      return;

   GFXDEBUGEVENT_SCOPE(RenderProbeMgr_render, ColorI::WHITE);

   //Sort the active probes
   mActiveProbes.sort(_probeScoreCmp);

   // Initialize and set the per-frame data
   _setupPerFrameParameters(state);

   // Early out if nothing to draw.
   if (!RenderProbeMgr::smRenderReflectionProbes || (!state->isDiffusePass() && !state->isReflectPass()) || (mProbeData.effectiveProbeCount == 0 && mSkylightCubemapIdx == -1))
   {
      getProbeArrayEffect()->setSkip(true);
      return;
   }

   GFXTransformSaver saver;

   //Visualization
   String useDebugAtten = Con::getVariable("$Probes::showAttenuation", "0");
   mProbeArrayEffect->setShaderMacro("DEBUGVIZ_ATTENUATION", useDebugAtten);

   String useDebugSpecCubemap = Con::getVariable("$Probes::showSpecularCubemaps", "0");
   mProbeArrayEffect->setShaderMacro("DEBUGVIZ_SPECCUBEMAP", useDebugSpecCubemap);

   String useDebugDiffuseCubemap = Con::getVariable("$Probes::showDiffuseCubemaps", "0");
   mProbeArrayEffect->setShaderMacro("DEBUGVIZ_DIFFCUBEMAP", useDebugDiffuseCubemap);

   String useDebugContrib = Con::getVariable("$Probes::showProbeContrib", "0");
   mProbeArrayEffect->setShaderMacro("DEBUGVIZ_CONTRIB", useDebugContrib);

   if(mProbeData.skyLightIdx != -1 && mProbeData.effectiveProbeCount == 0)
      mProbeArrayEffect->setShaderMacro("SKYLIGHT_ONLY", "1");
   else
      mProbeArrayEffect->setShaderMacro("SKYLIGHT_ONLY", "0");

   //ssao mask
   if (AdvancedLightBinManager::smUseSSAOMask)
   {
      //find ssaoMask
      NamedTexTargetRef ssaoTarget = NamedTexTarget::find("ssaoMask");
      GFXTextureObject* pTexObj = ssaoTarget->getTexture();
      if (pTexObj)
      {
         mProbeArrayEffect->setShaderMacro("USE_SSAO_MASK");
         mProbeArrayEffect->setTexture(6, pTexObj);
      }
   }
   else
   {
      mProbeArrayEffect->setTexture(6, GFXTexHandle(NULL)); 
   }
   
   mProbeArrayEffect->setTexture(3, mBRDFTexture);
   mProbeArrayEffect->setCubemapArrayTexture(4, mPrefilterArray);
   mProbeArrayEffect->setCubemapArrayTexture(5, mIrradianceArray);

   mProbeArrayEffect->setShaderConst("$numProbes", (S32)mProbeData.effectiveProbeCount);
   mProbeArrayEffect->setShaderConst("$skylightCubemapIdx", (S32)mProbeData.skyLightIdx);

   mProbeArrayEffect->setShaderConst("$cubeMips", (float)mMipCount);

   //also set up some colors
   Vector<Point4F> contribColors;

   contribColors.setSize(mProbeData.effectiveProbeCount);

   if (mProbeData.effectiveProbeCount != 0)
   {
      if (useDebugContrib == String("1"))
      {
         MRandomLCG RandomGen;
         RandomGen.setSeed(mProbeData.effectiveProbeCount);

         for (U32 i = 0; i < mProbeData.effectiveProbeCount; i++)
         {
            //we're going to cheat here a little for consistent debugging behavior. The first 3 probes will always have R G and then B for their colors, every other will be random
            if (i == 0)
               contribColors[i] = Point4F(1, 0, 0, 1);
            else if (i == 1)
               contribColors[i] = Point4F(0, 1, 0, 1);
            else if (i == 2)
               contribColors[i] = Point4F(0, 0, 1, 1);
            else
               contribColors[i] = Point4F(RandomGen.randF(0, 1), RandomGen.randF(0, 1), RandomGen.randF(0, 1), 1);
         }
      }
   }

   mProbeArrayEffect->setShaderConst("$probeContribColors", contribColors);

   mProbeArrayEffect->setShaderConst("$inProbePosArray", mProbeData.probePositionArray);
   mProbeArrayEffect->setShaderConst("$inRefPosArray", mProbeData.probeRefPositionArray);
   mProbeArrayEffect->setShaderConst("$worldToObjArray", mProbeData.probeWorldToObjArray);
   mProbeArrayEffect->setShaderConst("$refBoxMinArray", mProbeData.refBoxMinArray);
   mProbeArrayEffect->setShaderConst("$refBoxMaxArray", mProbeData.refBoxMaxArray);
   mProbeArrayEffect->setShaderConst("$probeConfigData", mProbeData.probeConfigArray);

   // Make sure the effect is gonna render.
   getProbeArrayEffect()->setSkip(false);

   mActiveProbes.clear();
}

void RenderProbeMgr::bakeProbe(ReflectionProbe* probe, bool writeFiles)
{
   GFXDEBUGEVENT_SCOPE(RenderProbeMgr_Bake, ColorI::WHITE);

   Con::warnf("RenderProbeMgr::bakeProbe() - Beginning bake!");
   U32 startMSTime = Platform::getRealMilliseconds();

   String path = Con::getVariable("$pref::ReflectionProbes::CurrentLevelPath", "levels/");
   U32 resolution = Con::getIntVariable("$pref::ReflectionProbes::BakeResolution", 64);
   U32 prefilterMipLevels = mLog2(F32(resolution))+1;
   bool renderWithProbes = Con::getIntVariable("$pref::ReflectionProbes::RenderWithProbes", false);

   ReflectionProbe* clientProbe = nullptr;

   if (probe->isClientObject())
      clientProbe = probe;
   else
      clientProbe = static_cast<ReflectionProbe*>(probe->getClientObject());

   if (clientProbe == nullptr)
      return;

   String probePrefilterPath = clientProbe->getPrefilterMapPath();
   String probeIrradPath = clientProbe->getIrradianceMapPath();

   if (clientProbe->mReflectionModeType != ReflectionProbe::DynamicCubemap)
   {
      //Prep our bake path
      if (probePrefilterPath.isEmpty() || probeIrradPath.isEmpty())
      {
         Con::errorf("RenderProbeMgr::bake() - Unable to bake our captures because probe doesn't have a path set");
         return;
      }
   }

   // Save the current transforms so we can restore
   // it for child control rendering below.
   GFXTransformSaver saver;

   bool probeRenderState = RenderProbeMgr::smRenderReflectionProbes;

   F32 farPlane = 1000.0f;

   ReflectorDesc reflDesc;
   reflDesc.texSize = resolution;
   reflDesc.farDist = farPlane;
   reflDesc.detailAdjust = 1;
   reflDesc.objectTypeMask = probe->mProbeShapeType == ProbeRenderInst::ProbeShapeType::Skylight ? SKYLIGHT_CAPTURE_TYPEMASK : REFLECTION_PROBE_CAPTURE_TYPEMASK;

   if(!mCubeReflector.isEnabled())
      mCubeReflector.registerReflector(probe, &reflDesc);

   ReflectParams reflParams;

   //need to get the query somehow. Likely do some sort of get function to fetch from the guiTSControl that's active
   CameraQuery query; //need to get the last cameraQuery
   query.fov = 90; //90 degree slices for each of the 6 sides
   query.nearPlane = 0.1f;
   query.farPlane = farPlane;
   query.headMatrix = MatrixF();
   query.cameraMatrix = clientProbe->getTransform();

   Frustum culler;
   culler.set(false,
      query.fov,
      1.0f,
      query.nearPlane,
      query.farPlane,
      query.cameraMatrix);

   S32 stereoTarget = GFX->getCurrentStereoTarget();

   Point2I maxRes(2048, 2048); //basically a boundary so we don't go over this and break stuff

   reflParams.culler = culler;
   reflParams.eyeId = stereoTarget;
   reflParams.query = &query;
   reflParams.startOfUpdateMs = startMSTime;
   reflParams.viewportExtent = maxRes;

   if (!renderWithProbes)
      RenderProbeMgr::smRenderReflectionProbes = false;

   GFXFormat reflectFormat;

   if (mUseHDRCaptures)
      reflectFormat = GFXFormatR16G16B16A16F;
   else
      reflectFormat = GFXFormatR8G8B8A8;
   const GFXFormat oldRefFmt = REFLECTMGR->getReflectFormat();
   REFLECTMGR->setReflectFormat(reflectFormat);
   mCubeReflector.updateReflection(reflParams);

   //Now, save out the maps
   if (mCubeReflector.getCubemap())
   {
      CubemapData* prefilterMapData = getPrefilterMapData();
      CubemapData* irradMapData = getPrefilterMapData();

      //Prep it with whatever resolution we've dictated for our bake
      irradMapData->mCubemap->initDynamic(resolution, reflectFormat);
      prefilterMapData->mCubemap->initDynamic(resolution, reflectFormat);

      if (mBakeRenderTarget == nullptr)
         mBakeRenderTarget = GFX->allocRenderToTextureTarget(false);
      else
         mBakeRenderTarget->resurrect();

      IBLUtilities::GenerateIrradianceMap(mBakeRenderTarget, mCubeReflector.getCubemap(), irradMapData->mCubemap);
      IBLUtilities::GeneratePrefilterMap(mBakeRenderTarget, mCubeReflector.getCubemap(), prefilterMipLevels, prefilterMapData->mCubemap);

      U32 endMSTime = Platform::getRealMilliseconds();
      F32 diffTime = F32(endMSTime - startMSTime);
      Con::warnf("RenderProbeMgr::bake() - Finished Capture! Took %g milliseconds", diffTime);

      if (writeFiles)
      {
         Con::warnf("RenderProbeMgr::bake() - Beginning save now!");

         IBLUtilities::SaveCubeMap(clientProbe->getIrradianceMapPath(), irradMapData->mCubemap);
         IBLUtilities::SaveCubeMap(clientProbe->getPrefilterMapPath(), prefilterMapData->mCubemap);
      }

      mBakeRenderTarget->zombify();

      probe->mCubemapDirty = true;
   }
   else
   {
      Con::errorf("RenderProbeMgr::bake() - Didn't generate a valid scene capture cubemap, unable to generate prefilter and irradiance maps!");
   }


   if (!renderWithProbes)
      RenderProbeMgr::smRenderReflectionProbes = probeRenderState;

   U32 endMSTime = Platform::getRealMilliseconds();
   F32 diffTime = F32(endMSTime - startMSTime);

   probe->setMaskBits(-1);

   Con::warnf("RenderProbeMgr::bake() - Finished bake! Took %g milliseconds", diffTime);
   REFLECTMGR->setReflectFormat(oldRefFmt);
}

void RenderProbeMgr::bakeProbes()
{
   Vector<ReflectionProbe*> probes;

   Scene::getRootScene()->findObjectByType<ReflectionProbe>(probes);

   for (U32 i = 0; i < probes.size(); i++)
   {
      if (probes[i]->isClientObject())
         continue;

      bakeProbe(probes[i], true);
   }
}

DefineEngineMethod(RenderProbeMgr, bakeProbe, void, (ReflectionProbe* probe), (nullAsType< ReflectionProbe*>()),
   "@brief Bakes the cubemaps for a reflection probe\n\n.")
{
   if(probe != nullptr)
      object->bakeProbe(probe);
}

DefineEngineMethod(RenderProbeMgr, bakeProbes, void, (),, "@brief Iterates over all reflection probes in the scene and bakes their cubemaps\n\n.")
{
   object->bakeProbes();
}
