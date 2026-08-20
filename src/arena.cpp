#include "arena.h"
#include <cstdint>

#define KB(bytes) ((unsigned long long)(bytes) * 1024ULL)
#define MB(bytes) (KB(bytes) * 1024ULL)
#define GB(bytes) (MB(bytes) * 1024ULL)

#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define VM_RESERVE(size) VirtualAlloc(NULL, (size), MEM_RESERVE, PAGE_READWRITE)
#define VM_COMMIT(ptr, size)                                                   \
    VirtualAlloc((ptr), (size), MEM_COMMIT, PAGE_READWRITE)
#define VM_DECOMMIT(ptr, size) VirtualFree((ptr), (size), MEM_DECOMMIT)
#define VM_FREE(ptr, size) ((void)(size), VirtualFree((ptr), 0, MEM_RELEASE))
#define VM_FAILED(ptr) ((ptr) == NULL)

#elif defined(__unix__) || defined(__APPLE__) || defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

#define VM_RESERVE(size)                                                       \
    mmap(NULL, (size), PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)
#define VM_COMMIT(ptr, size)                                                   \
    (mprotect((ptr), (size), PROT_READ | PROT_WRITE) == 0 ? (ptr) : MAP_FAILED)
#define VM_DECOMMIT(ptr, size)                                                 \
    (madvise((ptr), (size), MADV_DONTNEED), mprotect((ptr), (size), PROT_NONE))
#define VM_FREE(ptr, size) munmap((ptr), (size))
#define VM_FAILED(ptr) ((ptr) == MAP_FAILED || (ptr) == NULL)

#else
#error "Unsupported platform for native virtual memory macros."
#endif

namespace GraphicsEngine {

Arena::Arena(size_t reserve_size, bool exec)
    : _memory(nullptr), _size(reserve_size), _allocate(0), _committed(0),
      _exec(exec), _finalized(false) {
    _memory = VM_RESERVE(_size);

    if (VM_FAILED(_memory)) {
        _memory = nullptr;
        _size = 0;
    }
}

Arena::~Arena() { free_all(); }

Arena::Arena(Arena &&other) noexcept
    : _memory(other._memory), _size(other._size), _allocate(other._allocate),
      _committed(other._committed), _exec(other._exec),
      _finalized(other._finalized) {
    other._memory = nullptr;
    other._size = 0;
    other._allocate = 0;
    other._committed = 0;
    other._exec = false;
    other._finalized = false;
}

Arena &Arena::operator=(Arena &&other) noexcept {
    if (this != &other) {
        free_all();

        _memory = other._memory;
        _size = other._size;
        _allocate = other._allocate;
        _committed = other._committed;
        _exec = other._exec;
        _finalized = other._finalized;

        other._memory = nullptr;
        other._size = 0;
        other._allocate = 0;
        other._committed = 0;
        other._exec = false;
        other._finalized = false;
    }
    return *this;
}

void *Arena::alloc(size_t size, size_t alignment) {
    if (!_memory || size == 0 || _finalized) {
        return nullptr;
    }

    uintptr_t curr =
        reinterpret_cast<uintptr_t>(static_cast<char *>(_memory) + _allocate);
    uintptr_t aligned = (curr + (alignment - 1)) & ~(alignment - 1);
    size_t padding = aligned - curr;
    size_t total_needed = size + padding;

    if (_allocate + total_needed > _size) {
        return nullptr;
    }

    if (_allocate + total_needed > _committed) {
        constexpr size_t PAGE_SIZE = 4096;
        size_t commit_target =
            (_allocate + total_needed + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        if (commit_target > _size) {
            commit_target = _size;
        }

        size_t bytes_to_commit = commit_target - _committed;
        void *commit_ptr = static_cast<char *>(_memory) + _committed;

        void *res = VM_COMMIT(commit_ptr, bytes_to_commit);

        if (VM_FAILED(res)) {
            return nullptr;
        }

        _committed = commit_target;
    }

    _allocate += total_needed;
    return reinterpret_cast<void *>(aligned);
}

bool Arena::protect() {
    if (!_exec || !_memory || _committed == 0 || _finalized) {
        return false;
    }

#if defined(_WIN32) || defined(_WIN64)
    DWORD old_protect;
    if (!VirtualProtect(_memory, _committed, PAGE_EXECUTE_READ, &old_protect)) {
        return false;
    }
#else
    if (mprotect(_memory, _committed, PROT_READ | PROT_EXEC) != 0) {
        return false;
    }
#endif

    _finalized = true;
    return true;
}

void Arena::reset() {
    if (_finalized) {
#if defined(_WIN32) || defined(_WIN64)
        DWORD old_protect;
        VirtualProtect(_memory, _committed, PAGE_READWRITE, &old_protect);
#else
        mprotect(_memory, _committed, PROT_READ | PROT_WRITE);
#endif
        _finalized = false;
    }
    _allocate = 0;
}

void Arena::free_all() {
    if (_memory) {
        if (_committed > 0) {
            VM_DECOMMIT(_memory, _committed);
        }
        VM_FREE(_memory, _size);
        _memory = nullptr;
    }
    _size = 0;
    _allocate = 0;
    _committed = 0;
    _finalized = false;
}

} // namespace GraphicsEngine
