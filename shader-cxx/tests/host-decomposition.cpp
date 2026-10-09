int main() {
  float x = 6.5f;
  float v[2] = {x, -x};
  int exp[2] = {2, 3};
  auto a = modf(x);
  auto b = frexp(x);
  auto c = modf(v);
  auto d = frexp(v);
  auto e = ldexp(v, exp);
  return a.fraction != .5f || a.whole != 6 || b.significand != .8125f ||
         b.exponent != 3 || c.fraction[1] != -.5f || c.whole[1] != -6 ||
         d.significand[1] != -.8125f || d.exponent[1] != 3 || e[0] != 26 ||
         e[1] != -52;
}
