#pragma once

#include "SDL3/SDL.h"
#include "graphics_device.h"
#include "material.h"
#include "mesh.h"
#include "window.h"
#include <unordered_map>

namespace GraphicsEngine {

class Renderer {
  public:
    Renderer(Window &window, GraphicsDevice &device);
    ~Renderer();

    Renderer(const Renderer &) = delete;
    Renderer &operator=(const Renderer &) = delete;

    bool draw_begin();
    bool draw_begin(float r, float g, float b, float a);
    void draw(const Mesh &mesh, Material &material);
    void draw_end();

    template <typename Func>
        requires std::invocable<Func>
    void frame(Func &&draw_commands) {
        if (!draw_begin())
            return;
        std::forward<Func>(draw_commands)();
        draw_end();
    }
    template <typename Func>
        requires std::invocable<Func>
    void frame(Func &&draw_commands, const float r, const float g,
               const float b, const float a) {
        if (!draw_begin(r, g, b, a))
            return;
        std::forward<Func>(draw_commands)();
        draw_end();
    }

  private:
    ::SDL_GPUGraphicsPipeline *_get_or_create_pipeline(Material &material);

    Window &_window;
    GraphicsDevice &_device;

    ::SDL_GPUCommandBuffer *_current_cmd{nullptr};
    ::SDL_GPUTexture *_swapchain_texture{nullptr};
    ::SDL_GPURenderPass *_current_pass{nullptr};

    struct MaterialKey {
        ::SDL_GPUShader *vs;
        ::SDL_GPUShader *ps;

        bool operator==(const MaterialKey &other) const {
            return vs == other.vs && ps == other.ps;
        }
    };

    struct MaterialKeyHash {
        std::size_t operator()(const MaterialKey &key) const {
            return std::hash<void *>{}(key.vs) ^
                   (std::hash<void *>{}(key.ps) << 1);
        }
    };

    std::unordered_map<MaterialKey, ::SDL_GPUGraphicsPipeline *,
                       MaterialKeyHash>
        _pipeline_cache;
};

} // namespace GraphicsEngine
