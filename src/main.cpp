#include <iostream>
#include <print>
#include <string>

#include "event_system.h"
#include "exception.h"
#include "graphics_device.h"
#include "renderer.h"
#include "resource_manager.h"
#include "sdl_context.h"
#include "timer.h"
#include "window.h"

static const std::string shader = R"(
struct VS_INPUT {
    float3 pos: POSITION;
    float4 color: COLOR;
};

struct PS_INPUT {
    float4 pos: SV_POSITION;
    float4 color: COLOR;
};

PS_INPUT vertex(VS_INPUT input) {
    PS_INPUT output;
    output.pos = float4(input.pos, 1.0f);
    output.color = input.color;
    return output;
}

float4 pixel(PS_INPUT input): SV_TARGET {
    return input.color;
}
)";

int main(int, char *[]) {
    try {
        GraphicsEngine::SDLContext context;
        GraphicsEngine::EventSystem event_system;
        GraphicsEngine::Window main_window{1280u, 640u};
        GraphicsEngine::GraphicsDevice device{main_window};
        GraphicsEngine::Renderer renderer{main_window, device};
        GraphicsEngine::ResourceManager resource_manager{device};
        GraphicsEngine::Timer timer;

        bool is_running = true;

        event_system.subscribe(
            ::SDL_EventType::SDL_EVENT_QUIT,
            [&is_running](const ::SDL_Event &) { is_running = false; });

        while (is_running) {
            timer.tick();
            event_system.poll_event();
        }
    } catch (const GraphicsEngine::Exception &err) {
        std::println(std::cerr, "{}", err);
    } catch (...) {
        std::println(std::cerr, "unknown exception");
    }
    return 0;
}
