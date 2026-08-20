#include "material.h"

#include "SDL3_shadercross/SDL_shadercross.h"
#include "error.h"

namespace GraphicsEngine {

Material::Material(const Shader &vertex_shader, const Shader &pixel_shader)
    : _cpu_vertex_shader{vertex_shader}, _cpu_pixel_shader{pixel_shader},
      _gpu_vertex_shader{}, _gpu_pixel_shader{} {}

void Material::_instance(::SDL_GPUDevice *device) {
    auto deleter = [device](::SDL_GPUShader *shader) {
        if (shader) {
            SDL_ReleaseGPUShader(device, shader);
        }
    };

    _gpu_vertex_shader = {::SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(
                              device, _cpu_vertex_shader.shader_info(),
                              _cpu_vertex_shader.resource_info(), 0),
                          deleter};

    ensure(_gpu_vertex_shader,
           "unable to compile vertex shader for material: {}", SDL_GetError());

    _gpu_pixel_shader = {::SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(
                             device, _cpu_pixel_shader.shader_info(),
                             _cpu_pixel_shader.resource_info(), 0),
                         deleter};
    ensure(_gpu_pixel_shader, "unable to compile pixel shader for material: {}",
           SDL_GetError());
}

void Material::instance_if_not(::SDL_GPUDevice *device) {
    if (!_gpu_pixel_shader || !_gpu_vertex_shader)
        _instance(device);
}

::SDL_GPUShader *Material::get_vertex_shader() const {
    return _gpu_vertex_shader.get();
}

::SDL_GPUShader *Material::get_pixel_shader() const {
    return _gpu_pixel_shader.get();
}

void Material::set_vertex_uniform_bytes(const void *data, size_t size) {
    _vs_uniform_data.assign(static_cast<const uint8_t *>(data),
                            static_cast<const uint8_t *>(data) + size);
}

void Material::set_pixel_uniform_bytes(const void *data, size_t size) {
    _ps_uniform_data.assign(static_cast<const uint8_t *>(data),
                            static_cast<const uint8_t *>(data) + size);
}

const std::vector<uint8_t> &Material::get_vertex_uniform_buffer() const {
    return _vs_uniform_data;
}

const std::vector<uint8_t> &Material::get_pixel_uniform_buffer() const {
    return _ps_uniform_data;
}

const Shader &Material::get_cpu_vertex_shader() const {
    return _cpu_vertex_shader;
}

const Shader &Material::get_cpu_pixel_shader() const {
    return _cpu_pixel_shader;
}

} // namespace GraphicsEngine
