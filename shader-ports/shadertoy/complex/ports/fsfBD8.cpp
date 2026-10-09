// Port of https://www.shadertoy.com/view/fsfBD8; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fsfBD8.json.
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
f32x3 pal(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(6.28318f * (c * t + d));
}
float h21(Context &__ctx, f32x2 a) {
  return fract(sin(dot(a.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
float mlength(Context &__ctx, f32x2 uv) {
  return max(abs(uv.data[0]), abs(uv.data[1]));
}
float mlength(Context &__ctx, f32x3 uv) {
  return max(max(abs(uv.data[0]), abs(uv.data[1])), abs(uv.data[2]));
}
float smin(Context &__ctx, float a, float b) {
  float k = 0.12f;
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float h21(Context &__ctx, float a, float b, float zoom) {
  a = mod(a, zoom);
  b = mod(b, zoom);
  return fract(sin(dot(f32x2(a, b), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
float rand(Context &__ctx, f32x2 ipos, float val) {
  float a = 2.f * 3.14159f * h21(__ctx, ipos);
  float c = cos(a), s = sin(a);
  float f = floor(val);
  float v = h21(__ctx, f32x2(c * f, s * f) + 0.01f * ipos);
  float v2 = h21(__ctx, f32x2(c * (f + 1.f), s * (f + 1.f)) + 0.01f * ipos);
  return mix(v, v2, smoothstep(0.f, 1.f, fract(val)));
}
float GetDist(Context &__ctx, f32x3 p) {
  f32x2 uv = p.swizzle<0, 2>();
  uv *= Rot(__ctx, 0.28f * p.data[1] - 0.1f * iTime);
  uv *= 3.f;
  f32x2 ipos = floor(uv) + 0.5f;
  f32x2 fpos = uv - ipos;
  float h = h21(__ctx, ipos);
  float h2 = rand(__ctx, ipos, h + 0.2f * iTime);
  float time = h * 4.f * iTime + p.data[1] * h2 * 9.f;
  f32x2 q = h2 * 0.1f * f32x2(cos(time), sin(time));
  float r =
      0.1f * (1.f + 0.5f * cos(2.f * 3.14159f * h + 4.f * p.data[1] + iTime));
  float d = length(fpos - q) - r;
  float sd = length(p - f32x3(0, -3.5f, -1.3f)) - 0.7f;
  d = -smin(__ctx, -0.35f * d, sd);
  return d;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd, float z) {
  float dO = 0.f;
  for (int i = 0; i < 400; i++) {
    f32x3 p = ro + rd * dO;
    float dS = GetDist(__ctx, p);
    if (abs(dS) < .001f || dO > 35.f)
      break;
    dO += dS * z;
  }
  return min(dO, 35.f);
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
  f32x3 ro = f32x3(0, -3.5f, -1.3f);
  if (iMouse.data[2] > 0.f) {
    ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() *
                         (Rot(__ctx, -m.data[1] * 3.14f + 1.f)));
    ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() *
                         (Rot(__ctx, -m.data[0] * 6.2831f)));
  }
  f32x3 rd = GetRayDir(__ctx, uv, ro, f32x3(0, 0.f, 0), 1.8f);
  f32x3 col = f32x3(0);
  float d = RayMarch(__ctx, ro, rd, 1.f);
  float IOR = 1.05f;
  if (d < 35.f) {
    f32x3 p = ro + rd * d;
    f32x3 n = GetNormal(__ctx, p);
    f32x3 rdIn = refract(rd, n, 1.f / IOR);
    f32x3 pEnter = p - n * .001f * 30.f;
    float dIn = RayMarch(__ctx, pEnter, rdIn, -1.f);
    f32x3 pExit = pEnter + rdIn * dIn;
    f32x3 nExit = -GetNormal(__ctx, pExit);
    float fresnel = pow(1.f + dot(rd, n), 3.f);
    col = 2.5f * f32x3(fresnel);
    col *= 0.55f + 0.45f * cross(nExit, n);
    float ln = length(p.swizzle<0, 2>());
    f32x3 e = f32x3(1.f);
    col *= pal(
        __ctx, 4.f * ln + .05f * iTime, e, e, e,
        0.5f + 0.5f *
                   tanh(3.f * cos(20.f * p.data[1] + 10.f * ln - 8.f * iTime)) /
                   tanh(3.f) * f32x3(0, 1, 2) / 3.f);
    col *= (0.32f * p.data[1] + .95f);
    col *= 0.8f + 1.f * n.data[1];
    col *= clamp(1.f - 0.3f * ln, 0.f, 1.f);
    fresnel = pow(1.f + dot(rd, n), 1.f);
    col = mix(col, f32x3(fresnel), clamp(-0.3f + 0.15f * p.data[1], 0.f, 1.f));
  }
  col = pow(col, f32x3(.4545f));
  col += 0.04f;
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
