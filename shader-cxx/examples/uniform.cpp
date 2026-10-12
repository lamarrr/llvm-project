struct Constants {
  float factor;
};
struct Params {
  __storage float *data;
  unsigned count;
};
__compute
__workgroup_size(64, 1, 1)
void uniform(__binding(2, 7) __uniform const Constants *constants,
             __push_constant const Params *p) {
  auto i = global_invocation_id.x;
  if (i < p->count)
    p->data[i] *= constants->factor;
}
