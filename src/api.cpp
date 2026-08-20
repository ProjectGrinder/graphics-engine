#include "api.h"
#include "SDL3/SDL_events.h"
#include "error.h"
#include <cassert>
#include <cstring>

#define VERSION 0x00010000

namespace GraphicsEngine {

namespace {
Api *g_instance = nullptr;
}

#define API_FUNC_NAME(NAME) Api::Builder::NAME
#define API_BUILDER_FUNC(NAME, IN_TYPE, VAR_NAME)                              \
    Api::Builder &API_FUNC_NAME(NAME)(IN_TYPE sys) {                           \
        VAR_NAME = sys;                                                        \
        return *this;                                                          \
    }

#define BIND_API_VOID(NAME, PARAMS, ARGS)                                      \
    _handle.NAME = [] PARAMS {                                                 \
        if (g_instance)                                                        \
            g_instance->_##NAME ARGS;                                          \
    }

#define BIND_API_RET(NAME, RET_TYPE, DEFAULT_VAL, PARAMS, ARGS)                \
    _handle.NAME = [] PARAMS -> RET_TYPE {                                     \
        return g_instance ? g_instance->_##NAME ARGS : (DEFAULT_VAL);          \
    }

Api::Builder::Builder()
    : _event{nullptr}, _graphics{nullptr}, _renderer{nullptr},
      _manager{nullptr}, _timer{nullptr}, _window{nullptr} {}

API_BUILDER_FUNC(with_event, EventSystem *, _event);
API_BUILDER_FUNC(with_graphics, GraphicsDevice *, _graphics);
API_BUILDER_FUNC(with_renderer, Renderer *, _renderer);
API_BUILDER_FUNC(with_resource, ResourceManager *, _manager);
API_BUILDER_FUNC(with_timer, Timer *, _timer);
API_BUILDER_FUNC(with_window, Window *, _window);

Api Api::Builder::operator()() {
    return Api{_event, _graphics, _renderer, _manager, _timer, _window};
}

Api::Api(EventSystem *event, GraphicsDevice *graphics, Renderer *renderer,
         ResourceManager *manager, Timer *timer, Window *window)
    : _event{event}, _graphics{graphics}, _renderer{renderer},
      _manager{manager}, _timer{timer}, _window{window}, _version{VERSION},
      _handle{} {
    ensure(g_instance == nullptr, "Only one active Api instance allowed!");

    g_instance = this;
    _handle.version = _version;

    BIND_API_VOID(draw_mesh, (MeshHandler m, MaterialHandler mat), (m, mat));
    BIND_API_RET(create_mesh, MeshHandler, 0,
                 (const void *vb, size_t vs, const void *ib, size_t is),
                 (vb, vs, ib, is));
    BIND_API_RET(create_material, MaterialHandler, 0,
                 (const char *src, size_t sz), (src, sz));
    BIND_API_VOID(bind_vertex_data,
                  (MaterialHandler mat, void *data, size_t sz),
                  (mat, data, sz));
    BIND_API_VOID(bind_pixel_data, (MaterialHandler mat, void *data, size_t sz),
                  (mat, data, sz));
    BIND_API_RET(event_subscribe, uint32_t, 0,
                 (::SDL_EventType ev, EventFunction fn), (ev, fn));
    BIND_API_VOID(event_unsubscribe, (::SDL_EventType ev, uint32_t id),
                  (ev, id));
}

Api::~Api() {
    if (g_instance == this) {
        g_instance = nullptr;
    }
}

GraphicsEngineAPI *Api::native_handle() { return &_handle; }

void Api::_draw_mesh(MeshHandler mesh, MaterialHandler material) {
    if (_renderer && _manager) {
        auto raw_mesh = _manager->get_mesh(mesh);
        auto raw_mat = _manager->get_material(material);
        if (raw_mesh && raw_mat) {
            _renderer->draw(*raw_mesh, *raw_mat);
        }
    }
}

MeshHandler Api::_create_mesh(const void *vbuf, size_t vbuf_size,
                              const void *ibuf, size_t ibuf_size) {
    if (!_manager || !vbuf || !ibuf)
        return 0;

    std::vector<std::byte> vertices(vbuf_size);
    std::memcpy(vertices.data(), vbuf, vbuf_size);

    size_t index_count = ibuf_size / sizeof(uint32_t);
    std::vector<uint32_t> indices(index_count);
    std::memcpy(indices.data(), ibuf, ibuf_size);

    return _manager->create_mesh(vertices, indices);
}

MaterialHandler Api::_create_material(const char *shader_source,
                                      size_t source_size) {
    if (!_manager || !shader_source)
        return 0;
    std::string str{shader_source, source_size};
    return _manager->create_material(str);
}

void Api::_bind_vertex_data(MaterialHandler material, void *data,
                            size_t data_size) {
    if (_manager && data && data_size > 0) {
        if (auto raw_mat = _manager->get_material(material)) {
            raw_mat->set_vertex_uniform_bytes(data, data_size);
        }
    }
}

void Api::_bind_pixel_data(MaterialHandler material, void *data,
                           size_t data_size) {
    if (_manager && data && data_size > 0) {
        if (auto raw_mat = _manager->get_material(material)) {
            raw_mat->set_pixel_uniform_bytes(data, data_size);
        }
    }
}

uint32_t Api::_event_subscribe(::SDL_EventType event, EventFunction func) {
    return _event ? _event->subscribe(event, func) : 0;
}

void Api::_event_unsubscribe(::SDL_EventType event, uint32_t id) {
    if (_event) {
        _event->unsubscribe(event, id);
    }
}

} // namespace GraphicsEngine
