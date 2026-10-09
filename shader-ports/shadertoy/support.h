// Explicit support for the Shadertoy ports, not a compiler default header.
#pragma once

#include <types.hpp>

namespace glsl {
inline float fract(float x) { return x - ::floor(x); }
inline float mod(float x, float y) { return x - y * ::floor(x / y); }
inline float sign(float x) { return x > 0 ? 1.0f : x < 0 ? -1.0f : 0.0f; }
inline float step(float edge, float x) { return x < edge ? 0.0f : 1.0f; }
inline float smoothstep(float a, float b, float x) {
  float t = ::clamp((x - a) / (b - a), 0.0f, 1.0f);
  return t * t * (3.0f - 2.0f * t);
}
inline float mix(float a, float b, float t) { return ::lerp(a, b, t); }
inline float inversesqrt(float x) { return 1.0f / ::sqrt(x); }
inline float exp2(float x) { return ::exp2f(x); }
inline float log2(float x) { return ::log2f(x); }
inline float round(float x) { return ::floor(x + 0.5f); }
inline float trunc(float x) { return ::truncf(x); }
inline float atan(float y, float x) { return ::atan2(y, x); }
inline float length(float x) { return ::abs(x); }
inline float distance(float a, float b) { return ::abs(a - b); }
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
template <class T, unsigned N> vec<T, N> inversesqrt(vec<T, N> a) {
  vec<T, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = inversesqrt(a[i]);
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
template <class T, unsigned N> vec<T, N> mod(vec<T, N> a, vec<T, N> b) {
  vec<T, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = mod(a[i], b[i]);
  return r;
}
template <class T, unsigned N, class S> vec<T, N> mod(vec<T, N> a, S b) {
  return mod(a, vec<T, N>(b));
}
template <class S, class T, unsigned N> vec<T, N> mod(S a, vec<T, N> b) {
  return mod(vec<T, N>(a), b);
}
template <class T, unsigned N> vec<T, N> step(vec<T, N> a, vec<T, N> b) {
  vec<T, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = step(a[i], b[i]);
  return r;
}
template <class T, unsigned N, class S> vec<T, N> step(vec<T, N> a, S b) {
  return step(a, vec<T, N>(b));
}
template <class S, class T, unsigned N> vec<T, N> step(S a, vec<T, N> b) {
  return step(vec<T, N>(a), b);
}
template <class T, unsigned N> vec<T, N> atan(vec<T, N> a, vec<T, N> b) {
  return ::atan2(a, b);
}
template <class T, unsigned N>
vec<T, N> mix(vec<T, N> a, vec<T, N> b, vec<T, N> t) {
  return ::lerp(a, b, t);
}
template <class T, unsigned N, class S>
vec<T, N> mix(vec<T, N> a, vec<T, N> b, S t) {
  return ::lerp(a, b, vec<T, N>(t));
}
template <class T, unsigned N> vec<T, N> smoothstep(T a, T b, vec<T, N> x) {
  vec<T, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = smoothstep(a, b, x[i]);
  return r;
}
template <class T, unsigned N>
vec<T, N> smoothstep(vec<T, N> a, vec<T, N> b, vec<T, N> x) {
  vec<T, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = smoothstep(a[i], b[i], x[i]);
  return r;
}
template <class T, unsigned N, class S>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> min(vec<T, N> a, S b) {
  return ::min(a, vec<T, N>(b));
}
template <class S, class T, unsigned N>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> min(S a, vec<T, N> b) {
  return ::min(vec<T, N>(a), b);
}
template <class T, unsigned N, class S>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> max(vec<T, N> a, S b) {
  return ::max(a, vec<T, N>(b));
}
template <class S, class T, unsigned N>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> max(S a, vec<T, N> b) {
  return ::max(vec<T, N>(a), b);
}
template <class T, unsigned N, class S>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> pow(vec<T, N> a, S b) {
  return ::pow(a, vec<T, N>(b));
}
template <class S, class T, unsigned N>
  requires(!requires(S s) { s.data[0]; })
vec<T, N> pow(S a, vec<T, N> b) {
  return ::pow(vec<T, N>(a), b);
}
template <class T, unsigned N, class A, class B>
vec<T, N> clamp(vec<T, N> v, A a, B b) {
  return ::clamp(v, vec<T, N>(a), vec<T, N>(b));
}
template <class T, unsigned N> bool any(vec<T, N> x) {
  for (unsigned i = 0; i < N; ++i)
    if (x[i])
      return true;
  return false;
}
template <class T, unsigned N> bool all(vec<T, N> x) {
  for (unsigned i = 0; i < N; ++i)
    if (!x[i])
      return false;
  return true;
}
template <class T, unsigned N> vec<bool, N> lessThan(vec<T, N> a, vec<T, N> b) {
  vec<bool, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = a[i] < b[i];
  return r;
}
template <class T, unsigned N>
vec<bool, N> greaterThan(vec<T, N> a, vec<T, N> b) {
  vec<bool, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = a[i] > b[i];
  return r;
}
template <class T, unsigned N>
vec<bool, N> lessThanEqual(vec<T, N> a, vec<T, N> b) {
  vec<bool, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = a[i] <= b[i];
  return r;
}
template <class T, unsigned N>
vec<bool, N> greaterThanEqual(vec<T, N> a, vec<T, N> b) {
  vec<bool, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = a[i] >= b[i];
  return r;
}
template <class T, unsigned N> vec<bool, N> equal(vec<T, N> a, vec<T, N> b) {
  vec<bool, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = a[i] == b[i];
  return r;
}
template <class T, unsigned N> vec<bool, N> notEqual(vec<T, N> a, vec<T, N> b) {
  vec<bool, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = a[i] != b[i];
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
inline float dFdx(float x) { return ::ddx(x); }
template <unsigned N> vec<float, N> dFdx(vec<float, N> x) {
  vec<float, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = ::ddx(x[i]);
  return r;
}
inline float dFdy(float x) { return ::ddy(x); }
template <unsigned N> vec<float, N> dFdy(vec<float, N> x) {
  vec<float, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = ::ddy(x[i]);
  return r;
}
template <unsigned N> vec<float, N> fwidth(vec<float, N> x) {
  vec<float, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = ::fwidth(x[i]);
  return r;
}
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
inline unsigned floatBitsToUint(float x) {
  return __builtin_bit_cast(unsigned, x);
}
template <unsigned N> vec<unsigned, N> floatBitsToUint(vec<float, N> x) {
  vec<unsigned, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = floatBitsToUint(x[i]);
  return r;
}
inline int floatBitsToInt(float x) { return __builtin_bit_cast(int, x); }
template <unsigned N> vec<int, N> floatBitsToInt(vec<float, N> x) {
  vec<int, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = floatBitsToInt(x[i]);
  return r;
}
inline float uintBitsToFloat(unsigned x) {
  return __builtin_bit_cast(float, x);
}
template <unsigned N> vec<float, N> uintBitsToFloat(vec<unsigned, N> x) {
  vec<float, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = uintBitsToFloat(x[i]);
  return r;
}
inline float intBitsToFloat(int x) { return __builtin_bit_cast(float, x); }
template <unsigned N> vec<float, N> intBitsToFloat(vec<int, N> x) {
  vec<float, N> r;
  for (unsigned i = 0; i < N; ++i)
    r[i] = intBitsToFloat(x[i]);
  return r;
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
