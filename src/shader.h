#pragma once

#include "auto_release.h"
#include "exception.h"
#include <format>
#include <utility>
namespace GraphicsEngine {

using ShaderBinary = uint8_t *;

enum class ShaderType { VERTEX, PIXEL };

class Shader {
  public:
    Shader(const std::string &source, ShaderType type);

    ShaderType type() const;
    uint64_t size() const;

  private:
    AutoRelease<ShaderBinary> _spirv;
    ShaderType _type;
    uint64_t _size;
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
