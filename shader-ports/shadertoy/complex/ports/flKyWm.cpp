// Port of https://www.shadertoy.com/view/flKyWm; author: LorenzoFiestas.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/flKyWm.json.
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
float rand(Context &__ctx, f32x2 st);
float rand(Context &__ctx, f32x3 p);
float valueNoise(Context &__ctx, f32x2 st);
float valueNoise(Context &__ctx, f32x3 p);
float valueNoise(Context &__ctx, f32x3 p, int octaves);
f32x3 hsv2rgb(Context &__ctx, f32x3 hsv);
f32x3 rgb2hsv(Context &__ctx, f32x3 rgb);
f32x2x2 rotate(Context &__ctx, float t);
float line(Context &__ctx, f32x2 st, f32x2 p0, f32x2 p1) {
  float width =
      .15f + .125f * (.5f * valueNoise(__ctx, 2.f * st + iTime) - .5f);
  float dx = p1.data[0] - p0.data[0];
  bool divBy0 = dx == 0.f;
  if (divBy0)
    return smoothstep(width, -width, abs(st.data[0] - p0.data[0]));
  float m = (p1.data[1] - p0.data[1]) / dx;
  float b = p0.data[1] - m * p0.data[0];
  float wFix = abs(cos(atan(m)));
  width /= wFix;
  return smoothstep(width, -width, abs(m * st.data[0] + b - st.data[1]));
}
float func(Context &__ctx, float x) {
  float t = .3f * iTime;
  float ret = 0.f;
  float func0 = 1.f / x;
  float func1 = exp(x) + x;
  float func2 = 1.f / x + x;
  float func3 = cos(x);
  ret += ((smoothstep(0.f, 1.f, 1.f - abs((mod(t + 0.f, 4.f)) - 1.f))) + .01f) *
         func0;
  ret += ((smoothstep(0.f, 1.f, 1.f - abs((mod(t + 1.f, 4.f)) - 1.f))) + .01f) *
         func1;
  ret += ((smoothstep(0.f, 1.f, 1.f - abs((mod(t + 2.f, 4.f)) - 1.f))) + .01f) *
         func2;
  ret += ((smoothstep(0.f, 1.f, 1.f - abs((mod(t + 3.f, 4.f)) - 1.f))) + .01f) *
         func3;
  float twist = 2.f * sin(.1f * iTime) + 2.f;
  return ret + twist;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv0 =
      (2.f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  const float ZOOM = 4.f;
  f32x2 uv = uv0 * ZOOM * rotate(__ctx, -.25f * iTime);
  f32x2 uvr = rotate(__ctx, 0.f * (3.141592f) / 3.f) * uv;
  f32x2 uvg = rotate(__ctx, 2.f * (3.141592f) / 3.f) * uv;
  f32x2 uvb = rotate(__ctx, 4.f * (3.141592f) / 3.f) * uv;
  float mask = 0.f;
  f32x3 col = f32x3(0.f);
  const float RANGE = 5.f;
  const float LINES = 100.f;
  for (float x = -RANGE; x <= RANGE; x += RANGE / LINES) {
    mask = line(__ctx, uvr, f32x2(x, 1.f), f32x2(func(__ctx, x), -1.f));
    col.data[0] += mask;
    mask = line(__ctx, uvg, f32x2(x, 1.f), f32x2(func(__ctx, x), -1.f));
    col.data[1] += mask;
    mask = line(__ctx, uvb, f32x2(x, 1.f), f32x2(func(__ctx, x), -1.f));
    col.data[2] += mask;
  }
  col *= atan(length(col)) / (length(col) + .001f);
  f32x3 hsv = rgb2hsv(__ctx, col);
  hsv += .4f * valueNoise(__ctx,
                          f32x3(3.f * uv0, 5.f * (hsv[0] + hsv[1]) + iTime), 5);
  hsv[2] *= 1.f - smoothstep(1.f, .5f, hsv[1] + .5f);
  f32x3 rgb = hsv2rgb(__ctx, hsv);
  fragColor = f32x4(rgb, 1.f);
}
float rand(Context &__ctx, f32x2 st) {
  return fract(6373.35391f * st.data[0] *
                   sin(734.255f * (st.data[1] - st.data[0] + 35.3f)) +
               344.872343f * st.data[1] *
                   sin(5443.65f * (st.data[1] - st.data[0] + 11.2f)) +
               395.47f);
}
float rand(Context &__ctx, f32x3 p) {
  return fract(6373.35391f * p.data[0] *
                   sin(734.255f * (p.data[1] - p.data[0] - p.data[2] + 35.3f)) +
               344.872343f * p.data[1] *
                   sin(5443.65f * (p.data[1] - p.data[0] - p.data[2] + 11.2f)) +
               -42.349436f * p.data[2] *
                   sin(-246.42f * (p.data[1] - p.data[0] - p.data[2] + 7.26f)) +
               395.47f);
}
float valueNoise(Context &__ctx, f32x2 st) {
  float x0y0 = rand(__ctx, floor(st + f32x2(0.f, 0.f)));
  float x0y1 = rand(__ctx, floor(st + f32x2(0.f, 1.f)));
  float x1y0 = rand(__ctx, floor(st + f32x2(1.f, 0.f)));
  float x1y1 = rand(__ctx, floor(st + f32x2(1.f, 1.f)));
  float l = mix(x0y0, x0y1, fract(st.data[1]));
  float r = mix(x1y0, x1y1, fract(st.data[1]));
  return mix(l, r, fract(st.data[0]));
}
float valueNoise(Context &__ctx, f32x3 p) {
  float x0y0z0 = rand(__ctx, floor(p + f32x3(0.f, 0.f, 0.f)));
  float x0y1z0 = rand(__ctx, floor(p + f32x3(0.f, 1.f, 0.f)));
  float x1y0z0 = rand(__ctx, floor(p + f32x3(1.f, 0.f, 0.f)));
  float x1y1z0 = rand(__ctx, floor(p + f32x3(1.f, 1.f, 0.f)));
  float x0y0z1 = rand(__ctx, floor(p + f32x3(0.f, 0.f, 1.f)));
  float x0y1z1 = rand(__ctx, floor(p + f32x3(0.f, 1.f, 1.f)));
  float x1y0z1 = rand(__ctx, floor(p + f32x3(1.f, 0.f, 1.f)));
  float x1y1z1 = rand(__ctx, floor(p + f32x3(1.f, 1.f, 1.f)));
  float l0 = mix(x0y0z0, x0y1z0, fract(p.data[1]));
  float r0 = mix(x1y0z0, x1y1z0, fract(p.data[1]));
  float l1 = mix(x0y0z1, x0y1z1, fract(p.data[1]));
  float r1 = mix(x1y0z1, x1y1z1, fract(p.data[1]));
  return mix(mix(l0, r0, fract(p.data[0])), mix(l1, r1, fract(p.data[0])),
             fract(p.data[2]));
}
float valueNoise(Context &__ctx, f32x3 p, int octaves) {
  float k = 1.f;
  float f = 1.f;
  float ret = 0.f;
  for (int i = 0; i < octaves; i++) {
    ret += k * valueNoise(__ctx, f * p);
    k *= 1.f / sqrt(2.f);
    f *= 2.f;
  }
  return ret / (sqrt(float(octaves)));
}
f32x3 hsv2rgb(Context &__ctx, f32x3 hsv) {
  f32x3 rgb = clamp(
      abs(mod(hsv.data[0] * 6.0f + f32x3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) -
          1.0f,
      0.0f, 1.0f);
  return hsv.data[2] * mix(f32x3(1.0f), rgb, hsv.data[1]);
}
f32x3 rgb2hsv(Context &__ctx, f32x3 rgb) {
  f32x4 K = f32x4(0.0f, -1.0f / 3.0f, 2.0f / 3.0f, -1.0f);
  f32x4 p = mix(f32x4(rgb.swizzle<2, 1>(), K.swizzle<3, 2>()),
                f32x4(rgb.swizzle<1, 2>(), K.swizzle<0, 1>()),
                step(rgb.data[2], rgb.data[1]));
  f32x4 q = mix(f32x4(p.swizzle<0, 1, 3>(), rgb.data[0]),
                f32x4(rgb.data[0], p.swizzle<1, 2, 0>()),
                step(p.data[0], rgb.data[0]));
  float d = q.data[0] - min(q.data[3], q.data[1]);
  float e = 1.0e-10f;
  return f32x3(abs(q.data[2] + (q.data[3] - q.data[1]) / (6.0f * d + e)),
               d / (q.data[0] + e), q.data[0]);
}
f32x2x2 rotate(Context &__ctx, float t) {
  return f32x2x2(cos(t), -sin(t), sin(t), cos(t));
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
