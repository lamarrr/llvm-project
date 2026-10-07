struct Params {
  __storage__ float *data;
};
template <class T>
concept Arithmetic = requires(T a) {
  a + a;
  a * a;
};
consteval unsigned length() { return 4; }
constexpr float factor(unsigned n) { return n == 4 ? 2.0f : 1.0f; }
template <Arithmetic T> struct Scale {
  T value;
  T operator()(T x) const { return x * value; }
};
__compute__
__workgroup_size__(1, 1, 1) void cxx20(__push_constant__ const Params *p) {
  Scale<float> operation{factor(length())};
  auto apply = [&](unsigned i) { p->data[i] = operation(p->data[i]); };
  for (unsigned i = 0; i < length(); ++i)
    apply(i);
}
