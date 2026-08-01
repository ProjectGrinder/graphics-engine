#pragma once

#include <SDL3/SDL.h>
#include <cstdint>

#include "auto_release.h"

namespace GraphicsEngine {

class Window {
  public:
    Window(std::uint32_t width, std::uint32_t height);
    ~Window() = default;

    void drawing_begin(const uint8_t r = 0u, const uint8_t g = 0u,
                       const uint8_t b = 0u, const uint8_t a = 255u);
    void drawing_end();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    Window(Window &&) = default;
    Window &operator=(Window &&) = default;

  private:
    AutoRelease<::SDL_Window *> _window;
    AutoRelease<::SDL_Renderer *> _renderer;
};

} // namespace GraphicsEngine
