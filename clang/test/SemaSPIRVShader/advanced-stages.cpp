// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s

// expected-error@+1 {{geometry mode requires}}
__geometry_mode(18, 29, 3)
void wrong_geometry();
// expected-error@+1 {{geometry invocations must be positive}}
__geometry_invocations(0)
void wrong_invocations();
// expected-error@+1 {{output vertex count must be positive}}
__output_vertices(0)
void wrong_vertices();
// expected-error@+1 {{tessellation mode requires}}
__tessellation_mode(22, 0, 5)
void wrong_tessellation();
// expected-error@+1 {{mesh output requires}}
__mesh_output(29, 3, 1)
void wrong_mesh();
// expected-error@+1 {{shader payload and record parameters must be pointers}}
void wrong_payload(__ray_payload unsigned payload);

struct Float3 {
  float x, y, z;
};
void invalid_calls(acceleration_structure scene, unsigned *local) {
  // expected-error@+1 {{execute_callable requires a writable callable data pointer}}
  execute_callable(0u, local);
  // expected-error@+1 {{emit_mesh_tasks requires a task payload pointer}}
  emit_mesh_tasks(1u, 1u, 1u, local);
  // expected-error@+1 {{geometry stream must be a constant in 0..3}}
  emit_stream_vertex(4u);
  // expected-error@+1 {{trace_ray requires an acceleration structure}}
  trace_ray(scene, 0u, 255u, 0u, 0u, 0u, Float3{0, 0, 0}, 0.0f, Float3{0, 0, 1},
            1.0f, local);
}
