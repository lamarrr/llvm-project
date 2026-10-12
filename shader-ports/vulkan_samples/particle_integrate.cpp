// Port of upstream/particle_integrate.comp; see upstream/LICENSE.
struct Particle {
  float pos[4];
  float vel[4];
};
static_assert(sizeof(Particle) == 32);
struct IntegrateParams {
  __storage Particle *particles;
  float delta_t;
  int particle_count;
};
static_assert(sizeof(IntegrateParams) == 16);

__compute
__workgroup_size(256, 1, 1)
void vulkan_particle_integrate(__push_constant const IntegrateParams *params) {
  unsigned index = global_invocation_id.x;
  for (unsigned component = 0; component < 4; ++component)
    params->particles[index].pos[component] +=
        params->delta_t * 0.05f * params->particles[index].vel[component];
}
