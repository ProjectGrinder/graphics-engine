#include "shader.h"
#include "SDL3_shadercross/SDL_shadercross.h"
#include "auto_release.h"
#include "error.h"
#include <print>

namespace GraphicsEngine {

ShaderVariableType
shadercross_to_shadervariable(const ::SDL_ShaderCross_IOVarType var_type) {
    switch (var_type) {
        using enum ShaderVariableType;
    case SDL_SHADERCROSS_IOVAR_TYPE_UNKNOWN:
        return UNKNOWN;
    case SDL_SHADERCROSS_IOVAR_TYPE_INT8:
        return INT8;
    case SDL_SHADERCROSS_IOVAR_TYPE_UINT8:
        return UINT8;
    case SDL_SHADERCROSS_IOVAR_TYPE_INT16:
        return INT16;
    case SDL_SHADERCROSS_IOVAR_TYPE_UINT16:
        return UINT16;
    case SDL_SHADERCROSS_IOVAR_TYPE_INT32:
        return INT32;
    case SDL_SHADERCROSS_IOVAR_TYPE_UINT32:
        return UINT32;
    case SDL_SHADERCROSS_IOVAR_TYPE_INT64:
        return INT64;
    case SDL_SHADERCROSS_IOVAR_TYPE_UINT64:
        return UINT64;
    case SDL_SHADERCROSS_IOVAR_TYPE_FLOAT16:
        return FLOAT16;
    case SDL_SHADERCROSS_IOVAR_TYPE_FLOAT32:
        return FLOAT32;
    case SDL_SHADERCROSS_IOVAR_TYPE_FLOAT64:
        return FLOAT64;
    }

    throw Exception("unknown shader variable type: {}",
                    std::to_underlying(var_type));
}

Shader::Shader(const std::string &source, ShaderType type)
    : _type(type), _size{0}, _impl{std::make_shared<Impl>()} {

    const char *entrypoint = (type == ShaderType::VERTEX) ? "vertex" : "pixel";
    ::SDL_ShaderCross_ShaderStage stage =
        (type == ShaderType::VERTEX) ? SDL_SHADERCROSS_SHADERSTAGE_VERTEX
                                     : SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;

    ::SDL_ShaderCross_HLSL_Info shader_info{source.c_str(), entrypoint, nullptr,
                                            nullptr,        stage,      0};
    _impl->shader_data = {
        static_cast<uint8_t *>(
            ::SDL_ShaderCross_CompileSPIRVFromHLSL(&shader_info, &_size)),
        ::SDL_free};
    ensure(_impl->shader_data, "unable to load shader {}", SDL_GetError());

    _impl->reflect = {
        ::SDL_ShaderCross_ReflectGraphicsSPIRV(_impl->shader_data, _size, 0),
        ::SDL_free};
    ensure(_impl->reflect, "unable to get shader reflection {}",
           SDL_GetError());

    _impl->info = {
        _impl->shader_data.get(), _size, entrypoint, stage, 0,
    };

    if (_type != ShaderType::VERTEX)
        return;

    for (int i = 0; i < _impl->reflect.get()->num_inputs; ++i) {
        auto [name, location, vector_type, vector_size] =
            _impl->reflect.get()->inputs[i];
        std::println("{}", name);
        std::string clean{name};
        if (clean.starts_with("in.var.")) {
            clean.erase(0, 7);
        }

        _impl->variables[clean] = {clean,
                                   shadercross_to_shadervariable(vector_type),
                                   location, vector_size};
    }
}

ShaderType Shader::type() const { return _type; }

uint64_t Shader::size() const { return _size; }

const ::SDL_ShaderCross_SPIRV_Info *Shader::shader_info() const {
    return &_impl->info;
}
const ::SDL_ShaderCross_GraphicsShaderResourceInfo *
Shader::resource_info() const {
    return &_impl->reflect.get()->resource_info;
}

const std::unordered_map<std::string, ShaderVariableEntry> &
Shader::variables() const {
    return _impl->variables;
}

} // namespace GraphicsEngine
