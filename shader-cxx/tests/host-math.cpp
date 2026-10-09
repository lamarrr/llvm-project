#include "math-body.h"
#include <cmath>
#include <cstdio>
int main() {
  float data[78] = {1};
  exercise_math(data);
  const float expected[78] = {32,
                              std::sqrt(14.0f),
                              std::sqrt(27.0f),
                              1 / std::sqrt(14.0f),
                              -3,
                              -1,
                              -1,
                              -1,
                              5,
                              -3,
                              4,
                              .25f,
                              1,
                              4,
                              1,
                              2.5f,
                              1,
                              1,
                              1,
                              1,
                              std::sin(1.0f),
                              std::cos(1.0f),
                              std::exp(1.0f),
                              0,
                              1,
                              1 / std::sqrt(14.0f),
                              1,
                              7,
                              3,
                              -2,
                              -2,
                              58,
                              154,
                              1,
                              1,
                              1,
                              6,
                              10,
                              3,
                              5,
                              6,
                              4,
                              2,
                              7,
                              3,
                              4,
                              2,
                              2,
                              2,
                              3,
                              0,
                              1,
                              1,
                              0,
                              8,
                              -2,
                              7,
                              1.5f,
                              -2,
                              1,
                              2,
                              -1,
                              -3,
                              -2,
                              -2,
                              1,
                              5 / std::sqrt(155.0f),
                              -3,
                              10,
                              2,
                              -.5f,
                              4,
                              9,
                              12,
                              9,
                              7,
                              18,
                              2};
  for (unsigned i = 0; i < 78; ++i) {
    if (!std::isfinite(data[i]) || std::fabs(data[i] - expected[i]) > 1e-5f) {
      std::fprintf(stderr, "math[%u]: got %g expected %g\n", i, data[i],
                   expected[i]);
      return 1;
    }
  }
  bool a[2] = {true, false};
  bool b[2] = {true, true};
  auto sum = add(a, b);
  auto difference = sub(a, b);
  auto minimum = min(a, b);
  if (!sum[0] || !sum[1] || difference[0] || !difference[1] || !minimum[0] ||
      minimum[1] || add(true, true) != 2) {
    std::fputs("bool arithmetic did not use promoted C++ operands\n", stderr);
    return 1;
  }
  puts("host vector/matrix math: PASS");
}
