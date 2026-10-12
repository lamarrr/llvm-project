// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s

void take_storage(__storage__ int *);
void take_local(int *); // expected-note {{candidate function not viable}}

void deduce_local(__storage__ int *storage,
                  __uniform__ const int *uniform,
                  __push_constant__ const int *push) {
  int *from_storage = storage;
  static_assert(__is_same(decltype(from_storage), __storage__ int *));
  take_storage(from_storage);

  const int *read_only_storage = storage;
  static_assert(
      __is_same(decltype(read_only_storage), __storage__ const int *));

  const int *from_uniform(uniform);
  static_assert(
      __is_same(decltype(from_uniform), __uniform__ const int *));

  const int *braced_uniform{uniform};
  static_assert(__is_same(decltype(braced_uniform), __uniform__ const int *));

  const int *const from_push = push;
  static_assert(__is_same(decltype(from_push),
                          __push_constant__ const int *const));

  static __shared__ int scratch;
  int *from_shared = &scratch;
  static_assert(__is_same(decltype(from_shared), __shared__ int *));

  int local = 0;
  int *local_pointer = &local;
  static_assert(__is_same(decltype(local_pointer), int *));
  local_pointer = storage; // expected-error {{changes address space of pointer}}
  take_local(storage);     // expected-error {{no matching function for call to 'take_local'}}

  int *mutable_uniform = uniform; // expected-error {{cannot initialize a variable of type}}
  float *different_pointee = storage; // expected-error {{cannot initialize a variable of type}}
}
