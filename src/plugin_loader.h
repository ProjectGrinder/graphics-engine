#pragma once
#include "auto_release.h"
#include <SDL3/SDL_loadso.h>
#include <filesystem>
#include <graphics_engine.h>

namespace GraphicsEngine {

class Plugin {
  public:
    explicit Plugin(const std::filesystem::path &path);

    Plugin(const Plugin &) = delete;
    Plugin &operator=(const Plugin &) = delete;
    Plugin(Plugin &&) = default;
    Plugin &operator=(Plugin &&) = default;

    void init(const GraphicsEngineAPI *api) const;
    void update(const GraphicsEngineAPI *api) const;
    void render(const GraphicsEngineAPI *api) const;
    void exit(const GraphicsEngineAPI *api) const;

  private:
    AutoRelease<::SDL_SharedObject *> _handle;
    AppInitFn _init;
    AppUpdateFn _update;
    AppRenderFn _render;
    AppExitFn _exit;
};

} // namespace GraphicsEngine
