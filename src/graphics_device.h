#pragma once
#include <SDL3/SDL.h>

#include "auto_release.h"
#include "window.h"

namespace GraphicsEngine {

class GraphicsDevice {
  public:
    GraphicsDevice(Window& window, bool debug_mode = true);
    ~GraphicsDevice() = default;

    ::SDL_GPUDevice *native_handle() const;

    GraphicsDevice(const GraphicsDevice &) = delete;
    GraphicsDevice &operator=(const GraphicsDevice &) = delete;

    GraphicsDevice(GraphicsDevice &&) = default;
    GraphicsDevice &operator=(GraphicsDevice &&) = default;

  private:
    AutoRelease<::SDL_GPUDevice *> _device;
};

}