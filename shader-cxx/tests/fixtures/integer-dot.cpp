#include <spirv/types.hpp>
using namespace spirv;

struct Output {
  __storage int *data;
};

__compute
__workgroup_size(1, 1, 1)
void integer_dot_test(__push_constant const Output *output) {
  i8x4 signed_values(1, 2, 3, 4);
  u8x4 unsigned_values(5, 6, 7, 8);
  output->data[0] = integer_dot(signed_values, signed_values);
  output->data[1] = integer_dot(signed_values, unsigned_values);
  output->data[2] = integer_dot_accumulate_sat(signed_values, signed_values, 2);
  output->data[3] =
      integer_dot_accumulate_sat(signed_values, unsigned_values, 2);
  output->data[4] =
      static_cast<int>(integer_dot(unsigned_values, unsigned_values));
  output->data[5] = static_cast<int>(
      integer_dot_accumulate_sat(unsigned_values, unsigned_values, 2u));
}
