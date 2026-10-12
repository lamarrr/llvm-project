#include <spirv/types.hpp>

using namespace spirv;

struct PrintParams {
  float value;
  int index;
};

__compute__
__workgroup_size__(1, 1, 1)
void printf_test(__push_constant__ const PrintParams *params) {
  printf("value %f", params->value);
  printf("value %f", params->value);
  printf("index %d vector %v2f", params->index, f32x2(params->value, 2.0f));
  printf("wide %lu", 1ull);
  printf("literal %% and quote \"%d\" $", params->index);
  printf("path C:\\data");
  printf("no values");
}
