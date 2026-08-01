#pragma once
#include <functional>

namespace GraphicsEngine {
template <class T, T Invalid = {}> class AutoRelease {
  public:
    AutoRelease() : AutoRelease(Invalid, nullptr) {}

    AutoRelease(T obj, std::function<void(T)> deleter)
        : _obj(obj), _deleter(deleter) {}

    ~AutoRelease() {
        if (_obj != Invalid && _deleter) {
            _deleter(_obj);
        }
    }

    AutoRelease(AutoRelease &&other) : AutoRelease() { swap(other); }
    AutoRelease &operator=(AutoRelease &&other) {
        AutoRelease new_obj{std::move(other)};
        swap(new_obj);
        return *this;
    }

    AutoRelease(const AutoRelease &) = delete;

    AutoRelease &operator=(const AutoRelease &) = delete;

    void swap(AutoRelease &other) noexcept {
        std::ranges::swap(_obj, other._obj);
        std::ranges::swap(_deleter, other._deleter);
    }

    T get() const { return _obj; };

    operator T() const { return _obj; }

    explicit operator bool() const { return _obj != Invalid; }

  private:
    T _obj;
    std::function<void(T)> _deleter;
};
} // namespace GraphicsEngine
