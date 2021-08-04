#include "platform/platform.h"
#include "T2D/objects/levelInfo2D.h"

#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "T2D/Scene/Scene2D.h"
#include "lighting/advanced/advancedLightManager.h"
#include "lighting/advanced/advancedLightBinManager.h"
#include "sfx/sfxAmbience.h"
#include "sfx/sfxSoundscape.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxTypes.h"
#include "console/engineAPI.h"
#include "math/mathIO.h"

#include "torqueConfig.h"

IMPLEMENT_CO_NETOBJECT_V1(LevelInfo2D);

ConsoleDocClass(LevelInfo2D,
   "@brief Stores and controls the rendering and status information for a 2d game level.\n\n"

   "@tsexample\n"
   "new LevelInfo2D(theLevelInfo2D)\n"
   "{\n"
   "  cameraSize = \"16.0f 9.0f\";\n"
   "  canvasClearColor = \"0 0 0 255\";\n"
   "  canSaveDynamicFields = \"1\";\n"
   "  levelName = \"Blank Room\";\n"
   "  desc0 = \"A blank room ready to be populated with Torque objects.\";\n"
   "  Enabled = \"1\";\n"
   "};\n"
   "@endtsexample\n"
   "@ingroup enviroMisc\n"
);

/// The color used to clear the canvas.
/// @see GuiCanvas
extern ColorI gCanvasClearColor;

/// Default SFXAmbience used to reset the global soundscape.
static SFXAmbience sDefaultAmbience;

void LevelInfo2D::_updateSceneGraph2D()
{
   if (isClientObject())
      gClientScene2DGraph->setCameraSize(mCameraSize);

#ifndef TORQUE_DEDICATED
   if (isClientObject())
      _onLMActivate(LIGHTMGR->getId(), true);
#endif

   gCanvasClearColor = mCanvasClearColor;

}

void LevelInfo2D::_onLMActivate(const char * lm, bool enable)
{
#ifndef TORQUE_DEDICATED
   // Advanced light manager
   if (enable && String(lm) == String("ADVLM"))
   {
      AssertFatal(dynamic_cast<AdvancedLightManager *>(LIGHTMGR), "Bad light manager type!");
      AdvancedLightManager *lightMgr = static_cast<AdvancedLightManager *>(LIGHTMGR);
      lightMgr->getLightBinManager()->MRTLightmapsDuringDeferred(mAdvancedLightmapSupport);
   }
#endif
}

LevelInfo2D::LevelInfo2D()
   :  mCameraSize(16.0f, 9.0f),
      mSoundAmbience(NULL),
      mSoundscape(NULL),
      mSoundDistanceModel(SFXDistanceModelLinear)
{
   mNetFlags.set(ScopeAlways | Ghostable);

   mAdvancedLightmapSupport = true;

   LightManager::smActivateSignal.notify(this, &LevelInfo2D::_onLMActivate, 0.01f);
}

LevelInfo2D::~LevelInfo2D()
{
   LightManager::smActivateSignal.remove(this, &LevelInfo2D::_onLMActivate);
}

bool LevelInfo2D::onAdd()
{
   if (!Parent::onAdd())
      return false;

   if (!mSoundAmbience)
      Sim::findObject("AudioAmbienceDefault", mSoundAmbience);

   // Set up sound on client.

   if (isClientObject())
   {
      SFX->setDistanceModel(mSoundDistanceModel);

      // Set up the global ambient soundscape.

      mSoundscape = SFX->getSoundscapeManager()->getGlobalSoundscape();
      if (mSoundAmbience)
         mSoundscape->setAmbience(mSoundAmbience);
   }

   _updateSceneGraph2D();

   return true;
}

void LevelInfo2D::onRemove()
{
   if (mSoundscape)
      mSoundscape->setAmbience(&sDefaultAmbience);

   Parent::onRemove();
}

void LevelInfo2D::inspectPostApply()
{
   _updateSceneGraph2D();
   setMaskBits(0xFFFFFFFF);

   Parent::inspectPostApply();
}

void LevelInfo2D::initPersistFields()
{
   addGroup("LevelInfo");

   addField("cameraSize", TypePoint2F, Offset(mCameraSize, LevelInfo2D),
      "Set the camera size for the scene.");

   addField("canvasClearColor", TypeColorI, Offset(mCanvasClearColor, LevelInfo2D),
      "The color used to clear the background before the scene or any GUIs are rendered.");

   endGroup("LevelInfo");

   addGroup("Sound");

   addField("soundAmbience", TypeSFXAmbienceName, Offset(mSoundAmbience, LevelInfo2D),
      "The global ambient sound environment.");

   addField("soundDistanceModel", TypeSFXDistanceModel, Offset(mSoundDistanceModel, LevelInfo2D),
      "The distance attenuation model to use.");

   endGroup("Sound");

}

U32 LevelInfo2D::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(conn, mask, stream);
   Con::printf("Level info pack");
   /// these need to be read out separately for some reason =/ 
   stream->write(mCameraSize.x);
   stream->write(mCameraSize.y);

   stream->write(mCanvasClearColor);
   stream->writeFlag(mAdvancedLightmapSupport);
   sfxWrite(stream, mSoundAmbience);
   stream->writeInt(mSoundDistanceModel, 1);

   return retMask;
}

void LevelInfo2D::unpackUpdate(NetConnection *conn, BitStream *stream)
{
   Parent::unpackUpdate(conn, stream);
   Con::printf("Level info unpack");
   /// the aforementioned shitty read out separately stuff
   stream->read(&mCameraSize.x);
   stream->read(&mCameraSize.y);

   stream->read(&mCanvasClearColor);
   mAdvancedLightmapSupport = stream->readFlag();

   String errorStr;
   if (!sfxReadAndResolve(stream, &mSoundAmbience, errorStr))
      Con::errorf("%s", errorStr.c_str());
   mSoundDistanceModel = (SFXDistanceModel)stream->readInt(1);

   if (isProperlyAdded())
   {
      _updateSceneGraph2D();

      if (mSoundscape)
      {
         if (mSoundAmbience)
            mSoundscape->setAmbience(mSoundAmbience);
         else
            mSoundscape->setAmbience(&sDefaultAmbience);
      }

      SFX->setDistanceModel(mSoundDistanceModel);
   }

}
