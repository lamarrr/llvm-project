float state = 1.0f;
float next() { return ++state; }
struct Params {
  __storage__ float *data;
};
__compute__
__workgroup_size__(1, 1,
                   1) void invocation_state(__push_constant__ const Params *p) {
  p->data[0] = state;
  p->data[1] = next();
  p->data[2] = next();
  p->data[3] = state;
}
