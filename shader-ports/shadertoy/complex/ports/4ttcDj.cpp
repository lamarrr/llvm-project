// Port of https://www.shadertoy.com/view/4ttcDj; author: skaven.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/4ttcDj.json.
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
struct Context {
  TextureCube channel0;
  TextureCube channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float rand(Context &__ctx, f32x2 c) {
  return fract(sin(dot(c.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453f);
}
float noise(Context &__ctx, f32x2 p, float freq) {
  float unit = 4.0f / freq;
  f32x2 ij = floor(p / unit);
  f32x2 xy = mod(p, unit) / unit;
  xy = .5f * (1.f - cos(3.141592f * xy));
  float a = rand(__ctx, (ij + f32x2(0.f, 0.f)));
  float b = rand(__ctx, (ij + f32x2(1.f, 0.f)));
  float c = rand(__ctx, (ij + f32x2(0.f, 1.f)));
  float d = rand(__ctx, (ij + f32x2(1.f, 1.f)));
  float x1 = mix(a, b, xy.data[0]);
  float x2 = mix(c, d, xy.data[0]);
  return mix(x1, x2, xy.data[1]);
}
float pNoise(Context &__ctx, f32x2 p, int res) {
  float persistance = .5f;
  float n = 0.f;
  float normK = 0.f;
  float f = 4.f;
  float amp = 1.f;
  int iCount = 0;
  for (int i = 0; i < 50; i++) {
    n += amp * noise(__ctx, p, f);
    f *= 2.f;
    normK += amp;
    amp *= persistance;
    if (iCount == res)
      break;
    iCount++;
  }
  float nf = n / normK;
  return nf * nf * nf * nf;
}
f32x3x3 rotateX(Context &__ctx, float theta) {
  float c = cos(theta);
  float s = sin(theta);
  return f32x3x3(f32x3(1, 0, 0), f32x3(0, c, -s), f32x3(0, s, c));
}
f32x3x3 rotateY(Context &__ctx, float theta) {
  float c = cos(theta);
  float s = sin(theta);
  return f32x3x3(f32x3(c, 0, s), f32x3(0, 1, 0), f32x3(-s, 0, c));
}
float rand(Context &__ctx, float n) { return fract(sin(n) * 43758.5453123f); }
float noise(Context &__ctx, float p) {
  float fl = floor(p);
  float fc = fract(p);
  return mix(rand(__ctx, fl), rand(__ctx, fl + 1.0f), fc);
}
float distToCircle(Context &__ctx, f32x2 uv, float radius, float phase,
                   float ffly) {
  float ng = atan(uv.data[1], uv.data[0]) + phase + iTime * (0.1f + ffly);
  float distDist =
      sin(ng * 2.0f) * cos(4.0f * ng + sin(ng) - sin(ng * 12.0f) * 0.1f);
  return abs(length(uv) - radius + distDist * 0.01f) +
         abs(tan(ng + phase * 1.4f)) * 0.004f * ffly;
}
float dToC(Context &__ctx, f32x2 ngDist, float dradius) {
  float ng = ngDist.data[0] + iTime * 10.0f;
  float noise = pNoise(__ctx, f32x2(cos(ng), sin(ng)) + f32x2(0.5f), 2);
  ngDist.data[1] +=
      noise * 0.2f + dradius +
      (sin(iTime * 5.171f) * sin(ngDist.data[0] * 2.0f)) * dradius - 0.1f;
  float thick1 = 0.004f;
  float thick2 = 0.008f;
  return smoothstep(0.0f, thick1, ngDist.data[1]) -
         smoothstep(thick1, thick2, ngDist.data[1]);
}
f32x2 distoAngleDist(Context &__ctx, f32x3 dir) {
  f32x2 dp = dir.swizzle<0, 1>() / dir.data[2];
  float ng = atan(dp.data[1], dp.data[0]);
  float r = pNoise(__ctx, f32x2(cos(ng), sin(ng)) + f32x2(iTime * 0.5f), 1);
  r = 0.5f + r * 0.1f;
  return f32x2(ng, length(dp) - r);
}
float distToPortal(Context &__ctx, f32x2 ngDist) {
  return smoothstep(0.0f, 0.01f, ngDist.data[1]);
}
float aero(Context &__ctx, f32x2 ngDist) {
  ngDist.data[0] += iTime * 0.01f;
  float baseMask = min(1.0f - max(ngDist.data[1] * 1.5f, 0.0f), 1.0f) *
                   smoothstep(0.0f, 0.01f, ngDist.data[1]);
  float bore = (sin(ngDist.data[0] * 4.0f));
  bore += (sin(ngDist.data[0] * 6.0f + iTime * 1.374f));
  bore += (sin(-ngDist.data[0] * 8.0f + iTime * 0.51f)) * 0.1f;
  baseMask *= clamp(bore * 0.3f + 0.5f, 0.3f, 1.0f);
  return clamp(baseMask, 0.0f, 1.0f);
}
float lightning(Context &__ctx, f32x2 ngDist, float time) {
  float growth = fract(time * 2.0f) * 0.1f;
  float ng = ngDist.data[0];
  float noise = pNoise(
      __ctx,
      f32x2(cos(ng), sin(ng)) * (6.0f - growth * 6.0f) + f32x2(time * 0.5f), 2);
  noise += pNoise(__ctx,
                  f32x2(cos(ng), sin(ng)) * (4.0f - growth * 4.0f) +
                      f32x2(time * 0.1f),
                  2) *
           0.2f;
  float adist = abs(ngDist.data[1] - noise * 0.2f - growth);
  float diflum = min(sin(ng * 4.0f + time * 1.1f) + 0.8f, 1.0f);
  return clamp((1.0f - smoothstep(0.0f, 0.01f, adist)) * growth * 10.0f *
                   diflum,
               0.0f, 1.0f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  uv -= 0.5f;
  uv.data[1] *= iResolution.data[1] / iResolution.data[0];
  float col = 20.0f;
  for (float i = 0.0f; i < 1.0f; i += 0.1f) {
    col -= smoothstep(0.0f, 0.002f,
                      distToCircle(__ctx, uv, 0.17f + i * 0.01f, i, 0.0f));
    col -= smoothstep(0.0f, 0.002f,
                      distToCircle(__ctx, uv, 0.18f + i * 0.03f, i, 1.0f));
  }
  f32x3 eyeSrc = normalize(f32x3(uv * 4.0f, 1.0f));
  f32x2 mouse =
      (iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>()) * 0.8f - 0.4f;
  f32x3 eye =
      rotateY(__ctx, mouse.data[0]) * rotateX(__ctx, mouse.data[1]) * eyeSrc;
  f32x2 ngDist = distoAngleDist(__ctx, eye);
  fragColor = f32x4(f32x3(col) * f32x3(0.4f, 0.8f, 1.0f) * 3.0f, 1.0f);
  fragColor = mix(texture(__ctx.channel1, eye), texture(__ctx.channel0, eye),
                  distToPortal(__ctx, ngDist));
  fragColor += f32x4(lightning(__ctx, ngDist, iTime) +
                     lightning(__ctx, ngDist, iTime + 12.37981f));
  f32x4 auraCol =
      mix(f32x4(0.12f, 0.745f, 0.364f, 1.0f),
          f32x4(0.15f, 0.474f, 0.891f, 1.0f), sin(iTime) * 0.5f + 0.5f);
  fragColor += auraCol * aero(__ctx, ngDist);
  float r1 = 0.0f;
  for (float i = 0.0f; i < 1.0f; i += 0.1f) {
    r1 += dToC(__ctx, ngDist, i * 0.08f);
  }
  fragColor += r1 * auraCol * 2.0f;
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_cube<f32x4> channel0,
               __binding(1, 0) sampler sampling0,
               __binding(0, 1) texture_cube<f32x4> channel1,
               __binding(1, 1) sampler sampling1) {
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
  Context __ctx{{channel0, sampling0}, {channel1, sampling1}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
