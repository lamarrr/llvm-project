// Port of https://www.shadertoy.com/view/ctjXzd; author: lanx06.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctjXzd.json.
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
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

f32x2 wavedx(Context &__ctx, f32x2 position, f32x2 direction, float speed,
             float frequency, float timeshift) {
  float x = dot(direction, position) * frequency + timeshift * speed;
  float wave = exp(sin(x) - 1.0f);
  float dx = wave * cos(x);
  return f32x2(wave, -dx);
}
float getwaves(Context &__ctx, f32x2 position, int iterations, float t) {
  float iter = 0.0f;
  float phase = 6.0f;
  float speed = 2.0f;
  float weight = 1.0f;
  float w = 0.0f;
  float ws = 0.0f;
  for (int i = 0; i < iterations; i++) {
    f32x2 p = f32x2(sin(iter), cos(iter));
    f32x2 res = wavedx(__ctx, position, p, speed, phase, t);
    position += p * res.data[1] * weight * 0.048f;
    w += res.data[0] * weight;
    iter += 12.0f;
    ws += weight;
    weight = mix(weight, 0.0f, 0.2f);
    phase *= 1.18f;
    speed *= 1.07f;
  }
  return w / ws;
}
f32x3 rgb2hsv(Context &__ctx, f32x3 c) {
  f32x4 K = f32x4(0.0f, -1.0f / 3.0f, 2.0f / 3.0f, -1.0f);
  f32x4 p = mix(f32x4(c.swizzle<2, 1>(), K.swizzle<3, 2>()),
                f32x4(c.swizzle<1, 2>(), K.swizzle<0, 1>()),
                step(c.data[2], c.data[1]));
  f32x4 q =
      mix(f32x4(p.swizzle<0, 1, 3>(), c.data[0]),
          f32x4(c.data[0], p.swizzle<1, 2, 0>()), step(p.data[0], c.data[0]));
  float d = q.data[0] - min(q.data[3], q.data[1]);
  float e = 1.0e-10f;
  return f32x3(abs(q.data[2] + (q.data[3] - q.data[1]) / (6.0f * d + e)),
               d / (q.data[0] + e), q.data[0]);
}
f32x3 hsv2rgb(Context &__ctx, f32x3 c) {
  f32x4 K = f32x4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
  f32x3 p = abs(fract(c.swizzle<0, 0, 0>() + K.swizzle<0, 1, 2>()) * 6.0f -
                K.swizzle<3, 3, 3>());
  return c.data[2] * mix(K.swizzle<0, 0, 0>(),
                         clamp(p - K.swizzle<0, 0, 0>(), 0.0f, 1.0f),
                         c.data[1]);
}
float line_segment(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 ba = b - a;
  f32x2 pa = p - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.f, 1.f);
  return length(pa - h * ba);
}
f32x3 saturate(Context &__ctx, f32x3 x) {
  return min(f32x3(1.f, 1.f, 1.f), max(f32x3(0.f, 0.f, 0.f), x));
}
f32x3 spectral_bruton(Context &__ctx, float w) {
  w = mix(400.0f, 800.0f, w);
  f32x3 c;
  if (w >= 380.f && w < 440.f)
    c = f32x3(-(w - 440.f) / (440.f - 380.f), 0.0f, 1.0f);
  else if (w >= 440.f && w < 490.f)
    c = f32x3(0.0f, (w - 440.f) / (490.f - 440.f), 1.0f);
  else if (w >= 490.f && w < 510.f)
    c = f32x3(0.0f, 1.0f, -(w - 510.f) / (510.f - 490.f));
  else if (w >= 510.f && w < 580.f)
    c = f32x3((w - 510.f) / (580.f - 510.f), 1.0f, 0.0f);
  else if (w >= 580.f && w < 645.f)
    c = f32x3(1.0f, -(w - 645.f) / (645.f - 580.f), 0.0f);
  else if (w >= 645.f && w <= 780.f)
    c = f32x3(1.0f, 0.0f, 0.0f);
  else
    c = f32x3(0.0f, 0.0f, 0.0f);
  return saturate(__ctx, c);
}
f32x3 spectral_jet(Context &__ctx, float w) {
  float x = w;
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
f32x2 stdNormalMap(Context &__ctx, f32x2 uv) {
  float h = texture(__ctx.channel0, uv).data[0];
  return -f32x2(ddx(h), ddy(h));
}
f32x3 normalPlane(Context &__ctx, f32x2 uv) {
  f32x3 normalMap;
  const int textureOffset = 1;
  uv *= 0.5f +
        length(iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>()) * 1.5f;
  float p = getwaves(__ctx, uv, 20, iTime * 0.2f);
  float h1 = getwaves(
      __ctx, uv + f32x2(textureOffset, 0) / iResolution.swizzle<0, 1>(), 20,
      iTime * 0.2f);
  float v1 = getwaves(
      __ctx, uv + f32x2(0, textureOffset) / iResolution.swizzle<0, 1>(), 20,
      iTime * 0.2f);
  normalMap.set_swizzle<0, 1>((p - f32x2(h1, v1)) * 100.0f);
  normalMap += 0.5f;
  normalMap.data[2] = 1.0f;
  f32x3 normal;
  f32x3x3 TBN = f32x3x3(f32x3(1, 0, 0), f32x3(0, 1, 0), f32x3(0, 0, 1));
  normal = normalMap;
  return normal;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 lightdir = f32x2(0.5f, 0.5f);
  float row = iResolution.data[0] / float(int(iResolution.data[1] / 1.5f));
  f32x3 col;
  f32x2 block = f32x2(floor(fragCoord / f32x2(row)));
  if (uv.data[0] < 0.5f && uv.data[1] > 0.5f)
    col = normalPlane(__ctx, uv);
  if (uv.data[0] < 0.5f)
    col = normalPlane(__ctx, uv);
  else
    for (int i = 0; i < 7 * 7; i++) {
      f32x2 id = f32x2(i % 7, i / 7) + f32x2(-7 / 2);
      f32x2 simaplepos = (id + f32x2(0.5f)) * row + fragCoord;
      ;
      float pix = 0.5f;
      float radius = 0.5f;
      f32x3 normal =
          normalPlane(__ctx, simaplepos / iResolution.swizzle<0, 1>());
      f32x2 offset = refract(f32x3(lightdir, -0.707f), normal, 1.0f / 1.3f)
                         .swizzle<0, 1>();
      float len = row * float(7) * 0.7f;
      f32x2 midpos = simaplepos + offset * len;
      float l = length((fragCoord - midpos) / row);
      f32x2 lo = normalize(offset) * len * 0.2f;
      float ldis =
          line_segment(__ctx, fragCoord, midpos - lo, midpos + lo) / row * 1.0f;
      float tem = dot((fragCoord - midpos), normalize(offset)) /
                  pow(length(lo) + (pix + radius) * row, 1.0f);
      f32x3 color = spectral_jet(__ctx, (tem + 1.0f) * .5f);
      color = rgb2hsv(__ctx, color);
      color.data[1] = pow(l, 2.0f);
      color = hsv2rgb(__ctx, color);
      color.data[1] *= 0.7f;
      col += (smoothstep(radius + pix, radius - pix, ldis)) / float(7) * 1.0f *
             color;
    }
  fragColor = f32x4(col, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
  Context __ctx{{channel0, sampling0}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
