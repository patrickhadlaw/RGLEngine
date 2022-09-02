#include "rgle/Math.h"

rgle::math::quadratic::Roots rgle::math::quadratic::compute_roots(float a, float b, float c) {
  float discriminant = b * b - 4 * a * c;
  if (discriminant >= 0.0f) {
    return Roots(Two {
      .x0 = (-b + std::sqrt(discriminant)) / (2 * a),
      .x1 = (-b - std::sqrt(discriminant)) / (2 * a)
    });
  }
  else if (discriminant == 0.0f) {
    return Roots(One {
      .x0 = -b / (2 * a),
    });
  }
  return Roots(None {});
}
