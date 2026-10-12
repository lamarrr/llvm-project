__compute__
__workgroup_size__(1, 1, 1)
void image_atomic(__binding__(0, 0) storage_texture_2d<unsigned, r32ui> image) {
  unsigned previous = image.atomic_add(0, 0, 1u);
}
