#pragma once

#include <SDL3/SDL.h>
#include <cstdint>

#include "auto_release.h"

namespace GraphicsEngine {

class Window {
  public:
    Window(std::uint32_t width, std::uint32_t height);
    ~Window() = default;

    ::SDL_Window *native_handle() const;
    std::uint32_t width() const;
    std::uint32_t height() const;

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    Window(Window &&) = default;
    Window &operator=(Window &&) = default;

  private:
    AutoRelease<::SDL_Window *> _window;
    uint32_t _width;
    uint32_t _height;
};

} // namespace GraphicsEngine
