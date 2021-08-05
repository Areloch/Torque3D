#include "platform/platform.h"
#include "gui/2d/guiTS2DControl.h"
#include "gui/core/guiOffscreenCanvas.h"
#include "console/engineAPI.h"
#include "scene/sceneManager.h"
#include "T2D/Scene/Scene2D.h"
#include "lighting/lightManager.h"
#include "gfx/sim/debugDraw.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/screenshot.h"
#include "math/mathUtils.h"
#include "T2D/Math2D/Vector2.h"
#include "gui/core/guiCanvas.h"
#include "scene/reflectionManager.h"
#include "postFx/postEffectManager.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxDebugEvent.h"
#include "core/stream/fileStream.h"
#include "platform/output/IDisplayDevice.h"
#include "T3D/gameBase/extended/extendedMove.h"

IMPLEMENT_CONOBJECT(GuiTS2DCtrl);

ConsoleDocClass(GuiTS2DCtrl,
   "@brief Abstract base class for controls that render 2D scenes.\n\n"

   "GuiTS2DCtrl is the base class for controls that render 2D camera views in Torque.  The class itself "
   "does not implement a concrete scene rendering.  Use GuiObjectView to display invidiual shapes in "
   "the Gui and GameTS2DCtrl to render full scenes.\n\n"

   "@see GameTS2DCtrl\n"
   "@see GuiObjectView\n"
   "@ingroup Gui2D\n"
);

U32 GuiTS2DCtrl::smFrameCount = 0;
bool GuiTS2DCtrl::smUseLatestDisplayTransform = true;
Vector<GuiTS2DCtrl*> GuiTS2DCtrl::smAwakeTS2DCtrls;

GuiTS2DCtrl::GuiTS2DCtrl()
{
   mCameraZRot = 0;
   mReflectPriority = 1.0f;

   mSaveModelview.identity();
   mSaveProjection.identity();
   mSaveViewport.set(0, 0, 10, 10);
   mSaveWorldToScreenScale.set(0, 0);

   mLastCameraQuery.cameraMatrix.identity();

   /// useless fov is useless.
   mLastCameraQuery.fov = 50.0f;
   mLastCameraQuery.object = NULL;
   mLastCameraQuery.farPlane = MAX_LAYERS_SUPPORTED;
   mLastCameraQuery.nearPlane = 0.01f;
   mLastCameraQuery.hasFovPort = false;
   mLastCameraQuery.hasStereoTargets = false;
   mLastCameraQuery.ortho = true;

   /// these are going to be overwritten by the scene.
   mLastCameraQuery.mSceneMin.set(0.0f, 0.0f);
   mLastCameraQuery.mSceneMax.set(16.0f, 9.0f);
   mLastCameraQuery.mCameraSize.set(16.0f, 9.0f);
   mLastCameraQuery.mCameraScale.set(1.0f, 1.0f);

   /// setup a default cam area so we are actually rendering something.
   mLastCameraQuery.mCamArea = RectF(0.0f - (mLastCameraQuery.mCameraSize.x * 0.5f),
                                     0.0f - (mLastCameraQuery.mCameraSize.y * 0.5f),
                                     mLastCameraQuery.mCameraSize.x,
                                     mLastCameraQuery.mCameraSize.y);

   mOrthoWidth = 0.1f;
   mOrthoHeight = 0.1f;
}

void GuiTS2DCtrl::renderWorld(const RectI & updateRect)
{
}

F32 GuiTS2DCtrl::projectRadius(F32 dist, F32 radius) const
{
   // Fixup any negative or zero distance so we
   // don't get a divide by zero.
   dist = dist > 0.0f ? dist : 0.001f;
   return (radius / dist) * mSaveWorldToScreenScale.y;
}

void GuiTS2DCtrl::initPersistFields()
{
   Parent::initPersistFields();
}

void GuiTS2DCtrl::consoleInit()
{
   Con::addVariable("$TS2DControl::frameCount", TypeS32, &smFrameCount, "The number of frames that have been rendered since this control was created.\n"
      "@ingroup Rendering\n");
   Con::addVariable("$TS2DControl::useLatestDisplayTransform", TypeBool, &smUseLatestDisplayTransform, "Use the latest view transform when rendering stereo instead of the one calculated by the last move.\n"
      "@ingroup Rendering\n");
}

