#include "../math-body.h"
struct MathParams {
  __storage float *data;
};
__compute
__workgroup_size(1, 1, 1)
void vector_matrix(__push_constant const MathParams *params) {
  exercise_math(params->data);
}
