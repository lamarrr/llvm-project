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

#define FORMAT(Name, Value) static_assert(static_cast<unsigned>(ImageFormat::Name) == Value)
FORMAT(Unknown, 0); FORMAT(Rgba32f, 1); FORMAT(Rgba16f, 2);
FORMAT(R32f, 3); FORMAT(Rgba8, 4); FORMAT(Rgba8Snorm, 5);
FORMAT(Rg32f, 6); FORMAT(Rg16f, 7); FORMAT(R11fG11fB10f, 8);
FORMAT(R16f, 9); FORMAT(Rgba16, 10); FORMAT(Rgb10A2, 11);
FORMAT(Rg16, 12); FORMAT(Rg8, 13); FORMAT(R16, 14); FORMAT(R8, 15);
FORMAT(Rgba16Snorm, 16); FORMAT(Rg16Snorm, 17); FORMAT(Rg8Snorm, 18);
FORMAT(R16Snorm, 19); FORMAT(R8Snorm, 20); FORMAT(Rgba32i, 21);
FORMAT(Rgba16i, 22); FORMAT(Rgba8i, 23); FORMAT(R32i, 24);
FORMAT(Rg32i, 25); FORMAT(Rg16i, 26); FORMAT(Rg8i, 27);
FORMAT(R16i, 28); FORMAT(R8i, 29); FORMAT(Rgba32ui, 30);
FORMAT(Rgba16ui, 31); FORMAT(Rgba8ui, 32); FORMAT(R32ui, 33);
FORMAT(Rgb10a2ui, 34); FORMAT(Rg32ui, 35); FORMAT(Rg16ui, 36);
FORMAT(Rg8ui, 37); FORMAT(R16ui, 38); FORMAT(R8ui, 39);
FORMAT(R64ui, 40); FORMAT(R64i, 41);
#undef FORMAT
static_assert(__is_same(__underlying_type(ImageFormat), unsigned));
static_assert(sizeof(__SIZE_TYPE__) == 8);
static_assert(sizeof(__PTRDIFF_TYPE__) == 8);

void components() {
  const unsigned *decayed = global_invocation_id;
  const unsigned (&borrowed)[3] = global_invocation_id;
  static_assert(__is_same(decltype(global_invocation_id.x), const unsigned &));
  unsigned xyz = global_invocation_id.x + global_invocation_id.y + global_invocation_id.z;
  float xyzw = frag_coord.x + frag_coord.y + frag_coord.z + frag_coord.w;
  unsigned roundtrip = decayed[2] + borrowed[1] + xyz;
}