bool GuiTS2DCtrl::onWake()
{
   if (!Parent::onWake())
      return false;

   // Add ourselves to the active viewport list.
   AssertFatal(!smAwakeTS2DCtrls.contains(this),
      "GuiTS2DCtrl::onWake - This control is already in the awake list!");
   smAwakeTS2DCtrls.push_back(this);

   return true;
}

void GuiTS2DCtrl::onSleep()
{
   Parent::onSleep();

   AssertFatal(smAwakeTS2DCtrls.contains(this),
      "GuiTS2DCtrl::onSleep - This control is not in the awake list!");
   smAwakeTS2DCtrls.remove(this);
}

bool GuiTS2DCtrl::project(const Point3F & pt, Point3F * dest) const
{
   return MathUtils::mProjectWorldToScreen(pt, dest, mSaveViewport, mSaveModelview, mSaveProjection);
}

bool GuiTS2DCtrl::unproject(const Point3F & pt, Point3F * dest) const
{
   MathUtils::mProjectScreenToWorld(pt, dest, mSaveViewport, mSaveModelview, mSaveProjection, mLastCameraQuery.farPlane, mLastCameraQuery.nearPlane);
   return true;
}

void GuiTS2DCtrl::onPreRender()
{
   setUpdate();
}

bool GuiTS2DCtrl::processCameraQuery(CameraQuery * query)
{
   return false;
}

static FovPort CalculateFovPortForCanvas(const RectI viewport, const CameraQuery &cameraQuery)
{
   F32 wwidth;
   F32 wheight;
   F32 renderWidth = viewport.extent.x;
   F32 renderHeight = viewport.extent.y;
   F32 aspectRatio = renderWidth / renderHeight;

   if (!cameraQuery.ortho)
   {
      wheight = /*cameraQuery.nearPlane * */ mTan(cameraQuery.fov / 2.0f);
      wwidth = aspectRatio * wheight;
   }
   else
   {
      wheight = cameraQuery.fov;
      wwidth = aspectRatio * wheight;
   }

   F32 hscale = wwidth * 2.0f / renderWidth;
   F32 vscale = wheight * 2.0f / renderHeight;

   F32 left = 0.0f * hscale - wwidth;
   F32 right = renderWidth * hscale - wwidth;
   F32 top = wheight - vscale * 0.0f;
   F32 bottom = wheight - vscale * renderHeight;

   FovPort fovPort;
   fovPort.upTan = top;
   fovPort.downTan = -bottom;
   fovPort.leftTan = -left;
   fovPort.rightTan = right;

   return fovPort;

}

void GuiTS2DCtrl::_internalRender(RectI guiViewPort, RectI renderViewport, Frustum &frustum)
{
   GFXTransformSaver saver;
   Point2I renderSize = renderViewport.extent;
   GFXTarget *origTarget = GFX->getActiveRenderTarget();
   S32 origStereoTarget = GFX->getCurrentStereoTarget();

   mLastCameraQuery.cameraMatrix.setColumn(0, Point3F(1.0, 0.0, 0.0));
   mLastCameraQuery.cameraMatrix.setColumn(1, Point3F(0.0, 0.0, -1.0));
   mLastCameraQuery.cameraMatrix.setColumn(2, Point3F(0.0, 1.0, 0.0));

   if (mCameraZRot)
   {
      MatrixF rotMat(EulerF(0, 0, mDegToRad(mCameraZRot)));
      mLastCameraQuery.cameraMatrix.mul(rotMat);
   }

   if (mReflectPriority > 0)
   {
      // Get the total reflection priority.
      F32 totalPriority = 0;
      for (U32 i = 0; i < smAwakeTS2DCtrls.size(); i++)
         if (smAwakeTS2DCtrls[i]->isVisible())
            totalPriority += smAwakeTS2DCtrls[i]->mReflectPriority;

      REFLECTMGR->update(mReflectPriority / totalPriority,
         renderSize,
         mLastCameraQuery);
   }

   GFX->setActiveRenderTarget(origTarget);
   GFX->setCurrentStereoTarget(origStereoTarget);
   GFX->setViewport(renderViewport);
   // Clear the zBuffer so GUI doesn't hose object rendering accidentally
   GFX->clear(GFXClearZBuffer, ColorI(20, 20, 20), 1.0f, 0);

   GFX->setFrustum(frustum);
   mSaveProjection = GFX->getProjectionMatrix();

   if (mLastCameraQuery.ortho)
   {
      mOrthoWidth = frustum.getWidth();
      mOrthoHeight = frustum.getHeight();
   }

   /// not needed
   //gClientSceneGraph->setDisplayTargetResolution(renderSize);

   MatrixF worldToCamera = mLastCameraQuery.cameraMatrix;
   worldToCamera.inverse();
   GFX->setWorldMatrix(worldToCamera);

   mSaveProjection = GFX->getProjectionMatrix();
   mSaveModelview = GFX->getWorldMatrix();
   mSaveViewport = guiViewPort;
   mSaveWorldToScreenScale = GFX->getWorldToScreenScale();
   mSaveFrustum = GFX->getFrustum();
   mSaveFrustum.setTransform(mLastCameraQuery.cameraMatrix);

   /// not needed
   //gClientSceneGraph->setNonClipProjection(mSaveProjection);
   PFXMGR->setFrameMatrices(mSaveModelview, mSaveProjection);

   renderWorld(guiViewPort);

   DebugDrawer* debugDraw = DebugDrawer::get();
   debugDraw->render();

   saver.restore();

}

