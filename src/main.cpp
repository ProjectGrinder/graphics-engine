#include <SDL3/SDL.h>
#include <iostream>
#include <print>
#include <string>

#include "SDL3_shadercross/SDL_shadercross.h"
#include "error.h"
#include "event_system.h"
#include "exception.h"
#include "sdl_context.h"
#include "shader.h"
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
        GraphicsEngine::Timer timer;
        bool is_running = true;

        event_system.subscribe(
            ::SDL_EventType::SDL_EVENT_QUIT,
            [&is_running](const ::SDL_Event &) { is_running = false; });

        GraphicsEngine::Shader vertex{shader,
                                      GraphicsEngine::ShaderType::VERTEX};

        GraphicsEngine::Shader pixel{shader, GraphicsEngine::ShaderType::PIXEL};

        std::println(std::cout, "{}", vertex.size());
        std::println(std::cout, "{}", pixel.size());

        while (is_running) {
            timer.tick();
            event_system.poll_event();
            main_window.drawing_begin();
            main_window.drawing_end();
        }
    } catch (const GraphicsEngine::Exception &err) {
        std::println(std::cerr, "{}", err);
    } catch (...) {
        std::println(std::cerr, "unknown exception");
    }

    return 0;
}
