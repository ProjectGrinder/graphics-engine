#pragma once

#include <cstddef>

namespace GraphicsEngine {

class Arena {
  public:
    explicit Arena(size_t reserve_size = 64 * 1024 * 1024, bool exec = false);
    ~Arena();

    Arena(const Arena &) = delete;
    Arena &operator=(const Arena &) = delete;
    Arena(Arena &&other) noexcept;
    Arena &operator=(Arena &&other) noexcept;

    void *alloc(size_t size, size_t alignment = alignof(std::max_align_t));
    bool protect();
    void reset();
    void free_all();

    size_t capacity() const { return _size; }
    size_t allocated() const { return _allocate; }
    size_t committed() const { return _committed; }

  private:
    void *_memory;
    size_t _size;
    size_t _allocate;
    size_t _committed;
    bool _exec;
    bool _finalized{false};
};

} // namespace GraphicsEngine
