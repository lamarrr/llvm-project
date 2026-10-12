struct Params {
  __storage float *data;
};
template <class T>
concept Arithmetic = requires(T a) {
  a + a;
  a * a;
};
consteval unsigned element_count() { return 4; }
constexpr float factor(unsigned n) { return n == 4 ? 2.0f : 1.0f; }
template <Arithmetic T> struct Scale {
  T value;
  T operator()(T x) const { return x * value; }
};
__compute
__workgroup_size(1, 1, 1)
void cxx20(__push_constant const Params *p) {
  Scale<float> operation{factor(element_count())};
  auto apply = [&](unsigned i) { p->data[i] = operation(p->data[i]); };
  for (unsigned i = 0; i < element_count(); ++i)
    apply(i);
}
