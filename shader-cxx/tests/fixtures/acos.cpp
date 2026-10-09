struct Params {
  __storage__ float *data;
};
__compute__
__workgroup_size__(1, 1, 1)
void acos_math(__push_constant__ const Params *params) {
  float x = params->data[0] * 0.25f;
  params->data[0] = acos(x);
  float vector[3] = {x, -x, 0.0f};
  auto result = acos(vector);
  params->data[1] = result[0];
  params->data[2] = result[1];
  params->data[3] = result[2];
}
