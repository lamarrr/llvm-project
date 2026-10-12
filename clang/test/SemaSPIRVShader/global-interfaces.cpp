// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s

__input(0) extern const float
    input;                       // expected-error@-1 {{entry-point parameters}}
__output(0) extern float output; // expected-error {{entry-point parameters}}
__binding(0, 1) int resource;    // expected-error {{entry-point parameters}}
extern __uniform float *uniform; // expected-error {{entry-point parameters}}
extern __push_constant float *push; // expected-error {{entry-point parameters}}
extern __storage float *storage;    // expected-error {{entry-point parameters}}
extern __shared float scratch[4];   // expected-error {{entry-point parameters}}

namespace nested {
__output(2) extern float value; // expected-error {{entry-point parameters}}
__storage float *resource;      // expected-error {{entry-point parameters}}
} // namespace nested
using DevicePointer = __storage float *;
DevicePointer alias;    // expected-error {{entry-point parameters}}
DevicePointer array[2]; // expected-error {{entry-point parameters}}
struct Record {
  DevicePointer data;
};
Record record; // expected-error {{entry-point parameters}}
struct Container {
  Record records[2];
};
Container container; // expected-error {{entry-point parameters}}
struct StaticMember {
  static DevicePointer data; // expected-error {{entry-point parameters}}
};
auto deduced = static_cast<DevicePointer>(
    nullptr); // expected-error@-1 {{entry-point parameters}}

void local_resources() {
  extern DevicePointer external;   // expected-error {{entry-point parameters}}
  static DevicePointer persistent; // expected-error {{entry-point parameters}}
  __input(0) float input;          // expected-error {{entry-point parameters}}
  __output(0) float output;        // expected-error {{entry-point parameters}}
  __binding(0, 1) int bound;       // expected-error {{entry-point parameters}}
}

// Ordinary global state, recursive C++ records, local pointer borrowing and
// lexically scoped workgroup scratch are still valid.
float state = 1.0f;
struct Node {
  Node *next;
};
Node head;
void borrow(DevicePointer parameter) {
  auto pointer = parameter;
  static __shared float scratch[4];
  scratch[0] = *pointer;
}
__compute
__workgroup_size(1, 1, 1)
void entry(__binding(0, 1) __uniform const Record *uniform,
           __push_constant const Record *push) {
  auto pointer = push->data;
  *pointer = uniform->data[0] + state;
}

texture_2d<float>
    globalTexture; // expected-error {{global shader interfaces and resources are not supported}}
sampler
    globalSampler; // expected-error {{global shader interfaces and resources are not supported}}
texture_2d<float> globalDescriptors
    [4]; // expected-error@-1 {{global shader interfaces and resources are not supported}}
ray_query
    globalQuery; // expected-error {{global shader interfaces and resources are not supported}}
