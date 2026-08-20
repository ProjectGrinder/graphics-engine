#include "shader.h"
#include "SDL3_shadercross/SDL_shadercross.h"
#include "auto_release.h"
#include "error.h"

namespace GraphicsEngine {

Shader::Shader(const std::string &source, ShaderType type)
    : _spirv{}, _type(type), _size{0} {

    const char *entrypoint = (type == ShaderType::VERTEX) ? "vertex" : "pixel";
    ::SDL_ShaderCross_ShaderStage stage =
        (type == ShaderType::VERTEX) ? SDL_SHADERCROSS_SHADERSTAGE_VERTEX
                                     : SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;

    ::SDL_ShaderCross_HLSL_Info shader_info{source.c_str(), entrypoint, nullptr,
                                            nullptr,        stage,      0};

    if (type == ShaderType::PIXEL)
        shader_info.shader_stage =
            SDL_ShaderCross_ShaderStage::SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;

    _spirv = {static_cast<uint8_t *>(
                  ::SDL_ShaderCross_CompileSPIRVFromHLSL(&shader_info, &_size)),
              ::SDL_free};
    ensure(_spirv, "unable to load shader {}", SDL_GetError());

    auto meta_data = SDL_ShaderCross_ReflectGraphicsSPIRV(_spirv, _size, 0);
    ensure(meta_data, "unable to get metadata of shader {}", SDL_GetError());
    SDL_free(meta_data);
}

ShaderType Shader::type() const { return _type; }

uint64_t Shader::size() const { return _size; }

} // namespace GraphicsEngine
