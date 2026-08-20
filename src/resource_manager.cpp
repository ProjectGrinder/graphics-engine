#include "resource_manager.h"

namespace GraphicsEngine {
ResourceManager::ResourceManager(GraphicsDevice &device)
    : _device(device), _next_mesh_id(0), _next_mat_id(0) {}

MeshHandler ResourceManager::create_mesh(std::span<const std::byte> vbuf,
                                         std::span<const uint32_t> ibuf) {
    auto id = _next_mesh_id++;
    _meshes[id] = std::make_unique<Mesh>(_device, vbuf, ibuf);
    return id;
}

MaterialHandler ResourceManager::create_material(std::string_view hlsl_source) {
    auto vs =
        std::make_shared<Shader>(std::string(hlsl_source), ShaderType::VERTEX);
    auto ps =
        std::make_shared<Shader>(std::string(hlsl_source), ShaderType::PIXEL);

    auto id = _next_mat_id++;
    _materials[id] = std::make_unique<Material>(*vs, *ps);
    return id;
}

GraphicsEngine::Mesh *ResourceManager::get_mesh(MeshHandler id) const {
    auto it = _meshes.find(id);
    return it != _meshes.end() ? it->second.get() : nullptr;
}

GraphicsEngine::Material *
ResourceManager::get_material(MaterialHandler id) const {
    auto it = _materials.find(id);
    return it != _materials.end() ? it->second.get() : nullptr;
}
} // namespace GraphicsEngine
