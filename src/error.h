#pragma once

#include <format>
#include <string_view>

#include "auto_release.h"
#include "exception.h"

namespace GraphicsEngine {

template <class... Args>
void ensure(bool predicate, std::string_view msg, Args &&...args) {
    if (!predicate) {
        throw Exception(std::vformat(msg, std::make_format_args(args...)), 2u);
    }
}

template <class T, T Invalid, class... Args>
void ensure(AutoRelease<T, Invalid> &obj, std::string_view msg,
            Args &&...args) {
    ensure(!!obj, msg, std::forward<Args>(args)...);
}

} // namespace GraphicsEngine
