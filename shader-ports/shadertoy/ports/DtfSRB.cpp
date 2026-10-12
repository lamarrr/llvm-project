// Port of https://www.shadertoy.com/view/DtfSRB; author: afterlifevj.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtfSRB.json.
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
struct Context {
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

f32x2 random2(Context &__ctx, f32x2 st) {
  st = f32x2(dot(st, f32x2(127.1f, 311.7f)), dot(st, f32x2(269.5f, 183.3f)));
  return -1.0f + 2.0f * fract(sin(st) * 43758.5453123f);
}
f32x3 random3(Context &__ctx, f32x3 st) {
  st = f32x3(dot(st, f32x3(17.1f, 61.7f, 19.73f)),
             dot(st, f32x3(69.43f, 33.3f, 93.8f)),
             dot(st, f32x3(74.31f, 36.1f, 83.9f)));
  return -1.0f + 2.0f * fract(sin(st) * 437581.5453123f);
}
float noise(Context &__ctx, f32x2 st, float travel) {
  st += f32x2(0.f, 0.f);
  f32x3 i = floor(f32x3(st, travel));
  f32x3 f = fract(f32x3(st, travel));
  f32x3 u = smoothstep(0.f, 1.f, f);
  float base00 = dot(random3(__ctx, i + f32x3(0.0f, 0.0f, 0.0f)),
                     f - f32x3(0.0f, 0.0f, 0.0f));
  float base10 = dot(random3(__ctx, i + f32x3(1.0f, 0.0f, 0.0f)),
                     f - f32x3(1.0f, 0.0f, 0.0f));
  float base01 = dot(random3(__ctx, i + f32x3(0.0f, 1.0f, 0.0f)),
                     f - f32x3(0.0f, 1.0f, 0.0f));
  float base11 = dot(random3(__ctx, i + f32x3(1.0f, 1.0f, 0.0f)),
                     f - f32x3(1.0f, 1.0f, 0.0f));
  float top00 = dot(random3(__ctx, i + f32x3(0.0f, 0.0f, 1.0f)),
                    f - f32x3(0.0f, 0.0f, 1.0f));
  float top10 = dot(random3(__ctx, i + f32x3(1.0f, 0.0f, 1.0f)),
                    f - f32x3(1.0f, 0.0f, 1.0f));
  float top01 = dot(random3(__ctx, i + f32x3(0.0f, 1.0f, 1.0f)),
                    f - f32x3(0.0f, 1.0f, 1.0f));
  float top11 = dot(random3(__ctx, i + f32x3(1.0f, 1.0f, 1.0f)),
                    f - f32x3(1.0f, 1.0f, 1.0f));
  float base = mix(mix(base00, base10, u.data[0]),
                   mix(base01, base11, u.data[0]), u.data[1]);
  float top = mix(mix(top00, top10, u.data[0]), mix(top01, top11, u.data[0]),
                  u.data[1]);
  return mix(base, top, u.data[2]);
}
float bumpFunc(Context &__ctx, f32x2 st) {
  f32x2 aspect = f32x2(iResolution.data[0] / iResolution.data[1], 1.f);
  float center = smoothstep(1.f, 0.f, length(st));
  f32x2 noisedist =
      f32x2(noise(__ctx, (st + 0.3f) * 2.f, 10.f),
            noise(__ctx, (st + f32x2(13.1238f, 11.8837f)) * 2.f, iTime * 0.1f));
  noisedist += f32x2(noise(__ctx, (st) * 8.f, 3.f),
                     noise(__ctx, (st + f32x2(13.1238f, 11.8837f)) * 8.f,
                           iTime * 0.1f + 28.f)) *
               0.1f;
  float noisevalue =
      noise(__ctx, (st + noisedist * 0.07f + f32x2(0.f, iTime * 0.05f) * 5.f),
            iTime * 2.f);
  noisevalue +=
      noise(__ctx, (st + noisedist * 0.35f + f32x2(0.f, iTime * 0.05f)) * 25.f,
            iTime * 0.5f) *
      0.25f;
  noisevalue += noise(__ctx, st * 5.5f, iTime * 0.15f) * 0.15f;
  return center * 0.f + smoothstep(-1.25f, 1.25f, noisevalue);
}
f32x3 bumpMap(Context &__ctx, f32x3 st) {
  f32x3 sp = st;
  f32x2 eps = f32x2(4.f / iResolution.data[1], 0.f);
  float f = bumpFunc(__ctx, sp.swizzle<0, 1>());
  float fx = bumpFunc(__ctx, sp.swizzle<0, 1>() - eps.swizzle<0, 1>());
  float fy = bumpFunc(__ctx, sp.swizzle<0, 1>() - eps.swizzle<1, 0>());
  const float bumpFactor = 0.1f;
  fx = (fx - f) / eps.data[0];
  fy = (fy - f) / eps.data[0];
  return f32x3(fx, fy, 0.f) * bumpFactor;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - iResolution.swizzle<0, 1>() * .5f) / iResolution.data[1];
  f32x3 sp = f32x3(uv, 0);
  f32x3 rd = normalize(f32x3(uv, 1.f));
  f32x3 lp = f32x3(iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>() * 2.f -
                       f32x2(1.f),
                   -0.85f);
  f32x3 sn = f32x3(0.f, 0.f, -1);
  sn = normalize(sn + bumpMap(__ctx, sp));
  f32x3 ld = lp - sp;
  float lDist = max(length(ld), 0.001f);
  ld /= lDist;
  float atten = 1.f / (1.0f + lDist * lDist * 0.05f);
  float diff = max(dot(sn, ld), 0.f);
  float spec = pow(max(dot(reflect(-ld, sn), -rd), 0.f), 31.f);
  f32x3 ref = reflect(
      f32x3(sp.swizzle<0, 1>(), bumpFunc(__ctx, sp.swizzle<0, 1>()) * 0.8f),
      sn);
  float m = 2.f * sqrt(pow(ref.data[0], 2.f) + pow(ref.data[1], 2.f) +
                       pow(ref.data[2] + 1.f, 2.f));
  f32x2 tcx = ref.swizzle<0, 1>() / m + .5f;
  f32x3 env_color = texture(__ctx.channel0, tcx).swizzle<0, 1, 2>();
  float grain = noise(__ctx, uv.swizzle<0, 1>() * 10.f, 0.1f);
  f32x3 color =
      diff * 0.35f * f32x3(0.9450980392156862f, 1.0f, 0.3215686274509804f) +
      f32x3(0.8666666666666667f, 0.4549019607843137f, 0.9705882352941176f) *
          spec * 1.75f +
      pow(env_color, f32x3(2.9f)) * 0.65f + f32x3(grain * 0.1f);
  fragColor = f32x4(color, 1.f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
               __binding(1, 0) sampler sampling0) {
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
