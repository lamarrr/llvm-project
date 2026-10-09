struct ClassificationParams {
  __storage__ unsigned *flags;
  float value;
};

__compute__
__workgroup_size__(1, 1, 1)
void classification(__push_constant__ const ClassificationParams *params) {
  float values[2] = {params->value, -params->value};
  auto nan = is_nan(values);
  auto inf = is_inf(values);
  auto finite = is_finite(values);
  auto normal = is_normal(values);
  auto negative = sign_bit(values);
  for (unsigned i = 0; i != 2; ++i)
    params->flags[i] = unsigned(nan[i]) | (unsigned(inf[i]) << 1) |
                       (unsigned(finite[i]) << 2) | (unsigned(normal[i]) << 3) |
                       (unsigned(negative[i]) << 4);
  _Float16 half = (_Float16)params->value;
  double wide = params->value;
  params->flags[2] = is_nan(half) | (is_inf(half) << 1) |
                     (is_finite(half) << 2) | (is_normal(half) << 3) |
                     (sign_bit(half) << 4);
  params->flags[3] = is_nan(wide) | (is_inf(wide) << 1) |
                     (is_finite(wide) << 2) | (is_normal(wide) << 3) |
                     (sign_bit(wide) << 4);
}
