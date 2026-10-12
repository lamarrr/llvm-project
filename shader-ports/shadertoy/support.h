// Explicit support for the Shadertoy ports, not a compiler default header.
#pragma once

#include <spirv/types.hpp>

namespace glsl {
using namespace spirv;
inline float fract(float x) { return x - ::floor(x); }
inline float sign(float x) { return x > 0 ? 1.0f : x < 0 ? -1.0f : 0.0f; }
inline float exp2(float x) { return ::exp2f(x); }
inline float log2(float x) { return ::log2f(x); }
inline float round(float x) { return ::floor(x + 0.5f); }
inline float trunc(float x) { return ::truncf(x); }
template <class T, unsigned N> vec<T, N> fract(vec<T, N> a) {
  vec<T, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = fract(a[i]);
  return r;
}
template <class T, unsigned N> vec<T, N> sign(vec<T, N> a) {
  vec<T, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = sign(a[i]);
  return r;
}
template <class T, unsigned N> vec<T, N> exp2(vec<T, N> a) {
  vec<T, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = exp2(a[i]);
  return r;
}
template <class T, unsigned N> vec<T, N> log2(vec<T, N> a) {
  vec<T, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = log2(a[i]);
  return r;
}
template <class T, unsigned N> vec<T, N> round(vec<T, N> a) {
  vec<T, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = round(a[i]);
  return r;
}
template <class T, unsigned N> vec<T, N> trunc(vec<T, N> a) {
  vec<T, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = trunc(a[i]);
  return r;
}

inline float sin(int x) { return ::sin(static_cast<float>(x)); }
inline float cos(int x) { return ::cos(static_cast<float>(x)); }
inline float tan(int x) { return ::tan(static_cast<float>(x)); }
inline float asin(int x) { return ::asin(static_cast<float>(x)); }
inline float acos(int x) { return ::acos(static_cast<float>(x)); }
inline float atan(int x) { return ::atan(static_cast<float>(x)); }
inline float sinh(int x) { return ::sinh(static_cast<float>(x)); }
inline float cosh(int x) { return ::cosh(static_cast<float>(x)); }
inline float tanh(int x) { return ::tanh(static_cast<float>(x)); }
inline float asinh(int x) { return ::asinh(static_cast<float>(x)); }
inline float acosh(int x) { return ::acosh(static_cast<float>(x)); }
inline float atanh(int x) { return ::atanh(static_cast<float>(x)); }
inline float radians(int x) { return ::radians(static_cast<float>(x)); }
inline float degrees(int x) { return ::degrees(static_cast<float>(x)); }
template <class T, unsigned N> bool operator==(vec<T, N> a, vec<T, N> b) {
  for (unsigned i = 0; i < N; ++i)
    if (a[i] != b[i])
      return false;
  return true;
}
template <class T, unsigned N> bool operator!=(vec<T, N> a, vec<T, N> b) {
  return !(a == b);
}
template <class T, unsigned N> vec<T, N> operator&(vec<T, N> a, vec<T, N> b) {
  vec<T, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = a[i] & b[i];
  return r;
}
template <class T, unsigned N, class S>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> operator&(vec<T, N> a, S b) {
  return a & vec<T, N>(b);
}
template <class T, unsigned N> vec<T, N> operator|(vec<T, N> a, vec<T, N> b) {
  vec<T, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = a[i] | b[i];
  return r;
}
template <class T, unsigned N, class S>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> operator|(vec<T, N> a, S b) {
  return a | vec<T, N>(b);
}
template <class T, unsigned N> vec<T, N> operator^(vec<T, N> a, vec<T, N> b) {
  vec<T, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = a[i] ^ b[i];
  return r;
}
template <class T, unsigned N, class S>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> operator^(vec<T, N> a, S b) {
  return a ^ vec<T, N>(b);
}
template <class T, unsigned N> vec<T, N> operator<<(vec<T, N> a, vec<T, N> b) {
  vec<T, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = a[i] << b[i];
  return r;
}
template <class T, unsigned N, class S>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> operator<<(vec<T, N> a, S b) {
  return a << vec<T, N>(b);
}
template <class T, unsigned N> vec<T, N> operator>>(vec<T, N> a, vec<T, N> b) {
  vec<T, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = a[i] >> b[i];
  return r;
}
template <class T, unsigned N, class S>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> operator>>(vec<T, N> a, S b) {
  return a >> vec<T, N>(b);
}
struct Params {
  f32x4 resolution_time, mouse, date;
  float time_delta, frame_rate;
  int frame;
  float sample_rate;
  float channel_time[4];
  f32x3 channel_resolution[4];
};
static_assert(sizeof(Params) == 128);
} // namespace glsl
