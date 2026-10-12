// Port of https://www.shadertoy.com/view/Ws3fDH; author: emymin.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Ws3fDH.json.
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
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float Hash21(Context &__ctx, f32x2 p) {
  p = fract(p * f32x2(123.34f, 233.53f));
  p += dot(p, p + 23.234f);
  return fract(p.data[0] * p.data[1]);
}
float opSmoothUnion(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  return mix(d2, d1, h) - k * h * (1.0f - h);
}
float repeat(Context &__ctx, float x, float margin) {
  return (mod(x + 0.5f * margin, margin) - 0.5f * margin);
}
f32x2 repeat_id(Context &__ctx, float x, float margin) {
  float p = mod(x + 0.5f * margin, margin) - 0.5f * margin;
  float id = floor((x + 0.5f * margin) / margin);
  return f32x2(p, id);
}
float sdBox(Context &__ctx, f32x3 p, f32x3 s) {
  p = abs(p) - s;
  return length(max(p, 0.f)) +
         min(max(p.data[0], max(p.data[1], p.data[2])), 0.f);
}
float archimedes(Context &__ctx, f32x3 p) {
  p.data[1] = repeat(__ctx, p.data[1], 2.f);
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() *
                      Rot(__ctx, p.data[1] * 3.14159f * 0.5f));
  float d = sdBox(__ctx, p, f32x3(1));
  return d - .5f;
}
f32x2 GetDist_id(Context &__ctx, f32x3 p) {
  f32x2 ax = repeat_id(__ctx, p.data[0], 20.f);
  f32x2 az = repeat_id(__ctx, p.data[2], 20.f);
  f32x3 repeatedspace = f32x3(ax.data[0], p.data[1], az.data[0]);
  f32x3 randomoffsetspace =
      repeatedspace +
      f32x3(0.f, 1.f, 0.f) * Hash21(__ctx, f32x2(ax.data[1], az.data[1])) *
          20.f +
      f32x3(0.f, 1.f, 0.f) *
          (80.f * Hash21(__ctx, f32x2(az.data[1], ax.data[1])) + iTime * -10.f);
  randomoffsetspace.data[1] =
      (repeat(__ctx, randomoffsetspace.data[1], 100.f)) / 1.5f;
  float sphered = length(randomoffsetspace) -
                  Hash21(__ctx, f32x2(ax.data[1], az.data[1])) * 8.f;
  float d = archimedes(__ctx, repeatedspace);
  float id = 0.f;
  d = opSmoothUnion(__ctx, d, sphered, 2.5f);
  return f32x2(d * .5f, sphered / d);
}
float GetDist(Context &__ctx, f32x3 p) {
  return f32x2(GetDist_id(__ctx, p)).data[0];
}
f32x2 RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.f;
  float id = 0.f;
  for (int i = 0; i < 1000; i++) {
    f32x3 p = ro + rd * dO;
    f32x2 dist = GetDist_id(__ctx, p);
    id = dist.data[1];
    float dS = dist.data[0];
    dO += dS * 1.f;
    if (dO > 100000.f || abs(dS) < .001f)
      break;
  }
  return f32x2(dO, id);
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p);
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()));
  return normalize(n);
}
f32x3 GetRayDir(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i);
  return d;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 col;
  f32x3 ro = f32x3(5.f + sin(iTime / 10.f) * 100.f, 0,
                   5.f + cos(iTime / 10.f) * 100.f);
  f32x3 rd = GetRayDir(__ctx, uv, ro, ro + f32x3(2.f * 3.14159f, 0, 10), 1.f);
  f32x2 result = RayMarch(__ctx, ro, rd);
  float d = result.data[0];
  float id = result.data[1];
  if (d < 100000.f) {
    f32x3 p = ro + rd * d;
    f32x3 n = GetNormal(__ctx, p);
    col = clamp(mix(f32x3(1, 0, 0), f32x3(0, 0, 0), id / 1000.f),
                f32x3(0, 0, 0), f32x3(1, 0, 0));
    float dif = dot(n, normalize(f32x3(1, 2, 3))) * .5f + .5f;
    col += dif;
    col *= 1.f - (d / 200.f);
  }
  if (d < 0.f) {
    col = f32x3(0.5f, 0.5f, 0.5f);
  }
  col = pow(col, f32x3(.4545f));
  fragColor = f32x4(col, 1.0f);
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
