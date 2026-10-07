struct Params {
  __storage__ __align__(16) float *data;
};
__compute__
__workgroup_size__(1, 1, 1) void align(__push_constant__ const Params *p) {
  auto q = p->data;
  *q *= 2;
  ++q;
  *q *= 2;
}
