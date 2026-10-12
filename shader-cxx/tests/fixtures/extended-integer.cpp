struct ExtendedIntegerParams {
  __storage unsigned *out;
  unsigned left;
  unsigned right;
};

__compute
__workgroup_size(1, 1, 1)
void extended_integer(__push_constant const ExtendedIntegerParams *params) {
  unsigned a[2] = {params->left, params->right};
  unsigned b[2] = {params->right, params->left};
  auto sum = add_carry(a, b);
  auto difference = sub_borrow(a, b);
  auto product = mul_extended(a, b);
  params->out[0] = sum.value[0];
  params->out[1] = sum.carry[0];
  params->out[2] = difference.value[1];
  params->out[3] = difference.borrow[1];
  params->out[4] = product.low[0];
  params->out[5] = product.high[0];

  unsigned long long wide = params->left;
  auto wide_sum = add_carry(wide, ~wide);
  auto wide_product = mul_extended(wide, wide);
  params->out[6] = unsigned(wide_sum.carry);
  params->out[7] = unsigned(wide_product.high);
  auto signed_product = mul_extended(int(params->left), int(params->right));
  params->out[8] = unsigned(signed_product.high);
}
