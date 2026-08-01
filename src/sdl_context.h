#pragma once

namespace GraphicsEngine {
class SDLContext {
  public:
    SDLContext();
    ~SDLContext();

    SDLContext(const SDLContext &) = delete;
    SDLContext &operator=(const SDLContext &) = delete;
    SDLContext(SDLContext &&) = default;
    SDLContext &operator=(SDLContext &&) = default;
};
} // namespace GraphicsEngine
