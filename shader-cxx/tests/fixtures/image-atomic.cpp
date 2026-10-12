__compute
__workgroup_size(1, 1, 1)
void image_atomic(__binding(0, 0) storage_texture_2d<unsigned, r32ui> image) {
  unsigned previous = image.atomic_add(0, 0, 1u);
}
