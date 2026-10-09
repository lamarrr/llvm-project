#include <cstdio>
struct R {
  float x, y;
};
int main() {
  volatile float input = .25f;
  float x = input;
  double d = x;
  float a[2] = {x, x};
  R r{x, x};
  if (abs(sin(x) - 0.24740395925452294f) > 2e-6f ||
      abs(sin(d) - 0.24740395925452294) > 2e-12)
    return 1;
  auto a0 = sin(a);
  auto r0 = sin(r);
  if (abs(a0[0] - 0.24740395925452294f) > 2e-6f ||
      abs(r0.x - 0.24740395925452294f) > 2e-6f)
    return 1;
  if (abs(cos(x) - 0.96891242171064473f) > 2e-6f ||
      abs(cos(d) - 0.96891242171064473) > 2e-12)
    return 1;
  auto a1 = cos(a);
  auto r1 = cos(r);
  if (abs(a1[0] - 0.96891242171064473f) > 2e-6f ||
      abs(r1.x - 0.96891242171064473f) > 2e-6f)
    return 1;
  if (abs(tan(x) - 0.25534192122103627f) > 2e-6f ||
      abs(tan(d) - 0.25534192122103627) > 2e-12)
    return 1;
  auto a2 = tan(a);
  auto r2 = tan(r);
  if (abs(a2[0] - 0.25534192122103627f) > 2e-6f ||
      abs(r2.x - 0.25534192122103627f) > 2e-6f)
    return 1;
  if (abs(asin(x) - 0.25268025514207865f) > 2e-6f ||
      abs(asin(d) - 0.25268025514207865) > 2e-12)
    return 1;
  auto a3 = asin(a);
  auto r3 = asin(r);
  if (abs(a3[0] - 0.25268025514207865f) > 2e-6f ||
      abs(r3.x - 0.25268025514207865f) > 2e-6f)
    return 1;
  if (abs(acos(x) - 1.318116071652818f) > 2e-6f ||
      abs(acos(d) - 1.318116071652818) > 2e-12)
    return 1;
  auto a4 = acos(a);
  auto r4 = acos(r);
  if (abs(a4[0] - 1.318116071652818f) > 2e-6f ||
      abs(r4.x - 1.318116071652818f) > 2e-6f)
    return 1;
  if (abs(atan(x) - 0.24497866312686414f) > 2e-6f ||
      abs(atan(d) - 0.24497866312686414) > 2e-12)
    return 1;
  auto a5 = atan(a);
  auto r5 = atan(r);
  if (abs(a5[0] - 0.24497866312686414f) > 2e-6f ||
      abs(r5.x - 0.24497866312686414f) > 2e-6f)
    return 1;
  if (abs(atan2(x, -x) - 2.3561944901923448f) > 2e-6f ||
      abs(atan2(d, -d) - 2.3561944901923448) > 2e-12)
    return 1;
  auto a6 = atan2(a, a);
  auto r6 = atan2(r, r);
  if (abs(a6[0] - 0.78539816339744828f) > 2e-6f ||
      abs(r6.x - 0.78539816339744828f) > 2e-6f)
    return 1;
  if (abs(sinh(x) - 0.25261231680816831f) > 2e-6f ||
      abs(sinh(d) - 0.25261231680816831) > 2e-12)
    return 1;
  auto a7 = sinh(a);
  auto r7 = sinh(r);
  if (abs(a7[0] - 0.25261231680816831f) > 2e-6f ||
      abs(r7.x - 0.25261231680816831f) > 2e-6f)
    return 1;
  if (abs(cosh(x) - 1.0314130998795732f) > 2e-6f ||
      abs(cosh(d) - 1.0314130998795732) > 2e-12)
    return 1;
  auto a8 = cosh(a);
  auto r8 = cosh(r);
  if (abs(a8[0] - 1.0314130998795732f) > 2e-6f ||
      abs(r8.x - 1.0314130998795732f) > 2e-6f)
    return 1;
  if (abs(tanh(x) - 0.24491866240370913f) > 2e-6f ||
      abs(tanh(d) - 0.24491866240370913) > 2e-12)
    return 1;
  auto a9 = tanh(a);
  auto r9 = tanh(r);
  if (abs(a9[0] - 0.24491866240370913f) > 2e-6f ||
      abs(r9.x - 0.24491866240370913f) > 2e-6f)
    return 1;
  if (abs(asinh(x) - 0.24746646154726346f) > 2e-6f ||
      abs(asinh(d) - 0.24746646154726346) > 2e-12)
    return 1;
  auto a10 = asinh(a);
  auto r10 = asinh(r);
  if (abs(a10[0] - 0.24746646154726346f) > 2e-6f ||
      abs(r10.x - 0.24746646154726346f) > 2e-6f)
    return 1;
  if (abs(acosh(x + 1.0f) - 0.69314718055994529f) > 2e-6f ||
      abs(acosh(d + 1.0) - 0.69314718055994529) > 2e-12)
    return 1;
  float positive[2] = {x + 1, x + 1};
  R b{x + 1, x + 1};
  auto a11 = acosh(positive);
  auto r11 = acosh(b);
  if (abs(a11[0] - 0.69314718055994529f) > 2e-6f ||
      abs(r11.x - 0.69314718055994529f) > 2e-6f)
    return 1;
  if (abs(atanh(x) - 0.25541281188299536f) > 2e-6f ||
      abs(atanh(d) - 0.25541281188299536) > 2e-12)
    return 1;
  auto a12 = atanh(a);
  auto r12 = atanh(r);
  if (abs(a12[0] - 0.25541281188299536f) > 2e-6f ||
      abs(r12.x - 0.25541281188299536f) > 2e-6f)
    return 1;
  if (abs(radians(x) - 0.0043633231299858239f) > 2e-6f ||
      abs(radians(d) - 0.0043633231299858239) > 2e-12)
    return 1;
  auto a13 = radians(a);
  auto r13 = radians(r);
  if (abs(a13[0] - 0.0043633231299858239f) > 2e-6f ||
      abs(r13.x - 0.0043633231299858239f) > 2e-6f)
    return 1;
  if (abs(degrees(x) - 14.323944878270581f) > 2e-6f ||
      abs(degrees(d) - 14.323944878270581) > 2e-12)
    return 1;
  auto a14 = degrees(a);
  auto r14 = degrees(r);
  if (abs(a14[0] - 14.323944878270581f) > 2e-6f ||
      abs(r14.x - 14.323944878270581f) > 2e-6f)
    return 1;
  std::puts("host complete trig: PASS");
}
