#pragma once

#include "auto_release.h"
#include "shader.h"

#include <cstring>
#include <vector>

namespace GraphicsEngine {

class Material {
  public:
    Material(const Shader &vertex_shader, const Shader &pixel_shader);

    void instance_if_not(::SDL_GPUDevice *device);

    ::SDL_GPUShader *get_vertex_shader() const;
    ::SDL_GPUShader *get_pixel_shader() const;

    void set_vertex_uniform_bytes(const void *data, size_t size);
    void set_pixel_uniform_bytes(const void *data, size_t size);
    template <typename T> void set_vertex_uniform(const T &uniform_data) {
        set_vertex_uniform_bytes(&uniform_data, sizeof(T));
    }

    template <typename T> void set_pixel_uniform(const T &uniform_data) {
        set_pixel_uniform_bytes(&uniform_data, sizeof(T));
    }

    const std::vector<uint8_t> &get_vertex_uniform_buffer() const;
    const std::vector<uint8_t> &get_pixel_uniform_buffer() const;

    const Shader &get_cpu_vertex_shader() const;
    const Shader &get_cpu_pixel_shader() const;

  private:
    void _instance(::SDL_GPUDevice *device);

    Shader _cpu_vertex_shader;
    Shader _cpu_pixel_shader;
    AutoRelease<::SDL_GPUShader *> _gpu_vertex_shader;
    AutoRelease<::SDL_GPUShader *> _gpu_pixel_shader;

    std::vector<uint8_t> _vs_uniform_data;
    std::vector<uint8_t> _ps_uniform_data;
};

} // namespace GraphicsEngine
