// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s
// expected-no-diagnostics

static_assert(__is_enum(ImageFormat));
static_assert(static_cast<unsigned>(ImageFormat::Rgba32f) == 1);
static_assert(static_cast<unsigned>(ImageFormat::R64i) == 41);
template <ImageFormat F> struct Format { static constexpr auto value = F; };
static_assert(Format<ImageFormat::R32f>::value == ImageFormat::R32f);

template <class T, unsigned N>
constexpr unsigned extent(const T (&)[N]) { return N; }
static_assert(sizeof(global_invocation_id) == 3 * sizeof(unsigned));
static_assert(__is_same(decltype(global_invocation_id), const unsigned[3]));
static_assert(__is_same(decltype(vertex_idx), const int));
static_assert(__is_same(decltype(instance_idx), const int));
static_assert(__is_same(decltype(primitive_idx), const int));

struct Params { __storage__ __align__(16) float *data; unsigned count; };
static_assert(sizeof(Params) == 16);
static_assert(__builtin_offsetof(Params, count) == 8);

__compute__ __workgroup_size__(64, 1, 1)
void kernel(__push_constant__ const Params *p) {
  const unsigned (&id)[3] = global_invocation_id;
  const unsigned *component = global_invocation_id;
  unsigned x = global_invocation_id.x;
  unsigned n = extent(global_invocation_id);
  __storage__ float *cursor = p->data;
  ++cursor; cursor++; --cursor; cursor--;
  auto difference = (cursor + 1) - cursor;
  bool equal = cursor == p->data;
  *cursor = id[0] + component[1] + x + n + difference + equal;
}

__binding__(2, 7) __uniform__ const Params *uniform_parameters;
