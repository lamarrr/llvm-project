#include <cstdio>

struct Three {
  float x, y, z;
};

int main() {
  volatile float input = 0.25f;
  float x = input;
  float vector[3] = {x, -x, 0.0f};
  auto result = acos(vector);
  Three record{x, -x, 0.0f};
  Three record_result = acos(record);
  const float expected[3] = {1.318116071652818f, 1.823476581936975f,
                             1.570796326794897f};
  for (unsigned i = 0; i != 3; ++i)
    if (abs(result[i] - expected[i]) > 0.000002f)
      return 1;
  if (abs(record_result.x - expected[0]) > 0.000002f ||
      abs(record_result.y - expected[1]) > 0.000002f ||
      abs(record_result.z - expected[2]) > 0.000002f ||
      abs(acos(x) - expected[0]) > 0.000002f)
    return 1;
  double d = x;
  if (abs(acos(d) - 1.318116071652818) > 0.000000000001)
    return 1;
  static_assert(__is_same(decltype(acos(1)), double));
  if (acos(1) != 0.0)
    return 1;
  std::puts("host acos: PASS");
}
