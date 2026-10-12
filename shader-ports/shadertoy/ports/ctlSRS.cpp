// Port of https://www.shadertoy.com/view/ctlSRS; author: oseday.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctlSRS.json.
#include "../support.h"
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
struct Context {
  TextureCube channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x4x4 build_transform(Context &__ctx, f32x3 pos, f32x3 ang) {
  float cosX = cos(ang.data[0]);
  float sinX = sin(ang.data[0]);
  float cosY = cos(ang.data[1]);
  float sinY = sin(ang.data[1]);
  float cosZ = cos(ang.data[2]);
  float sinZ = sin(ang.data[2]);
  f32x4x4 m;
  float m00 = cosY * cosZ + sinX * sinY * sinZ;
  float m01 = cosY * sinZ - sinX * sinY * cosZ;
  float m02 = cosX * sinY;
  float m03 = 0.0f;
  float m04 = -cosX * sinZ;
  float m05 = cosX * cosZ;
  float m06 = sinX;
  float m07 = 0.0f;
  float m08 = sinX * cosY * sinZ - sinY * cosZ;
  float m09 = -sinY * sinZ - sinX * cosY * cosZ;
  float m10 = cosX * cosY;
  float m11 = 0.0f;
  float m12 = pos.data[0];
  float m13 = pos.data[1];
  float m14 = pos.data[2];
  float m15 = 1.0f;
  m[0][0] = m00;
  m[0][1] = m01;
  m[0][2] = m02;
  m[0][3] = m03;
  m[1][0] = m04;
  m[1][1] = m05;
  m[1][2] = m06;
  m[1][3] = m07;
  m[2][0] = m08;
  m[2][1] = m09;
  m[2][2] = m10;
  m[2][3] = m11;
  m[3][0] = m12;
  m[3][1] = m13;
  m[3][2] = m14;
  m[3][3] = m15;
  return m;
}
f32x4x4 rotationMatrix(Context &__ctx, f32x3 axis, float angle) {
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0f - c;
  return f32x4x4(oc * axis.data[0] * axis.data[0] + c,
                 oc * axis.data[0] * axis.data[1] - axis.data[2] * s,
                 oc * axis.data[2] * axis.data[0] + axis.data[1] * s, 0.0f,
                 oc * axis.data[0] * axis.data[1] + axis.data[2] * s,
                 oc * axis.data[1] * axis.data[1] + c,
                 oc * axis.data[1] * axis.data[2] - axis.data[0] * s, 0.0f,
                 oc * axis.data[2] * axis.data[0] - axis.data[1] * s,
                 oc * axis.data[1] * axis.data[2] + axis.data[0] * s,
                 oc * axis.data[2] * axis.data[2] + c, 0.0f, 0.0f, 0.0f, 0.0f,
                 1.0f);
}
f32x3 rotateabout(Context &__ctx, f32x3 vec, f32x3 axis, float angle) {
  return (rotationMatrix(__ctx, axis, angle) * f32x4(vec, 1.0f))
      .swizzle<0, 1, 2>();
}
float time;
float cross4D(Context &__ctx, f32x4 p) {
  float aa = length(p.swizzle<0, 1>()) - 0.0001f;
  float bb = length(p.swizzle<2, 3>()) - 0.0001f;
  float cc = length(p.swizzle<1, 3>()) - 0.0001f;
  float dd = length(p.swizzle<0, 3>()) - 0.0001f;
  return min(min(min(aa, bb), cc), dd);
}
float de4d(Context &__ctx, f32x4 p) {
  p.set_swizzle<2, 3>(
      p.swizzle<2, 3>() *
      (f32x2x2(cos(3.14159265359f / 2.f +
                   3.14159265359f * 0.5f * f32x4(0, 1, 3, 0)))));
  p.data[3] -= time * 1.87454f;
  p.data[2] -= 1.24f;
  f32x4 inG = (fract(p * 0.45f) - 0.5f);
  return cross4D(__ctx, inG);
}
float de(Context &__ctx, f32x3 p) {
  float d = dot(p, p);
  d = de4d(__ctx, f32x4(p / d, 0.f)) * d;
  return d;
}
f32x3 getNormal(Context &__ctx, f32x3 p) {
  f32x3 e = f32x3(0.0f, 0.00005f, 0.0f);
  return normalize(f32x3(
      de(__ctx, p + e.swizzle<1, 0, 0>()) - de(__ctx, p - e.swizzle<1, 0, 0>()),
      de(__ctx, p + e.swizzle<0, 1, 0>()) - de(__ctx, p - e.swizzle<0, 1, 0>()),
      de(__ctx, p + e.swizzle<0, 0, 1>()) -
          de(__ctx, p - e.swizzle<0, 0, 1>())));
}
struct rayresult {
  f32x3 o;
  f32x3 v;
  float dist;
};
f32x4x4 cam = f32x4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
f32x4x4 addmv(Context &__ctx, f32x4x4 mat, f32x3 v) {
  f32x4x4 n = mat;
  n[3] = n[3] + f32x4(v, 0);
  return n;
}
f32x4x4 mat4v(Context &__ctx, f32x3 v) {
  return f32x4x4(1, 0, 0, v.data[0], 0, 1, 0, v.data[1], 0, 0, 1, v.data[2], 0,
                 0, 0, 1);
}
f32x3 matpos(Context &__ctx, f32x4x4 mat) { return mat[3].swizzle<0, 1, 2>(); }
const float planedist = 1.00f;
float map(Context &__ctx, f32x3 p) { return de(__ctx, p); }
const int kRaymarchIterations = 100;
const float mapddivisor = 1.08f;
const f32x3 sundir = normalize(f32x3(0, 1, 1));
const f32x3 suncol = f32x3(0.8f, 0.8f, 0.8f);
const f32x3 objcol = f32x3(0.9f, 0.9f, 0.8f);
f32x3 manager(Context &__ctx, rayresult io) {
  f32x3 skycol = texture(__ctx.channel0, io.v).swizzle<0, 1, 2>();
  float t = 0.00f;
  for (int i = 0; i < kRaymarchIterations; i++) {
    f32x3 pos = io.o + io.v * t;
    float d = map(__ctx, pos);
    if (d < 0.002f) {
      if (length(pos) > 0.95f)
        return skycol;
      if (length(pos - io.o) > 0.3f) {
        f32x3 n = getNormal(__ctx, pos);
        float sundot = dot(n, sundir);
        f32x3 col = suncol * sundot + objcol;
        f32x3 sdir = reflect(io.v, n);
        col *= texture(__ctx.channel0, sdir).swizzle<0, 1, 2>();
        return col;
      } else {
        skycol *= 0.2f;
        t += 0.05f;
      }
    }
    t += d / mapddivisor;
  }
  return skycol;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float tick = iTime / 5.f;
  time = tick;
  f32x2 mouseuv =
      (iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>() / 2.0f) /
      iResolution.swizzle<0, 0>() * 2.f;
  f32x4x4 ncam =
      cam *
      build_transform(__ctx, f32x3(0, 0, 0),
                      f32x3(0, -mouseuv.data[0] * 3.14159265359f, 0)) *
      build_transform(__ctx, f32x3(0, 0, 0),
                      f32x3(-mouseuv.data[1] * 3.14159265359f, 0, 0));
  ncam = ncam * build_transform(__ctx, f32x3(0, 0, -1.00f), f32x3(0, 0, 0));
  f32x3 campos = matpos(__ctx, ncam);
  f32x2 uv = (fragCoord - iResolution.swizzle<0, 1>() / 2.0f) /
             iResolution.swizzle<0, 0>() * 2.f;
  f32x3 v = normalize(uv.data[0] * ncam[0].swizzle<0, 1, 2>() +
                      uv.data[1] * ncam[1].swizzle<0, 1, 2>() +
                      ncam[2].swizzle<0, 1, 2>() * planedist);
  f32x3 color = f32x3(0, 0, 0);
  rayresult ray;
  ray.o = campos;
  ray.v = v;
  ray.dist = 0.10f;
  color += manager(__ctx, ray);
  color = clamp(color, 0.f, 1.f);
  fragColor = f32x4(color, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_cube<f32x4> channel0,
               __binding(1, 0) sampler sampling0) {
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
  Context __ctx{{channel0, sampling0}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
