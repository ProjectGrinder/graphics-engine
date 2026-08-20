#pragma once

#include <cstdint>
#include <span>
#include <ranges>

#include "SDL3/SDL.h"
#include "auto_release.h"
#include "graphics_device.h"

namespace GraphicsEngine {

class Mesh {
public:
    Mesh() = default;

    template <std::ranges::contiguous_range VRange,
              std::ranges::contiguous_range IRange = std::initializer_list<uint32_t>>
    Mesh(GraphicsDevice &device, VRange&& vertices, IRange&& indices = {}) {
        using VertexType = std::ranges::range_value_t<VRange>;

        std::span<const VertexType> v_span{std::forward<VRange>(vertices)};
        std::span<const uint32_t> i_span{std::forward<IRange>(indices)};

        _upload_data(device,
                     v_span.data(),
                     static_cast<uint32_t>(v_span.size_bytes()),
                     i_span.data(),
                     static_cast<uint32_t>(i_span.size_bytes()),
                     static_cast<uint32_t>(v_span.size()),
                     static_cast<uint32_t>(i_span.size()));
    }

    template <std::ranges::input_range VRange,
              std::ranges::input_range IRange = std::initializer_list<uint32_t>>
    requires (!std::ranges::contiguous_range<VRange> || !std::ranges::contiguous_range<IRange>)
    Mesh(GraphicsDevice &device, VRange&& vertices, IRange&& indices = {}) {
        using VertexType = std::ranges::range_value_t<VRange>;

        std::vector<VertexType> v_buffer(std::ranges::begin(vertices), std::ranges::end(vertices));
        std::vector<uint32_t>   i_buffer(std::ranges::begin(indices),  std::ranges::end(indices));

        _upload_data(device,
                     v_buffer.data(),
                     static_cast<uint32_t>(v_buffer.size() * sizeof(VertexType)),
                     i_buffer.data(),
                     static_cast<uint32_t>(i_buffer.size() * sizeof(uint32_t)),
                     static_cast<uint32_t>(v_buffer.size()),
                     static_cast<uint32_t>(i_buffer.size()));
    }

    void bind(::SDL_GPURenderPass *pass) const;
    void draw(::SDL_GPURenderPass *pass) const;

    [[nodiscard]] uint32_t vertex_count() const { return _vertex_count; }
    [[nodiscard]] uint32_t index_count() const { return _index_count; }
    [[nodiscard]] bool is_indexed() const { return _index_count > 0; }

private:
    void _upload_data(GraphicsDevice &device,
                      const void *v_data, uint32_t v_size,
                      const uint32_t *i_data, uint32_t i_size,
                      uint32_t v_count, uint32_t i_count);

    AutoRelease<::SDL_GPUBuffer *> _vbo;
    AutoRelease<::SDL_GPUBuffer *> _ibo;
    uint32_t _vertex_count{0};
    uint32_t _index_count{0};
};

} // namespace GraphicsEngine