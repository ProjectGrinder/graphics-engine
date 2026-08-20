#pragma once

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#include <functional>
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
typedef void *GraphicsEngineContext;
typedef void (*EventFunction)(void);

typedef struct Transform {
    float position[3];
    float rotation[3];
    float scale[3];
} Transform;

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
    uint32_t version; // API Versioning guard (e.g., 0x00010000)

    void (*draw_mesh)(GraphicsEngineContext ctx, MeshHandler mesh,
                      MaterialHandler material, const Transform *transform);
    MeshHandler (*create_mesh)(GraphicsEngineContext ctx, const void *vbuf,
                               size_t vbuf_size, const void *ibuf,
                               size_t ibuf_size);
    MaterialHandler (*create_material)(GraphicsEngineContext ctx,
                                       const char *shader_source,
                                       size_t source_size);
    uint32_t (*event_subscribe)(GraphicsEngineContext ctx, int event,
                                EventFunction func);
    void (*event_unsubscribe)(GraphicsEngineContext ctx, int event,
                              uint32_t id);
} GraphicsEngineAPI;

#ifdef __cplusplus
namespace GraphicsEngine {

class Context {
  private:
    GraphicsEngineContext m_raw_ctx{nullptr};
    const GraphicsEngineAPI *m_api{nullptr};

  public:
    Context(GraphicsEngineContext raw_ctx, const GraphicsEngineAPI *api)
        : m_raw_ctx(raw_ctx), m_api(api) {}

    virtual ~Context() = default;

    void draw(MeshHandler mesh, MaterialHandler material,
              const Transform &transform = {}) {
        if (m_api && m_api->draw_mesh) {
            m_api->draw_mesh(m_raw_ctx, mesh, material, &transform);
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
            return m_api->create_material(m_raw_ctx, shader_source.data(),
                                          shader_source.size());
        }
        return 0;
    }

    uint32_t event_subscribe(int event, EventFunction func) {
        if (m_api && m_api->event_subscribe) {
            return m_api->event_subscribe(m_raw_ctx, event, func);
        }
        return 0;
    }

    void event_unsubscribe(int event, uint32_t id) {
        if (m_api && m_api->event_unsubscribe) {
            m_api->event_unsubscribe(m_raw_ctx, event, id);
        }
    }

  private:
    MeshHandler _create_mesh(std::span<const std::byte> vertex_bytes,
                             std::span<const uint32_t> indices) {
        if (m_api && m_api->create_mesh) {
            return m_api->create_mesh(m_raw_ctx, vertex_bytes.data(),
                                      vertex_bytes.size(), indices.data(),
                                      indices.size_bytes());
        }
        return 0;
    }
};

} // namespace GraphicsEngine
#endif

typedef void (*AppInitFn)(GraphicsEngineContext ctx,
                          const GraphicsEngineAPI *api);
typedef void (*AppUpdateFn)(GraphicsEngineContext ctx);
typedef void (*AppExitFn)(GraphicsEngineContext ctx);

#ifdef __cplusplus
void on_init(GraphicsEngine::Context *ctx);
void on_update(GraphicsEngine::Context *ctx);
void on_exit(GraphicsEngine::Context *ctx);

namespace GraphicsEngine {
inline Context *&get_global_context() {
    static Context *instance = nullptr;
    return instance;
}
} // namespace GraphicsEngine
#else
void on_init(GraphicsEngineContext ctx, const GraphicsEngineAPI *api);
void on_update(GraphicsEngineContext ctx);
void on_exit(GraphicsEngineContext ctx);
#endif

#ifdef __cplusplus
extern "C" {
#endif

inline PLUGIN_API void app_init(GraphicsEngineContext ctx,
                                const GraphicsEngineAPI *api) {
#ifdef __cplusplus
    static GraphicsEngine::Context cpp_ctx(ctx, api);
    GraphicsEngine::get_global_context() = &cpp_ctx;
    on_init(&cpp_ctx);
#else
    on_init(ctx, api);
#endif
}

inline PLUGIN_API void app_update(GraphicsEngineContext ctx) {
#ifdef __cplusplus
    on_update(GraphicsEngine::get_global_context());
#else
    on_update(ctx);
#endif
}

inline PLUGIN_API void app_exit(GraphicsEngineContext ctx) {
#ifdef __cplusplus
    on_exit(GraphicsEngine::get_global_context());
    GraphicsEngine::get_global_context() = nullptr;
#else
    on_exit(ctx);
#endif
}

#ifdef __cplusplus
}
#endif
