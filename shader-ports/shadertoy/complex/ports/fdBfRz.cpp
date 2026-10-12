// Port of https://www.shadertoy.com/view/fdBfRz; author: oneshade.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fdBfRz.json.
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
f32x2 barrelDistort(Context &__ctx, f32x2 p, f32x2 alpha) {
  return p / (1.0f - alpha * dot(p, p));
}
float Hash11(Context &__ctx, float p) {
  p = fract(p * 0.1031f);
  p *= p + 33.33f;
  p *= p + p;
  return fract(p);
}
float snoise11(Context &__ctx, float p) {
  return mix(Hash11(__ctx, floor(p)), Hash11(__ctx, ceil(p)),
             smoothstep(0.0f, 1.0f, fract(p)));
}
float fbm11(Context &__ctx, float p) {
  p *= 6.0f;
  float res = 0.0f;
  float amp = 1.0f;
  float totAmp = 0.0f;
  for (int oct = 0; oct < 5; oct++) {
    res += amp * snoise11(__ctx, p);
    totAmp += amp;
    p *= 2.0f;
    amp *= 0.5f;
  }
  return res / totAmp;
}
float Hash21(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(p.swizzle<0, 1, 0>() * 0.1031f);
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
float snoise21(Context &__ctx, f32x2 p) {
  f32x2 cell = floor(p);
  f32x2 local = smoothstep(0.0f, 1.0f, fract(p));
  float bl = Hash21(__ctx, cell);
  float br = Hash21(__ctx, cell + f32x2(1.0f, 0.0f));
  float tl = Hash21(__ctx, cell + f32x2(0.0f, 1.0f));
  float tr = Hash21(__ctx, cell + 1.0f);
  return mix(mix(bl, br, local.data[0]), mix(tl, tr, local.data[0]),
             local.data[1]);
}
float fbm21(Context &__ctx, f32x2 p) {
  float res = 0.0f;
  float amp = 1.0f;
  float totAmp = 0.0f;
  for (int oct = 0; oct < 5; oct++) {
    res += amp * abs(2.0f * snoise21(__ctx, p) - 1.0f);
    totAmp += amp;
    p *= 2.0f;
    amp *= 0.5f;
  }
  return res / totAmp;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 center = 0.5f * iResolution.swizzle<0, 1>();
  f32x2 uv = (fragCoord - center) / iResolution.data[1] * 4.0f;
  f32x2 mouse = (iMouse.swizzle<0, 1>() - center) / iResolution.data[1] * 4.0f;
  float unit = 8.0f / iResolution.data[1];
  f32x3 color = f32x3(1.0f);
  uv.data[0] = uv.data[0] - 1.4f * sign(uv.data[0]);
  f32x2 bulgeUv = barrelDistort(__ctx, uv, f32x2(0.25f));
  f32x2 eyeballUv = bulgeUv - 0.15f * mouse;
  float co = cos(1.4f), si = sin(1.4f);
  f32x2x2 rot = f32x2x2(co, si, -si, co);
  float veinFade = smoothstep(1.8f, 0.0f, length(eyeballUv));
  for (int i = 0; i < 4; i++) {
    color.set_swizzle<1, 2>(
        color.swizzle<1, 2>() -
        (0.7f * smoothstep(0.2f, 0.0f, fbm21(__ctx, 2.0f * eyeballUv - 10.0f)) *
         veinFade));
    eyeballUv *= rot;
  }
  color = max(color, 0.0f);
  f32x2 polar =
      f32x2(atan(eyeballUv.data[1], eyeballUv.data[0]), length(eyeballUv));
  float irisWidth = 0.125f + 0.015f * snoise11(__ctx, iTime);
  float irisRadius = 0.375f - irisWidth;
  float irisDist = abs(length(eyeballUv) - irisRadius) - irisWidth;
  f32x3 irisColor = f32x3(0.7f, 0.4f, 0.0f) * fbm11(__ctx, polar.data[0]);
  irisColor += f32x3(0.0f, 0.45f, 0.0f) * fbm11(__ctx, polar.data[0] - 10.0f);
  color = mix(color, irisColor * 0.9f, smoothstep(unit, 0.0f, irisDist));
  if (polar.data[1] < irisRadius + irisWidth)
    color *= smoothstep(0.1f, -0.1f, irisDist);
  color = max(
      color - smoothstep(unit, 0.0f, polar.data[1] - irisRadius + irisWidth),
      0.0f);
  color += smoothstep(0.2f, -0.2f, length(uv - f32x2(0.3f, 0.15f)));
  f32x2 eyeUv =
      f32x2(uv.data[0], 3.5f * uv.data[1] / (2.0f - uv.data[0] * uv.data[0]));
  float eyeMask = smoothstep(0.1f, -0.1f, length(eyeUv) - 1.0f);
  eyeMask *= smoothstep(2.0f, 0.0f, length(eyeUv));
  float blink = 1.0f - 2.0f * pow(abs(cos(mod(iTime, 6.2831853072f))), 1000.0f);
  float eyelidCurve = 0.15f * bulgeUv.data[0] * bulgeUv.data[0] * blink;
  eyeMask *= smoothstep(0.1f, 0.0f, bulgeUv.data[1] + eyelidCurve - blink);
  color *= eyeMask;
  fragColor = f32x4(color, 1.0f);
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
