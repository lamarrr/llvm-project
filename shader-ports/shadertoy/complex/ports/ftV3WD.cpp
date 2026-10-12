// Port of https://www.shadertoy.com/view/ftV3WD; author: Zi7ar21.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ftV3WD.json.
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
const float pi = 3.141592653589793f;
f32x4 hash44(Context &__ctx, f32x4 p4) {
  p4 *= f32x4(0.1031f, 0.1030f, 0.0973f, 0.1099f);
  p4 = fract(p4);
  p4 += dot(p4, p4.swizzle<3, 2, 0, 1>() + 33.33f);
  p4 = (p4.swizzle<0, 0, 1, 2>() + p4.swizzle<1, 2, 2, 3>()) *
       p4.swizzle<2, 1, 3, 0>();
  p4 = fract(p4);
  return p4;
}
float voronoi(Context &__ctx, f32x3 p, f32x3 &c) {
  float d = 10.0f;
  for (int pos_x = -1; pos_x <= 1; pos_x++) {
    for (int pos_y = -1; pos_y <= 1; pos_y++) {
      for (int pos_z = -1; pos_z <= 1; pos_z++) {
        f32x4 h =
            hash44(__ctx, f32x4(f32x3(pos_x, pos_y, pos_z) + floor(p), 0.0f));
        f32x3 point = f32x3(pos_x, pos_y, pos_z) + h.swizzle<0, 1, 2>();
        float t = dot(point - fract(p), point - fract(p));
        if (t < d) {
          c = hash44(__ctx, h.swizzle<0, 1, 2, 3>()).swizzle<0, 1, 2>();
        }
        d = min(t, d);
      }
    }
  }
  return d;
}
float gradient_noise(Context &__ctx, f32x3 p, float t) {
  f32x3 f = smoothstep(0.0f, 1.0f, fract(p));
  f32x3 i = floor(p);
  float h0 = hash44(__ctx, f32x4(i + f32x3(0.0f, 0.0f, 0.0f), t)).data[0];
  float h1 = hash44(__ctx, f32x4(i + f32x3(1.0f, 0.0f, 0.0f), t)).data[0];
  float h2 = hash44(__ctx, f32x4(i + f32x3(0.0f, 1.0f, 0.0f), t)).data[0];
  float h3 = hash44(__ctx, f32x4(i + f32x3(1.0f, 1.0f, 0.0f), t)).data[0];
  float h4 = hash44(__ctx, f32x4(i + f32x3(0.0f, 0.0f, 1.0f), t)).data[0];
  float h5 = hash44(__ctx, f32x4(i + f32x3(1.0f, 0.0f, 1.0f), t)).data[0];
  float h6 = hash44(__ctx, f32x4(i + f32x3(0.0f, 1.0f, 1.0f), t)).data[0];
  float h7 = hash44(__ctx, f32x4(i + f32x3(1.0f, 1.0f, 1.0f), t)).data[0];
  return mix(mix(mix(h0, h1, f.data[0]), mix(h2, h3, f.data[0]), f.data[1]),
             mix(mix(h4, h5, f.data[0]), mix(h6, h7, f.data[0]), f.data[1]),
             f.data[2]);
}
float fbm(Context &__ctx, f32x3 p) {
  float value = 0.0f;
  float accum = 0.0f;
  float atten = 0.5f;
  float scale = 1.0f;
  for (int i = 0; i < 8; i++) {
    value += atten * gradient_noise(__ctx, scale * p, float(i));
    accum += atten;
    atten *= 0.5f;
    scale *= 2.0f;
  }
  return value / accum;
}
f32x3 blackbody(Context &__ctx, float t) {
  float h = 6.6e-34f;
  float k = 1.4e-23f;
  float c = 3e8f;
  f32x3 w = f32x3(610.0f, 549.0f, 468.0f) / 1e9f;
  f32x3 w5 = w * w * w * w * w;
  f32x3 o = 2.0f * h * (c * c) / (w5 * (exp(h * c / (w * k * t)) - 1.0f));
  return o;
}
f32x3 sky_color(Context &__ctx, f32x3 dir) {
  const float t = 0.05f;
  f32x3 c;
  float v = (1.0f / t) * clamp(t - voronoi(__ctx, 80.0f * dir, c), 0.0f, 1.0f);
  f32x3 b =
      0.4f *
          clamp(1.0f - exp(-max(blackbody(__ctx, (300.0f * c.data[0]) + 600.0f),
                                0.0f) *
                           1.0f),
                0.0f, 1.0f) +
      0.6f;
  float n0 = fbm(__ctx, 3.0f * dir);
  float n1 = fbm(__ctx, 4.0f * (dir + 1.0f));
  float d0 = 3.0f * (max(n0 - 0.4f, 0.0f) + 0.07f);
  float d1 = smoothstep(0.0f, 1.0f,
                        max(n1 - (4.0f * dir.data[1] * dir.data[1]), 0.0f));
  return (b * v * d0 * (1.0f - d1)) + d1;
}
f32x2 rotate(Context &__ctx, f32x2 vec, float rot) {
  float s = sin(rot), c = cos(rot);
  return f32x2(vec.data[0] * c - vec.data[1] * s,
               vec.data[0] * s + vec.data[1] * c);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  fragColor = f32x4(0.0f, 0.0f, 0.0f, 1.0f);
  f32x2 uv = 2.0f * (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]);
  f32x3 rd = normalize(
      f32x3(rotate(__ctx, f32x2(1.0f * uv.data[0], -1.0f), 0.03f * pi * iTime),
            1.0f * uv.data[1])
          .swizzle<0, 2, 1>());
  fragColor.set_swizzle<0, 1, 2>(sky_color(__ctx, rd));
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
