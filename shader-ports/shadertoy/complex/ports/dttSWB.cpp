// Port of https://www.shadertoy.com/view/dttSWB; author: henrmota.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dttSWB.json.
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
  float c = cos(a), s = sin(a);
  return f32x2x2(c, -s, s, c);
}
float hash21(Context &__ctx, f32x2 st) {
  return fract(sin(dot(st, f32x2(12.9898f, 4.1414f))) * 43758.5453f);
}
float snoise(Context &__ctx, f32x2 st) {
  f32x2 id = floor(st);
  f32x2 gridUv = fract(st);
  f32x2 dx = f32x2(1.f, 0);
  float tl = hash21(__ctx, id);
  float tr = hash21(__ctx, id + dx);
  float bl = hash21(__ctx, id + dx.swizzle<1, 0>());
  float br = hash21(__ctx, id + dx.swizzle<0, 0>());
  gridUv = gridUv * gridUv * 3.f - 2.f * gridUv * gridUv * gridUv;
  float t = mix(tl, tr, gridUv.data[0]);
  float b = mix(bl, br, gridUv.data[0]);
  return mix(t, b, gridUv.data[1]);
}
float fbm(Context &__ctx, f32x2 st) {
  float amp = 1.f;
  float freq = 0.5f;
  float col = 0.f;
  for (float i = 0.f; i < 10.f; i++) {
    col += amp * snoise(__ctx, st * freq);
    amp *= 0.5f;
    freq *= 2.f;
  }
  return col / 2.f;
}
float brain(Context &__ctx, f32x2 uv) {
  float n = snoise(__ctx, uv);
  n += snoise(__ctx, uv * 3.f + n);
  n *= 0.5f;
  n += snoise(__ctx, uv * 4.f + n);
  n *= 0.5f;
  return n;
}
float getDist(Context &__ctx, f32x3 p) {
  f32x4 sphere = f32x4(0.f, 0.f, 3.f, .8f);
  p.data[0] = abs(p.data[0]);
  float n = brain(__ctx, abs(p.swizzle<0, 0>()) * 2.f + 0.1f * iTime);
  f32x3 cp =
      f32x3(sphere.data[0], clamp(p.data[1], -0.2f, 0.2f), sphere.data[2]);
  f32x3 noise =
      0.1f * f32x3(snoise(__ctx, abs(p.swizzle<1, 2>()) * 6.f + iTime + n),
                   snoise(__ctx, abs(p.swizzle<0, 0>() + 0.1f) * 6.f + iTime),
                   0.2f * snoise(__ctx, abs(p.swizzle<2, 2>()) * 10.f + iTime));
  cp -= mix(f32x3(0.f, 0.f, -p.data[1] * 0.25f), noise, p.data[0] * 1.2f);
  return length(p - cp) - sphere.data[3];
}
f32x3 getNormal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(0.001f, 0.f);
  float d = getDist(__ctx, p);
  return normalize(f32x3(getDist(__ctx, p + e.swizzle<0, 1, 1>()),
                         getDist(__ctx, p + e.swizzle<1, 0, 1>()),
                         getDist(__ctx, p + e.swizzle<1, 1, 0>())) -
                   f32x3(getDist(__ctx, p - e.swizzle<0, 1, 1>()),
                         getDist(__ctx, p - e.swizzle<1, 0, 1>()),
                         getDist(__ctx, p - e.swizzle<1, 1, 0>())));
}
float rayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.f;
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + rd * dO;
    float dS = getDist(__ctx, p);
    dO += dS;
    if (dO > 100.f || dS < 0.0001f)
      break;
  }
  return dO;
}
f32x2 getLight(Context &__ctx, f32x3 p, f32x3 ro) {
  f32x3 lightPos = f32x3(1.f, 0.f, -3.f);
  lightPos.data[0] += 3.f * sin(iTime);
  f32x3 l = normalize(lightPos - p);
  f32x3 n = getNormal(__ctx, p);
  float fresnel = 1.f - abs(dot(normalize(p - ro), normalize(n)));
  fresnel = pow(fresnel, 2.f);
  return f32x2(clamp(dot(n, l), 0.f, 1.f), fresnel);
}
f32x3 cosPalette(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(6.28318f * (c * t + d));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col = cosPalette(__ctx, fract(uv.data[0] * .1f + 0.1f * iTime),
                         f32x3(0.7f, 0.33f, 0.67f), f32x3(0.50f, 0.2f, 0.87f),
                         f32x3(1.0f, 1.00f, 0.67f), f32x3(0.30f, 0.33f, 0.67f));
  f32x3 ro = f32x3(0.f, 0.f, 0.f);
  f32x3 rd = normalize(f32x3(uv, 1.f));
  float d = rayMarch(__ctx, ro, rd);
  f32x3 p = ro + d * rd;
  if (d < 100.f) {
    f32x2 light = getLight(__ctx, p, ro);
    col +=
        max((cosPalette(__ctx,
                        snoise(__ctx, p.swizzle<1, 1>() * .1f + light.data[1] +
                                          iTime * 0.4f),
                        f32x3(0.7f, 0.33f, 0.67f), f32x3(0.50f, 0.2f, 0.87f),
                        f32x3(1.0f, 1.00f, 0.67f), f32x3(0.30f, 0.33f, 0.67f)) +
             light.data[0]) *
                .3f,
            light.data[1] * 1.2f);
  }
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
