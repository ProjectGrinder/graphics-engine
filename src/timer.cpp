#include <SDL3/SDL.h>

#include "timer.h"

namespace GraphicsEngine {

Timer::Timer() : _current(SDL_GetPerformanceCounter()) {}

float Timer::tick() {
    uint64_t now = SDL_GetPerformanceCounter();
    uint64_t freq = SDL_GetPerformanceFrequency();

    _delta = static_cast<float>(now - _current) / static_cast<float>(freq);
    _current = now;

    return _delta;
}

float Timer::get_delta() const { return _delta; }

} // namespace GraphicsEngine
