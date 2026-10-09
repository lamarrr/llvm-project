// Port of https://www.shadertoy.com/view/flXyRl; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/flXyRl.json.
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
float cc(Context &__ctx, float a, float b) {
  float f = tanh(a * cos(b)) / tanh(a);
  return sign(f) * pow(abs(f), 0.25f);
}
float cs(Context &__ctx, float a, float b) {
  float f = tanh(a * sin(b)) / tanh(a);
  return sign(f) * pow(abs(f), 0.25f);
}
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
  float k = 0.03f;
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
float sdBox(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.0f)) + min(max(d.data[0], d.data[1]), 0.0f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  float rep = 3.f * 3.14159f;
  float m = min(mod(iTime, rep), 0.5f * rep);
  float a = -3.14159f + rep * floor(iTime / rep) +
            rep * smoothstep(0.f, 0.5f * rep, m);
  float c = 0.8f * cos(0.4f * iTime);
  float sc = 4.5f;
  uv *= sc;
  uv.data[1] += 0.25f;
  uv.data[0] += c + mix(a, iTime, 0.5f);
  float r = 0.025f;
  float w = 0.02f;
  float k = sc / iResolution.data[1];
  float s =
      1.f -
      0.98f * smoothstep(-k, k, abs(-uv.data[1] + c * cos(uv.data[0])) - w);
  f32x3 e = f32x3(0.5f);
  f32x3 col = f32x3(s);
  float n = 40.f;
  for (float i = n - 1.f; i >= 0.f; i--) {
    a += mix(-1.f, 1.f, 0.5f - 0.5f * cos(iTime + i * 3.14159f / 400.f)) *
         3.14159f / n;
    r = mix(0.05f, 0.4f, cos(a + iTime + 1.f * 3.14159f * i / n) * 0.5f + 0.5f);
    float sgn = -1.f;
    float mx = mix(1.f, sqrt(2.f),
                   abs(tanh(1.f * sin(2.f * a + 4.f * iTime)) / tanh(1.f)));
    float sq = mx * sgn * (r + 0.5f * w) / sqrt(c * c * sin(a) * sin(a) + 1.f);
    f32x2 p = f32x2(a, c * cos(a)) - sq * f32x2(c * sin(a), 1.f);
    f32x2 uv2 = (uv - p) * f32x2x2(cos(a + c * sin(a)), -sin(a + c * sin(a)),
                                   sin(a + c * sin(a)), cos(a + c * sin(a)));
    float box =
        w - abs(sdBox(__ctx, uv2, f32x2(0.8f * r - 0.5f * w)) - 0.2f * r);
    s = (i / 40.f) * smoothstep(-k, k, box);
    f32x3 col2 = pal(__ctx, 0.5f * a + i / n * 2.f, e, e, e,
                     0.5f * f32x3(0, 1, 2) / 3.f);
    col2 = f32x3(1.f);
    col = mix(col, col2, s);
  }
  col = sqrt(col);
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
