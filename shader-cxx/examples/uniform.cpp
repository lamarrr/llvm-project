struct Constants {
  float factor;
};
struct Params {
  __storage__ float *data;
  unsigned count;
};
__compute__
__workgroup_size__(64, 1, 1)
void uniform(__binding__(2, 7) __uniform__ const Constants *constants,
             __push_constant__ const Params *p) {
  auto i = global_invocation_id.x;
  if (i < p->count)
    p->data[i] *= constants->factor;
}
