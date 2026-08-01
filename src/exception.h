#pragma once

#include <cstdint>
#include <format>
#include <iterator>
#include <stacktrace>
#include <stdexcept>
#include <string>

namespace GraphicsEngine {

class Exception : public std::runtime_error {
  public:
    Exception(const std::string &what, std::uint32_t skip = 1u);

    std::string stacktrace() const;

  private:
    std::stacktrace _trace;
};

} // namespace GraphicsEngine

template <> struct std::formatter<GraphicsEngine::Exception> {
    constexpr auto parse(std::format_parse_context &ctx) {
        return std::begin(ctx);
    }

    auto format(const GraphicsEngine::Exception &obj,
                std::format_context &ctx) const {
        return std::format_to(ctx.out(), "{}\n{}", obj.what(),
                              obj.stacktrace());
    }
};
