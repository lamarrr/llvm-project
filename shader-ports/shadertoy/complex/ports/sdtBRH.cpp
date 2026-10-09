// Port of https://www.shadertoy.com/view/sdtBRH; author: Peace.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/sdtBRH.json.
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
f32x3 hash(Context &__ctx, f32x3 p3) {
  p3 = fract(p3 * f32x3(.1031f, .11369f, .13787f));
  p3 += dot(p3, p3.swizzle<1, 0, 0>() + 19.19f);
  return -1.0f + 2.0f * fract(f32x3((p3.data[0] + p3.data[1]) * p3.data[2],
                                    (p3.data[0] + p3.data[2]) * p3.data[1],
                                    (p3.data[1] + p3.data[2]) * p3.data[0]));
}
float voronoi(Context &__ctx, f32x2 n, float t) {
  f32x3 p = f32x3(n.data[0], n.data[1], t);
  const float K1 = 0.333333333f;
  const float K2 = 0.166666667f;
  f32x3 i = floor(p + (p.data[0] + p.data[1] + p.data[2]) * K1);
  f32x3 d0 = p - (i - (i.data[0] + i.data[1] + i.data[2]) * K2);
  f32x3 e = step(f32x3(0.0f), d0 - d0.swizzle<1, 2, 0>());
  f32x3 i1 = e * (1.0f - e.swizzle<2, 0, 1>());
  f32x3 i2 = 1.0f - e.swizzle<2, 0, 1>() * (1.0f - e);
  f32x3 d1 = d0 - (i1 - 1.0f * K2);
  f32x3 d2 = d0 - (i2 - 2.0f * K2);
  f32x3 d3 = d0 - (1.0f - 3.0f * K2);
  f32x4 h = max(
      0.6f - f32x4(dot(d0, d0), dot(d1, d1), dot(d2, d2), dot(d3, d3)), 0.0f);
  f32x4 q = h * h * h * h *
            f32x4(dot(d0, hash(__ctx, i)), dot(d1, hash(__ctx, i + i1)),
                  dot(d2, hash(__ctx, i + i2)), dot(d3, hash(__ctx, i + 1.0f)));
  return dot(f32x4(50.f), q);
}
float voronoiFbm(Context &__ctx, f32x2 p, int iter) {
  float f = 0.0f;
  float a = 1.0f;
  float norm = 0.0f;
  for (int i = 0; i < iter; i++) {
    f += a * abs(voronoi(__ctx, p, 0.0f));
    norm += a;
    p *= 2.0f;
    a *= 0.65f;
  }
  return 1.0f - f / norm;
}
float fbm(Context &__ctx, f32x2 p, int iter, float freq, float amp) {
  float n = 0.0f;
  float a = 1.0f;
  float norm = 0.0f;
  for (int i = 0; i < iter; ++i) {
    n += noise(__ctx, p) * a;
    norm += a;
    p *= freq;
    a *= amp;
  }
  return n / norm;
}
float fbm(Context &__ctx, f32x2 p, int iter) {
  return fbm(__ctx, p, iter, 2.0f, 0.5f);
}
f32x3 starColor(Context &__ctx, f32x2 p) {
  return sin(f32x3(.2f, .3f, .9f) *
             fract(rand(__ctx, p + f32x2(7.91f, 246.792f)) * 2345.2f) *
             123.2f) *
             f32x3(0.15f, 0.1f, 0.15f) +
         0.75f;
}
void stars(Context &__ctx, f32x3 &col, f32x2 fragCoord, float density) {
  f32x2 p = fragCoord;
  if (rand(__ctx, f32x2(i32x2(fragCoord / 2.0f)) * 2.0f) < 0.3f)
    fragCoord = f32x2(i32x2(fragCoord / 2.0f)) * 2.0f;
  f32x3 star = col * 0.5f + 0.5f * starColor(__ctx, p);
  if (rand(__ctx, fragCoord + f32x2(1071.52f, -662.1f)) > 0.0f)
    return;
  col = star;
}
void bigStars(Context &__ctx, f32x3 &col, f32x2 fragCoord, float density) {
  f32x2 p = f32x2(i32x2(fragCoord / 16.f)) * 16.f;
  float star_size = 16.f * pow(rand(__ctx, p), 2.0f);
  f32x3 star = col * 0.5f + 0.5f * starColor(__ctx, p);
  float d =
      max(1.0f - distance(p + star_size * 0.5f, fragCoord) / star_size * 2.0f,
          0.0f);
  star += d * d * d * d;
  if (rand(__ctx, p + f32x2(6181.616f, 9028.1f)) > 0.1f)
    return;
  col = mix(col, star, d * d * d);
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
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x3 col = f32x3(0.1f, 0.2f, 0.6f);
  float primary =
      sqrt(fbm(__ctx, uv * 3.0f + f32x2(6.175f, -23.612f), 8, 1.5f, 0.5f));
  float secondary = fbm(__ctx, uv * 4.0f, 8);
  float nebula =
      4.0f *
      pow(fbm(__ctx, uv * 4.0f + f32x2(-12.1f, 2.62f), 18, 2.0f, 0.5f), 5.0f);
  float space = primary * secondary * nebula;
  f32x3 variation =
      normalize(f32x3(fbm(__ctx, (uv + f32x2(11.61f)) * 2.0f, 8),
                      fbm(__ctx, (uv - f32x2(2.11f)) * 2.0f, 8),
                      fbm(__ctx, (uv + f32x2(66.61f)) * 2.0f, 8)));
  col *= space;
  col = mix(col, secondary * f32x3(0.49f, 0.45f, 0.655f), 0.5f);
  col = mix(variation, col, 0.93f);
  col += f32x3(0.432941f, 0.38627f, 0.69020f) *
         f32x3(pow(fbm(__ctx, uv * 12.0f, 18, 2.0f, 0.5f), 3.0f) * 0.5f);
  col += f32x3(0.7f, 0.6f, 0.5f) * nebula;
  stars(__ctx, col, fragCoord, space);
  bigStars(__ctx, col, fragCoord, space);
  col /=
      1.0f +
      f32x3(1.0f, 1.3f, 1.5f) *
          (4.0f * f32x3(pow(max(voronoiFbm(__ctx, uv * 2.0f, 8), 0.0f), 6.0f)));
  col = rgb2hsv(__ctx, col);
  col *= f32x3(1.0f, 1.0f + iMouse.data[0] / 500.0f,
               1.0f + iMouse.data[1] / 500.0f);
  col = hsv2rgb(__ctx, col);
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
