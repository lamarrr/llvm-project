//===-- vec.hpp - C-array-backed vector utilities for Shader C++ ---------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#pragma once

namespace spirv {

namespace intr {
using ::bit_count;
using ::clamp;
using ::cross;
using ::dot;
using ::faceforward;
using ::find_lsb;
using ::find_msb;
using ::inverse_sqrt;
using ::is_inf;
using ::is_nan;
using ::lerp;
using ::max;
using ::min;
using ::pack_double2x32;
using ::pack_half2x16;
using ::pack_snorm2x16;
using ::pack_snorm4x8;
using ::pack_unorm2x16;
using ::pack_unorm4x8;
using ::reflect;
using ::round_even;
using ::smoothstep;
using ::step;
using ::unpack_double2x32;
using ::unpack_half2x16;
using ::unpack_snorm2x16;
using ::unpack_snorm4x8;
using ::unpack_unorm2x16;
using ::unpack_unorm4x8;
} // namespace intr

template <class T, unsigned N> struct vec {
  T data[N];
  constexpr vec() = default;
  template <class S> constexpr vec(S s) { fill<0>(s); }
  template <class A, class B, class... C> constexpr vec(A a, B b, C... c) {
    append_components<0>(a, b, c...);
  }
  template <unsigned I, class S> constexpr void fill(S s) {
    if constexpr (I < N) {
      if constexpr (requires { s.data[0]; })
        data[I] = static_cast<T>(s.data[I]);
      else
        data[I] = static_cast<T>(s);
      fill<I + 1>(s);
    }
  }
  template <unsigned Offset, unsigned I, class S>
  constexpr void copy_components(S s) {
    if constexpr (I < sizeof(s.data) / sizeof(s.data[0])) {
      data[Offset + I] = static_cast<T>(s.data[I]);
      copy_components<Offset, I + 1>(s);
    }
  }
  template <unsigned Offset, class A, class... Rest>
  constexpr void append_components(A a, Rest... rest) {
    if constexpr (requires { a.data[0]; }) {
      constexpr unsigned Count = sizeof(a.data) / sizeof(a.data[0]);
      static_assert(Offset + Count <= N);
      copy_components<Offset, 0>(a);
      if constexpr (sizeof...(Rest))
        append_components<Offset + Count>(rest...);
    } else {
      static_assert(Offset < N);
      data[Offset] = static_cast<T>(a);
      if constexpr (sizeof...(Rest))
        append_components<Offset + 1>(rest...);
    }
  }
  template <class S> constexpr void append(unsigned &offset, S s) {
    if constexpr (requires { s.data[0]; }) {
      for (unsigned i = 0; i < sizeof(s.data) / sizeof(s.data[0]); ++i)
        data[offset++] = static_cast<T>(s.data[i]);
    } else
      data[offset++] = static_cast<T>(s);
  }
  constexpr T &operator[](unsigned i) { return data[i]; }
  constexpr const T &operator[](unsigned i) const { return data[i]; }
  constexpr unsigned length() const { return N; }
#ifdef __SPIRV__
  template <class Self>
  static constexpr decltype(auto) component(Self &self, unsigned i) {
    if constexpr (__is_same(decltype(&self.data[0]), T __storage__ *) ||
                  __is_same(decltype(&self.data[0]), const T __storage__ *))
      return *(&self.data[0] + i);
    else
      return (self.data[i]);
  }
  constexpr decltype(auto) operator[](this auto &self, unsigned i) {
    return component(self, i);
  }
  constexpr unsigned length(this auto const &) { return N; }
#endif
  constexpr T &x()
    requires(N > 0)
  {
    return data[0];
  }
  constexpr const T &x() const
    requires(N > 0)
  {
    return data[0];
  }
#ifdef __SPIRV__
  constexpr decltype(auto) x(this auto &self)
    requires(N > 0)
  {
    return component(self, 0);
  }
#endif
  constexpr T &y()
    requires(N > 1)
  {
    return data[1];
  }
  constexpr const T &y() const
    requires(N > 1)
  {
    return data[1];
  }
#ifdef __SPIRV__
  constexpr decltype(auto) y(this auto &self)
    requires(N > 1)
  {
    return component(self, 1);
  }
#endif
  constexpr T &z()
    requires(N > 2)
  {
    return data[2];
  }
  constexpr const T &z() const
    requires(N > 2)
  {
    return data[2];
  }
#ifdef __SPIRV__
  constexpr decltype(auto) z(this auto &self)
    requires(N > 2)
  {
    return component(self, 2);
  }
#endif
  constexpr T &w()
    requires(N > 3)
  {
    return data[3];
  }
  constexpr const T &w() const
    requires(N > 3)
  {
    return data[3];
  }
#ifdef __SPIRV__
  constexpr decltype(auto) w(this auto &self)
    requires(N > 3)
  {
    return component(self, 3);
  }
#endif
  template <unsigned... I> constexpr vec<T, sizeof...(I)> swizzle() const {
    static_assert(((I < N) && ...));
    return vec<T, sizeof...(I)>(data[I]...);
  }
#ifdef __SPIRV__
  template <unsigned... I>
  constexpr vec<T, sizeof...(I)> swizzle(this auto const &self) {
    static_assert(((I < N) && ...));
    return vec<T, sizeof...(I)>(component(self, I)...);
  }
#define GLSL_SWIZZLE_SPACE_2(A, AI, B, BI)                                     \
  constexpr vec<T, 2> A##B(this auto const &self)                              \
    requires(N > AI && N > BI)                                                 \
  {                                                                            \
    return self.template swizzle<AI, BI>();                                    \
  }
#define GLSL_SWIZZLE_SPACE_3(A, AI, B, BI, C, CI)                              \
  constexpr vec<T, 3> A##B##C(this auto const &self)                           \
    requires(N > AI && N > BI && N > CI)                                       \
  {                                                                            \
    return self.template swizzle<AI, BI, CI>();                                \
  }
#define GLSL_SWIZZLE_SPACE_4(A, AI, B, BI, C, CI, D, DI)                       \
  constexpr vec<T, 4> A##B##C##D(this auto const &self)                        \
    requires(N > AI && N > BI && N > CI && N > DI)                             \
  {                                                                            \
    return self.template swizzle<AI, BI, CI, DI>();                            \
  }
#else
#define GLSL_SWIZZLE_SPACE_2(A, AI, B, BI)
#define GLSL_SWIZZLE_SPACE_3(A, AI, B, BI, C, CI)
#define GLSL_SWIZZLE_SPACE_4(A, AI, B, BI, C, CI, D, DI)
#endif
#define GLSL_SWIZZLE_2(A, AI, B, BI)                                           \
  constexpr vec<T, 2> A##B() const                                             \
    requires(N > AI && N > BI)                                                 \
  {                                                                            \
    return swizzle<AI, BI>();                                                  \
  }                                                                            \
  GLSL_SWIZZLE_SPACE_2(A, AI, B, BI)
#define GLSL_SWIZZLE_3(A, AI, B, BI, C, CI)                                    \
  constexpr vec<T, 3> A##B##C() const                                          \
    requires(N > AI && N > BI && N > CI)                                       \
  {                                                                            \
    return swizzle<AI, BI, CI>();                                              \
  }                                                                            \
  GLSL_SWIZZLE_SPACE_3(A, AI, B, BI, C, CI)
#define GLSL_SWIZZLE_4(A, AI, B, BI, C, CI, D, DI)                             \
  constexpr vec<T, 4> A##B##C##D() const                                       \
    requires(N > AI && N > BI && N > CI && N > DI)                             \
  {                                                                            \
    return swizzle<AI, BI, CI, DI>();                                          \
  }                                                                            \
  GLSL_SWIZZLE_SPACE_4(A, AI, B, BI, C, CI, D, DI)
#define GLSL_SWIZZLE_THIRD(A, AI, B, BI, C, CI)                                \
  GLSL_SWIZZLE_3(A, AI, B, BI, C, CI)                                          \
  GLSL_SWIZZLE_4(A, AI, B, BI, C, CI, x, 0)                                    \
  GLSL_SWIZZLE_4(A, AI, B, BI, C, CI, y, 1)                                    \
  GLSL_SWIZZLE_4(A, AI, B, BI, C, CI, z, 2)                                    \
  GLSL_SWIZZLE_4(A, AI, B, BI, C, CI, w, 3)
#define GLSL_SWIZZLE_SECOND(A, AI, B, BI)                                      \
  GLSL_SWIZZLE_2(A, AI, B, BI)                                                 \
  GLSL_SWIZZLE_THIRD(A, AI, B, BI, x, 0)                                       \
  GLSL_SWIZZLE_THIRD(A, AI, B, BI, y, 1)                                       \
  GLSL_SWIZZLE_THIRD(A, AI, B, BI, z, 2)                                       \
  GLSL_SWIZZLE_THIRD(A, AI, B, BI, w, 3)
#define GLSL_SWIZZLE_FIRST(A, AI)                                              \
  GLSL_SWIZZLE_SECOND(A, AI, x, 0)                                             \
  GLSL_SWIZZLE_SECOND(A, AI, y, 1)                                             \
  GLSL_SWIZZLE_SECOND(A, AI, z, 2)                                             \
  GLSL_SWIZZLE_SECOND(A, AI, w, 3)
  GLSL_SWIZZLE_FIRST(x, 0)
  GLSL_SWIZZLE_FIRST(y, 1)
  GLSL_SWIZZLE_FIRST(z, 2)
  GLSL_SWIZZLE_FIRST(w, 3)
#undef GLSL_SWIZZLE_FIRST
#undef GLSL_SWIZZLE_SECOND
#undef GLSL_SWIZZLE_THIRD
#undef GLSL_SWIZZLE_4
#undef GLSL_SWIZZLE_3
#undef GLSL_SWIZZLE_2
#undef GLSL_SWIZZLE_SPACE_4
#undef GLSL_SWIZZLE_SPACE_3
#undef GLSL_SWIZZLE_SPACE_2
  template <unsigned... I, class S> constexpr void set_swizzle(S value) {
    vec<T, sizeof...(I)> v(value);
    unsigned k = 0;
    ((data[I] = v.data[k++]), ...);
  }
#ifdef __SPIRV__
  template <unsigned... I, class S>
  constexpr void set_swizzle(this auto &self, S value) {
    vec<T, sizeof...(I)> v(value);
    unsigned index = 0;
    ((component(self, I) = v.data[index++]), ...);
  }
#endif
};
template <class T, unsigned N>
constexpr vec<T, N> operator+(vec<T, N> a, vec<T, N> b) {
  if (__builtin_is_constant_evaluated()) {
    vec<T, N> result;
    for (unsigned index = 0; index < N; ++index)
      result[index] = a[index] + b[index];
    return result;
  }
  return ::add(a, b);
}
template <class T, unsigned N, class S>
  requires(!requires(S s) { s.data[0]; })
constexpr vec<T, N> operator+(vec<T, N> a, S b) {
  return a + vec<T, N>(b);
}
template <class S, class T, unsigned N>
  requires(!requires(S s) { s.data[0]; })
constexpr vec<T, N> operator+(S a, vec<T, N> b) {
  return vec<T, N>(a) + b;
}
template <class T, unsigned N, class S>
constexpr vec<T, N> &operator+=(vec<T, N> &a, S b) {
  a = a + b;
  return a;
}
template <class T, unsigned N>
constexpr vec<T, N> operator-(vec<T, N> a, vec<T, N> b) {
  if (__builtin_is_constant_evaluated()) {
    vec<T, N> result;
    for (unsigned index = 0; index < N; ++index)
      result[index] = a[index] - b[index];
    return result;
  }
  return ::sub(a, b);
}
template <class T, unsigned N, class S>
  requires(!requires(S s) { s.data[0]; })
constexpr vec<T, N> operator-(vec<T, N> a, S b) {
  return a - vec<T, N>(b);
}
template <class S, class T, unsigned N>
  requires(!requires(S s) { s.data[0]; })
constexpr vec<T, N> operator-(S a, vec<T, N> b) {
  return vec<T, N>(a) - b;
}
template <class T, unsigned N, class S>
constexpr vec<T, N> &operator-=(vec<T, N> &a, S b) {
  a = a - b;
  return a;
}
template <class T, unsigned N>
constexpr vec<T, N> operator*(vec<T, N> a, vec<T, N> b) {
  if (__builtin_is_constant_evaluated()) {
    vec<T, N> result;
    for (unsigned index = 0; index < N; ++index)
      result[index] = a[index] * b[index];
    return result;
  }
  return ::mul(a, b);
}
template <class T, unsigned N, class S>
  requires(!requires(S s) { s.data[0]; })
constexpr vec<T, N> operator*(vec<T, N> a, S b) {
  return a * vec<T, N>(b);
}
template <class S, class T, unsigned N>
  requires(!requires(S s) { s.data[0]; })
constexpr vec<T, N> operator*(S a, vec<T, N> b) {
  return vec<T, N>(a) * b;
}
template <class T, unsigned N, class S>
constexpr vec<T, N> &operator*=(vec<T, N> &a, S b) {
  a = a * b;
  return a;
}
template <class T, unsigned N>
constexpr vec<T, N> operator/(vec<T, N> a, vec<T, N> b) {
  if (__builtin_is_constant_evaluated()) {
    vec<T, N> result;
    for (unsigned index = 0; index < N; ++index)
      result[index] = a[index] / b[index];
    return result;
  }
  return ::div(a, b);
}
template <class T, unsigned N, class S>
  requires(!requires(S s) { s.data[0]; })
constexpr vec<T, N> operator/(vec<T, N> a, S b) {
  return a / vec<T, N>(b);
}
template <class S, class T, unsigned N>
  requires(!requires(S s) { s.data[0]; })
constexpr vec<T, N> operator/(S a, vec<T, N> b) {
  return vec<T, N>(a) / b;
}
template <class T, unsigned N, class S>
constexpr vec<T, N> &operator/=(vec<T, N> &a, S b) {
  a = a / b;
  return a;
}
template <class T, unsigned N> constexpr vec<T, N> operator-(vec<T, N> a) {
  return vec<T, N>(0) - a;
}
template <class T, unsigned N> constexpr vec<T, N> operator+(vec<T, N> a) {
  return a;
}

#define SHADER_VEC_INTEGER_BINARY_OP(OP, ASSIGN)                               \
  template <class T, unsigned N>                                               \
    requires(__is_integral(T) && !__is_same(T, bool))                          \
  constexpr vec<T, N> operator OP(vec<T, N> first, vec<T, N> second) {         \
    if constexpr ((N == 2 || N == 3 || N == 4) &&                              \
                  (sizeof(T) == 4 || sizeof(T) == 8)) {                        \
      if (!__builtin_is_constant_evaluated()) {                                \
        using native_vec = T __attribute__((ext_vector_type(N)));              \
        if constexpr (N == 3) {                                                \
          native_vec native_first{first[0], first[1], first[2]};               \
          native_vec native_second{second[0], second[1], second[2]};           \
          native_vec native_result = native_first OP native_second;            \
          return vec<T, N>(native_result[0], native_result[1],                 \
                           native_result[2]);                                  \
        } else {                                                               \
          native_vec native_first = __builtin_bit_cast(native_vec, first);     \
          native_vec native_second = __builtin_bit_cast(native_vec, second);   \
          return __builtin_bit_cast(vec<T, N>, native_first OP native_second); \
        }                                                                      \
      }                                                                        \
    }                                                                          \
    vec<T, N> result;                                                          \
    for (unsigned index = 0; index < N; ++index)                               \
      result[index] = static_cast<T>(first[index] OP second[index]);           \
    return result;                                                             \
  }                                                                            \
  template <class T, unsigned N, class S>                                      \
    requires(__is_integral(T) && !__is_same(T, bool) && __is_integral(S) &&    \
             !__is_same(S, bool))                                              \
  constexpr vec<T, N> operator OP(vec<T, N> first, S second) {                 \
    return first OP vec<T, N>(second);                                         \
  }                                                                            \
  template <class S, class T, unsigned N>                                      \
    requires(__is_integral(S) && !__is_same(S, bool) && __is_integral(T) &&    \
             !__is_same(T, bool))                                              \
  constexpr vec<T, N> operator OP(S first, vec<T, N> second) {                 \
    return vec<T, N>(first) OP second;                                         \
  }                                                                            \
  template <class T, unsigned N, class S>                                      \
    requires(__is_integral(T) && !__is_same(T, bool))                          \
  constexpr vec<T, N> &operator ASSIGN(vec<T, N> &first, S second) {           \
    first = first OP second;                                                   \
    return first;                                                              \
  }

SHADER_VEC_INTEGER_BINARY_OP(%, %=)
SHADER_VEC_INTEGER_BINARY_OP(&, &=)
SHADER_VEC_INTEGER_BINARY_OP(|, |=)
SHADER_VEC_INTEGER_BINARY_OP(^, ^=)
SHADER_VEC_INTEGER_BINARY_OP(<<, <<=)
SHADER_VEC_INTEGER_BINARY_OP(>>, >>=)
#undef SHADER_VEC_INTEGER_BINARY_OP

template <class T, unsigned N>
  requires(__is_integral(T) && !__is_same(T, bool))
constexpr vec<T, N> operator~(vec<T, N> value) {
  return value ^ vec<T, N>(static_cast<T>(~T(0)));
}

#define SHADER_VEC_BOOL_BINARY_OP(OP, ASSIGN)                                  \
  template <unsigned N>                                                        \
  constexpr vec<bool, N> operator OP(vec<bool, N> first,                       \
                                     vec<bool, N> second) {                    \
    if constexpr (N == 2 || N == 3 || N == 4) {                                \
      if (!__builtin_is_constant_evaluated()) {                                \
        using native_vec = bool __attribute__((ext_vector_type(N)));           \
        native_vec native_first{};                                             \
        native_vec native_second{};                                            \
        for (unsigned index = 0; index < N; ++index) {                         \
          native_first[index] = first[index];                                  \
          native_second[index] = second[index];                                \
        }                                                                      \
        native_vec native_result = native_first OP native_second;              \
        vec<bool, N> result;                                                   \
        for (unsigned index = 0; index < N; ++index)                           \
          result[index] = native_result[index];                                \
        return result;                                                         \
      }                                                                        \
    }                                                                          \
    vec<bool, N> result;                                                       \
    for (unsigned index = 0; index < N; ++index)                               \
      result[index] = first[index] OP second[index];                           \
    return result;                                                             \
  }                                                                            \
  template <unsigned N>                                                        \
  constexpr vec<bool, N> operator OP(vec<bool, N> first, bool second) {        \
    return first OP vec<bool, N>(second);                                      \
  }                                                                            \
  template <unsigned N>                                                        \
  constexpr vec<bool, N> operator OP(bool first, vec<bool, N> second) {        \
    return vec<bool, N>(first) OP second;                                      \
  }                                                                            \
  template <unsigned N, class S>                                               \
  constexpr vec<bool, N> &operator ASSIGN(vec<bool, N> &first, S second) {     \
    first = first OP second;                                                   \
    return first;                                                              \
  }

SHADER_VEC_BOOL_BINARY_OP(&, &=)
SHADER_VEC_BOOL_BINARY_OP(|, |=)
SHADER_VEC_BOOL_BINARY_OP(^, ^=)
#undef SHADER_VEC_BOOL_BINARY_OP

template <unsigned N> constexpr vec<bool, N> operator!(vec<bool, N> value) {
  return value ^ vec<bool, N>(true);
}
template <unsigned N>
constexpr vec<bool, N> operator||(vec<bool, N> first, vec<bool, N> second) {
  return first | second;
}
template <unsigned N>
constexpr vec<bool, N> operator||(vec<bool, N> first, bool second) {
  return first || vec<bool, N>(second);
}
template <unsigned N>
constexpr vec<bool, N> operator||(bool first, vec<bool, N> second) {
  return vec<bool, N>(first) || second;
}
template <unsigned N>
constexpr vec<bool, N> operator&&(vec<bool, N> first, vec<bool, N> second) {
  return first & second;
}
template <unsigned N>
constexpr vec<bool, N> operator&&(vec<bool, N> first, bool second) {
  return first && vec<bool, N>(second);
}
template <unsigned N>
constexpr vec<bool, N> operator&&(bool first, vec<bool, N> second) {
  return vec<bool, N>(first) && second;
}

template <class T, unsigned N>
  requires(__is_floating_point(T))
constexpr T dot(vec<T, N> first, vec<T, N> second) {
  if (__builtin_is_constant_evaluated()) {
    T result = T(0);
    for (unsigned index = 0; index < N; ++index)
      result += first[index] * second[index];
    return result;
  }
  return intr::dot(first, second);
}
template <class T>
  requires(__is_floating_point(T))
constexpr vec<T, 3> cross(vec<T, 3> first, vec<T, 3> second) {
  if (__builtin_is_constant_evaluated())
    return vec<T, 3>(first[1] * second[2] - first[2] * second[1],
                     first[2] * second[0] - first[0] * second[2],
                     first[0] * second[1] - first[1] * second[0]);
  return intr::cross(first, second);
}
template <class T, unsigned N>
  requires(__is_floating_point(T))
constexpr vec<T, N> reflect(vec<T, N> incident, vec<T, N> normal) {
  if (__builtin_is_constant_evaluated())
    return incident - T(2) * dot(normal, incident) * normal;
  return intr::reflect(incident, normal);
}
template <class T, unsigned N>
  requires(__is_floating_point(T))
constexpr vec<T, N> faceforward(vec<T, N> normal, vec<T, N> incident,
                                vec<T, N> reference) {
  if (__builtin_is_constant_evaluated())
    return dot(reference, incident) < T(0) ? normal : -normal;
  return intr::faceforward(normal, incident, reference);
}

template <class T>
  requires(__is_floating_point(T))
constexpr T mix(T first, T second, T weight) {
  if (__builtin_is_constant_evaluated())
    return first * (T(1) - weight) + second * weight;
  return ::lerp(first, second, weight);
}
template <class T, unsigned N>
  requires(__is_floating_point(T))
constexpr vec<T, N> lerp(vec<T, N> first, vec<T, N> second, vec<T, N> weight) {
  if (__builtin_is_constant_evaluated()) {
    vec<T, N> result;
    for (unsigned index = 0; index < N; ++index)
      result[index] =
          first[index] * (T(1) - weight[index]) + second[index] * weight[index];
    return result;
  }
  return intr::lerp(first, second, weight);
}
template <class T, unsigned N>
  requires(__is_floating_point(T))
constexpr vec<T, N> mix(vec<T, N> first, vec<T, N> second, vec<T, N> weight) {
  return spirv::lerp(first, second, weight);
}
template <class T, unsigned N, class S>
  requires(__is_floating_point(T) && !requires(S value) { value.data[0]; })
constexpr vec<T, N> lerp(vec<T, N> first, vec<T, N> second, S weight) {
  return spirv::lerp(first, second, vec<T, N>(weight));
}
template <class T, unsigned N, class S>
  requires(__is_floating_point(T) && !requires(S value) { value.data[0]; })
constexpr vec<T, N> mix(vec<T, N> first, vec<T, N> second, S weight) {
  return spirv::lerp(first, second, vec<T, N>(weight));
}
template <class T> constexpr T mix(T first, T second, bool select) {
  return select ? second : first;
}
template <class T, unsigned N>
constexpr vec<T, N> mix(vec<T, N> first, vec<T, N> second,
                        vec<bool, N> select) {
  vec<T, N> result;
  for (unsigned index = 0; index < N; ++index)
    result[index] = select[index] ? second[index] : first[index];
  return result;
}
template <class T>
  requires(__is_floating_point(T))
constexpr T inverse_sqrt(T value) {
  return intr::inverse_sqrt(value);
}
template <class T, unsigned N>
  requires(__is_floating_point(T))
constexpr vec<T, N> inverse_sqrt(vec<T, N> value) {
  return intr::inverse_sqrt(value);
}
template <class T, unsigned N, class S>
  requires(__is_floating_point(T) && !requires(S value) { value.data[0]; })
constexpr vec<T, N> mod(vec<T, N> first, S second) {
  return ::mod(first, vec<T, N>(second));
}
template <class S, class T, unsigned N>
  requires(__is_floating_point(T) && !requires(S value) { value.data[0]; })
constexpr vec<T, N> mod(S first, vec<T, N> second) {
  return ::mod(vec<T, N>(first), second);
}
template <class T, unsigned N>
  requires(__is_floating_point(T))
constexpr vec<T, N> step(vec<T, N> edge, vec<T, N> value) {
  if (__builtin_is_constant_evaluated()) {
    vec<T, N> result;
    for (unsigned index = 0; index < N; ++index)
      result[index] = value[index] < edge[index] ? T(0) : T(1);
    return result;
  }
  return intr::step(edge, value);
}
template <class T, unsigned N, class S>
  requires(__is_floating_point(T) && !requires(S value) { value.data[0]; })
constexpr vec<T, N> step(vec<T, N> edge, S value) {
  return spirv::step(edge, vec<T, N>(value));
}
template <class S, class T, unsigned N>
  requires(__is_floating_point(T) && !requires(S value) { value.data[0]; })
constexpr vec<T, N> step(S edge, vec<T, N> value) {
  return spirv::step(vec<T, N>(edge), value);
}
template <class T, unsigned N>
  requires(__is_floating_point(T))
constexpr vec<T, N> smoothstep(vec<T, N> first, vec<T, N> second,
                               vec<T, N> value) {
  if (__builtin_is_constant_evaluated()) {
    vec<T, N> result;
    for (unsigned index = 0; index < N; ++index) {
      T weight = (value[index] - first[index]) / (second[index] - first[index]);
      weight = weight < T(0) ? T(0) : weight > T(1) ? T(1) : weight;
      result[index] = weight * weight * (T(3) - T(2) * weight);
    }
    return result;
  }
  return intr::smoothstep(first, second, value);
}
template <class T, unsigned N>
  requires(__is_floating_point(T))
constexpr vec<T, N> smoothstep(T first, T second, vec<T, N> value) {
  return spirv::smoothstep(vec<T, N>(first), vec<T, N>(second), value);
}
template <class T, unsigned N>
constexpr vec<T, N> min(vec<T, N> first, vec<T, N> second) {
  if (__builtin_is_constant_evaluated()) {
    vec<T, N> result;
    for (unsigned index = 0; index < N; ++index)
      result[index] =
          first[index] < second[index] ? first[index] : second[index];
    return result;
  }
  return intr::min(first, second);
}
template <class T, unsigned N, class S>
  requires(!requires(S value) { value.data[0]; })
constexpr vec<T, N> min(vec<T, N> first, S second) {
  return spirv::min(first, vec<T, N>(second));
}
template <class S, class T, unsigned N>
  requires(!requires(S value) { value.data[0]; })
constexpr vec<T, N> min(S first, vec<T, N> second) {
  return spirv::min(vec<T, N>(first), second);
}
template <class T, unsigned N>
constexpr vec<T, N> max(vec<T, N> first, vec<T, N> second) {
  if (__builtin_is_constant_evaluated()) {
    vec<T, N> result;
    for (unsigned index = 0; index < N; ++index)
      result[index] =
          first[index] > second[index] ? first[index] : second[index];
    return result;
  }
  return intr::max(first, second);
}
template <class T, unsigned N, class S>
  requires(!requires(S value) { value.data[0]; })
constexpr vec<T, N> max(vec<T, N> first, S second) {
  return spirv::max(first, vec<T, N>(second));
}
template <class S, class T, unsigned N>
  requires(!requires(S value) { value.data[0]; })
constexpr vec<T, N> max(S first, vec<T, N> second) {
  return spirv::max(vec<T, N>(first), second);
}
template <class T, unsigned N, class S>
  requires(__is_floating_point(T) && !requires(S value) { value.data[0]; })
constexpr vec<T, N> pow(vec<T, N> first, S second) {
  return ::pow(first, vec<T, N>(second));
}
template <class S, class T, unsigned N>
  requires(__is_floating_point(T) && !requires(S value) { value.data[0]; })
constexpr vec<T, N> pow(S first, vec<T, N> second) {
  return ::pow(vec<T, N>(first), second);
}
template <class T, unsigned N>
constexpr vec<T, N> clamp(vec<T, N> value, vec<T, N> lower, vec<T, N> upper) {
  if (__builtin_is_constant_evaluated())
    return min(max(value, lower), upper);
  return intr::clamp(value, lower, upper);
}
template <class T, unsigned N, class A, class B>
  requires(
      !requires(A value) { value.data[0]; } ||
      !requires(B value) { value.data[0]; })
constexpr vec<T, N> clamp(vec<T, N> value, A lower, B upper) {
  return spirv::clamp(value, vec<T, N>(lower), vec<T, N>(upper));
}

template <class T>
  requires(__is_floating_point(T))
constexpr T atan(T vertical, T horizontal) {
  return ::atan2(vertical, horizontal);
}
template <class T, unsigned N>
  requires(__is_floating_point(T))
constexpr vec<T, N> atan(vec<T, N> vertical, vec<T, N> horizontal) {
  return ::atan2(vertical, horizontal);
}
template <class T>
  requires(__is_floating_point(T))
constexpr T length(T value) {
  return value < T(0) ? -value : value;
}
template <class T>
  requires(__is_floating_point(T))
constexpr T distance(T first, T second) {
  return length(first - second);
}
template <class T, unsigned N> constexpr bool any(vec<T, N> value) {
  for (unsigned index = 0; index < N; ++index)
    if (value[index])
      return true;
  return false;
}
template <class T, unsigned N> constexpr bool all(vec<T, N> value) {
  for (unsigned index = 0; index < N; ++index)
    if (!value[index])
      return false;
  return true;
}
template <unsigned N> constexpr vec<bool, N> logical_not(vec<bool, N> value) {
  return !value;
}
template <class T, unsigned N>
constexpr vec<bool, N> less_than(vec<T, N> first, vec<T, N> second) {
  vec<bool, N> result;
  for (unsigned index = 0; index < N; ++index)
    result[index] = first[index] < second[index];
  return result;
}
template <class T, unsigned N>
constexpr vec<bool, N> less_than_equal(vec<T, N> first, vec<T, N> second) {
  vec<bool, N> result;
  for (unsigned index = 0; index < N; ++index)
    result[index] = first[index] <= second[index];
  return result;
}
template <class T, unsigned N>
constexpr vec<bool, N> greater_than(vec<T, N> first, vec<T, N> second) {
  vec<bool, N> result;
  for (unsigned index = 0; index < N; ++index)
    result[index] = first[index] > second[index];
  return result;
}
template <class T, unsigned N>
constexpr vec<bool, N> greater_than_equal(vec<T, N> first, vec<T, N> second) {
  vec<bool, N> result;
  for (unsigned index = 0; index < N; ++index)
    result[index] = first[index] >= second[index];
  return result;
}
template <class T, unsigned N>
constexpr vec<bool, N> equal(vec<T, N> first, vec<T, N> second) {
  vec<bool, N> result;
  for (unsigned index = 0; index < N; ++index)
    result[index] = first[index] == second[index];
  return result;
}
template <class T, unsigned N>
constexpr vec<bool, N> not_equal(vec<T, N> first, vec<T, N> second) {
  vec<bool, N> result;
  for (unsigned index = 0; index < N; ++index)
    result[index] = first[index] != second[index];
  return result;
}
constexpr unsigned float_bits_to_uint(float value) {
  return __builtin_bit_cast(unsigned, value);
}
constexpr int float_bits_to_int(float value) {
  return __builtin_bit_cast(int, value);
}
constexpr float uint_bits_to_float(unsigned value) {
  return __builtin_bit_cast(float, value);
}
constexpr float int_bits_to_float(int value) {
  return __builtin_bit_cast(float, value);
}
template <unsigned N>
constexpr vec<unsigned, N> float_bits_to_uint(vec<float, N> value) {
  vec<unsigned, N> result;
  for (unsigned index = 0; index < N; ++index)
    result[index] = float_bits_to_uint(value[index]);
  return result;
}
template <unsigned N>
constexpr vec<int, N> float_bits_to_int(vec<float, N> value) {
  vec<int, N> result;
  for (unsigned index = 0; index < N; ++index)
    result[index] = float_bits_to_int(value[index]);
  return result;
}
template <unsigned N>
constexpr vec<float, N> uint_bits_to_float(vec<unsigned, N> value) {
  vec<float, N> result;
  for (unsigned index = 0; index < N; ++index)
    result[index] = uint_bits_to_float(value[index]);
  return result;
}
template <unsigned N>
constexpr vec<float, N> int_bits_to_float(vec<int, N> value) {
  vec<float, N> result;
  for (unsigned index = 0; index < N; ++index)
    result[index] = int_bits_to_float(value[index]);
  return result;
}
#ifdef __SPIRV__
template <class T> constexpr T uadd_carry(T first, T second, T &carry) {
  auto result = ::add_carry(first, second);
  carry = result.carry;
  return result.value;
}
template <class T, unsigned N>
constexpr vec<T, N> uadd_carry(vec<T, N> first, vec<T, N> second,
                               vec<T, N> &carry) {
  auto result = ::add_carry(first, second);
  carry = vec<T, N>(result.carry);
  return vec<T, N>(result.value);
}
template <class T> constexpr T usub_borrow(T first, T second, T &borrow) {
  auto result = ::sub_borrow(first, second);
  borrow = result.borrow;
  return result.value;
}
template <class T, unsigned N>
constexpr vec<T, N> usub_borrow(vec<T, N> first, vec<T, N> second,
                                vec<T, N> &borrow) {
  auto result = ::sub_borrow(first, second);
  borrow = vec<T, N>(result.borrow);
  return vec<T, N>(result.value);
}
template <class T>
constexpr void umul_extended(T first, T second, T &low, T &high) {
  auto result = ::mul_extended(first, second);
  low = result.low;
  high = result.high;
}
template <class T, unsigned N>
constexpr void umul_extended(vec<T, N> first, vec<T, N> second, vec<T, N> &low,
                             vec<T, N> &high) {
  auto result = ::mul_extended(first, second);
  low = vec<T, N>(result.low);
  high = vec<T, N>(result.high);
}
template <class T>
constexpr void imul_extended(T first, T second, T &low, T &high) {
  umul_extended(first, second, low, high);
}
template <class T, unsigned N>
constexpr void imul_extended(vec<T, N> first, vec<T, N> second, vec<T, N> &low,
                             vec<T, N> &high) {
  umul_extended(first, second, low, high);
}
template <class T> constexpr T bitfield_reverse(T value) {
  return ::bit_reverse(value);
}
template <class T, unsigned N>
constexpr vec<T, N> bitfield_reverse(vec<T, N> value) {
  return vec<T, N>(::bit_reverse(value));
}
template <class T> constexpr int bit_count(T value) {
  return intr::bit_count(value);
}
template <class T, unsigned N>
constexpr vec<int, N> bit_count(vec<T, N> value) {
  return vec<int, N>(intr::bit_count(value));
}
template <class T> constexpr int find_lsb(T value) {
  return intr::find_lsb(value);
}
template <class T, unsigned N> constexpr vec<int, N> find_lsb(vec<T, N> value) {
  return vec<int, N>(intr::find_lsb(value));
}
template <class T> constexpr int find_msb(T value) {
  return intr::find_msb(value);
}
template <class T, unsigned N> constexpr vec<int, N> find_msb(vec<T, N> value) {
  return vec<int, N>(intr::find_msb(value));
}
template <class T>
constexpr T bitfield_extract(T value, unsigned offset, unsigned bits) {
  return ::bit_extract(value, offset, bits);
}
template <class T, unsigned N>
constexpr vec<T, N> bitfield_extract(vec<T, N> value, unsigned offset,
                                     unsigned bits) {
  return vec<T, N>(::bit_extract(value, offset, bits));
}
template <class T>
constexpr T bitfield_insert(T base, T inserted, unsigned offset,
                            unsigned bits) {
  return ::bit_insert(base, inserted, offset, bits);
}
template <class T, unsigned N>
constexpr vec<T, N> bitfield_insert(vec<T, N> base, vec<T, N> inserted,
                                    unsigned offset, unsigned bits) {
  return vec<T, N>(::bit_insert(base, inserted, offset, bits));
}
inline constexpr unsigned pack_snorm4x8(vec<float, 4> value) {
  return intr::pack_snorm4x8(value);
}
inline constexpr unsigned pack_unorm4x8(vec<float, 4> value) {
  return intr::pack_unorm4x8(value);
}
inline constexpr unsigned pack_snorm2x16(vec<float, 2> value) {
  return intr::pack_snorm2x16(value);
}
inline constexpr unsigned pack_unorm2x16(vec<float, 2> value) {
  return intr::pack_unorm2x16(value);
}
inline constexpr unsigned pack_half2x16(vec<float, 2> value) {
  return intr::pack_half2x16(value);
}
inline constexpr double pack_double2x32(vec<unsigned, 2> value) {
  return intr::pack_double2x32(value);
}
inline constexpr vec<float, 4> unpack_snorm4x8(unsigned value) {
  return vec<float, 4>(intr::unpack_snorm4x8(value));
}
inline constexpr vec<float, 4> unpack_unorm4x8(unsigned value) {
  return vec<float, 4>(intr::unpack_unorm4x8(value));
}
inline constexpr vec<float, 2> unpack_snorm2x16(unsigned value) {
  return vec<float, 2>(intr::unpack_snorm2x16(value));
}
inline constexpr vec<float, 2> unpack_unorm2x16(unsigned value) {
  return vec<float, 2>(intr::unpack_unorm2x16(value));
}
inline constexpr vec<float, 2> unpack_half2x16(unsigned value) {
  return vec<float, 2>(intr::unpack_half2x16(value));
}
inline constexpr vec<unsigned, 2> unpack_double2x32(double value) {
  return vec<unsigned, 2>(intr::unpack_double2x32(value));
}
#endif
template <class T>
  requires(__is_floating_point(T))
constexpr bool is_nan(T value) {
  return intr::is_nan(value);
}
template <class T, unsigned N>
  requires(__is_floating_point(T))
constexpr vec<bool, N> is_nan(vec<T, N> value) {
  return intr::is_nan(value);
}
template <class T>
  requires(__is_floating_point(T))
constexpr bool is_inf(T value) {
  return intr::is_inf(value);
}
template <class T, unsigned N>
  requires(__is_floating_point(T))
constexpr vec<bool, N> is_inf(vec<T, N> value) {
  return intr::is_inf(value);
}
template <class T>
  requires(__is_floating_point(T))
constexpr T round_even(T value) {
  return intr::round_even(value);
}
template <class T, unsigned N>
  requires(__is_floating_point(T))
constexpr vec<T, N> round_even(vec<T, N> value) {
  return intr::round_even(value);
}

template <class T>
  requires(__is_floating_point(T))
constexpr T modf(T value, T &whole) {
  auto parts = ::modf(value);
  whole = parts.whole;
  return parts.fraction;
}
template <class T, unsigned N>
  requires(__is_floating_point(T))
constexpr vec<T, N> modf(vec<T, N> value, vec<T, N> &whole) {
  auto parts = ::modf(value);
  whole = vec<T, N>(parts.whole);
  return vec<T, N>(parts.fraction);
}
template <class T>
  requires(__is_floating_point(T))
constexpr T frexp(T value, int &exponent) {
  auto parts = ::frexp(value);
  exponent = parts.exponent;
  return parts.significand;
}
template <class T, unsigned N>
  requires(__is_floating_point(T))
constexpr vec<T, N> frexp(vec<T, N> value, vec<int, N> &exponent) {
  auto parts = ::frexp(value);
  exponent = vec<int, N>(parts.exponent);
  return vec<T, N>(parts.significand);
}

} // namespace spirv
