int main() {
  float x = .25f;
  float v[2] = {x, .75f};
  struct R {
    float x, y;
  } r{x, .75f};
  int signs[2] = {-2, 0};
  unsigned positive[2] = {0, 0xffffffffu};
  return round(x) != 0 || round_even(2.5f) != 2 || round_even(3.5f) != 4 ||
         trunc(-1.75f) != -1 || sign(signs)[0] != -1 ||
         sign(positive)[1] != 1 || fract(-1.25f) != .75f || exp2(3.0f) != 8 ||
         log2(8.0f) != 3 || inverse_sqrt(4.0f) != .5f || step(.5f, x) != 0 ||
         smoothstep(0.0f, 1.0f, .5f) != .5f || fma(2.0f, 3.0f, 4.0f) != 10 ||
         mod(-3.0f, 2.0f) != 1 || round(v)[1] != 1 || fract(r).y != .75f;
}
