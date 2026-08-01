#include "sdl_context.h"
#include "SDL3_shadercross/SDL_shadercross.h"
#include "error.h"
#include <SDL3/SDL.h>

namespace GraphicsEngine {
SDLContext::SDLContext() {
    ensure(::SDL_Init(SDL_INIT_VIDEO), ::SDL_GetError());
    ensure(::SDL_ShaderCross_Init(), "Unable to initialize SDL_ShaderCross");
}
SDLContext::~SDLContext() {
    ::SDL_ShaderCross_Quit();
    ::SDL_Quit();
}

} // namespace GraphicsEngine
