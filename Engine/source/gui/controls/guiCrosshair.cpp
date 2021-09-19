#include "platform/platform.h"
#include "guiCrosshair.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "gui/core/guiControl.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/gameFunctions.h"
#include "T3D/player.h"
#include "scene/sceneObject.h"

IMPLEMENT_CONOBJECT(GuiCrosshairCtrl);

ImplementEnumType(enumCurState,"")
   {GuiCrosshairCtrl::NORMAL, "NORMAL"},
   {GuiCrosshairCtrl::FRIEND, "FRIEND"},
   {GuiCrosshairCtrl::ENEMY, "ENEMY"},
EndImplementEnumType;

GuiCrosshairCtrl::GuiCrosshairCtrl()
   :  mCrossWidth(4),
      mCrossHeight(1),
      mMinSpread(0.02f),
      mMaxSpread(0.6f),
      mSpreadSpeed(0.05f),
      mSpreadIncMult(2.0f),
      mNeutralColor(ColorI::WHITE),
      mEnemyColor(ColorI::RED),
      mFriendColor(ColorI::GREEN),
      mAnimated(true),
      mUseViewDistance(false),
      mFpsMode(true),
      mUseBitmapCrosshair(false),
      mDist(20.0f)
{
   mCurState = NORMAL;
   mCrossHairSection = NULL;
   mInteractTexture = NULL;
   mSelectedObj = NULL;

}

void GuiCrosshairCtrl::initPersistFields()
{
   Parent::initPersistFields();

   addField("crossHeight", TypeS32, Offset(mCrossHeight, GuiCrosshairCtrl),
      "Vertical height of crosshair section.");
   addField("crossWidth", TypeS32, Offset(mCrossWidth, GuiCrosshairCtrl),
      "Horizontal width of crosshair section.");
   addField("minSpread", TypeF32, Offset(mMinSpread, GuiCrosshairCtrl),
      "Minimum spread for weapon.");
   addField("maxSpread", TypeF32, Offset(mMaxSpread, GuiCrosshairCtrl),
      "Maximum spread for weapon.");
   addField("spreadSpeed", TypeF32, Offset(mSpreadSpeed , GuiCrosshairCtrl),
      "Spread speed.");
   addField("spreadIncreaseMult", TypeF32, Offset(mSpreadIncMult, GuiCrosshairCtrl),
      "Spread speed multiplier for increasing spread.");
   addField("neutralColor", TypeColorI, Offset(mNeutralColor, GuiCrosshairCtrl),
      "Crosshair Color when nothing is inside.");
   addField("enemyColor", TypeColorI, Offset(mEnemyColor, GuiCrosshairCtrl),
      "Crosshair Color when an enemy is inside.");
   addField("friendColor", TypeColorI, Offset(mFriendColor, GuiCrosshairCtrl),
      "Crosshair Color when an friend is inside.");
   addField("distance", TypeF32, Offset(mDist, GuiCrosshairCtrl),
      "Distance to cast ray.");
   addField("animated", TypeBool, Offset(mAnimated, GuiCrosshairCtrl),
      "Speed of player animates the offset of the crosshair sections.");
   addField("useViewDistance", TypeBool, Offset(mUseViewDistance, GuiCrosshairCtrl),
      "Use the view distance of the scene for the raycast (when true distance is not used).");
   addField("fpsMode", TypeBool, Offset(mFpsMode, GuiCrosshairCtrl),
      "FPS mode uses muzzlepoint for raycast, false uses camera for raycast.");
   

}

F32 GuiCrosshairCtrl::updateOffset(F32 speed)
{
   F32 retOffset(0);

   if (speed > 0.01)
   {
      F32 incAmount = (mSpreadSpeed * TickSec) * mSpreadIncMult;
      retOffset = mCurrentSpread + incAmount;
      if (retOffset > mMaxSpread)
         retOffset = mMaxSpread;
   }
   else
   {
      F32 decAmount = mSpreadSpeed * TickSec;

      Con::printf("decAmount %3.4f", decAmount);

      retOffset = mCurrentSpread - decAmount;

      if (retOffset < mMinSpread)
         retOffset = mMinSpread;

   }

   Con::printf("retOffset %3.4f", retOffset);

   return retOffset;
}


