#include "window.h"
#include "error.h"

#define S(x) #x
#define STR(x) S(x)

#ifndef PROJECT_NAME
#define PROJECT_NAME GRAPHICS_WINDOWS
#endif

namespace GraphicsEngine {

Window::Window(std::uint32_t width, std::uint32_t height) : _window({}) {
    _window = {::SDL_CreateWindow(STR(PROJECT_NAME), width, height, 0),
               ::SDL_DestroyWindow};
    ensure(_window, "Could not create window: {}", ::SDL_GetError());

    _renderer = {::SDL_CreateRenderer(_window, nullptr), ::SDL_DestroyRenderer};
    ensure(_renderer, "Could not renderer: {}", ::SDL_GetError());
}

void Window::drawing_begin(const uint8_t r, const uint8_t g, const uint8_t b,
                           const uint8_t a) {
    SDL_SetRenderDrawColor(_renderer, r, g, b, a);
    SDL_RenderClear(_renderer);
}

void Window::drawing_end() { SDL_RenderPresent(_renderer); }

} // namespace GraphicsEngine
