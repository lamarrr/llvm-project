struct ImageAtomicOutput {
  __storage__ unsigned *integer;
  __storage__ float *floating;
  __storage__ unsigned long long *wide;
};

__compute__ __workgroup_size__(1, 1, 1)
void image_atomic_policy(
    __binding__(0, 0) storage_texture_2d<unsigned, r32ui> image,
    __binding__(0, 1) storage_texture_2d<int, r32i> signed_image,
    __binding__(0, 2) storage_texture_2d<float, r32f> float_image,
    __binding__(0, 3) storage_texture_2d<unsigned long long, r64ui> wide_image,
    __binding__(0, 4) storage_texture_2d<long long, r64i> signed_wide,
    __binding__(0, 5) storage_texture_3d<unsigned, r32ui> volume,
    __binding__(0, 6) storage_texture_buffer<unsigned, r32ui> buffer,
    __binding__(0, 7) storage_texture_2d_ms_layers<unsigned, r32ui> multisample,
    __push_constant__ const ImageAtomicOutput *output) {
  image.atomic_store(0, 0, 1u, memory_order::release, memory_scope::device);
  output->integer[0] = image.atomic_load(0, 0, memory_order::acquire, memory_scope::device);
  output->integer[1] = image.atomic_add(0, 0, 1u);
  output->integer[2] = image.atomic_sub(0, 0, 1u);
  output->integer[3] = image.atomic_exchange(0, 0, 2u);
  output->integer[4] = image.atomic_min(0, 0, 1u);
  output->integer[5] = image.atomic_max(0, 0, 3u);
  output->integer[6] = image.atomic_and(0, 0, 7u);
  output->integer[7] = image.atomic_or(0, 0, 8u);
  output->integer[8] = image.atomic_xor(0, 0, 2u);
  output->integer[9] = image.atomic_compare_exchange(
      0, 0, 3u, 4u, memory_order::acquire_release, memory_order::acquire,
      memory_scope::device);
  output->integer[10] = signed_image.atomic_min(0, 0, -1);
  output->integer[11] = signed_image.atomic_max(0, 0, 1);
  output->integer[12] = signed_image.atomic_add(0, 0, -2);
  output->integer[13] = signed_image.atomic_compare_exchange(0, 0, -2, -1);
  output->integer[14] = volume.atomic_add(0, 0, 0, 1u);
  output->integer[15] = buffer.atomic_add(0, 1u);
  output->integer[16] = multisample.atomic_add(0, 0, 0, 0u, 1u);
  float_image.atomic_store(0, 0, 1.0f);
  output->floating[0] = float_image.atomic_load(0, 0);
  output->floating[1] = float_image.atomic_exchange(0, 0, 2.0f);
  output->floating[2] = float_image.atomic_add(0, 0, 1.0f);
  output->floating[3] = float_image.atomic_sub(0, 0, 1.0f);
  output->floating[4] = float_image.atomic_min(0, 0, -1.0f);
  output->floating[5] = float_image.atomic_max(0, 0, 3.0f);
  output->wide[0] = wide_image.atomic_add(0, 0, 1ull);
  output->wide[1] = signed_wide.atomic_compare_exchange(0, 0, -1ll, 1ll);
}
