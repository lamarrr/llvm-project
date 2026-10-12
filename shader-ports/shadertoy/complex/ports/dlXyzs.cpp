// Port of https://www.shadertoy.com/view/dlXyzs; author: ollj.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dlXyzs.json.
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
bool logEps(Context &__ctx, float s, float c) {
  if (s < 0.f)
    return true;
  return log(c * c / s / s / 1000000.f) > 0.5f;
}
float N(Context &__ctx, float uv) { return fract(sin(uv + 5.0f) * 39352.212f); }
float noise(Context &__ctx, float uv) {
  float s = uv;
  float frac = fract(s);
  float id = floor(s);
  float l = N(__ctx, id);
  float r = N(__ctx, id + 1.0f);
  float m = mix(l, r, frac);
  return m;
}
float fbnoise(Context &__ctx, float uv) {
  float n = noise(__ctx, uv * 4.0f);
  n += noise(__ctx, uv * 8.0f) * 0.5f;
  n += noise(__ctx, uv * 16.0f) * 0.25f;
  n += noise(__ctx, uv * 32.0f) * 0.125f;
  n /= 1.875f;
  return n;
}
float line(Context &__ctx, f32x2 uv, f32x2 a, f32x2 b) {
  f32x2 pa = uv - a;
  f32x2 ba = b - a;
  float p = dot(pa, ba) / dot(ba, ba);
  return length(uv - ba * min(max(p, 0.0f), 1.0f));
}
float map(Context &__ctx, f32x2 uv) {
  return uv.data[1] - 0.8f +
         fbnoise(__ctx, uv.data[0] * 0.5f + iTime * 0.05f) * 2.0f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 mouseP = (2.0f * iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
                 iResolution.data[1];
  float d = map(__ctx, uv);
  f32x3 col =
      mix(mix(f32x3(0.6f, 0.6f, 0.0f), f32x3(0.5f, 0.6f, 0.9f), step(0.0f, d)),
          f32x3(0.8f), sin(d * 50.0f + iTime * 4.0f * sign(-d)) * 0.3f + 0.3f);
  col -= smoothstep(0.01f, 0.0f, abs(d));
  f32x2 origin = f32x2(0.0f);
  f32x2 dir = normalize(mouseP - origin);
  float t = 0.0f;
  for (int i = 0; i < 100 && t < 5.0f; i++) {
    f32x2 p = origin + dir * t;
    float dist = map(__ctx, p);
    col = mix(col, f32x3(1.0f, 0.0f, 0.0f),
              step(abs(length(uv - p) - dist) - 0.005f, 0.0f));
    if (logEps(__ctx, dist, t))
      break;
    t += dist * mix(0.2f, 1.f, (cos(iTime * 10.f) * .5f + .5f));
  }
  col = mix(col, f32x3(0.0f),
            step(line(__ctx, uv, origin, origin + dir * t) - 0.01f, 0.0f));
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
