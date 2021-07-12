#ifndef _GUITS2DCONTROL_H_
#define _GUITS2DCONTROL_H_

#ifndef _GUICONTAINER_H_
#include "gui/containers/guiContainer.h"
#endif
#ifndef _MMATH_H_
#include "math/mMath.h"
#endif

#ifndef _MATTEXTURETARGET_H_
#include "materials/matTextureTarget.h"
#endif

#ifndef _GUIOFFSCREENCANVAS_H_
#include "gui/core/guiOffscreenCanvas.h"
#endif

class IDisplayDevice;
class GuiOffscreenCanvas;

struct CameraQuery
{
   SimObject* object;
   RectF mSourceArea;
   F32 fov;
   F32 mCameraZoom;
   F32 mCameraAngle;

   Point2F mSceneMin;
   Point2F mSceneMax;
   Point2F mSceneWindowScale;

   MatrixF cameraMatrix;

   IDisplayDevice* displayDevice;
};

class GuiTS2DCtrl : public GuiContainer
{
   typedef GuiContainer Parent;

protected:
   static U32 smFrameCount;
   static bool smUseLatestDisplayTransform;

   static Vector<GuiTS2DCtrl*> smAwakeTS2DCtrls;

   MatrixF     mSaveModelview;
   MatrixF     mSaveProjection;
   RectI       mSaveViewport;
   Frustum		mSaveFrustum;
   /// The saved world to screen space scale.
   /// @see getWorldToScreenScale
   Point2F mSaveWorldToScreenScale;

   /// The last camera query set in onRender.
   /// @see getLastCameraQuery
   CameraQuery mLastCameraQuery;

public:
   GuiTS2DCtrl();

   void onPreRender();
   void _internalRender(RectI guiViewPort, RectI renderViewport, Frustum &frustum);
   void onRender(Point2I offset, const RectI &updateRect);
   virtual bool processCameraQuery(CameraQuery *query);

   /// subclasses can override this to perform 2D rendering. 
   virtual void renderWorld(const RectI &updateRect);

   /// Subclasses can override this to perform 2D rendering.   
   virtual void renderGui(Point2I offset, const RectI &updateRect) {}

   static void initPersistFields();
   static void consoleInit();

   virtual bool onWake();
   virtual void onSleep();

   /// Returns the last World Matrix set in onRender.
   const MatrixF& getLastWorldMatrix() const { return mSaveModelview; }

   /// Returns the last Projection Matrix set in onRender.
   const MatrixF& getLastProjectionMatrix() const { return mSaveProjection; }

   /// Returns the last Viewport Rect set in onRender.
   const RectI&   getLastViewportRect() const { return mSaveViewport; }

   /// Returns the last Frustum set in onRender.
   const Frustum&	getLastFrustum() const { return mSaveFrustum; }

   /// Returns the scale for converting world space 
   /// units to screen space units... aka pixels.
   /// @see GFXDevice::getWorldToScreenScale
   const Point2F& getWorldToScreenScale() const { return mSaveWorldToScreenScale; }

   /// Returns the last camera query set in onRender.
   const CameraQuery& getLastCameraQuery() const { return mLastCameraQuery; }

   /// Returns the screen space X,Y and Z for world space point.
  /// The input z coord is depth, from 0 to 1.
   bool project(const Point3F &pt, Point3F *dest) const;

   /// Returns the world space point for X, Y and Z.  The ouput
   /// z coord is depth, from 0 to 1
   bool unproject(const Point3F &pt, Point3F *dest) const;

   DECLARE_CONOBJECT(GuiTS2DCtrl);
   DECLARE_CATEGORY("Gui 2D");
   DECLARE_DESCRIPTION("Abstract base class for controls that render a 2D viewport.");

};

#endif