void GuiCrosshairCtrl::onRender(Point2I offset, const RectI & updateRect)
{
   GameConnection* conn = GameConnection::getConnectionToServer();
   if (!conn)
      return;

   ColorI mCurColor;

   switch (mCurState)
   {
   case FRIEND:
      mCurColor = mFriendColor;
      break;
   case ENEMY:
      mCurColor = mEnemyColor;
      break;
   default:
      mCurColor = mNeutralColor;
      break;
   }

   /// conObj is important for disable collisions.
   GameBase* conObj = conn->getControlObject();

   MatrixF cam;
   Point3F camPos;
   if (!mFpsMode)
   {
      conn->getControlCameraTransform(0, &cam);
   }
   else
   {
      Player* playObj = static_cast<Player*>(conObj);
      playObj->getMuzzleTransform(0, &cam);
   }

   camPos = cam.getPosition();
   Point3F rayEnd = cam.getForwardVector();

   if (mUseViewDistance)
   {
      rayEnd *= gClientSceneGraph->getVisibleDistance();
   }
   else
   {
      rayEnd *= mDist;
   }

   rayEnd += camPos;
   static U32 mask = TerrainObjectType | ShapeBaseObjectType;
   conObj->disableCollision();

   mSelectedObj = NULL;

   RayInfo camRay;
   if (gClientContainer.castRay(camPos, rayEnd, mask, &camRay))
   {
      mSelectedObj = camRay.object;
      if (Con::isFunction("onCrosshairHover"))
         Con::executef("onCrosshairHover", this->getIdString(), mSelectedObj->getIdString(), rayEnd);
   }

   conObj->enableCollision();

   /// cant get velocity of camera?
   Point3F camVec = camPos - mPrevCamPos;

   F32 speed = 0.0f;
   speed = camVec.len();

   F32 offAmt(0.0f);
   offAmt = updateOffset(speed);

   mCurrentSpread = offAmt;
   mPrevCamPos = camPos;
   mPrevSpeed = speed;
   Point2I centerPt = Point2I(getExtent().x / 2, getExtent().y / 2);

   /// default
   RectI rect(centerPt, Point2I(mCrossWidth, mCrossHeight));
   /// draw east, needs +1 for the rect draw pos
   rect.point.x = centerPt.x + 1;
   rect.point.x = rect.point.x + (rect.point.x * offAmt);
   rect.point.y = centerPt.y - (mCrossHeight / 2);
   GFX->getDrawUtil()->drawRectFill(rect, mCurColor);

   /// draw west
   rect.point.x = (centerPt.x - mCrossWidth);
   rect.point.x = rect.point.x - (rect.point.x * offAmt);
   /// don't need to set the y
   GFX->getDrawUtil()->drawRectFill(rect, mCurColor);
   /// draw north
   /// flip the extent.
   rect.extent = Point2I(mCrossHeight, mCrossWidth);
   rect.point.x = centerPt.x - (mCrossHeight / 2);
   rect.point.y = (centerPt.y - mCrossWidth);
   rect.point.y = rect.point.y - (rect.point.y * offAmt);
   GFX->getDrawUtil()->drawRectFill(rect, mCurColor);
   /// draw south, needs +1 for the rect draw pos.
   rect.point.y = centerPt.y + 1;
   rect.point.y = rect.point.y + (rect.point.y * offAmt);
   GFX->getDrawUtil()->drawRectFill(rect, mCurColor);

}

void GuiCrosshairCtrl::setMode(const CurState &mode)
{
   mCurState = mode;
}

static GuiCrosshairCtrl::CurState getModeTypeEnum(const char *pLabel)
{
   GuiCrosshairCtrl::CurState out;
   if (!castConsoleTypeFromString(out, pLabel))
   {
      return GuiCrosshairCtrl::NORMAL;
   }

   return out;

}

DefineEngineMethod(GuiCrosshairCtrl, setMode, void, (String mode),,
   "Set Crosshair mode. (\"NORMAL\" | \"ENEMY\" | \"FRIEND\")")
{
   const GuiCrosshairCtrl::CurState &type = getModeTypeEnum(mode);
   object->setMode(type);
}
