// Port of https://www.shadertoy.com/view/NlKyRV; author: ufffd.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NlKyRV.json.
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
float mod289(Context &__ctx, float x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}
f32x4 mod289(Context &__ctx, f32x4 x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}
f32x4 perm(Context &__ctx, f32x4 x) {
  return mod289(__ctx, ((x * 34.0f) + 1.0f) * x);
}
float noise(Context &__ctx, f32x3 p) {
  f32x3 a = floor(p);
  f32x3 d = p - a;
  d = d * d * (3.0f - 2.0f * d);
  f32x4 b = a.swizzle<0, 0, 1, 1>() + f32x4(0.0f, 1.0f, 0.0f, 1.0f);
  f32x4 k1 = perm(__ctx, b.swizzle<0, 1, 0, 1>());
  f32x4 k2 = perm(__ctx, k1.swizzle<0, 1, 0, 1>() + b.swizzle<2, 2, 3, 3>());
  f32x4 c = k2 + a.swizzle<2, 2, 2, 2>();
  f32x4 k3 = perm(__ctx, c);
  f32x4 k4 = perm(__ctx, c + 1.0f);
  f32x4 o1 = fract(k3 * (1.0f / 41.0f));
  f32x4 o2 = fract(k4 * (1.0f / 41.0f));
  f32x4 o3 = o2 * d.data[2] + o1 * (1.0f - d.data[2]);
  f32x2 o4 =
      o3.swizzle<1, 3>() * d.data[0] + o3.swizzle<0, 2>() * (1.0f - d.data[0]);
  return o4.data[1] * d.data[1] + o4.data[0] * (1.0f - d.data[1]);
}
f32x2 rotate(Context &__ctx, f32x2 v, float a) {
  float s = sin(a);
  float c = cos(a);
  f32x2x2 m = f32x2x2(c, -s, s, c);
  return m * v;
}
float saturate(Context &__ctx, float x) { return min(1.0f, max(0.0f, x)); }
f32x3 saturate(Context &__ctx, f32x3 x) {
  return min(f32x3(1.f, 1.f, 1.f), max(f32x3(0.f, 0.f, 0.f), x));
}
f32x3 bump3y(Context &__ctx, f32x3 x, f32x3 yoffset) {
  f32x3 y = f32x3(1.f, 1.f, 1.f) - x * x;
  y = saturate(__ctx, y - yoffset);
  return y;
}
f32x3 spectral_zucconi6(Context &__ctx, float w) {
  float x = saturate(__ctx, (w - 400.0f) / 300.0f);
  const f32x3 c1 = f32x3(3.54585104f, 2.93225262f, 2.41593945f);
  const f32x3 x1 = f32x3(0.69549072f, 0.49228336f, 0.27699880f);
  const f32x3 y1 = f32x3(0.02312639f, 0.15225084f, 0.52607955f);
  const f32x3 c2 = f32x3(3.90307140f, 3.21182957f, 3.96587128f);
  const f32x3 x2 = f32x3(0.11748627f, 0.86755042f, 0.66077860f);
  const f32x3 y2 = f32x3(0.84897130f, 0.88445281f, 0.73949448f);
  return bump3y(__ctx, c1 * (x - x1), y1) + bump3y(__ctx, c2 * (x - x2), y2);
}
f32x3 spectral_jet(Context &__ctx, float w) {
  float x = saturate(__ctx, (w - 400.0f) / 300.0f);
  f32x3 c;
  if (x < 0.25f)
    c = f32x3(0.0f, 4.0f * x, 1.0f);
  else if (x < 0.5f)
    c = f32x3(0.0f, 1.0f, 1.0f + 4.0f * (0.25f - x));
  else if (x < 0.75f)
    c = f32x3(4.0f * (x - 0.5f), 1.0f, 0.0f);
  else
    c = f32x3(1.0f, 1.0f + 4.0f * (0.75f - x), 0.0f);
  return saturate(__ctx, c);
}
float cubicPulse(Context &__ctx, float c, float w, float x) {
  x = abs(x - c);
  if (x > w)
    return 0.0f;
  x /= w;
  return 1.0f - x * x * (3.0f - 2.0f * x);
}
f32x3 filmicToneMapping(Context &__ctx, f32x3 color) {
  color = max(f32x3(0.f), color - f32x3(0.004f));
  color =
      (color * (6.2f * color + .5f)) / (color * (6.2f * color + 1.7f) + 0.06f);
  return color;
}
float drawSquare(Context &__ctx, f32x2 uv, f32x2 p, float size) {
  return smoothstep(
      0.5f - -0.02f, 0.5f + -0.02f,
      (abs(uv.data[0] - p.data[0]) + abs(uv.data[1] - p.data[1])) / size);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 R = iResolution.swizzle<0, 1>();
  f32x2 uv = (2.f * fragCoord - R) / R.data[1];
  f32x3 col = f32x3(0.f);
  float piTime = iTime * 3.141592653589793f;
  float spinTime = piTime / 6.f;
  float capTime = iTime;
  float colorTime = iTime / 6.f;
  for (int i = 0; i < 4; i++) {
    float fi = float(i);
    f32x2 nuv = rotate(__ctx, uv, -piTime / 12.f);
    f32x2 suv = rotate(__ctx, uv, spinTime);
    suv = rotate(__ctx, suv, sin(fi * 3.141592653589793f / 8.f));
    suv *= 1.f + sin(piTime + fi * 3.141592653589793f / 6.f) * 0.1f;
    suv *= 1.f + fi * 0.1f;
    float sq = max(abs(suv.data[0]), abs(suv.data[1]));
    sq = smoothstep(0.5f - 0.5f, 0.5f + 0.5f, abs(sq - .15f));
    sq = cubicPulse(__ctx, 0.5f, 0.05f, sq) * 5.f;
    col += sq * spectral_zucconi6(__ctx,
                                  fract(colorTime + fi / 20.f) * 300.f + 400.f);
    suv *= 0.9f + noise(__ctx, f32x3(nuv * 10.f, iTime * 2.f)) * 0.2f;
    sq = max(abs(suv.data[0]), abs(suv.data[1]));
    sq = smoothstep(0.5f - 0.5f, 0.5f + 0.5f, abs(sq - .15f));
    sq = cubicPulse(__ctx, 0.5f, 0.05f, sq) * 5.f;
    col +=
        0.5f * sq *
        spectral_zucconi6(__ctx, fract(colorTime + fi / 20.f) * 300.f + 400.f);
    suv *= 0.9f + noise(__ctx, f32x3(nuv * 30.f, iTime * 3.f)) * 0.2f;
    sq = max(abs(suv.data[0]), abs(suv.data[1]));
    sq = smoothstep(0.5f - 0.5f, 0.5f + 0.5f, abs(sq - .15f));
    sq = cubicPulse(__ctx, 0.5f, 0.05f, sq) * 5.f;
    col +=
        0.5f * sq *
        spectral_zucconi6(__ctx, fract(colorTime + fi / 20.f) * 300.f + 400.f);
  }
  f32x2 pt = f32x2(0.f, 1.f);
  f32x2 pb = f32x2(0.f, -1.f);
  f32x2 pl = f32x2(-1.f, 0.f);
  f32x2 pr = f32x2(1.f, 0.f);
  float t3 = fract((5.f / 6.f) * capTime * 0.4f) * 3.f;
  float t4 = fract((5.f / 6.f) * capTime * 0.3f + 1.f) * 4.f;
  f32x2 p1, p2;
  if (t3 < 1.f) {
    p1 = mix(pb, pt, fract(t3));
  } else if (t3 < 2.f) {
    p1 = mix(pt, pr, fract(t3));
  } else {
    p1 = mix(pr, pb, fract(t3));
  }
  if (t4 < 1.f) {
    p2 = mix(pb, pl, fract(t4));
  } else if (t4 < 2.f) {
    p2 = mix(pl, pt, fract(t4));
  } else if (t4 < 3.f) {
    p2 = mix(pt, pr, fract(t4));
  } else {
    p2 = mix(pr, pb, fract(t4));
  }
  p1 *= 0.5f;
  p2 *= 0.5f;
  float d1 = drawSquare(__ctx, uv, p1, 0.25f);
  col += d1 * spectral_zucconi6(__ctx, fract(colorTime) * 300.f + 400.f);
  float d2 = drawSquare(__ctx, uv, p2, 0.25f);
  col += d2 * spectral_zucconi6(
                  __ctx, fract(colorTime + 3.141592653589793f) * 300.f + 400.f);
  col = filmicToneMapping(__ctx, col);
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
