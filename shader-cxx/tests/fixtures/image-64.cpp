__compute__
__workgroup_size__(1, 1, 1)
void image_64(
    __binding__(0, 0) storage_texture_2d<unsigned long long, r64ui>
        unsigned_image,
    __binding__(0, 1) storage_texture_2d<long long, r64i> signed_image) {
  unsigned long long unsigned_value = unsigned_image.load(0, 0);
  long long signed_value = signed_image.load(0, 0);
  unsigned_image.store(1, 0, unsigned_value + 1);
  signed_image.store(1, 0, signed_value - 1);
}
