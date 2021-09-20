#include "platform/platform.h"
#include "gui/2d/editor/sceneEditor.h"

#include "core/stream/memStream.h"
#include "gui/core/guiCanvas.h"
#include "T3D/gameBase/gameConnection.h"
#include "console/consoleInternal.h"
#include "console/engineAPI.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxDebugEvent.h"
#include "platform/typetraits.h"

#include "T2D/Scene/Scene2D.h"

IMPLEMENT_CONOBJECT(SceneEditor);

ConsoleDocClass(SceneEditor,
   "@brief The main scene editor for 2d scenes.\n"
   "Editor use only.\n"
   "@internal"
);


