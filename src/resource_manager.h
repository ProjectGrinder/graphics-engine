#pragma once
#include <graphics_engine.h>
#include <memory>
#include <span>
#include <unordered_map>

#include "graphics_device.h"
#include "material.h"
#include "mesh.h"

namespace GraphicsEngine {
class ResourceManager {
  public:
    explicit ResourceManager(GraphicsDevice &device);

    MeshHandler create_mesh(std::span<const std::byte> vbuf,
                            std::span<const uint32_t> ibuf);
    MaterialHandler create_material(std::string_view hlsl_source);
    Mesh *get_mesh(MeshHandler id) const;
    Material *get_material(MaterialHandler id) const;

  private:
    GraphicsDevice &_device;
    std::unordered_map<MeshHandler, std::unique_ptr<Mesh>> _meshes;
    std::unordered_map<MaterialHandler, std::unique_ptr<Material>> _materials;

    MeshHandler _next_mesh_id;
    MaterialHandler _next_mat_id;
};
} // namespace GraphicsEngine
