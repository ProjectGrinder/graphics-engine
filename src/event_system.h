#pragma once

#include "SDL3/SDL_events.h"
#include <functional>
#include <unordered_map>
#include <vector>
namespace GraphicsEngine {

class EventSystem {
  public:
    using callback_function = std::function<void(const ::SDL_Event *)>;
    void poll_event();
    uint64_t subscribe(::SDL_EventType type, callback_function callback);
    void unsubscribe(::SDL_EventType type, uint64_t id);

    EventSystem() = default;
    EventSystem(const EventSystem &) = delete;
    EventSystem &operator=(const EventSystem &) = delete;
    EventSystem(EventSystem &&) = default;
    EventSystem &operator=(EventSystem &&) = default;

  private:
    struct CallbackEntry {
        uint64_t id;
        callback_function callback;
    };

    std::unordered_map<::SDL_EventType, std::vector<CallbackEntry>> _event_map;
    uint64_t _next_id{0};
};

} // namespace GraphicsEngine
