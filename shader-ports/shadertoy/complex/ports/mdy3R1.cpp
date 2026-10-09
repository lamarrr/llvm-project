// Port of https://www.shadertoy.com/view/mdy3R1; author: dean_the_coder.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/mdy3R1.json.
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
float sum2(Context &__ctx, f32x2 v) { return dot(v, f32x2(1)); }
float h31(Context &__ctx, f32x3 p3) {
  p3 = fract(p3 * .1031f);
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 333.3456f);
  return fract(sum2(__ctx, p3.swizzle<0, 1>()) * p3.data[2]);
}
float h21(Context &__ctx, f32x2 p) { return h31(__ctx, p.swizzle<0, 1, 0>()); }
float n31(Context &__ctx, f32x3 p) {
  const f32x3 s = f32x3(7, 157, 113);
  f32x3 ip = floor(p);
  p = fract(p);
  p = p * p * (3.f - 2.f * p);
  f32x4 h =
      f32x4(0, s.swizzle<1, 2>(), sum2(__ctx, s.swizzle<1, 2>())) + dot(ip, s);
  h = mix(fract(sin(h) * 43758.545f), fract(sin(h + s.data[0]) * 43758.545f),
          p.data[0]);
  h.set_swizzle<0, 1>(mix(h.swizzle<0, 2>(), h.swizzle<1, 3>(), p.data[1]));
  return mix(h.data[0], h.data[1], p.data[2]);
}
float fbm(Context &__ctx, f32x3 p, int octaves, float roughness) {
  float sum = 0.f, amp = 1.f, tot = 0.f;
  roughness = clamp(roughness, 0.f, 1.f);
  for (int i = 0; i < octaves; i++) {
    sum += amp * n31(__ctx, p);
    tot += amp;
    amp *= roughness;
    p *= 2.f;
  }
  return sum / tot;
}
f32x3 randomPos(Context &__ctx, float seed) {
  f32x4 s = f32x4(seed, 0, 1, 2);
  return f32x3(h21(__ctx, s.swizzle<0, 1>()), h21(__ctx, s.swizzle<0, 2>()),
               h21(__ctx, s.swizzle<0, 3>())) *
             1e2f +
         1e2f;
}
float fbmDistorted(Context &__ctx, f32x3 p) {
  p += (f32x3(n31(__ctx, p + randomPos(__ctx, 0.f)),
              n31(__ctx, p + randomPos(__ctx, 1.f)),
              n31(__ctx, p + randomPos(__ctx, 2.f))) *
            2.f -
        1.f) *
       1.12f;
  return fbm(__ctx, p, 8, .5f);
}
float musgraveFbm(Context &__ctx, f32x3 p, float octaves, float dimension,
                  float lacunarity) {
  float sum = 0.f, amp = 1.f, m = pow(lacunarity, -dimension);
  for (float i = 0.f; i < octaves; i++) {
    float n = n31(__ctx, p) * 2.f - 1.f;
    sum += n * amp;
    amp *= m;
    p *= lacunarity;
  }
  return sum;
}
f32x3 waveFbmX(Context &__ctx, f32x3 p) {
  float n = p.data[0] * 20.f;
  n += .4f * fbm(__ctx, p * 3.f, 3, 3.f);
  return f32x3(sin(n) * .5f + .5f, p.swizzle<1, 2>());
}
float remap01(Context &__ctx, float f, float in1, float in2) {
  return clamp((f - in1) / (in2 - in1), 0.f, 1.f);
}
f32x3 matWood(Context &__ctx, f32x3 p) {
  float n1 = fbmDistorted(__ctx, p * f32x3(7.8f, 1.17f, 1.17f));
  n1 = mix(n1, 1.f, .2f);
  float n2 =
            mix(musgraveFbm(__ctx, f32x3(n1 * 4.6f), 8.f, 0.f, 2.5f), n1, .85f),
        dirt = 1.f - musgraveFbm(__ctx,
                                 waveFbmX(__ctx, p * f32x3(.01f, .15f, .15f)),
                                 15.f, .26f, 2.4f) *
                         .4f;
  float grain = 1.f - smoothstep(.2f, 1.f,
                                 musgraveFbm(__ctx, p * f32x3(500, 6, 1), 2.f,
                                             2.f, 2.5f)) *
                          .2f;
  n2 *= dirt * grain;
  return mix(mix(f32x3(.03f, .012f, .003f), f32x3(.25f, .11f, .04f),
                 remap01(__ctx, n2, .19f, .56f)),
             f32x3(.52f, .32f, .19f), remap01(__ctx, n2, .56f, 1.f));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fc) {
  f32x3 p =
      f32x3((fc - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1],
            floor(mod(iTime, 8.0f)));
  fragColor = f32x4(pow(matWood(__ctx, p), f32x3(.4545f)), 0);
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