F32 GuiTS2DCtrl::calculateViewDistance(F32 radius)
{
   F32 fov = mLastCameraQuery.fov;
   F32 wwidth;
   F32 wheight;
   F32 renderWidth =  F32(getWidth());
   F32 renderHeight = F32(getHeight());
   F32 aspectRatio = renderWidth / renderHeight;

   // Use the FOV to calculate the viewport height scale
   // then generate the width scale from the aspect ratio.
   if (!mLastCameraQuery.ortho)
   {
      wheight = mLastCameraQuery.nearPlane * mTan(mLastCameraQuery.fov / 2.0f);
      wwidth = aspectRatio * wheight;
   }
   else
   {
      wheight = mLastCameraQuery.fov;
      wwidth = aspectRatio * wheight;
   }

   // Now determine if we should use the width 
   // fov or height fov.
   //
   // If the window is taller than it is wide, use the 
   // width fov to keep the object completely in view.
   if (wheight > wwidth)
      fov = mAtan(wwidth / mLastCameraQuery.nearPlane) * 2.0f;

   return radius / mTan(fov / 2.0f);
}


void GuiTS2DCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   GFXTransformSaver saver;

   mLastCameraQuery.displayDevice = NULL;

   if (!processCameraQuery(&mLastCameraQuery))
   {
      // We have no camera, but render the GUI children 
      // anyway.  This makes editing GuiTSCtrl derived
      // controls easier in the GuiEditor.
      renderChildControls(offset, updateRect);
      return;
   }


   if (mLastCameraQuery.displayDevice)
   {
      mLastCameraQuery.displayDevice->setDrawMode(GFXDevice::RS_Standard);

      mLastCameraQuery.displayDevice->getStereoViewports(mLastCameraQuery.stereoViewports);
      mLastCameraQuery.displayDevice->getStereoTargets(mLastCameraQuery.stereoTargets);

      mLastCameraQuery.hasStereoTargets = mLastCameraQuery.stereoTargets[0];

   }

   GFXTargetRef origTarget = GFX->getActiveRenderTarget();
   U32 origStyle = GFX->getCurrentRenderStyle();

   // Set up the appropriate render style
   Point2I renderSize = getExtent();
   F32 aspectRatio = renderSize.x / renderSize.y;
   Frustum frustum;

   mLastCameraQuery.currentEye = -1;

   // set up the camera and viewport stuff:
   const RectI& bounds = getBounds();

   mLastCameraQuery.mSceneMin.x = mLastCameraQuery.mCamArea.point.x;
   mLastCameraQuery.mSceneMin.y = mLastCameraQuery.mCamArea.point.y;
   mLastCameraQuery.mSceneMax.x = mLastCameraQuery.mSceneMin.x + mLastCameraQuery.mCamArea.len_x();
   mLastCameraQuery.mSceneMax.y = mLastCameraQuery.mSceneMin.y + mLastCameraQuery.mCamArea.len_y();

   mLastCameraQuery.mCameraScale.x = (mLastCameraQuery.mSceneMax.x - mLastCameraQuery.mSceneMin.x) / bounds.len_x();
   mLastCameraQuery.mCameraScale.y = (mLastCameraQuery.mSceneMax.y - mLastCameraQuery.mSceneMin.y) / bounds.len_y();

   const Point2I globalTopLeft(updateRect.point.x, updateRect.point.y);
   const Point2I localTopLeft = globalToLocalCoord(globalTopLeft);
   const Point2I globalBottomRight(updateRect.point.x + updateRect.extent.x, updateRect.point.y + updateRect.extent.y);
   const Point2I localBottomRight = globalToLocalCoord(globalBottomRight);

   const Point2F& cameraScale = mLastCameraQuery.mCameraScale;
   Point2F sceneMin = mLastCameraQuery.mSceneMin;
   Point2F sceneMax = mLastCameraQuery.mSceneMax;

   if (localTopLeft.y > 0)
      sceneMax.y -= localTopLeft.y * cameraScale.y;

   if (localTopLeft.x > 0)
      sceneMin.x += localTopLeft.x * cameraScale.x;

   if (localBottomRight.y < bounds.extent.y)
      sceneMin.y += (bounds.extent.y - localBottomRight.y) * cameraScale.y;

   if (localBottomRight.x < bounds.extent.x)
      sceneMax.x -= (bounds.extent.x - localBottomRight.x) * cameraScale.x;

   frustum.set(mLastCameraQuery.ortho, sceneMin.x, sceneMax.x, sceneMax.y, sceneMin.y, mLastCameraQuery.nearPlane, mLastCameraQuery.farPlane);
   GFX->setOrtho(sceneMin.x, sceneMax.x, sceneMin.y, sceneMax.y, 0.00f, mLastCameraQuery.farPlane);
   // Manipulate the frustum for tiled screenshots
   const bool screenShotMode = gScreenShot && gScreenShot->isPending();
   if (screenShotMode)
   {
      gScreenShot->tileFrustum(frustum);
      GFX->setViewMatrix(MatrixF::Identity);
   }

   RectI tempRect = updateRect;

   _internalRender(tempRect, tempRect, frustum);

   // Allow subclasses to render 2D elements.
   GFX->setActiveRenderTarget(origTarget);
   GFX->setCurrentRenderStyle(origStyle);
   GFX->setClipRect(updateRect);
   renderGui(offset, updateRect);

   renderChildControls(offset, updateRect);

   smFrameCount++;

}

