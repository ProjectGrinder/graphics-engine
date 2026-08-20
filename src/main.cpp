#include <iostream>
#include <print>

#include "api.h"
#include "event_system.h"
#include "exception.h"
#include "graphics_device.h"
#include "plugin_loader.h"
#include "renderer.h"
#include "resource_manager.h"
#include "sdl_context.h"
#include "timer.h"
#include "window.h"

int main(int argc, char *argv[]) {
    try {
        using namespace GraphicsEngine;

        SDLContext sdl_context{};
        EventSystem event_system{};
        Window main_window{1280u, 720u};
        GraphicsDevice device{main_window};
        Renderer renderer{main_window, device};
        ResourceManager resource_manager{device};
        Timer timer{};

        auto &api_builder = Api::Builder()
                                .with_event(&event_system)
                                .with_window(&main_window)
                                .with_graphics(&device)
                                .with_renderer(&renderer)
                                .with_resource(&resource_manager)
                                .with_timer(&timer);

        Api api = api_builder();
        bool is_running = true;

        event_system.subscribe(
            ::SDL_EventType::SDL_EVENT_QUIT,
            [&is_running](const ::SDL_Event *) { is_running = false; });

        Plugin plugin{"./menu.dll"};
        plugin.init(api.native_handle());

        while (is_running) {
            timer.tick();
            event_system.poll_event();
            renderer.frame([&]() { plugin.render(api.native_handle()); });
        }

        plugin.exit(api.native_handle());

    } catch (const GraphicsEngine::Exception &err) {
        std::println(std::cerr, "Engine Error: {}", err.what());
    } catch (const std::exception &e) {
        std::println(std::cerr, "Standard Exception: {}", e.what());
    } catch (...) {
        std::println(std::cerr, "Unknown Exception caught in main!");
    }
    return 0;
}
