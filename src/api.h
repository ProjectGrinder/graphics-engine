#pragma once
#include <graphics_engine.h>

#include "event_system.h"
#include "graphics_device.h"
#include "renderer.h"
#include "resource_manager.h"
#include "timer.h"
#include "window.h"

namespace GraphicsEngine {
class Api final {
  public:
    ~Api();

    Api(const Api &) = delete;
    Api &operator=(const Api &) = delete;
    Api(Api &&) = delete;
    Api &operator=(Api &&) = delete;

    GraphicsEngineAPI *native_handle();

    class Builder {
      public:
        Builder();
        ~Builder() = default;
        Builder(const Builder &) = delete;
        Builder &operator=(const Builder &) = delete;
        Builder(Builder &&) = default;
        Builder &operator=(Builder &&) = default;

        Builder &with_event(EventSystem *sys);
        Builder &with_graphics(GraphicsDevice *sys);
        Builder &with_renderer(Renderer *sys);
        Builder &with_resource(ResourceManager *sys);
        Builder &with_timer(Timer *sys);
        Builder &with_window(Window *sys);
        Api operator()();

      private:
        EventSystem *_event;
        GraphicsDevice *_graphics;
        Renderer *_renderer;
        ResourceManager *_manager;
        Timer *_timer;
        Window *_window;
    };

  private:
    Api(EventSystem *event, GraphicsDevice *graphics, Renderer *renderer,
        ResourceManager *manager, Timer *timer, Window *window);

    EventSystem *_event;
    GraphicsDevice *_graphics;
    Renderer *_renderer;
    ResourceManager *_manager;
    Timer *_timer;
    Window *_window;
    uint32_t _version;
    GraphicsEngineAPI _handle;

    void _draw_mesh(MeshHandler mesh, MaterialHandler material);
    MeshHandler _create_mesh(const void *vbuf, size_t vbuf_size,
                             const void *ibuf, size_t ibuf_size);
    MaterialHandler _create_material(const char *shader_source,
                                     size_t source_size);
    void _bind_vertex_data(MaterialHandler material, void *data,
                           size_t data_size);
    void _bind_pixel_data(MaterialHandler material, void *data,
                          size_t data_size);
    uint32_t _event_subscribe(::SDL_EventType event, EventFunction func);
    void _event_unsubscribe(::SDL_EventType event, uint32_t id);
};
} // namespace GraphicsEngine
