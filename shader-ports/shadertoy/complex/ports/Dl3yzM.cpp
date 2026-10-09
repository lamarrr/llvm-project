// Port of https://www.shadertoy.com/view/Dl3yzM; author: Zi7ar21.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Dl3yzM.json.
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
  Texture2D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

float fBm(Context &__ctx, f32x2 x) {
  return 0.5333333333333333f *
             textureLod(__ctx.channel0, 0.00390625f * x, 0.0f).data[0] +
         0.2666666666666667f *
             textureLod(__ctx.channel0, 0.00781250f * x, 0.0f).data[1] +
         0.1333333333333333f *
             textureLod(__ctx.channel0, 0.01562500f * x, 0.0f).data[2] +
         0.0666666666666667f *
             textureLod(__ctx.channel0, 0.03125000f * x, 0.0f).data[3];
}
float fBm_MipMap(Context &__ctx, f32x2 x) {
  return 0.5333333333333333f *
             texture(__ctx.channel0, 0.00390625f * x).data[0] +
         0.2666666666666667f *
             texture(__ctx.channel0, 0.00781250f * x).data[1] +
         0.1333333333333333f *
             texture(__ctx.channel0, 0.01562500f * x).data[2] +
         0.0666666666666667f * texture(__ctx.channel0, 0.03125000f * x).data[3];
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  fragColor = f32x4(0.0f, 0.0f, 0.0f, 1.0f);
  f32x2 uv = (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) /
             (0.5f * iResolution.data[1]);
  f32x3 ro = f32x3(0.0f, -1.0f, 0.0f);
  f32x3 rd = f32x3(uv.data[0], uv.data[1], -1.0f) / sqrt(dot(uv, uv) + 1.0f);
  ro.data[2] = -5.0f * iTime;
  const float step_size = 0.5f * 0.3333333333333333f;
  f32x3 transmittance = f32x3(1);
  float dithering_offset =
      texture(__ctx.channel1, 0.0009765625f * fragCoord).data[0];
  for (int i = 0; i < 64; i++) {
    if (transmittance.data[0] < 0.00390625f)
      break;
    float t = step_size * (float(i) + dithering_offset);
    f32x3 x = ro + t * rd;
    if (x.data[1] < -0.5f || x.data[1] > 0.5f)
      continue;
    float hmax = fBm(__ctx, x.swizzle<0, 2>()) - 0.4f;
    float density = max(hmax - abs(x.data[1]), 0.0f);
    float illumin = 2.0f * exp(-5.0f * max(hmax - max(x.data[1], -hmax), 0.f));
    density *= 50.0f;
    transmittance *= exp(-step_size * density);
    fragColor.set_swizzle<0, 1, 2>(
        fragColor.swizzle<0, 1, 2>() +
        (step_size * transmittance * density * illumin));
  }
  f32x4 p = f32x4(0.0f, 1.0f, 0.0f, 1.0f);
  p = normalize(p);
  float ground_t =
      -(dot(ro + f32x3(0.0f, 3.0f, 0.0f), p.swizzle<0, 1, 2>()) - p.data[1]) /
      dot(rd, p.swizzle<0, 1, 2>());
  f32x3 gx = ro + ground_t * rd;
  float hmax = fBm_MipMap(__ctx, gx.swizzle<0, 2>()) - 0.4f;
  float ill = 0.2f * exp(-2.0f * max(hmax - max(gx.data[1], -hmax), 0.f));
  fragColor.set_swizzle<0, 1, 2>(
      fragColor.swizzle<0, 1, 2>() +
      (ground_t > 0.0f ? transmittance * f32x3(0.8f) * ill
                       : transmittance * (0.9f * max(rd.data[1], 0.f) + 0.1f) *
                             f32x3(0.6f, 0.7f, 0.8f)));
  fragColor.set_swizzle<0, 1, 2>(fragColor.swizzle<0, 1, 2>() * (2.0f));
  fragColor.set_swizzle<0, 1, 2>(fragColor.swizzle<0, 1, 2>() /
                                 (fragColor.swizzle<0, 1, 2>() + 1.0f));
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0,
               __binding__(0, 1) texture_2d<f32x4> channel1,
               __binding__(1, 1) sampler sampling1) {
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
  Context __ctx{{channel0, sampling0}, {channel1, sampling1}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