DefineEngineMethod(GuiTS2DCtrl, project, Point3F, (Point3F worldPosition), ,
   "Transform world-space coordinates to screen-space (x, y, depth) coordinates.\n"
   "@param worldPosition The world-space position to transform to screen-space.\n"
   "@return The ")
{
   Point3F screenPos;
   object->project(worldPosition, &screenPos);
   return screenPos;
}

//-----------------------------------------------------------------------------

DefineEngineMethod(GuiTS2DCtrl, getWorldToScreenScale, Point2F, (), ,
   "Get the ratio between world-space units and pixels.\n"
   "@return The amount of world-space units covered by the extent of a single pixel.")
{
   return object->getWorldToScreenScale();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(GuiTS2DCtrl, calculateViewDistance, F32, (F32 radius), ,
   "Given the camera's current FOV, get the distance from the camera's viewpoint at which the given radius will fit in the render area.\n"
   "@param radius Radius in world-space units which should fit in the view.\n"
   "@return The distance from the viewpoint at which the given radius would be fully visible.")
{
   return object->calculateViewDistance(radius);
}

DefineEngineMethod(GuiTS2DCtrl, unproject, Point3F, (Point3F screenPosition), ,
   "Transform 3D screen-space coordinates (x, y, depth) to world space.\n"
   "This method can be, for example, used to find the world-space position relating to the current mouse cursor position.\n"
   "@param screenPosition The x/y position on the screen plus the depth from the screen-plane outwards.\n"
   "@return The world-space position corresponding to the given screen-space coordinates.")
{
   Point3F worldPos;
   object->unproject(screenPosition, &worldPos);
   return worldPos;
}
