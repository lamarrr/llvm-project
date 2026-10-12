// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s
// expected-no-diagnostics

// Pin public types independently of the compiler's builtin catalog.
#define SCALAR(Name, T) static_assert(__is_same(decltype(Name), const T))
#define ARRAY(Name, T, N) static_assert(__is_same(decltype(Name), const T[N]))
ARRAY(global_invocation_id, unsigned, 3);
ARRAY(local_invocation_id, unsigned, 3);
ARRAY(workgroup_id, unsigned, 3);
ARRAY(num_workgroups, unsigned, 3);
ARRAY(workgroup_size, unsigned, 3);
SCALAR(local_invocation_idx, unsigned);
SCALAR(subgroup_size, unsigned);
SCALAR(num_subgroups, unsigned);
SCALAR(subgroup_id, unsigned);
SCALAR(subgroup_invocation_id, unsigned);
SCALAR(vertex_idx, int);
SCALAR(instance_idx, int);
SCALAR(primitive_idx, int);
SCALAR(invocation_id, int);
SCALAR(patch_vertices_in, int);
ARRAY(tess_coord, float, 3);
ARRAY(tess_level_outer, float, 4);
ARRAY(tess_level_inner, float, 2);
ARRAY(frag_coord, float, 4);
ARRAY(point_coord, float, 2);
SCALAR(front_facing, bool);
SCALAR(sample_id, int);
ARRAY(sample_position, float, 2);
ARRAY(bary_coord, float, 3);
ARRAY(bary_coord_noperspective, float, 3);
SCALAR(shading_rate, unsigned);
SCALAR(helper_invocation, bool);
SCALAR(view_idx, unsigned);
SCALAR(base_vertex, int);
SCALAR(base_instance, int);
SCALAR(draw_idx, int);
ARRAY(launch_id, unsigned, 3);
ARRAY(launch_size, unsigned, 3);
ARRAY(world_ray_origin, float, 3);
ARRAY(world_ray_direction, float, 3);
ARRAY(object_ray_origin, float, 3);
ARRAY(object_ray_direction, float, 3);
SCALAR(ray_tmin, float);
SCALAR(ray_tmax, float);
SCALAR(instance_custom_idx, int);
SCALAR(hit_kind, unsigned);
SCALAR(incoming_ray_flags, unsigned);
SCALAR(ray_geometry_idx, int);
#undef SCALAR
#undef ARRAY

#define FORMAT(Name, Value)                                                    \
  static_assert(Name == Value);                                                \
  static_assert(format::Name == Name);                                         \
  static_assert(__is_same(decltype(Name), format))
FORMAT(unknown, 0);
FORMAT(rgba32f, 1);
FORMAT(rgba16f, 2);
FORMAT(r32f, 3);
FORMAT(rgba8, 4);
FORMAT(rgba8_snorm, 5);
FORMAT(rg32f, 6);
FORMAT(rg16f, 7);
FORMAT(r11f_g11f_b10f, 8);
FORMAT(r16f, 9);
FORMAT(rgba16, 10);
FORMAT(rgb10_a2, 11);
FORMAT(rg16, 12);
FORMAT(rg8, 13);
FORMAT(r16, 14);
FORMAT(r8, 15);
FORMAT(rgba16_snorm, 16);
FORMAT(rg16_snorm, 17);
FORMAT(rg8_snorm, 18);
FORMAT(r16_snorm, 19);
FORMAT(r8_snorm, 20);
FORMAT(rgba32i, 21);
FORMAT(rgba16i, 22);
FORMAT(rgba8i, 23);
FORMAT(r32i, 24);
FORMAT(rg32i, 25);
FORMAT(rg16i, 26);
FORMAT(rg8i, 27);
FORMAT(r16i, 28);
FORMAT(r8i, 29);
FORMAT(rgba32ui, 30);
FORMAT(rgba16ui, 31);
FORMAT(rgba8ui, 32);
FORMAT(r32ui, 33);
FORMAT(rgb10_a2ui, 34);
FORMAT(rg32ui, 35);
FORMAT(rg16ui, 36);
FORMAT(rg8ui, 37);
FORMAT(r16ui, 38);
FORMAT(r8ui, 39);
FORMAT(r64ui, 40);
FORMAT(r64i, 41);
#undef FORMAT
static_assert(__is_same(__underlying_type(format), unsigned));
static_assert(__is_same(__underlying_type(memory_order), unsigned));
static_assert(__is_same(__underlying_type(memory_scope), unsigned));
static_assert((unsigned)memory_order::relaxed == 0);
static_assert((unsigned)memory_order::acquire == 2);
static_assert((unsigned)memory_order::release == 4);
static_assert((unsigned)memory_order::acquire_release == 8);
static_assert((unsigned)memory_order::seq_cst == 16);
static_assert((unsigned)memory_scope::device == 1);
static_assert((unsigned)memory_scope::workgroup == 2);
static_assert((unsigned)memory_scope::subgroup == 3);
static_assert(sizeof(__SIZE_TYPE__) == 8);
static_assert(sizeof(__PTRDIFF_TYPE__) == 8);

void components() {
  const unsigned *decayed = global_invocation_id;
  const unsigned (&borrowed)[3] = global_invocation_id;
  static_assert(__is_same(decltype(global_invocation_id.x), const unsigned &));
  unsigned xyz =
      global_invocation_id.x + global_invocation_id.y + global_invocation_id.z;
  float xyzw = frag_coord.x + frag_coord.y + frag_coord.z + frag_coord.w;
  unsigned roundtrip = decayed[2] + borrowed[1] + xyz;
}
