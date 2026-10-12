#include <spirv/types.hpp>
using namespace spirv;

struct VectorData {
  f32x3 value;
};

struct Params {
  __storage__ float *result;
  f32x3 push_value;
};

__compute__
__workgroup_size__(1, 1, 1)
void logical_vector_index(__push_constant__ const Params *params,
                          __binding__(0, 0)
                          __uniform__ const VectorData *uniform_data) {
  params->result[0] = params->push_value.data[1] + uniform_data->value.data[2];
}

__compute__
__workgroup_size__(1, 1, 1)
void physical_large_offset(__push_constant__ const Params *params) {
  params->result[0x100000000ULL] = 1.0f;
}
