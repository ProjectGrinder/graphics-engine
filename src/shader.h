#pragma once

#include "SDL3_shadercross/SDL_shadercross.h"
#include "auto_release.h"
#include "exception.h"

#include <memory>
#include <format>
#include <utility>
#include <string>
#include <unordered_map>
namespace GraphicsEngine {

using ShaderBinary = uint8_t *;

enum class ShaderType { VERTEX, PIXEL };
enum class ShaderVariableType {
    UNKNOWN,
    INT8,
    UINT8,
    INT16,
    UINT16,
    INT32,
    UINT32,
    INT64,
    UINT64,
    FLOAT16,
    FLOAT32,
    FLOAT64
};

ShaderVariableType shadercross_to_shadervariable(const ::SDL_ShaderCross_IOVarType var_type);

struct ShaderVariableEntry {
    std::string name;
    ShaderVariableType type;
    std::uint32_t location;
    std::uint32_t size;
};

class Shader {
  public:
    Shader(const std::string &source, ShaderType type);
    ShaderType type() const;
    uint64_t size() const;
    const std::unordered_map<std::string, ShaderVariableEntry> &variables() const;
    const ::SDL_ShaderCross_SPIRV_Info *shader_info() const;
    const ::SDL_ShaderCross_GraphicsShaderResourceInfo *resource_info() const;

    Shader(const Shader &) = default;
    Shader &operator=(const Shader &) = default;
    Shader(Shader &&) noexcept = default;
    Shader &operator=(Shader &&) noexcept = default;

  private:
    struct Impl {
        AutoRelease<ShaderBinary> shader_data;
        AutoRelease<SDL_ShaderCross_GraphicsShaderMetadata*> reflect;
        ::SDL_ShaderCross_SPIRV_Info info;
        std::unordered_map<std::string, ShaderVariableEntry> variables;
    };
    ShaderType _type;
    uint64_t _size;
    std::shared_ptr<Impl> _impl;
};

} // namespace GraphicsEngine

template <> struct std::formatter<GraphicsEngine::ShaderType> {

    constexpr auto parse(std::format_parse_context &ctx) {
        return std::begin(ctx);
    }

    auto format(const GraphicsEngine::ShaderType &obj,
                std::format_context &ctx) const {
        switch (obj) {
            using enum GraphicsEngine::ShaderType;
        case VERTEX:
            return std::format_to(ctx.out(), "VERTEX");
        case PIXEL:
            return std::format_to(ctx.out(), "PIXEL");
        }

        throw GraphicsEngine::Exception("unknown shader type: {}",
                                        std::to_underlying(obj));
    }
};


template <> struct std::formatter<GraphicsEngine::ShaderVariableType> {

    constexpr auto parse(std::format_parse_context &ctx) {
        return std::begin(ctx);
    }

    auto format(const GraphicsEngine::ShaderVariableType &obj,
                std::format_context &ctx) const {
        switch (obj) {
            using enum GraphicsEngine::ShaderVariableType;
        case UNKNOWN:
            return std::format_to(ctx.out(), "UNKNOWN");
        case INT8:
            return std::format_to(ctx.out(), "INT8");
        case UINT8:
            return std::format_to(ctx.out(), "UINT8");
        case INT16:
            return std::format_to(ctx.out(), "INT16");
        case UINT16:
            return std::format_to(ctx.out(), "UINT16");
        case INT32:
            return std::format_to(ctx.out(), "INT32");
        case UINT32:
            return std::format_to(ctx.out(), "UINT32");
        case INT64:
            return std::format_to(ctx.out(), "INT64");
        case UINT64:
            return std::format_to(ctx.out(), "UINT64");
        case FLOAT16:
            return std::format_to(ctx.out(), "FLOAT16");
        case FLOAT32:
            return std::format_to(ctx.out(), "FLOAT32");
        case FLOAT64:
            return std::format_to(ctx.out(), "FLOAT64");
        }

        throw GraphicsEngine::Exception("unknown shader variable type: {}", std::to_underlying(obj));
    }
};

template <> struct std::formatter<GraphicsEngine::ShaderVariableEntry> {
    constexpr auto parse(std::format_parse_context &ctx) {
        return std::begin(ctx);
    }

    auto format(const GraphicsEngine::ShaderVariableEntry &obj,
                std::format_context &ctx) const {
        return std::format_to(
            ctx.out(),
            "Name: {}\nLocation: {}\nType: {}\nSize:{}",
            obj.name, obj.location, obj.type, obj.size);
    }
};
