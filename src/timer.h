#pragma once
#include <cstdint>

namespace GraphicsEngine {

class Timer {
  public:
    Timer();

    float tick();
    float get_delta() const;

  private:
    std::uint64_t _current{0};
    float _delta{0};
};

} // namespace GraphicsEngine
