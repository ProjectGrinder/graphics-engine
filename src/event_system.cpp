#include "event_system.h"
#include "SDL3/SDL_events.h"
#include <vector>

namespace GraphicsEngine {

void EventSystem::poll_event() {
    ::SDL_Event event;
    while (::SDL_PollEvent(&event))
        if (auto it = _event_map.find(static_cast<::SDL_EventType>(event.type));
            it != _event_map.end())
            for (auto &v : it->second)
                v.callback(event);
}

uint64_t EventSystem::subscribe(::SDL_EventType type,
                                callback_function callback) {
    uint64_t id = _next_id++;
    this->_event_map[type].push_back({id, std::move(callback)});
    return id;
}

void EventSystem::unsubscribe(::SDL_EventType type, uint64_t id) {
    if (auto it = _event_map.find(type); it != _event_map.end())
        std::erase_if(it->second, [id](const CallbackEntry &entry) {
            return entry.id == id;
        });
}

} // namespace GraphicsEngine
