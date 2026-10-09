// Port of https://www.shadertoy.com/view/7tVSDy; author: incription.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/7tVSDy.json.
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
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a);
  float c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float smin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.f, 1.f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
float sdCapsule(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 ab = b - a;
  f32x3 ap = p - a;
  float t = dot(ab, ap) / dot(ab, ab);
  t = clamp(t, 0.f, 1.f);
  f32x3 c = a + t * ab;
  return length(p - c) - r;
}
float sdCylinder(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 ab = b - a;
  f32x3 ap = p - a;
  float t = dot(ab, ap) / dot(ab, ab);
  t = clamp(t, 0.f, 1.f);
  f32x3 c = a + t * ab;
  float x = length(p - c) - r;
  float y = (abs(t - .5f) - .5f) * length(ab);
  float e = length(max(f32x2(x, y), 0.f));
  float i = min(max(x, y), 0.f);
  return e + i;
}
float sdTorus(Context &__ctx, f32x3 p, f32x2 r) {
  float x = length(p.swizzle<0, 2>()) - r.data[0];
  return length(f32x2(x, p.data[1])) - r.data[1];
}
float sdBox(Context &__ctx, f32x3 p, f32x3 s) {
  p = abs(p) - s;
  return length(max(p, 0.f)) +
         min(max(p.data[0], max(p.data[1], p.data[2])), 0.f);
}
float saturate(Context &__ctx, float x) { return min(1.0f, max(0.0f, x)); }
f32x3 saturate(Context &__ctx, f32x3 x) {
  return min(f32x3(1.f, 1.f, 1.f), max(f32x3(0.f, 0.f, 0.f), x));
}
f32x3 bump3y(Context &__ctx, f32x3 x, f32x3 yoffset) {
  f32x3 y = f32x3(1.f, 1.f, 1.f) - x * x;
  y = saturate(__ctx, y - yoffset);
  return y;
}
f32x3 spectral_zucconi(Context &__ctx, float w) {
  float x = saturate(__ctx, (w - 400.0f) / 300.0f);
  const f32x3 cs = f32x3(3.54541723f, 2.86670055f, 2.29421995f);
  const f32x3 xs = f32x3(0.69548916f, 0.49416934f, 0.28269708f);
  const f32x3 ys = f32x3(0.02320775f, 0.15936245f, 0.53520021f);
  return bump3y(__ctx, cs * (x - xs), ys);
}
f32x4 GetDist(Context &__ctx, f32x3 p, float t) {
  float plane = p.data[1];
  f32x3 v = f32x3(-sin(t) * 5.f, 0, 0);
  float box = sdBox(__ctx, p - f32x3(cos(t) * 5.f, 1, -3), f32x3(1));
  float rotationalSpeed = 0.7f;
  float rotational = rotationalSpeed * t * 3.14159f * 2.f;
  f32x3 rbp = p - f32x3(0, 1, 0);
  rbp.set_swizzle<0, 2>(rbp.swizzle<0, 2>() * (Rot(__ctx, rotational)));
  float rotatingBox = sdBox(__ctx, rbp, f32x3(1));
  float speedBoxSpeed = 10.f * 0.99f;
  float speedBox =
      sdBox(__ctx, p - f32x3(10.f * 10.f - speedBoxSpeed * t, 3, -5), f32x3(1));
  if (plane < box)
    v = f32x3(0);
  if (rotatingBox < plane && rotatingBox < box) {
    float _ = rotationalSpeed * 1.f;
    v = f32x3(sin(rotational) * _, 0, cos(rotational) * _);
  }
  if (speedBox < rotatingBox)
    f32x3(speedBoxSpeed, 0.f, 0.f);
  float d = min(plane, box);
  d = min(d, rotatingBox);
  d = min(d, speedBox);
  return f32x4(d, v);
}
f32x2 RayMarch(Context &__ctx, f32x3 ro, f32x3 rd, float t) {
  float dO = 0.f;
  for (int i = 0; i < 200; i++) {
    f32x3 p = ro + rd * dO;
    float dS = GetDist(__ctx, p, t).data[0] / 2.f;
    dO += dS;
    if (iMouse.data[2] < .5f)
      t -= dS / 10.f;
    if (dO > 100.f || dS < .001f)
      break;
  }
  return f32x2(dO, t);
}
f32x3 GetNormal(Context &__ctx, f32x3 p, float t) {
  float d = GetDist(__ctx, p, t).data[0];
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>(), t).data[0],
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>(), t).data[0],
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>(), t).data[0]);
  return normalize(n);
}
float GetLight(Context &__ctx, f32x3 p, float t) {
  f32x3 lightPos = f32x3(3, 5, 4);
  f32x3 l = normalize(lightPos - p);
  f32x3 n = GetNormal(__ctx, p, t);
  float dif = clamp(dot(n, l) * .5f + .5f, 0.f, 1.f);
  float d = RayMarch(__ctx, p + n * .001f * 2.f, l, t).data[0];
  return dif;
}
f32x3 R(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = p + f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i - p);
  return d;
}
f32x3 light(Context &__ctx, float l) {
  f32x3 i = f32x3(0);
  for (float w = 400.f; w < 800.f; w += 400.f / 1.f) {
    i += spectral_zucconi(__ctx, w * l);
  }
  return i / 20.f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 col = f32x3(0);
  f32x3 ro = f32x3(0.f, 4, -5);
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() *
                       (Rot(__ctx, -m.data[1] * 3.14f + 1.f)));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (Rot(__ctx, -m.data[0] * 6.2831f)));
  f32x3 v = f32x3(cos(iTime), 0.f, 0.f);
  f32x3 rd = R(__ctx, uv, ro, f32x3(0, 1, 0), 1.f);
  f32x2 r = RayMarch(__ctx, ro, rd, iTime);
  float d = r.data[0];
  float t = r.data[1];
  if (d < 100.f) {
    f32x3 p = ro + rd * d;
    float dif = GetLight(__ctx, p, t);
    col = f32x3(dif);
    f32x3 vreal = v - GetDist(__ctx, p, t).swizzle<1, 2, 3>();
    float v = length(vreal);
    float l = length(1.f / sqrt(1.f - ((v * v) / (10.f * 10.f))));
    if (iMouse.data[2] > .5f)
      l = 1.f;
    col *= spectral_zucconi(__ctx, 550.f * l);
  }
  col = pow(col, f32x3(.4545f));
  fragColor = f32x4(col, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output) {
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
