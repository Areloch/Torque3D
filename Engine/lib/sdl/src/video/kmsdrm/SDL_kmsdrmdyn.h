/*
  Simple DirectMedia Layer
<<<<<<<< HEAD:Engine/lib/sdl/src/video/mir/SDL_mirdyn.h
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>
========
  Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>
>>>>>>>> b76d2982b6dc305d52b8a231a6a57145f6f8cfac:Engine/lib/sdl/src/video/kmsdrm/SDL_kmsdrmdyn.h

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

<<<<<<<< HEAD:Engine/lib/sdl/src/video/mir/SDL_mirdyn.h
#ifndef SDL_mirdyn_h_
#define SDL_mirdyn_h_
========
#ifndef SDL_kmsdrmdyn_h_
#define SDL_kmsdrmdyn_h_
>>>>>>>> b76d2982b6dc305d52b8a231a6a57145f6f8cfac:Engine/lib/sdl/src/video/kmsdrm/SDL_kmsdrmdyn.h

#include "../../SDL_internal.h"

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>

#ifdef __cplusplus
extern "C" {
#endif

int SDL_KMSDRM_LoadSymbols(void);
void SDL_KMSDRM_UnloadSymbols(void);

/* Declare all the function pointers and wrappers... */
#define SDL_KMSDRM_SYM(rc,fn,params) \
    typedef rc (*SDL_DYNKMSDRMFN_##fn) params; \
    extern SDL_DYNKMSDRMFN_##fn KMSDRM_##fn;
#define SDL_KMSDRM_SYM_CONST(type, name) \
    typedef type SDL_DYNKMSDRMCONST_##name; \
    extern SDL_DYNKMSDRMCONST_##name KMSDRM_##name;
#include "SDL_kmsdrmsym.h"

#ifdef __cplusplus
}
#endif

<<<<<<<< HEAD:Engine/lib/sdl/src/video/mir/SDL_mirdyn.h
#endif /* !defined SDL_mirdyn_h_ */
========
#endif /* SDL_kmsdrmdyn_h_ */
>>>>>>>> b76d2982b6dc305d52b8a231a6a57145f6f8cfac:Engine/lib/sdl/src/video/kmsdrm/SDL_kmsdrmdyn.h

/* vi: set ts=4 sw=4 expandtab: */
