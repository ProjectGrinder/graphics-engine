#include "plugin_loader.h"
#include "error.h"

namespace GraphicsEngine {

Plugin::Plugin(const std::filesystem::path &path)
    : _handle{::SDL_LoadObject(path.string().c_str()), ::SDL_UnloadObject},
      _init{nullptr}, _update{nullptr}, _render{nullptr}, _exit{nullptr} {
    ensure(_handle, "Unable to load plugin dynamic library: {}", path.string());

    _init =
        reinterpret_cast<AppInitFn>(::SDL_LoadFunction(_handle, "app_init"));
    _update = reinterpret_cast<AppUpdateFn>(
        ::SDL_LoadFunction(_handle, "app_update"));
    _render = reinterpret_cast<AppRenderFn>(
        ::SDL_LoadFunction(_handle, "app_render"));
    _exit =
        reinterpret_cast<AppExitFn>(::SDL_LoadFunction(_handle, "app_exit"));

    ensure(_init && _update && _render && _exit,
           "Plugin missing required C-ABI entry symbols!");
}

void Plugin::init(const GraphicsEngineAPI *api) const {
    if (_init)
        _init(api);
}
void Plugin::update(const GraphicsEngineAPI *api) const {
    if (_update)
        _update(api);
}
void Plugin::render(const GraphicsEngineAPI *api) const {
    if (_render)
        _render(api);
}
void Plugin::exit(const GraphicsEngineAPI *api) const {
    if (_exit)
        _exit(api);
}

} // namespace GraphicsEngine
