//===-- vec.hpp - C-array-backed vector utilities for Shader C++ ---------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#pragma once

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
  template <unsigned... I> constexpr vec<T, sizeof...(I)> swizzle() const {
    static_assert(((I < N) && ...));
    return vec<T, sizeof...(I)>(data[I]...);
  }
#define GLSL_SWIZZLE_2(A, AI, B, BI)                                           \
  constexpr vec<T, 2> A##B() const                                             \
    requires(N > AI && N > BI)                                                 \
  {                                                                            \
    return swizzle<AI, BI>();                                                  \
  }
#define GLSL_SWIZZLE_3(A, AI, B, BI, C, CI)                                    \
  constexpr vec<T, 3> A##B##C() const                                          \
    requires(N > AI && N > BI && N > CI)                                       \
  {                                                                            \
    return swizzle<AI, BI, CI>();                                              \
  }
#define GLSL_SWIZZLE_4(A, AI, B, BI, C, CI, D, DI)                             \
  constexpr vec<T, 4> A##B##C##D() const                                       \
    requires(N > AI && N > BI && N > CI && N > DI)                             \
  {                                                                            \
    return swizzle<AI, BI, CI, DI>();                                          \
  }
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
  template <unsigned... I, class S> constexpr void set_swizzle(S value) {
    vec<T, sizeof...(I)> v(value);
    unsigned k = 0;
    ((data[I] = v.data[k++]), ...);
  }
};
template <class T, unsigned N> vec<T, N> operator+(vec<T, N> a, vec<T, N> b) {
  return ::add(a, b);
}
template <class T, unsigned N, class S>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> operator+(vec<T, N> a, S b) {
  return a + vec<T, N>(b);
}
template <class S, class T, unsigned N>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> operator+(S a, vec<T, N> b) {
  return vec<T, N>(a) + b;
}
template <class T, unsigned N, class S>
vec<T, N> &operator+=(vec<T, N> &a, S b) {
  a = a + b;
  return a;
}
template <class T, unsigned N> vec<T, N> operator-(vec<T, N> a, vec<T, N> b) {
  return ::sub(a, b);
}
template <class T, unsigned N, class S>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> operator-(vec<T, N> a, S b) {
  return a - vec<T, N>(b);
}
template <class S, class T, unsigned N>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> operator-(S a, vec<T, N> b) {
  return vec<T, N>(a) - b;
}
template <class T, unsigned N, class S>
vec<T, N> &operator-=(vec<T, N> &a, S b) {
  a = a - b;
  return a;
}
template <class T, unsigned N> vec<T, N> operator*(vec<T, N> a, vec<T, N> b) {
  return ::mul(a, b);
}
template <class T, unsigned N, class S>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> operator*(vec<T, N> a, S b) {
  return a * vec<T, N>(b);
}
template <class S, class T, unsigned N>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> operator*(S a, vec<T, N> b) {
  return vec<T, N>(a) * b;
}
template <class T, unsigned N, class S>
vec<T, N> &operator*=(vec<T, N> &a, S b) {
  a = a * b;
  return a;
}
template <class T, unsigned N> vec<T, N> operator/(vec<T, N> a, vec<T, N> b) {
  return ::div(a, b);
}
template <class T, unsigned N, class S>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> operator/(vec<T, N> a, S b) {
  return a / vec<T, N>(b);
}
template <class S, class T, unsigned N>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> operator/(S a, vec<T, N> b) {
  return vec<T, N>(a) / b;
}
template <class T, unsigned N, class S>
vec<T, N> &operator/=(vec<T, N> &a, S b) {
  a = a / b;
  return a;
}
template <class T, unsigned N> vec<T, N> operator-(vec<T, N> a) {
  return vec<T, N>(0) - a;
}
template <class T, unsigned N> vec<T, N> operator+(vec<T, N> a) { return a; }
