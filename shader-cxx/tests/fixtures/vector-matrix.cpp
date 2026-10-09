#include "../math-body.h"
struct MathParams {
  __storage__ float *data;
};
__compute__
__workgroup_size__(1, 1, 1)
void vector_matrix(__push_constant__ const MathParams *params) {
  exercise_math(params->data);
}
