// Port of https://www.shadertoy.com/view/clfGRs; author: chenxianming.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/clfGRs.json.
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
f32x2x2 rotation(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, s, -s, c);
}
const f32x2x2 m = f32x2x2(-1.1f, -.4f, .3f, 1.0f);
float noise(Context &__ctx, f32x2 p) { return sin(p.data[0]) * sin(p.data[1]); }
float fbm4(Context &__ctx, f32x2 p) {
  float f = 0.0f;
  for (float i = 0.f; i < 5.f; i++) {
    f += 1.5f * .1f * noise(__ctx, p * i);
    p = m * p * abs(i - 2.f) * 2.3f;
  }
  return f / 0.99999990f;
}
f32x2 fbm4_2(Context &__ctx, f32x2 p) {
  return f32x2(fbm4(__ctx, p), fbm4(__ctx, p + f32x2(7.8f)));
}
float myNoise(Context &__ctx, f32x2 f) {
  f *= m;
  return (cos(f.data[0] + .18975f) * sin(f.data[1] + .494516f) + .1615246f);
}
f32x2x2 rot(Context &__ctx, float d) {
  float s = sin(d), c = cos(d);
  return f32x2x2(c, s, -s, c);
}
float fbm(Context &__ctx, f32x2 x, float H) {
  float t = 0.0f;
  for (int i = 0; i < 7; i++) {
    float f = pow(2.0f, float(i));
    x *= rot(__ctx, float(i));
    float a = pow(f, -H);
    a = smoothstep(0.f, .05f, a);
    t += a * myNoise(__ctx, f * x * rot(__ctx, float(i)));
  }
  return t;
}
float coreCalc(Context &__ctx, f32x3 pos, int i) {
  f32x3 p = pos;
  p.set_swizzle<0, 2>(
      p.swizzle<0, 2>() *
      (rotation(__ctx, 3.1415926f * iTime * .1f * float(i) + float(i))));
  float d = length(p) - .5f +
            (fbm4_2(__ctx, p.swizzle<1, 2>() - iTime * .2f).data[1] * 1.5f *
             fbm4_2(__ctx, p.swizzle<0, 2>() - iTime * .2f).data[0] * 1.f *
             cos(p).data[1] * 1.f) *
                .6f;
  return d;
}
float map(Context &__ctx, f32x3 pos) {
  f32x3 p = pos;
  float d = coreCalc(__ctx, p, 1);
  float bg = length(pos) - .1f;
  d = mix(d, .05f, bg);
  return d;
}
f32x3 calcNormal(Context &__ctx, f32x3 pos) {
  f32x2 e = f32x2(1.0f, -1.0f) * 0.5773f;
  const float eps = 0.0005f;
  return normalize(
      e.swizzle<0, 1, 1>() * map(__ctx, pos + e.swizzle<0, 1, 1>() * eps) +
      e.swizzle<1, 1, 0>() * map(__ctx, pos + e.swizzle<1, 1, 0>() * eps) +
      e.swizzle<1, 0, 1>() * map(__ctx, pos + e.swizzle<1, 0, 1>() * eps) +
      e.swizzle<0, 0, 0>() * map(__ctx, pos + e.swizzle<0, 0, 0>() * eps));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  float tt = iTime * .4f;
  uv = (uv - .5f) * 2.f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 origin = f32x3(cos(tt) * -1.2f, 0.0f, sin(tt) * -1.2f);
  f32x3 ta = f32x3(0.0f, 0.f, 0.0f);
  origin = f32x3(0.f, 0.f, -1.f);
  f32x3 ww = normalize(ta - origin);
  f32x3 uu = normalize(cross(ww, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 vv = normalize(cross(uu, ww));
  f32x3 tot = f32x3(0.0f);
  f32x3 ray = normalize(uv.data[0] * uu + uv.data[1] * vv + 1.5f * ww);
  float t = 0.0f;
  const float tmax = 5.0f;
  for (int i = 0; i < 32; ++i) {
    f32x3 p = origin + ray * t;
    float distance = map(__ctx, p);
    t += distance * 0.9999f;
  }
  f32x3 col = f32x3(0.0f);
  f32x3 lig = normalize(f32x3(.57703f));
  f32x3 hal = normalize(lig - ray);
  if (t < tmax) {
    f32x3 pos = origin + t * ray;
    f32x3 nor = calcNormal(__ctx, pos);
    float dif = clamp(dot(nor, f32x3(.4f, 1.f, -.5f)), 0.0f, 1.0f);
    float p = fbm(
        __ctx, f32x2(fbm(__ctx, nor.swizzle<0, 1>() * .5f - iTime * .01f, 1.f)),
        .9f);
    float spe = pow(clamp(dot(nor, hal), 0.0f, 1.0f), 50.0f) * .6f;
    spe *= dif;
    float amb = dot(nor, f32x3(0.0f, 1.0f, 0.0f));
    col = spe + f32x3(p * .08f + .1f) * amb + f32x3(.6f) * dif + f32x3(.3f);
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
