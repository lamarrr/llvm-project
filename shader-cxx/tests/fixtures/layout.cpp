struct Inner {
  float coefficients[3];
  unsigned enabled;
};
struct alignas(16) Params {
  __storage float *data;
  unsigned count;
  Inner nested;
  float weights[2];
  unsigned padding[3];
};
static_assert(sizeof(Inner) == 16);
static_assert(alignof(Inner) == 4);
static_assert(sizeof(Params) == 48);
static_assert(alignof(Params) == 16);
static_assert(__builtin_offsetof(Params, nested) == 12);
static_assert(__builtin_offsetof(Params, weights) == 28);
static_assert(__builtin_offsetof(Params, padding) == 36);
__compute
__workgroup_size(1, 1, 1)
void layout(__push_constant const Params *p) {
  p->data[0] = p->weights[1] + p->nested.coefficients[2] + p->nested.enabled;
}
