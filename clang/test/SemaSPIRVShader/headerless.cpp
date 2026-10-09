// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s
// expected-no-diagnostics

static_assert(__is_enum(format));
static_assert(!__is_scoped_enum(format));
static_assert(__is_same(decltype(r32f), format));
constexpr unsigned numeric_format = r32f;
static_assert(numeric_format == 3);
static_assert(format::r32f == r32f);
using storage_image = storage_texture_2d<float, r32f>;
static_assert(__is_same(storage_image, storage_texture_2d<float, format::r32f>));
static_assert(static_cast<unsigned>(rgba32f) == 1);
static_assert(static_cast<unsigned>(r64i) == 41);
template <format F>
struct Format {
  static constexpr auto value = F;
};
static_assert(Format<r32f>::value == r32f);

template <class T, unsigned N>
constexpr unsigned extent(const T (&)[N]) { return N; }
static_assert(sizeof(global_invocation_id) == 3 * sizeof(unsigned));
static_assert(__is_same(decltype(global_invocation_id), const unsigned[3]));
static_assert(__is_same(decltype(vertex_idx), const int));
static_assert(__is_same(decltype(instance_idx), const int));
static_assert(__is_same(decltype(primitive_idx), const int));

struct Params {
  __storage__ __align__(16) float *data;
  unsigned count;
};
static_assert(sizeof(Params) == 16);
static_assert(__builtin_offsetof(Params, count) == 8);

__compute__
__workgroup_size__(64, 1, 1)
void kernel(__push_constant__ const Params *p) {
  const unsigned (&id)[3] = global_invocation_id;
  const unsigned *component = global_invocation_id;
  unsigned x = global_invocation_id.x;
  unsigned n = extent(global_invocation_id);
  __storage__ float *cursor = p->data;
  ++cursor;
  cursor++;
  --cursor;
  cursor--;
  auto difference = (cursor + 1) - cursor;
  bool equal = cursor == p->data;
  *cursor = id[0] + component[1] + x + n + difference + equal;
}

__compute__
__workgroup_size__(1, 1, 1)
void uniforms(__binding__(2, 7) __uniform__ const Params *uniform_parameters);

// The former enum name is available as an ordinary identifier.
int ImageFormat, image_format;
