// Port of https://www.shadertoy.com/view/DtBGWG; author: ducttapecode.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtBGWG.json.
#include "../support.h"
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
float random11(Context &__ctx, float t) {
  return fract(sin(t * 12.34f) * 567.89f);
}
float random21(Context &__ctx, f32x2 st) {
  return fract(sin(dot(st.swizzle<0, 1>(), f32x2(123.45f, 67.89f))) *
               98765.4321f);
}
f32x2 random2(Context &__ctx, f32x2 st) {
  st = f32x2(dot(st, f32x2(127.1f, 311.7f)), dot(st, f32x2(269.5f, 183.3f)));
  return -1.0f + 2.0f * fract(sin(st) * 43758.5453123f);
}
f32x2x2 rot1 = f32x2x2(4.f, -3.f, 4.f, 3.f) / 5.f;
float noise(Context &__ctx, f32x2 st) {
  f32x2 i = floor(st);
  f32x2 f = fract(st);
  f32x2 u = f * f * (3.0f - 2.0f * f);
  return mix(
      mix(dot(random2(__ctx, i + f32x2(0.0f, 0.0f)), f - f32x2(0.0f, 0.0f)),
          dot(random2(__ctx, i + f32x2(1.0f, 0.0f)), f - f32x2(1.0f, 0.0f)),
          u.data[0]),
      mix(dot(random2(__ctx, i + f32x2(0.0f, 1.0f)), f - f32x2(0.0f, 1.0f)),
          dot(random2(__ctx, i + f32x2(1.0f, 1.0f)), f - f32x2(1.0f, 1.0f)),
          u.data[0]),
      u.data[1]);
}
f32x3 hsb2rgb(Context &__ctx, f32x3 c) {
  f32x3 rgb = clamp(
      abs(mod(c.data[0] * 6.0f + f32x3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f,
      0.0f, 1.0f);
  rgb = rgb * rgb * (3.0f - 2.0f * rgb);
  return c.data[2] * mix(f32x3(1.0f), rgb, c.data[1]);
}
float blob_sdf(Context &__ctx, f32x2 uv, float r) {
  float d = length(uv);
  f32x2 n = uv / d;
  float magnitude = .5f;
  float frequency = .5f;
  f32x2 t = f32x2(iTime) * 0.5f;
  float offset = noise(__ctx, t + frequency * n);
  d += magnitude * offset;
  return d - r;
}
f32x3 hsb_noise(Context &__ctx, f32x2 uv, float hue) {
  f32x3 color = f32x3(0.0f);
  float s = 1.57f;
  for (int i = 0; i < 7; i++) {
    uv = uv * rot1 * 2.23f;
    color += hsb2rgb(__ctx, f32x3(hue, noise(__ctx, uv * 0.3f) * 0.5f + 0.45f,
                                  noise(__ctx, uv * 0.712f) * 0.2f + 0.8f)) /
             s;
    s *= 2.03f;
    uv += iTime * .25f;
  }
  return color;
}
float hue = 0.73f;
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) /
             min(iResolution.data[0], iResolution.data[1]) * 2.f;
  f32x2 hue_line = f32x2(0.123f, 0.2345f);
  hue = fract(uv.data[0] * hue_line.data[0] + uv.data[1] * hue_line.data[1] +
              0.1f * iTime);
  hue = fract(hue + 0.5f * smoothstep(-0.001f, 0.001f,
                                      uv.data[0] * -hue_line.data[1] * 1.73f +
                                          uv.data[1] * hue_line.data[0] +
                                          0.2f * sin(iTime * 0.8f)));
  f32x3 color = hsb_noise(__ctx, uv, hue);
  float r = blob_sdf(__ctx, uv, 0.75f);
  float t = 0.0f;
  t = smoothstep(-0.2f, 0.1f, r);
  color *= 1.f - 0.15f * t;
  t = smoothstep(-0.01f, 0.0f, r);
  color = mix(color, f32x3(0.97f, 0.95f, 0.93f), t);
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
