// Port of https://www.shadertoy.com/view/DdSyDW; author: Delincoter.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DdSyDW.json.
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
f32x2 hash22(Context &__ctx, f32x2 p) {
  p = f32x2(dot(p, f32x2(127.1f, 311.7f)), dot(p, f32x2(269.5f, 183.3f)));
  return -1.0f + 2.0f * fract(sin(p) * 43758.5453123f);
}
float noise(Context &__ctx, f32x2 p) {
  const float K1 = 0.366025404f;
  const float K2 = 0.211324865f;
  f32x2 i = floor(p + (p.data[0] + p.data[1]) * K1);
  f32x2 a = p - (i - (i.data[0] + i.data[1]) * K2);
  f32x2 o = (a.data[0] < a.data[1]) ? f32x2(0.0f, 1.0f) : f32x2(1.0f, 0.0f);
  f32x2 b = a - (o - K2);
  f32x2 c = a - (1.0f - 2.0f * K2);
  f32x3 h = max(0.5f - f32x3(dot(a, a), dot(b, b), dot(c, c)), 0.0f);
  f32x3 n = h * h * h * h *
            f32x3(dot(a, hash22(__ctx, i)), dot(b, hash22(__ctx, i + o)),
                  dot(c, hash22(__ctx, i + 1.0f)));
  return dot(f32x3(70.0f, 70.0f, 70.0f), n);
}
float noise_itself(Context &__ctx, f32x2 p) {
  float f = 0.f;
  p *= 8.f;
  f += 1.0000f * noise(__ctx, p);
  p = 6.0f * p;
  f += 0.1000f * noise(__ctx, p);
  p = 4.0f * p;
  f += 0.0100f * noise(__ctx, p);
  p = 3.0f * p;
  return f;
}
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float GetDist(Context &__ctx, f32x3 p) {
  float d = p.data[1] + 4.f;
  p.data[2] -= iTime;
  float h = max(0.f, noise_itself(__ctx, p.swizzle<0, 2>() * 0.01f + 13.7f));
  h = pow(h, 1.6f) * 8.f;
  d -= h;
  return d * 0.3f;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.f;
  for (int i = 0; i < 200; i++) {
    f32x3 p = ro + rd * dO;
    float dS = GetDist(__ctx, p);
    dO += dS;
    if (dO > 100.f || abs(dS) < .001f)
      break;
  }
  return dO;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = GetDist(__ctx, p) - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()),
                                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()),
                                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()));
  return normalize(n);
}
f32x3 GetRayDir(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u;
  return normalize(i);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = f32x2(-.05f, -0.5f);
  if (iMouse.data[2] > 0.f)
    m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 ro = f32x3(0, 3, -3);
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() *
                       (Rot(__ctx, -m.data[1] * 3.141592f + 1.f)));
  ro.data[1] = max(ro.data[1], -1.f);
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() *
                       (Rot(__ctx, -m.data[0] * 6.283185f)));
  f32x3 rd = GetRayDir(__ctx, uv, ro, f32x3(0, 0.f, 0), 1.f);
  f32x3 col = f32x3(0);
  float d = RayMarch(__ctx, ro, rd);
  float sun = smoothstep(0.998f, 1.f,
                         dot(normalize(rd), normalize(f32x3(0.f, 0.5f, -1.f))));
  if (d < 100.f) {
    f32x3 p = ro + rd * d;
    f32x3 n = GetNormal(__ctx, p);
    f32x3 lightDir = normalize(f32x3(0.0f, .5f, -.5f));
    float fre = max(0.f, dot(n, normalize(ro - p)));
    fre = pow(fre, .125f);
    fre = smoothstep(0.8f, 0.85f, fre);
    fre = mix(fre, 1.f, smoothstep(-0.0f, -4.f, p.data[1]));
    float heigh = smoothstep(-4.f, 0.f, p.data[1]);
    col = mix(f32x3(0.6588f, 0.5176f, 0.2824f), f32x3(0.3059f, 0.4627f, 0.633f),
              heigh) *
          fre;
    heigh = smoothstep(-4.1f, -3.9f, p.data[1]);
    col = mix(f32x3(0.6f, 0.55f, 0.2f) * smoothstep(0.f, 15.f, d), col, heigh);
    sun = .0f;
  }
  col = mix(col, f32x3(0.6588f, 0.5176f, 0.2824f) * 1.5f,
            smoothstep(0.f, 100.f, d));
  col = mix(col, f32x3(1.f, 0.4f, 0.f), sun);
  fragColor = f32x4(col, 1.f);
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
