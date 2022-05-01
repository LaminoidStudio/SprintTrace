/*
 * Nuklear - 4.9.4 - public domain
 */
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
#ifndef NK_SDL_RENDERER_H_
#define NK_SDL_RENDERER_H_

#include "nuklear.h"

#include <SDL2/SDL.h>
NK_API struct nk_context*   nk_sdl_init(SDL_Window *win, SDL_Renderer *renderer);
NK_API void                 nk_sdl_font_stash_begin(struct nk_font_atlas **atlas);
NK_API void                 nk_sdl_font_stash_end(void);
NK_API int                  nk_sdl_handle_event(SDL_Event *evt);
NK_API void                 nk_sdl_render(enum nk_anti_aliasing);
NK_API void                 nk_sdl_shutdown(void);

#if SDL_COMPILEDVERSION < SDL_VERSIONNUM(2, 0, 22)
/* Metal API does not support cliprects with negative coordinates or large
 * dimensions. The issue is fixed in SDL2 with version 2.0.22 but until
 * that version is released, the NK_SDL_CLAMP_CLIP_RECT flag can be used to
 * ensure the cliprect is itself clipped to the viewport.
 * See discussion at https://discourse.libsdl.org/t/rendergeometryraw-producing-different-results-in-metal-vs-opengl/34953
 */
#define NK_SDL_CLAMP_CLIP_RECT
#endif

#endif /* NK_SDL_RENDERER_H_ */
