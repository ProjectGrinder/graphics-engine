#pragma once
#include <SDL3/SDL.h>

#include "auto_release.h"
#include "shader.h"

namespace GraphicsEngine {

class Material {
public:
    Material(const Shader &vertex_shader, const Shader &pixel_shader);
    void instance_if_not(::SDL_GPUDevice *device);
    ::SDL_GPUShader *get_vertex_shader() const;
    ::SDL_GPUShader *get_pixel_shader() const;

private:
    void _instance(::SDL_GPUDevice *device);

    Shader _cpu_vertex_shader;
    Shader _cpu_pixel_shader;
    AutoRelease<::SDL_GPUShader *> _gpu_vertex_shader;
    AutoRelease<::SDL_GPUShader *> _gpu_pixel_shader;
};

}