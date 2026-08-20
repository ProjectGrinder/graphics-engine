#pragma once

#include "SDL3/SDL_events.h"

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>
#else
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#endif

typedef uint32_t MeshHandler;
typedef uint32_t MaterialHandler;
typedef void (*EventFunction)(const SDL_Event *);

#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef BUILDING_PLUGIN_DLL
#define PLUGIN_API __declspec(dllexport)
#else
#define PLUGIN_API __declspec(dllimport)
#endif
#else
#define PLUGIN_API __attribute__((visibility("default")))
#endif

typedef struct GraphicsEngineAPI {
    uint32_t version;

    void (*draw_mesh)(MeshHandler mesh, MaterialHandler material);
    MeshHandler (*create_mesh)(const void *vbuf, size_t vbuf_size,
                               const void *ibuf, size_t ibuf_size);
    MaterialHandler (*create_material)(const char *shader_source,
                                       size_t source_size);
    void (*bind_vertex_data)(MaterialHandler material, void *data,
                             size_t data_size);
    void (*bind_pixel_data)(MaterialHandler material, void *data,
                            size_t data_size);
    uint32_t (*event_subscribe)(SDL_EventType event, EventFunction func);
    void (*event_unsubscribe)(SDL_EventType event, uint32_t id);
} GraphicsEngineAPI;

#ifdef __cplusplus
namespace GraphicsEngine {

class Context {
  private:
    const GraphicsEngineAPI *m_api{nullptr};

  public:
    explicit Context(const GraphicsEngineAPI *api) : m_api(api) {}
    virtual ~Context() = default;

    void draw(MeshHandler mesh, MaterialHandler material) {
        if (m_api && m_api->draw_mesh) {
            m_api->draw_mesh(mesh, material);
        }
    }

    template <
        std::ranges::contiguous_range VRange,
        std::ranges::contiguous_range IRange = std::initializer_list<uint32_t>>
    MeshHandler create_mesh(VRange &&vertices, IRange &&indices = {}) {
        using VertexType = std::ranges::range_value_t<VRange>;
        std::span<const VertexType> v_span{std::forward<VRange>(vertices)};
        std::span<const uint32_t> i_span(std::ranges::begin(indices),
                                         std::ranges::end(indices));
        return _create_mesh(std::as_bytes(v_span), i_span);
    }

    template <std::ranges::input_range VRange,
              std::ranges::input_range IRange = std::initializer_list<uint32_t>>
        requires(!std::ranges::contiguous_range<VRange> ||
                 !std::ranges::contiguous_range<IRange>)
    MeshHandler create_mesh(VRange &&vertices, IRange &&indices = {}) {
        using VertexType = std::ranges::range_value_t<VRange>;
        std::vector<VertexType> v_buffer(std::ranges::begin(vertices),
                                         std::ranges::end(vertices));
        std::vector<uint32_t> i_buffer(std::ranges::begin(indices),
                                       std::ranges::end(indices));
        return _create_mesh(std::as_bytes(std::span{v_buffer}),
                            std::span{i_buffer});
    }

    MaterialHandler create_material(std::string_view shader_source) {
        if (m_api && m_api->create_material) {
            return m_api->create_material(shader_source.data(),
                                          shader_source.size());
        }
        return 0;
    }

    template <typename T>
    void bind_vertex_data(MaterialHandler material, const T &data) {
        if (m_api && m_api->bind_vertex_data) {
            m_api->bind_vertex_data(
                material, const_cast<void *>(static_cast<const void *>(&data)),
                sizeof(T));
        }
    }

    template <typename T>
    void bind_pixel_data(MaterialHandler material, const T &data) {
        if (m_api && m_api->bind_pixel_data) {
            m_api->bind_pixel_data(
                material, const_cast<void *>(static_cast<const void *>(&data)),
                sizeof(T));
        }
    }

    uint32_t event_subscribe(SDL_EventType event, EventFunction func) {
        if (m_api && m_api->event_subscribe) {
            return m_api->event_subscribe(event, func);
        }
        return 0;
    }

    void event_unsubscribe(SDL_EventType event, uint32_t id) {
        if (m_api && m_api->event_unsubscribe) {
            m_api->event_unsubscribe(event, id);
        }
    }

  private:
    MeshHandler _create_mesh(std::span<const std::byte> vertex_bytes,
                             std::span<const uint32_t> indices) {
        if (m_api && m_api->create_mesh) {
            return m_api->create_mesh(vertex_bytes.data(), vertex_bytes.size(),
                                      indices.data(), indices.size_bytes());
        }
        return 0;
    }
};

inline Context *&get_global_context() {
    static Context *instance = nullptr;
    return instance;
}

} // namespace GraphicsEngine

void on_init(GraphicsEngine::Context *ctx);
void on_update(GraphicsEngine::Context *ctx);
void on_render(GraphicsEngine::Context *ctx);
void on_exit(GraphicsEngine::Context *ctx);

typedef void (*AppInitFn)(const GraphicsEngineAPI *api);
typedef void (*AppUpdateFn)(const GraphicsEngineAPI *api);
typedef void (*AppRenderFn)(const GraphicsEngineAPI *api);
typedef void (*AppExitFn)(const GraphicsEngineAPI *api);

#define GRAPHICS_ENGINE_PLUGIN()                                               \
    extern "C" {                                                               \
    PLUGIN_API void app_init(const GraphicsEngineAPI *api) {                   \
        static GraphicsEngine::Context cpp_ctx(api);                           \
        GraphicsEngine::get_global_context() = &cpp_ctx;                       \
        on_init(&cpp_ctx);                                                     \
    }                                                                          \
    PLUGIN_API void app_update(const GraphicsEngineAPI *api) {                 \
        (void)(api);                                                           \
        on_update(GraphicsEngine::get_global_context());                       \
    }                                                                          \
    PLUGIN_API void app_render(const GraphicsEngineAPI *api) {                 \
        (void)(api);                                                           \
        on_render(GraphicsEngine::get_global_context());                       \
    }                                                                          \
    PLUGIN_API void app_exit(const GraphicsEngineAPI *api) {                   \
        (void)(api);                                                           \
        on_exit(GraphicsEngine::get_global_context());                         \
        GraphicsEngine::get_global_context() = nullptr;                        \
    }                                                                          \
    }

#else
void on_init(const GraphicsEngineAPI *api);
void on_update(const GraphicsEngineAPI *api);
void on_render(const GraphicsEngineAPI *api);
void on_exit(const GraphicsEngineAPI *api);

#define GRAPHICS_ENGINE_PLUGIN()                                               \
    PLUGIN_API void app_init(const GraphicsEngineAPI *api) { on_init(api); }   \
    PLUGIN_API void app_update(const GraphicsEngineAPI *api) {                 \
        on_update(api);                                                        \
    }                                                                          \
    PLUGIN_API void app_render(const GraphicsEngineAPI *api) {                 \
        on_render(api);                                                        \
    }                                                                          \
    PLUGIN_API void app_exit(const GraphicsEngineAPI *api) { on_exit(api); }

#endif
