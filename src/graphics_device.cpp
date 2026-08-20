#include "error.h"
#include "graphics_device.h"

namespace GraphicsEngine {

GraphicsDevice::GraphicsDevice(Window &window, bool debug_mode): _device {} {
    _device = {
        ::SDL_CreateGPUDevice(
            SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
            debug_mode,
            nullptr
        ),
        ::SDL_DestroyGPUDevice
    };

    ensure(_device, "unable to create graphics device {}", SDL_GetError());
    ensure(
        ::SDL_ClaimWindowForGPUDevice(_device, window.native_handle()),
        "unable to create SDL_ClaimWindowForGPUDevice {}",
        SDL_GetError()
    );
}

::SDL_GPUDevice *GraphicsDevice::native_handle() const {
    return _device;
}

}