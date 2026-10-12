struct Params {
  __storage float *data;
};
__compute
__workgroup_size(1, 1, 1)
void align(__push_constant const Params *p) {
  auto q = p->data;
  *q *= 2;
  ++q;
  *q *= 2;
}
