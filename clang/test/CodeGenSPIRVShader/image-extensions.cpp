// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -emit-llvm-bc -O0 -o %t.o0.bc %s
// RUN: clang-spirv-link --entry=sparse_image -o %t.sparse.o0.spv %t.o0.bc
// RUN: clang-spirv-link --entry=atomic_image -o %t.atomic.o0.spv %t.o0.bc
// RUN: clang-spirv-link --entry=wide_image -o %t.wide.o0.spv %t.o0.bc
// RUN: clang-spirv-link --entry=attachment_image -o %t.attach.o0.spv %t.o0.bc
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -emit-llvm-bc -O2 -o %t.o2.bc %s
// RUN: clang-spirv-link --entry=sparse_image -o %t.sparse.o2.spv %t.o2.bc
// RUN: clang-spirv-link --entry=atomic_image -o %t.atomic.o2.spv %t.o2.bc
// RUN: clang-spirv-link --entry=wide_image -o %t.wide.o2.spv %t.o2.bc
// RUN: clang-spirv-link --entry=attachment_image -o %t.attach.o2.spv %t.o2.bc

struct Output {
  __storage__ unsigned *data;
};

__compute__
__workgroup_size__(1, 1, 1)
void sparse_image(__binding__(0, 0) texture_2d<float> image,
                  __push_constant__ const Output *output) {
  float value;
  unsigned code = image.sparse_load(0, 0, 0u, &value);
  output->data[0] = sparse_resident(code) && value > 0.0f;
}

__compute__
__workgroup_size__(1, 1, 1)
void atomic_image(__binding__(0, 0) storage_texture_2d<unsigned, r32ui> image,
                  __push_constant__ const Output *output) {
  output->data[0] = image.atomic_add(0, 0, 1u);
}

__compute__
__workgroup_size__(1, 1, 1)
void wide_image(
    __binding__(0, 0) storage_texture_2d<unsigned long long, r64ui> image,
    __binding__(0, 1) storage_texture_2d<long long, r64i> signed_image) {
  image.store(1, 0, image.load(0, 0));
  signed_image.store(1, 0, signed_image.load(0, 0));
}

__fragment__
void attachment_image(__binding__(0, 0) input_attachment<float, 1> image,
                      __push_constant__ const Output *output) {
  output->data[0] = image.load() > 0.0f;
}
