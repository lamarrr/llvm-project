// Port of https://www.shadertoy.com/view/fd3fDN; author: Peace.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fd3fDN.json.
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
int STATE = 0;
f32x3 color = f32x3(0);
f32x3 col = f32x3(0);
f32x2 uv = f32x2(0);
float rand(Context &__ctx, float n) { return fract(sin(n) * 43758.5453123f); }
float noise(Context &__ctx, float p) {
  float fl = floor(p);
  float fc = fract(p);
  return mix(rand(__ctx, fl), rand(__ctx, fl + 1.0f), fc);
}
float rand(Context &__ctx, f32x2 n) {
  return fract(sin(dot(n, f32x2(12.9898f, 4.1414f))) * 43758.5453f);
}
float noise(Context &__ctx, f32x2 p) {
  f32x2 ip = floor(p);
  f32x2 u = fract(p);
  u = u * u * (3.0f - 2.0f * u);
  float res =
      mix(mix(rand(__ctx, ip), rand(__ctx, ip + f32x2(1.0f, 0.0f)), u.data[0]),
          mix(rand(__ctx, ip + f32x2(0.0f, 1.0f)),
              rand(__ctx, ip + f32x2(1.0f, 1.0f)), u.data[0]),
          u.data[1]);
  return res * res;
}
float fbm(Context &__ctx, f32x2 p, int octaves) {
  float n = 0.0f;
  float a = 1.0f;
  float norm = 0.0f;
  for (int i = 0; i < octaves; ++i) {
    n += noise(__ctx, p) * a;
    norm += a;
    p *= 2.0f;
    a *= 0.7f;
  }
  return n / norm;
}
float luma(Context &__ctx, f32x3 color) {
  return dot(color, f32x3(0.2126f, 0.7152f, 0.0722f));
}
void rect(Context &__ctx, float x, float y, float w, float h) {
  if (uv.data[0] >= x && uv.data[0] <= x + w && uv.data[1] >= y &&
      uv.data[1] <= y + h)
    color = col;
}
void srect(Context &__ctx, float x, float y, float w, float h, float r,
           float s) {
  f32x2 offset = abs((uv - f32x2(x + w * 0.5f, y + h * 0.5f)) / f32x2(w, h)) -
                 (0.5f - s - r);
  color = mix(col, color, smoothstep(0.0f, s, length(max(offset, 0.0f)) - r));
}
void background(Context &__ctx) {
  f32x2 luv = uv;
  uv.data[0] = abs(uv.data[0]);
  col = ((STATE == 1 || STATE == 2) ? f32x3(1.0f, 0.98f, 0.87f)
                                    : f32x3(0.235f, 0.24f, 0.266f));
  srect(__ctx, (iResolution.data[0] / iResolution.data[1]) * 0.3f, 0.6f,
        (iResolution.data[0] / iResolution.data[1]) * 0.55f, 0.06f, 0.05f,
        0.1f);
  srect(__ctx, (iResolution.data[0] / iResolution.data[1]) * 0.28f, 0.1f,
        (iResolution.data[0] / iResolution.data[1]) * 0.53f, 0.06f, 0.05f,
        0.1f);
  f32x3 lcolor = color;
  float l = luma(__ctx, color);
  col =
      ((STATE == 1 || STATE == 2) ? f32x3(0.9f, 0.76f, 0.55f) *
                                        (0.8f + 0.4f * fbm(__ctx, uv * 8.0f, 4))
                                  : f32x3(0.235f, 0.24f, 0.266f));
  rect(__ctx, -(iResolution.data[0] / iResolution.data[1]), -1.0f,
       (iResolution.data[0] / iResolution.data[1]) * 2.0f, 2.0f);
  col = ((STATE == 1 || STATE == 2) ? f32x3(0.1f, 0.06f, 0.03f)
                                    : f32x3(0.235f, 0.24f, 0.266f) * 0.7f);
  srect(__ctx, -(iResolution.data[0] / iResolution.data[1]) * 2.0f, -1.0f,
        (iResolution.data[0] / iResolution.data[1]) * 4.0f, 0.5f, 0.0f, 0.05f);
  col = col * ((STATE == 1 || STATE == 2) ? 2.0f : 1.5f);
  srect(__ctx, -(iResolution.data[0] / iResolution.data[1]) * 2.0f, -0.54f,
        (iResolution.data[0] / iResolution.data[1]) * 4.0f, 0.15f, 0.0f, 0.05f);
  col = col * 0.5f;
  srect(__ctx, 0.0f, -1.0f, (iResolution.data[0] / iResolution.data[1]), 0.4f,
        0.0f, 0.05f);
  col = ((STATE == 1 || STATE == 2) ? f32x3(0.1f, 0.06f, 0.03f)
                                    : f32x3(0.235f, 0.24f, 0.266f));
  srect(__ctx, (iResolution.data[0] / iResolution.data[1]) * 0.25f, -0.55f,
        0.08f, 0.15f, 0.0f, 0.2f);
  srect(__ctx, (iResolution.data[0] / iResolution.data[1]) * 0.23f, -0.6f,
        0.08f, 0.17f, 0.0f, 0.2f);
  srect(__ctx, (iResolution.data[0] / iResolution.data[1]) * 0.21f, -0.6f,
        0.08f, 0.14f, 0.0f, 0.2f);
  srect(__ctx, -0.1f, -0.6f, 0.6f, 0.125f, 0.0f, 0.2f);
  color = mix(color, lcolor, l * l * l * l);
  color += l * l * l * l * 0.2f;
  uv = luv;
}
void shades(Context &__ctx) {
  float shades = cos(uv.data[0] * 50.0f);
  float shade_col = 0.5f + 0.5f * abs(shades);
  shade_col /= 1.0f + smoothstep(-0.25f, -1.0f, -shades);
  shade_col *= 0.8f + fbm(__ctx, uv * 2.0f, 8) * 0.4f;
  shade_col *= 0.9f + noise(__ctx, uv.data[0] * 200.0f) * 0.2f;
  color = mix(color,
              sqrt(sqrt(luma(__ctx, color))) * 1.3f *
                  ((STATE == 1 || STATE == 2) ? f32x3(0.855f, 0.75f, 0.58f)
                                              : f32x3(0.235f, 0.24f, 0.266f)) *
                  shade_col,
              step(shades, STATE == 2 ? -0.2f : 0.95f));
}
void frame(Context &__ctx) {
  float w = 0.15f;
  float h = 0.6f;
  col = f32x3(0.5f) * (0.8f + 0.4f * fbm(__ctx, uv * 12.0f, 8));
  f32x2 luv = uv;
  uv = abs(uv);
  float l = 0.2f;
  float b = 0.2f;
  float f = float(uv.data[1] <= 1.0f - w);
  col += sqrt(smoothstep(1.0f - w * l, 1.0f, abs(uv.data[1]))) * b;
  col +=
      float(uv.data[1] >= 1.0f - w &&
            uv.data[0] <=
                (iResolution.data[0] / iResolution.data[1]) - w * h + w * l &&
            (uv.data[0] >= (iResolution.data[0] / iResolution.data[1]) * 0.6f -
                               w * h * l ||
             uv.data[0] <= (iResolution.data[0] / iResolution.data[1]) * 0.6f -
                               w * h + w * h * l)) *
      sqrt(smoothstep(1.0f - w + w * l, 1.0f - w, uv.data[1])) * b;
  col += sqrt(smoothstep((iResolution.data[0] / iResolution.data[1]) - w * l,
                         (iResolution.data[0] / iResolution.data[1]),
                         uv.data[0])) *
         b;
  col +=
      f *
      float(uv.data[0] >= (iResolution.data[0] / iResolution.data[1]) - w * h) *
      sqrt(smoothstep(
          (iResolution.data[0] / iResolution.data[1]) - w * h + w * l,
          (iResolution.data[0] / iResolution.data[1]) - w * h, uv.data[0])) *
      b;
  col +=
      f *
      float(uv.data[0] <= (iResolution.data[0] / iResolution.data[1]) * 0.6f) *
      sqrt(
          smoothstep((iResolution.data[0] / iResolution.data[1]) * 0.6f -
                         w * h + w * l * 0.8f,
                     (iResolution.data[0] / iResolution.data[1]) * 0.6f - w * h,
                     uv.data[0])) *
      b;
  col +=
      f *
      float(uv.data[0] <= (iResolution.data[0] / iResolution.data[1]) * 0.6f) *
      sqrt(smoothstep(
          (iResolution.data[0] / iResolution.data[1]) * 0.6f - w * l * 0.8f,
          (iResolution.data[0] / iResolution.data[1]) * 0.6f, uv.data[0])) *
      b;
  f32x3 lcol = col;
  col = color * 0.6f;
  float s = 0.3f;
  rect(__ctx, -(iResolution.data[0] / iResolution.data[1]),
       1.0f - w * (1.0f + s / (iResolution.data[0] / iResolution.data[1])),
       2.0f * (iResolution.data[0] / iResolution.data[1]),
       w * h * (1.0f + s * 2.0f));
  rect(__ctx, (iResolution.data[0] / iResolution.data[1]) - w * h * (1.0f + s),
       -1.0f, w * h * (1.0f + s * 2.0f), 2.0f);
  rect(__ctx,
       (iResolution.data[0] / iResolution.data[1]) * 0.6f - w * h * (1.0f + s),
       -1.0f, w * h * (1.0f + s * 2.0f), 2.0f);
  col = color * 0.6f;
  s *= 0.63f;
  rect(__ctx, -(iResolution.data[0] / iResolution.data[1]),
       1.0f - w * (1.0f + s / (iResolution.data[0] / iResolution.data[1])),
       2.0f * (iResolution.data[0] / iResolution.data[1]),
       w * h * (1.0f + s * 2.0f));
  rect(__ctx, (iResolution.data[0] / iResolution.data[1]) - w * h * (1.0f + s),
       -1.0f, w * h * (1.0f + s * 2.0f), 2.0f);
  rect(__ctx,
       (iResolution.data[0] / iResolution.data[1]) * 0.6f - w * h * (1.0f + s),
       -1.0f, w * h * (1.0f + s * 2.0f), 2.0f);
  col = min(lcol, lcol + b);
  rect(__ctx, -(iResolution.data[0] / iResolution.data[1]), 1.0f - w,
       2.0f * (iResolution.data[0] / iResolution.data[1]), w);
  rect(__ctx, (iResolution.data[0] / iResolution.data[1]) - w * h, -1.0f, w * h,
       2.0f);
  rect(__ctx, (iResolution.data[0] / iResolution.data[1]) * 0.6f - w * h, -1.0f,
       w * h, 2.0f);
  uv = luv;
}
void window(Context &__ctx) {
  background(__ctx);
  shades(__ctx);
  frame(__ctx);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 index = floor(fragCoord * 5.0f / iResolution.swizzle<0, 1>());
  fragCoord = mod(fragCoord * 5.0f, iResolution.swizzle<0, 1>());
  uv = (fragCoord * 2.0f - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  STATE = int(noise(__ctx, index + iTime * 0.5f) * 3.0f);
  window(__ctx);
  fragColor = f32x4(color, 1.0f);
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
