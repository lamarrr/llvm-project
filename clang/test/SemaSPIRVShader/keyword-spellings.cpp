// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST_OLD_NAMES -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST_ALIGNAS -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST_REMOVED_ALIGN -fsyntax-only -verify %s

#ifdef TEST_OLD_NAMES
// expected-no-diagnostics
int __any_hit__;
int __binding__;
int __callable__;
int __callable_data__;
int __centroid__;
int __clip_distance__;
int __closest_hit__;
int __compute__;
int __cull_distance__;
int __cull_primitive__;
int __depth_greater__;
int __depth_less__;
int __depth_unchanged__;
int __early_fragment_tests__;
int __flat__;
int __frag_depth__;
int __fragment__;
int __fragment_interlock__;
int __geometry__;
int __geometry_invocations__;
int __geometry_mode__;
int __hit_attribute__;
int __incoming_callable_data__;
int __incoming_ray_payload__;
int __input__;
int __intersection__;
int __invocation_id__;
int __layer__;
int __location__;
int __mesh__;
int __mesh_output__;
int __miss__;
int __noncoherent_color_attachment_read__;
int __noncoherent_depth_attachment_read__;
int __noncoherent_stencil_attachment_read__;
int __noperspective__;
int __output__;
int __output_vertices__;
int __patch__;
int __patch_vertices__;
int __per_primitive__;
int __point_size__;
int __position__;
int __post_depth_coverage__;
int __primitive_id__;
int __primitive_line_indices__;
int __primitive_point_indices__;
int __primitive_triangle_indices__;
int __push_constant__;
int __ray_generation__;
int __ray_payload__;
int __sample__;
int __sample_mask__;
int __shader_record__;
int __shared__;
int __stencil_ref__;
int __storage__;
int __task__;
int __task_payload__;
int __tess_coord__;
int __tess_level_inner__;
int __tess_level_outer__;
int __tessellation_control__;
int __tessellation_evaluation__;
int __tessellation_mode__;
int __uniform__;
int __varying__;
int __vertex__;
int __viewport_index__;
int __workgroup_size__;
int __workgroup_size_id__;
#endif

#ifdef TEST_ALIGNAS
struct alignas(16) Aligned {
  float value;
};
static_assert(alignof(Aligned) == 16);
static_assert(sizeof(Aligned) == 16);
struct PointerObject {
  alignas(16) __storage float *value;
};
static_assert(alignof(PointerObject) == 16);
static_assert(sizeof(PointerObject) == 16);
struct alignas(3)
    Invalid { // expected-error@-1 {{requested alignment is not a power of 2}}
  int value;
};
#endif

#ifdef TEST_REMOVED_ALIGN
void removed_alignment() {
  __align__(16); // expected-error {{use of undeclared identifier '__align__'}}
  __align(16);   // expected-error {{use of undeclared identifier '__align'}}
}
#endif
