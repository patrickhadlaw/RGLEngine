#pragma once

#include "rgle/Exception.h"

namespace rgle::math {
  namespace quadratic {
    struct None {};
    struct One {
      float x0;
    };
    struct Two {
      float x0;
      float x1;
    };
    using Roots = std::variant<None, One, Two>;

    Roots compute_roots(float a, float b, float c);
  }
  
}
