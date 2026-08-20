#include "mesh.h"
#include <cstring>
#include "error.h"

namespace GraphicsEngine {

void Mesh::_upload_data(GraphicsDevice &device,
                        const void *v_data, uint32_t v_size,
                        const uint32_t *i_data, uint32_t i_size,
                        uint32_t v_count, uint32_t i_count) {
    _vertex_count = v_count;
    _index_count = i_count;

    ::SDL_GPUDevice *gpu_device = device.native_handle();

    auto buffer_deleter = [gpu_device](::SDL_GPUBuffer *buf) {
        if (buf) ::SDL_ReleaseGPUBuffer(gpu_device, buf);
    };

    ::SDL_GPUBufferCreateInfo vbo_info{
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = v_size
    };
    _vbo = { ::SDL_CreateGPUBuffer(gpu_device, &vbo_info), buffer_deleter };
    ensure(_vbo, "Failed to create GPU Vertex Buffer: {}", ::SDL_GetError());

    if (i_size > 0) {
        ::SDL_GPUBufferCreateInfo ibo_info{
            .usage = SDL_GPU_BUFFERUSAGE_INDEX,
            .size = i_size
        };
        _ibo = { ::SDL_CreateGPUBuffer(gpu_device, &ibo_info), buffer_deleter };
        ensure(_ibo, "Failed to create GPU Index Buffer: {}", ::SDL_GetError());
    }

    uint32_t total_transfer_size = v_size + i_size;
    ::SDL_GPUTransferBufferCreateInfo xfer_info{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = total_transfer_size
    };
    ::SDL_GPUTransferBuffer *xfer_buffer = ::SDL_CreateGPUTransferBuffer(gpu_device, &xfer_info);
    ensure(xfer_buffer, "Failed to create Transfer Buffer: {}", ::SDL_GetError());

    auto *map_ptr = static_cast<uint8_t *>(::SDL_MapGPUTransferBuffer(gpu_device, xfer_buffer, false));
    ensure(map_ptr, "Failed to map Transfer Buffer memory: {}", ::SDL_GetError());

    std::memcpy(map_ptr, v_data, v_size);
    if (i_size > 0) {
        std::memcpy(map_ptr + v_size, i_data, i_size);
    }

    ::SDL_UnmapGPUTransferBuffer(gpu_device, xfer_buffer);

    ::SDL_GPUCommandBuffer *cmd = ::SDL_AcquireGPUCommandBuffer(gpu_device);
    ensure(cmd, "Failed to acquire command buffer for Mesh upload: {}", ::SDL_GetError());

    ::SDL_GPUCopyPass *copy_pass = ::SDL_BeginGPUCopyPass(cmd);

    ::SDL_GPUTransferBufferLocation vbo_src{.transfer_buffer = xfer_buffer, .offset = 0};
    ::SDL_GPUBufferRegion vbo_dst{.buffer = _vbo.get(), .offset = 0, .size = v_size};
    ::SDL_UploadToGPUBuffer(copy_pass, &vbo_src, &vbo_dst, false);

    if (i_size > 0) {
        ::SDL_GPUTransferBufferLocation ibo_src{.transfer_buffer = xfer_buffer, .offset = v_size};
        ::SDL_GPUBufferRegion ibo_dst{.buffer = _ibo.get(), .offset = 0, .size = i_size};
        ::SDL_UploadToGPUBuffer(copy_pass, &ibo_src, &ibo_dst, false);
    }

    ::SDL_EndGPUCopyPass(copy_pass);
    ::SDL_SubmitGPUCommandBuffer(cmd);

    ::SDL_ReleaseGPUTransferBuffer(gpu_device, xfer_buffer);
}

void Mesh::bind(::SDL_GPURenderPass *pass) const {
    if (!_vbo) return;

    ::SDL_GPUBufferBinding vbo_binding{.buffer = _vbo.get(), .offset = 0};
    ::SDL_BindGPUVertexBuffers(pass, 0, &vbo_binding, 1);

    if (_ibo) {
        ::SDL_GPUBufferBinding ibo_binding{.buffer = _ibo.get(), .offset = 0};
        ::SDL_BindGPUIndexBuffer(pass, &ibo_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);
    }
}

void Mesh::draw(::SDL_GPURenderPass *pass) const {
    bind(pass);
    if (is_indexed()) {
        ::SDL_DrawGPUIndexedPrimitives(pass, _index_count, 1, 0, 0, 0);
    } else {
        ::SDL_DrawGPUPrimitives(pass, _vertex_count, 1, 0, 0);
    }
}

} // namespace GraphicsEngine