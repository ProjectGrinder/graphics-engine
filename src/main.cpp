#include <SDL3/SDL.h>
#include <iostream>
#include <ostream>
#include <string>

#include "SDL3_shadercross/SDL_shadercross.h"
#include "error.h"
#include "event_system.h"
#include "exception.h"
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
        GraphicsEngine::Timer timer;
        bool is_running = true;

        event_system.subscribe(
            ::SDL_EventType::SDL_EVENT_QUIT,
            [&is_running](const ::SDL_Event &) { is_running = false; });

        SDL_ShaderCross_HLSL_Info vertex_shader_info{
            shader.c_str(),
            "vertex",
            nullptr,
            nullptr,
            SDL_SHADERCROSS_SHADERSTAGE_VERTEX,
            0};

        SDL_ShaderCross_HLSL_Info pixel_shader_info{
            shader.c_str(),
            "pixel",
            nullptr,
            nullptr,
            SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT,
            0};

        uint64_t size = 0;

        void *vs_code =
            SDL_ShaderCross_CompileSPIRVFromHLSL(&vertex_shader_info, &size);

        GraphicsEngine::ensure(vs_code != nullptr,
                               "Failed to compile Vertex HLSL to SPIR-V: {}",
                               ::SDL_GetError());
        std::println(std::cout, "vs size: {}", size);

        size = 0;
        void *ps_code =
            SDL_ShaderCross_CompileSPIRVFromHLSL(&pixel_shader_info, &size);

        GraphicsEngine::ensure(ps_code != nullptr,
                               "Failed to compile Vertex HLSL to SPIR-V: {}",
                               ::SDL_GetError());
        std::println(std::cout, "ps size: {}", size);

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
