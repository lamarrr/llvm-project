// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s

struct Payload {
  float value;
  float weights[2];
};
struct Params {
  Payload records[4];
  __storage Payload *device;
};

__fragment
void copy_records(__push_constant const Params *push,
                  __binding(0, 0) __uniform const Payload *uniform,
                  __input(0) __flat unsigned index, __output(0) float *output) {
  Payload from_push = push->records[index & 3];
  Payload from_uniform = *uniform;
  Payload from_storage = *push->device;
  Payload direct(*uniform);
  *output =
      from_push.value + from_uniform.value + from_storage.value + direct.value;
}

struct Defaulted {
  int value;
  Defaulted(const Defaulted &) = default;
};

void copy_defaulted(__push_constant const Defaulted *source) {
  Defaulted value(*source);
}

struct Nontrivial {
  int value;
  Nontrivial(const Nontrivial &other)
      : value(other.value) {
  } // expected-note@-2 {{candidate constructor not viable}}
};

void reject_nontrivial(__push_constant const Nontrivial *source) {
  Nontrivial copy = *source; // expected-error {{no matching constructor}}
}

struct Deleted {
  int value;
  Deleted(const Deleted &) =
      delete; // expected-note@-1 {{explicitly marked deleted}}
};

void reject_deleted(__uniform const Deleted *source) {
  Deleted copy = *source; // expected-error {{call to deleted constructor}}
}

void borrow(const Payload &); // expected-note {{candidate function not viable}}
void reject_reference(__push_constant const Payload *source) {
  borrow(*source); // expected-error {{no matching function}}
}
