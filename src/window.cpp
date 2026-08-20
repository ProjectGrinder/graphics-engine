#include "window.h"
#include "error.h"

#define S(x) #x
#define STR(x) S(x)

#ifndef PROJECT_NAME
#define PROJECT_NAME GRAPHICS_WINDOWS
#endif

namespace GraphicsEngine {

Window::Window(std::uint32_t width, std::uint32_t height) : _window{}, _width {width}, _height{height} {
    _window = {::SDL_CreateWindow(STR(PROJECT_NAME), width, height, 0),
               ::SDL_DestroyWindow};
    ensure(_window, "Could not create window: {}", ::SDL_GetError());
}


::SDL_Window *Window::native_handle() const { return _window; }
std::uint32_t Window::width() const { return _width; }
std::uint32_t Window::height() const { return _height; }

} // namespace GraphicsEngine
