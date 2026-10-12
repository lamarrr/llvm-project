__fragment
void fragment_operations(__binding(0, 0) texture_2d<float> tex,
                         __binding(0, 1) sampler sampler, __input(0) float x,
                         __output(0) float *out) {
  float sample = tex.sample(sampler, x, x);
  float grad = tex.sample_grad(sampler, x, x, 1.0f, 0.0f, 0.0f, 1.0f);
  auto gathered = tex.gather(sampler, x, x, 0);
  *out = sample + grad + gathered[0] + ddx(x) + ddy(x) + fwidth(x) +
         ddx_fine(x) + ddy_fine(x) + ddx_coarse(x) + ddy_coarse(x);
  if (x < 0)
    discard();
}
