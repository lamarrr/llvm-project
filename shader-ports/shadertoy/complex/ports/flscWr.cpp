// Port of https://www.shadertoy.com/view/flscWr; author: peabrainiac.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/flscWr.json.
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
const f32x2 a = f32x2(-1.0f, 0.0f);
const f32x2 scale = f32x2(0.5f, 0.0f);
const int n = 2;
const float brightnessAdjustment = 1.9f;
f32x2 cmul(Context &__ctx, f32x2 a, f32x2 b) {
  return f32x2(dot(a, f32x2(1.0f, -1.0f) * b), dot(a, b.swizzle<1, 0>()));
}
f32x2 cdiv(Context &__ctx, f32x2 a, f32x2 b) {
  return f32x2(dot(a, b), dot(a.swizzle<1, 0>(), f32x2(1.0f, -1.0f) * b)) /
         dot(b, b);
}
f32x2 cpow(Context &__ctx, f32x2 a, float b) {
  float arg = b * atan(a.data[1], a.data[0]);
  float r = pow(dot(a, a), 0.5f * b);
  return r * f32x2(cos(arg), sin(arg));
}
const int ITER = 250;
const float sampleGridSize = 0.5f;
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 adz = f32x2(1.0f, 0.0f);
  f32x2 adc = f32x2(0.0f, 0.0f);
  f32x2 z = a;
  for (int i = 1; i < n; i++) {
    adz = 2.0f * cmul(__ctx, adz, z);
    adc = 2.0f * cmul(__ctx, adc, z) + f32x2(1.0f, 0.0f);
    z = cmul(__ctx, z, z) + a;
  }
  f32x2 C = cmul(__ctx, a, cdiv(__ctx, scale, f32x2(1.0f, 0.0f) - scale));
  float time = mod(0.25f * iTime, 1.0f);
  float t = (0.5f + 0.5f * cos(time * 3.14159265358f));
  fragColor = f32x4(f32x3(0.0f), 1.0f);
  for (float offsetX = -0.5f + 0.5f * sampleGridSize; offsetX < 0.5f;
       offsetX += sampleGridSize) {
    for (float offsetY = -0.5f + 0.5f * sampleGridSize; offsetY < 0.5f;
         offsetY += sampleGridSize) {
      f32x2 c = -0.35f * a + 5.0f *
                                 (fragCoord + f32x2(offsetX, offsetY) -
                                  iResolution.swizzle<0, 1>() * 0.5f) /
                                 length(iResolution.swizzle<0, 1>());
      c = cmul(__ctx, (c - C), cpow(__ctx, scale, time)) + C;
      z = f32x2(0.0f, 0.0f);
      f32x2 dz = f32x2(1.0f, 0.0f);
      int i;
      for (i = 0; i < ITER && dot(z, z) < 100.0f; i++) {
        dz = i == 0 ? dz : 2.0f * cmul(__ctx, z, dz);
        z = cmul(__ctx, z, z) + c;
        f32x2 z2 = z + a;
        f32x2 dz2 = dz;
        for (int i2 = 1; i2 < n && dot(z2, z2) < 1e13f; i2++) {
          dz2 = 2.0f * cmul(__ctx, z2, dz2);
          z2 = cmul(__ctx, z2, z2) + c + a;
        }
        z = mix(cmul(__ctx, adz, z) + cmul(__ctx, adc, c), z2, t);
        dz = mix(cmul(__ctx, adz, dz), dz2, t);
      }
      float d = (i >= ITER)
                    ? 1.0f
                    : pow(dot(adz, adz), t) * sqrt(dot(z, z) / dot(dz, dz)) *
                          0.5f *
                          (log(dot(z, z)) + (1.0f - t) * log(dot(adz, adz)));
      d *= pow(dot(scale, scale), 0.5f - brightnessAdjustment * time);
      fragColor.set_swizzle<0, 1, 2>(
          fragColor.swizzle<0, 1, 2>() +
          (f32x3(max(0.0f,
                     1.0f - 0.15f * length(iResolution.swizzle<0, 1>()) * d)) *
           (sampleGridSize * sampleGridSize)));
    }
  }
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
