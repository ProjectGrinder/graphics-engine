#include "renderer.h"
#include "error.h"

#include <algorithm>
#include <print>
#include <ranges>
#include <vector>

namespace GraphicsEngine {
static SDL_GPUVertexElementFormat map_to_sdl_format(ShaderVariableType type,
                                                    uint32_t vector_size) {
    if (type == ShaderVariableType::FLOAT32) {
        switch (vector_size) {
        case 1:
            return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;
        case 2:
            return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        case 3:
            return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
        case 4:
            return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        }
    } else if (type == ShaderVariableType::INT32) {
        switch (vector_size) {
        case 1:
            return SDL_GPU_VERTEXELEMENTFORMAT_INT;
        case 2:
            return SDL_GPU_VERTEXELEMENTFORMAT_INT2;
        case 3:
            return SDL_GPU_VERTEXELEMENTFORMAT_INT3;
        case 4:
            return SDL_GPU_VERTEXELEMENTFORMAT_INT4;
        }
    } else if (type == ShaderVariableType::UINT32) {
        switch (vector_size) {
        case 1:
            return SDL_GPU_VERTEXELEMENTFORMAT_UINT;
        case 2:
            return SDL_GPU_VERTEXELEMENTFORMAT_UINT2;
        case 3:
            return SDL_GPU_VERTEXELEMENTFORMAT_UINT3;
        case 4:
            return SDL_GPU_VERTEXELEMENTFORMAT_UINT4;
        }
    }
    return SDL_GPU_VERTEXELEMENTFORMAT_INVALID;
}

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

bool Renderer::draw_begin() { return draw_begin(0.0f, 0.0f, 0.0f, 1.0f); }

bool Renderer::draw_begin(float r, float g, float b, float a) {
    _current_cmd = ::SDL_AcquireGPUCommandBuffer(_device.native_handle());
    if (!_current_cmd)
        return false;

    if (!::SDL_WaitAndAcquireGPUSwapchainTexture(
            _current_cmd, _window.native_handle(), &_swapchain_texture, nullptr,
            nullptr)) {
        ::SDL_SubmitGPUCommandBuffer(_current_cmd);
        _current_cmd = nullptr;
        return false;
    }

    if (_swapchain_texture) {
        const ::SDL_GPUColorTargetInfo color_target{
            .texture = _swapchain_texture,
            .clear_color = {r, g, b, a},
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE};

        _current_pass =
            ::SDL_BeginGPURenderPass(_current_cmd, &color_target, 1, nullptr);
        ensure(_current_pass, "Failed to begin GPU render pass: {}",
               ::SDL_GetError());
        return true;
    }

    return false;
}

::SDL_GPUGraphicsPipeline *
Renderer::_get_or_create_pipeline(Material &material) {
    material.instance_if_not(_device.native_handle());

    MaterialKey key{material.get_vertex_shader(), material.get_pixel_shader()};
    if (const auto it = _pipeline_cache.find(key);
        it != _pipeline_cache.end()) {
        return it->second;
    }

    const auto &inputs = material.get_cpu_vertex_shader().variables();

    std::vector<ShaderVariableEntry> sorted_inputs;
    sorted_inputs.reserve(inputs.size());
    for (const auto &[name, attr] : inputs) {
        sorted_inputs.push_back(attr);
    }
    std::ranges::sort(sorted_inputs, [](const auto &a, const auto &b) {
        return a.location < b.location;
    });

    std::vector<::SDL_GPUVertexAttribute> attributes;
    attributes.reserve(sorted_inputs.size());

    uint32_t stride = 0;
    for (const auto &attr : sorted_inputs) {
        std::println("{}", attr.location);
        ::SDL_GPUVertexAttribute attribute{
            .buffer_slot = 0,
            .format = map_to_sdl_format(attr.type, attr.size),
            .offset = stride};
        attributes.push_back(attribute);

        size_t element_size = sizeof(float);
        if (attr.type == ShaderVariableType::INT8 ||
            attr.type == ShaderVariableType::UINT8)
            element_size = 1;
        else if (attr.type == ShaderVariableType::INT16 ||
                 attr.type == ShaderVariableType::UINT16 ||
                 attr.type == ShaderVariableType::FLOAT16)
            element_size = 2;

        stride += static_cast<uint32_t>(attr.size * element_size);
    }

    const bool has_attributes = !attributes.empty();

    ::SDL_GPUVertexBufferDescription buffer_desc{
        .slot = 0,
        .pitch = stride,
        .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
        .instance_step_rate = 0};

    ::SDL_GPUColorTargetDescription color_target{};
    color_target.format = ::SDL_GetGPUSwapchainTextureFormat(
        _device.native_handle(), _window.native_handle());
    color_target.blend_state.enable_blend = false;
    color_target.blend_state.color_write_mask = 0xF;

    ::SDL_GPUGraphicsPipelineCreateInfo create_info{};
    create_info.vertex_shader = material.get_vertex_shader();
    create_info.fragment_shader = material.get_pixel_shader();

    create_info.vertex_input_state.vertex_buffer_descriptions =
        has_attributes ? &buffer_desc : nullptr;
    create_info.vertex_input_state.num_vertex_buffers = has_attributes ? 1 : 0;
    create_info.vertex_input_state.vertex_attributes =
        has_attributes ? attributes.data() : nullptr;
    create_info.vertex_input_state.num_vertex_attributes =
        static_cast<Uint32>(attributes.size());

    create_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    create_info.target_info.color_target_descriptions = &color_target;
    create_info.target_info.num_color_targets = 1;

    ::SDL_GPUGraphicsPipeline *pipeline =
        ::SDL_CreateGPUGraphicsPipeline(_device.native_handle(), &create_info);

    ensure(pipeline, "Failed to create GPU Graphics Pipeline: {}",
           ::SDL_GetError());

    _pipeline_cache[key] = pipeline;
    return pipeline;
}

void Renderer::draw(const Mesh &mesh, Material &material) {
    if (!_current_pass)
        return;

    ::SDL_GPUGraphicsPipeline *pipeline = _get_or_create_pipeline(material);
    ::SDL_BindGPUGraphicsPipeline(_current_pass, pipeline);

    const auto &vs_buffer = material.get_vertex_uniform_buffer();
    if (!vs_buffer.empty()) {
        ::SDL_PushGPUVertexUniformData(_current_cmd, 0, vs_buffer.data(),
                                       static_cast<uint32_t>(vs_buffer.size()));
    }

    const auto &ps_buffer = material.get_pixel_uniform_buffer();
    if (!ps_buffer.empty()) {
        ::SDL_PushGPUFragmentUniformData(
            _current_cmd, 0, ps_buffer.data(),
            static_cast<uint32_t>(ps_buffer.size()));
    }
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
