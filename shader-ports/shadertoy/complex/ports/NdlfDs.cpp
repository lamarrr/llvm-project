// Port of https://www.shadertoy.com/view/NdlfDs; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NdlfDs.json.
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
float sdSegment(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h);
}
float smin(Context &__ctx, float a, float b) {
  float k = 0.12f;
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
float h21(Context &__ctx, f32x2 a, float zoom) {
  a = mod(a, zoom);
  return fract(sin(dot(a, f32x2(12.9898f, 78.233f))) * 43758.5453123f);
}
float rand(Context &__ctx, f32x2 ipos, float num) {
  float val = h21(__ctx, ipos, num) + 0.4f * iTime;
  float a = 2.f * 3.14159f * h21(__ctx, ipos, num);
  float c = cos(a), s = sin(a);
  float f = floor(val);
  f32x2 v = f32x2(c, s);
  float v1 = h21(__ctx, f * v + 0.01f * ipos, num);
  float v2 = h21(__ctx, (f + 1.f) * v + 0.01f * ipos, num);
  float m = smoothstep(0.f, 1.f, fract(val));
  return mix(v1, v2, m);
}
f32x2 movingTiles(Context &__ctx, f32x2 uv, float sc, float spd) {
  float time = spd * iTime;
  float ft = fract(2.f * (abs(uv.data[0]) + abs(uv.data[1])) + time);
  uv *= sc;
  float k = step(0.5f, ft);
  uv.data[0] += k * sign(fract(uv.data[1] * 0.5f) - 0.5f) * ft * 2.f;
  uv.data[1] += (1.f - k) * sign(fract(uv.data[0] * 0.5f) - 0.5f) * ft * 2.f;
  return fract((2.f + 0.03f * h21(__ctx, uv)) * uv);
}
float grid(Context &__ctx, f32x2 uv, float sc) {
  f32x2 ipos = floor(sc * uv);
  f32x2 fpos = fract(sc * uv);
  float num = 301.f;
  float l = rand(__ctx, ipos + f32x2(1, 0), num);
  float t = rand(__ctx, ipos + f32x2(0, 1), num);
  float tl = rand(__ctx, ipos + f32x2(1, 1), num);
  float id = rand(__ctx, ipos + f32x2(0, 0), num);
  fpos = fpos * fpos * (3.f - 2.f * fpos);
  float v = l * fpos.data[0] * (1.f - fpos.data[1]) +
            t * (1.f - fpos.data[0]) * fpos.data[1] +
            tl * fpos.data[0] * fpos.data[1] +
            id * (1.f - fpos.data[0]) * (1.f - fpos.data[1]);
  return v;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  float sc = 10.f;
  float t = 0.2f * iTime;
  float m = 0.5f;
  f32x2 uv2 = uv;
  float v = 0.f;
  for (float i = 0.f; i < 5.f; i++) {
    t -= 2.f * uv.data[0];
    v += m * grid(__ctx, uv2, sc);
    float cs = cos(t), sn = sin(t);
    f32x2x2 rot = f32x2x2(cs, -sn, sn, cs);
    uv2 *= rot;
    m *= 0.35f;
  }
  f32x2 fpos = movingTiles(__ctx, uv, sc, -0.1f) - 0.5f;
  float d = 0.8f - mlength(__ctx, fpos);
  float k = 10.f / iResolution.data[1];
  float s = smoothstep(-k, k, -d + 0.68f * v);
  f32x3 col = f32x3(s);
  f32x3 e = f32x3(1.f);
  col += (1.f - s) *
         pal(__ctx, 0.43f + 0.15f * v, e, e, e, 0.45f * f32x3(0, 1, 2) / 3.f);
  col = clamp(col, 0.f, 1.f);
  col += s * f32x3(fpos, 1.f);
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
