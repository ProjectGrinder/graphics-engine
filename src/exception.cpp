#include "exception.h"

namespace GraphicsEngine {

Exception::Exception(const std::string &what, std::uint32_t skip)
    : std::runtime_error(what), _trace{std::stacktrace::current(skip)} {}

std::string Exception::stacktrace() const { return std::to_string(_trace); }

} // namespace GraphicsEngine
