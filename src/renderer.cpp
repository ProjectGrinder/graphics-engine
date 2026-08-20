#include "renderer.h"
#include "error.h"

#include <ranges>

namespace GraphicsEngine {

Renderer::Renderer(Window &window, GraphicsDevice &device)
    : _window(window), _device(device) {}

Renderer::~Renderer() {
    for (const auto &pipeline : _pipeline_cache | std::views::values) {
        if (pipeline) {
            ::SDL_ReleaseGPUGraphicsPipeline(_device.native_handle(), pipeline);
        }
    }
    _pipeline_cache.clear();
}

bool Renderer::draw_begin() {
    _current_cmd = ::SDL_AcquireGPUCommandBuffer(_device.native_handle());
    if (!_current_cmd) return false;

    if (!::SDL_WaitAndAcquireGPUSwapchainTexture(_current_cmd, _window.native_handle(), &_swapchain_texture, nullptr, nullptr)) {
        ::SDL_SubmitGPUCommandBuffer(_current_cmd);
        _current_cmd = nullptr;
        return false;
    }

    if (_swapchain_texture) {
        const ::SDL_GPUColorTargetInfo color_target{
            .texture = _swapchain_texture,
            .clear_color = { 0.0f, 0.0f, 0.0f, 1.0f }, // Dark slate clear background
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE
        };

        _current_pass = ::SDL_BeginGPURenderPass(_current_cmd, &color_target, 1, nullptr);
        ensure(_current_pass, "Failed to begin GPU render pass: {}", ::SDL_GetError());
        return true;
    }

    return false;
}

bool Renderer::draw_begin(float r, float g, float b, float a) {
    _current_cmd = ::SDL_AcquireGPUCommandBuffer(_device.native_handle());
    if (!_current_cmd) return false;

    if (!::SDL_WaitAndAcquireGPUSwapchainTexture(_current_cmd, _window.native_handle(), &_swapchain_texture, nullptr, nullptr)) {
        ::SDL_SubmitGPUCommandBuffer(_current_cmd);
        _current_cmd = nullptr;
        return false;
    }

    if (_swapchain_texture) {
        const ::SDL_GPUColorTargetInfo color_target{
            .texture = _swapchain_texture,
            .clear_color = { r, g, b, a }, // Dark slate clear background
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE
        };

        _current_pass = ::SDL_BeginGPURenderPass(_current_cmd, &color_target, 1, nullptr);
        ensure(_current_pass, "Failed to begin GPU render pass: {}", ::SDL_GetError());
        return true;
    }

    return false;
}

::SDL_GPUGraphicsPipeline *Renderer::_get_or_create_pipeline(Material &material) {
    material.instance_if_not(_device.native_handle());

    MaterialKey key{material.get_vertex_shader(), material.get_pixel_shader()};
    if (const auto it = _pipeline_cache.find(key); it != _pipeline_cache.end()) {
        return it->second;
    }

    ::SDL_GPUVertexBufferDescription buffer_desc{
        .slot = 0,
        .pitch = sizeof(float) * 7,
        .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
        .instance_step_rate = 0
    };

    ::SDL_GPUVertexAttribute attributes[2] = {
        {
            .location = 0,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset = 0
        },
        {
            .location = 1,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
            .offset = sizeof(float) * 3
        }
    };

    ::SDL_GPUColorTargetDescription color_target{
        .format = ::SDL_GetGPUSwapchainTextureFormat(_device.native_handle(), _window.native_handle()),
        .blend_state = {}
    };

    ::SDL_GPUGraphicsPipelineCreateInfo create_info{
        .vertex_shader = material.get_vertex_shader(),
        .fragment_shader = material.get_pixel_shader(),
        .vertex_input_state = {
            .vertex_buffer_descriptions = &buffer_desc,
            .num_vertex_buffers = 1,
            .vertex_attributes = attributes,
            .num_vertex_attributes = 2
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .target_info = {
            .color_target_descriptions = &color_target,
            .num_color_targets = 1
        }
    };

    ::SDL_GPUGraphicsPipeline *pipeline = ::SDL_CreateGPUGraphicsPipeline(_device.native_handle(), &create_info);
    ensure(pipeline, "Failed to create GPU Graphics Pipeline: {}", ::SDL_GetError());

    _pipeline_cache[key] = pipeline;
    return pipeline;
}

void Renderer::draw(const Mesh &mesh, Material &material, Transform /*transform*/) {
    if (!_current_pass) return;

    ::SDL_GPUGraphicsPipeline *pipeline = _get_or_create_pipeline(material);
    ::SDL_BindGPUGraphicsPipeline(_current_pass, pipeline);

    mesh.draw(_current_pass);
}

void Renderer::draw_end() {
    if (_current_pass) {
        ::SDL_EndGPURenderPass(_current_pass);
        _current_pass = nullptr;
    }

    if (_current_cmd) {
        ::SDL_SubmitGPUCommandBuffer(_current_cmd);
        _current_cmd = nullptr;
    }
    _swapchain_texture = nullptr;
}

} // namespace GraphicsEngine