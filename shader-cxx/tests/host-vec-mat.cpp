#include <types.hpp>

constexpr f32x3 components(1.0f, 2.0f, 3.0f);
static_assert(components.x() == 1.0f);
static_assert(components.yx()[0] == 2.0f);
static_assert(components.zyx()[2] == 1.0f);
static_assert(components.xx()[1] == 1.0f);
static_assert(sizeof(f32x3) == 3 * sizeof(f32));
static_assert(alignof(f32x3) == alignof(f32));

constexpr f32x2x2 matrix(1.0f, 2.0f, 3.0f, 4.0f);
static_assert(matrix[0][1] == 3.0f);
static_assert(sizeof(f32x2x2) == 4 * sizeof(f32));
static_assert(alignof(f32x2x2) == alignof(f32));

int main() {
  f32x2 a(1.0f, 2.0f);
  f32x2 b(3.0f, 4.0f);
  f32x2 sum = a + b;
  a.x() = 5.0f;
  f32x2 column = matrix * f32x2(5.0f, 6.0f);
  f32x2 row = f32x2(5.0f, 6.0f) * matrix;
  f32x2x2 product = matrix * matrix;
  f32x2x3 lhs(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
  f32x3x2 rhs(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
  f32x2x2 rectangular = lhs * rhs;
  return sum[0] == 4.0f && sum[1] == 6.0f && a.x() == 5.0f &&
                 column[0] == 23.0f && column[1] == 34.0f && row[0] == 17.0f &&
                 row[1] == 39.0f && product[0][0] == 7.0f &&
                 product[1][1] == 22.0f && rectangular[0][0] == 22.0f &&
                 rectangular[1][1] == 64.0f
             ? 0
             : 1;
}
