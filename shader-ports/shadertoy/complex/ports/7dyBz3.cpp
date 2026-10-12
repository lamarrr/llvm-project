// Port of https://www.shadertoy.com/view/7dyBz3; author: Del.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7dyBz3.json.
#include "../../support.h"
using namespace glsl;
struct Texture2D {
  texture_2d<f32x4> image;
  sampler sampling;
};
f32x4 texture(Texture2D t, f32x2 uv) {
  return t.image.sample(t.sampling, uv.data[0], uv.data[1]);
}
f32x4 textureLod(Texture2D t, f32x2 uv, float lod) {
  return t.image.sample_lod(t.sampling, uv.data[0], uv.data[1], lod);
}
i32x2 textureSize(Texture2D t, int lod) {
  auto size = t.image.dimensions(static_cast<unsigned>(lod));
  return i32x2(size[0], size[1]);
}
f32x4 texelFetch(Texture2D t, i32x2 uv) {
  return t.image.load(uv.data[0], uv.data[1]);
}
struct Texture3D {
  texture_3d<f32x4> image;
  sampler sampling;
};
f32x4 texture(Texture3D t, f32x3 uv) {
  return t.image.sample(t.sampling, uv.data[0], uv.data[1], uv.data[2]);
}
f32x4 textureLod(Texture3D t, f32x3 uv, float lod) {
  return t.image.sample_lod(t.sampling, uv.data[0], uv.data[1], uv.data[2],
                            lod);
}
i32x3 textureSize(Texture3D t, int lod) {
  auto size = t.image.dimensions(static_cast<unsigned>(lod));
  return i32x3(size[0], size[1], size[2]);
}
f32x4 texelFetch(Texture3D t, i32x3 uv) {
  return t.image.load(uv.data[0], uv.data[1], uv.data[2]);
}
struct TextureCube {
  texture_cube<f32x4> image;
  sampler sampling;
};
f32x4 texture(TextureCube t, f32x3 uv) {
  return t.image.sample(t.sampling, uv.data[0], uv.data[1], uv.data[2]);
}
f32x4 textureLod(TextureCube t, f32x3 uv, float lod) {
  return t.image.sample_lod(t.sampling, uv.data[0], uv.data[1], uv.data[2],
                            lod);
}
i32x2 textureSize(TextureCube t, int lod) {
  auto size = t.image.dimensions(static_cast<unsigned>(lod));
  return i32x2(size[0], size[1]);
}
using sampler2D = Texture2D;
using sampler3D = Texture3D;
using samplerCube = TextureCube;
struct Context {};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x4 sdBezierExtrude(Context &__ctx, f32x3 pos, f32x3 A, f32x3 B, f32x3 C) {
  f32x3 a = B - A;
  f32x3 b = A - 2.0f * B + C;
  f32x3 c = a * 2.0f;
  f32x3 d = A - pos;
  float kk = 1.0f / dot(b, b);
  float kx = kk * dot(a, b);
  float ky = kk * (2.0f * dot(a, a) + dot(d, b)) / 3.0f;
  float kz = kk * dot(d, a);
  float p = ky - kx * kx;
  float p3 = p * p * p;
  float q = kx * (2.0f * kx * kx - 3.0f * ky) + kz;
  float h = q * q + 4.0f * p3;
  float t;
  if (h >= 0.0f) {
    h = sqrt(h);
    f32x2 x = (f32x2(h, -h) - q) / 2.0f;
    f32x2 uv = sign(x) * pow(abs(x), f32x2(1.0f / 3.0f));
    t = clamp(uv.data[0] + uv.data[1] - kx, 0.0f, 1.0f);
  } else {
    float z = sqrt(-p);
    float v = acos(q / (p * z * 2.0f)) / 3.0f;
    float m = cos(v);
    float n = sin(v) * 1.732050808f;
    f32x3 _t = clamp(f32x3(m + m, -n - m, n - m) * z - kx, 0.0f, 1.0f);
    f32x3 r1 = d + (c + b * _t.data[0]) * _t.data[0];
    f32x3 r2 = d + (c + b * _t.data[1]) * _t.data[1];
    t = dot(r2, r2) < dot(r1, r1) ? _t.data[1] : _t.data[0];
  }
  f32x3 _tan = normalize((2.0f - 2.0f * t) * (B - A) + 2.0f * t * (C - B));
  f32x3 up = f32x3(0.0f, 1.0f, 0.0f);
  f32x3 binormal = normalize(cross(up, _tan));
  f32x3 _normal = cross(_tan, binormal);
  f32x3 t1 = cross(_normal, _tan);
  f32x3x3 mm = f32x3x3(t1, cross(_tan, t1), _tan);
  pos.set_swizzle<0, 1, 2>(mix(mix(A, B, t), mix(B, C, t), t) - pos);
  return f32x4(pos.swizzle<0, 1, 2>() * mm, t);
}
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 d = abs(p) - b;
  return min(max(d.data[0], max(d.data[1], d.data[2])), 0.0f) +
         length(max(d, 0.0f));
}
float sdTorus(Context &__ctx, f32x3 p, f32x2 t) {
  return length(f32x2(length(p.swizzle<0, 2>()) - t.data[0], p.data[1])) -
         t.data[1];
}
f32x2x2 rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, s, -s, c);
}
float map(Context &__ctx, f32x3 pos) {
  f32x3 a = f32x3(-5.5f, 0.0f, 0.0f);
  f32x3 b = f32x3(0.0f, sin(iTime * 1.1f) * 6.0f,
                  (0.5f + sin(iTime * 2.8f) * 0.5f) * 5.0f);
  f32x3 c = f32x3(5.5f, 0.0f, 0.0f);
  f32x4 bz = sdBezierExtrude(__ctx, pos, a, b, c);
  {
    float twist = (0.5f + sin(iTime * 1.8f) * 0.5f) * 1.1f;
    bz.set_swizzle<0, 1>(bz.swizzle<0, 1>() *
                         (rot(__ctx, (3.14f * twist) * bz.data[3])));
  }
  float d;
  if (iMouse.data[2] > 0.5f)
    d = sdTorus(__ctx, bz.swizzle<0, 1, 2>(), f32x2(0.75f, 0.35f));
  else {
    d = sdBox(__ctx, bz.swizzle<0, 1, 2>(), f32x3(1.2f, 0.1f, 0.01f)) - 0.15f;
    d = min(length(bz.swizzle<0, 1, 2>()) - 0.3f, d);
  }
  d *= 0.8f;
  return d;
}
f32x3 calcNormal(Context &__ctx, f32x3 pos) {
  f32x2 e = f32x2(1.0f, -1.0f) * 0.5773f;
  const float eps = 0.01f;
  return normalize(
      e.swizzle<0, 1, 1>() * map(__ctx, pos + e.swizzle<0, 1, 1>() * eps) +
      e.swizzle<1, 1, 0>() * map(__ctx, pos + e.swizzle<1, 1, 0>() * eps) +
      e.swizzle<1, 0, 1>() * map(__ctx, pos + e.swizzle<1, 0, 1>() * eps) +
      e.swizzle<0, 0, 0>() * map(__ctx, pos + e.swizzle<0, 0, 0>() * eps));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float an = 0.5f * iTime;
  float yy = 4.0f;
  f32x3 ro = f32x3(9.f * cos(an), yy, 9.0f * sin(an));
  f32x3 ta = f32x3(0.0f, 0.0f, 0.0f);
  f32x3 ww = normalize(ta - ro);
  f32x3 uu = normalize(cross(ww, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 vv = normalize(cross(uu, ww));
  f32x3 tot = f32x3(0.0f);
  f32x2 p =
      (-iResolution.swizzle<0, 1>() + 2.0f * fragCoord) / iResolution.data[1];
  f32x3 rd = normalize(p.data[0] * uu + p.data[1] * vv + 1.8f * ww);
  const float tmax = 50.0f;
  float t = 0.0f;
  for (int i = 0; i < 128; i++) {
    f32x3 pos = ro + t * rd;
    float h = map(__ctx, pos);
    if (h < 0.01f || t > tmax)
      break;
    t += h;
  }
  f32x3 col = f32x3(0.22f, 0.1f, 0.4f) * smoothstep(1.0f, 0.0f, abs(p.data[1]));
  if (t < tmax) {
    f32x3 pos = ro + t * rd;
    f32x3 nor = calcNormal(__ctx, pos);
    f32x3 rf = reflect(ww, nor);
    float sha = map(__ctx, pos + rf) + .5f;
    float factor = sha * length(sin(rf * 3.f) * 0.5f + 0.5f) / sqrt(2.f);
    col = mix(f32x3(0.15f, 0.05f, 0.26f), f32x3(0.28f, 0.6f, 0.2f), factor) +
          pow(factor * 0.7f, 6.f);
  }
  col = sqrt(col);
  tot += col;
  fragColor = f32x4(tot, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output) {
  iResolution = f32x3(p->resolution_time.data[0], p->resolution_time.data[1],
                      p->resolution_time.data[2]);
  gl_FragCoord =
      f32x4(frag_coord[0], frag_coord[1], frag_coord[2], frag_coord[3]);
  iTime = p->resolution_time.data[3];
  iMouse = f32x4(p->mouse.data[0], p->mouse.data[1], p->mouse.data[2],
                 p->mouse.data[3]);
  iDate =
      f32x4(p->date.data[0], p->date.data[1], p->date.data[2], p->date.data[3]);
  iTimeDelta = p->time_delta;
  iFrameRate = p->frame_rate;
  iFrame = p->frame;
  iSampleRate = p->sample_rate;
  for (unsigned i = 0; i < 4; ++i) {
    iChannelTime[i] = p->channel_time[i];
    iChannelResolution[i] = f32x3(p->channel_resolution[i].data[0],
                                  p->channel_resolution[i].data[1],
                                  p->channel_resolution[i].data[2]);
  }
  Context __ctx{};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
