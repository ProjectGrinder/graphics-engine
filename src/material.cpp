#include "material.h"

#include "error.h"
#include "SDL3_shadercross/SDL_shadercross.h"

namespace GraphicsEngine {

Material::Material(const Shader &vertex_shader, const Shader &pixel_shader)
: _cpu_vertex_shader{vertex_shader}, _cpu_pixel_shader{pixel_shader},
_gpu_vertex_shader{}, _gpu_pixel_shader{} {}

void Material::_instance(::SDL_GPUDevice *device) {
    auto deleter = [device](::SDL_GPUShader *shader) {
        SDL_ReleaseGPUShader(device, shader);
    };

    _gpu_vertex_shader = {
        ::SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(
            device,
            _cpu_vertex_shader.shader_info(),
            _cpu_vertex_shader.resource_info(),
            0
        ),
        deleter
    };

    ensure(_gpu_vertex_shader, "unable to compile vertex shader for material: {}", SDL_GetError());

    _gpu_pixel_shader = {
        ::SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(
            device,
            _cpu_pixel_shader.shader_info(),
            _cpu_pixel_shader.resource_info(),
            0
        ),
        deleter
    };
    ensure(_gpu_pixel_shader, "unable to compile pixel shader for material: {}", SDL_GetError());
}

void Material::instance_if_not(::SDL_GPUDevice *device) {
    if (!_gpu_pixel_shader || !_gpu_vertex_shader)
        _instance(device);
}

::SDL_GPUShader *Material::get_vertex_shader() const {
   return _gpu_vertex_shader;
}
::SDL_GPUShader *Material::get_pixel_shader() const {
   return _gpu_pixel_shader;
}

}