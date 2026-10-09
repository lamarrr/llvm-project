// Port of https://www.shadertoy.com/view/sd3yW2; author: vipiao.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/sd3yW2.json.
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
float hash13(Context &__ctx, f32x3 p3) {
  p3 = fract(p3 * .1031f);
  p3 += dot(p3, p3.swizzle<2, 1, 0>() + 31.32f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
float boxNoise(Context &__ctx, f32x3 pos) {
  f32x3 ind = f32x3(floor(pos));
  f32x3 f = fract(pos);
  f32x3 u = f * f * (3.0f - 2.0f * f);
  return mix(mix(mix(hash13(__ctx, ind + f32x3(0, 0, 0)),
                     hash13(__ctx, ind + f32x3(1, 0, 0)), u.data[0]),
                 mix(hash13(__ctx, ind + f32x3(0, 1, 0)),
                     hash13(__ctx, ind + f32x3(1, 1, 0)), u.data[0]),
                 u.data[1]),
             mix(mix(hash13(__ctx, ind + f32x3(0, 0, 1)),
                     hash13(__ctx, ind + f32x3(1, 0, 1)), u.data[0]),
                 mix(hash13(__ctx, ind + f32x3(0, 1, 1)),
                     hash13(__ctx, ind + f32x3(1, 1, 1)), u.data[0]),
                 u.data[1]),
             u.data[2]);
}
f32x2 whirl(Context &__ctx, f32x2 pos, f32x2 center, float ff, float seed) {
  f32x2 relPos = pos - center;
  float ll = length(relPos);
  float mm = 1.f - 2.f * boxNoise(__ctx, f32x3(center, ll * ff + seed));
  f32x2 dir = f32x2(-relPos.data[1], relPos.data[0]) / ll;
  return dir * mm * ll / (1.f + abs(ll));
}
f32x2 whirldNoise(Context &__ctx, f32x2 pos, float seed) {
  f32x2 botL = floor(pos);
  f32x2 ee = f32x2(1, 0);
  float ff = 2.f;
  return whirl(__ctx, pos, botL, ff, seed) *
             max(1.f - length(pos - botL), 0.f) * 1.f +
         whirl(__ctx, pos, botL + ee.swizzle<0, 1>(), ff, seed) *
             max(1.f - length(pos - botL - ee.swizzle<0, 1>()), 0.f) * 1.f +
         whirl(__ctx, pos, botL + ee.swizzle<1, 0>(), ff, seed) *
             max(1.f - length(pos - botL - ee.swizzle<1, 0>()), 0.f) * 1.f +
         whirl(__ctx, pos, botL + ee.swizzle<0, 0>(), ff, seed) *
             max(1.f - length(pos - botL - ee.swizzle<0, 0>()), 0.f) * 1.f;
}
f32x2 layeredWhirlNoise(Context &__ctx, f32x2 pos, int nn, float seed) {
  f32x2 rr = f32x2(0.f);
  float ff = 1.f;
  for (int ii = 0; ii < nn; ii++) {
    rr += whirldNoise(__ctx, pos / ff, seed) * ff;
    ff *= 0.67f;
    pos += f32x2(1, -1);
  }
  return rr;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[0];
  float aspectRatio = iResolution.data[0] / iResolution.data[1];
  f32x2 camPos = (f32x2(iMouse.swizzle<0, 1>()) -
                  f32x2(iResolution.swizzle<0, 1>()) * 0.5f) *
                 0.05f;
  float zoom = 6.f;
  f32x2 pos = camPos.swizzle<0, 1>() + uv * zoom * 2.f;
  f32x2 ww = 1.0f * 4.0f *
             layeredWhirlNoise(__ctx, pos * 0.1f, 5, iTime * 0.2f * 1.0f);
  float ff = 2.f * 1.0f;
  float t0 = mod(ff * iTime, 1.f);
  float t1 = mod(ff * iTime + 0.5f, 1.f);
  f32x3 n0 =
      f32x3(boxNoise(__ctx, f32x3((pos - t0 * ww) * 40.f, 0.f + 0.5f * iTime)),
            boxNoise(__ctx, f32x3((pos - t0 * ww) * 40.f, 1.f + 0.5f * iTime)),
            boxNoise(__ctx, f32x3((pos - t0 * ww) * 40.f, 2.f + 0.5f * iTime)));
  f32x3 n1 =
      f32x3(boxNoise(__ctx, f32x3((pos - t1 * ww) * 40.f, 3.f + 0.5f * iTime)),
            boxNoise(__ctx, f32x3((pos - t1 * ww) * 40.f, 4.f + 0.5f * iTime)),
            boxNoise(__ctx, f32x3((pos - t1 * ww) * 40.f, 5.f + 0.5f * iTime)));
  float mm = -abs(t1 * 2.f - 1.f) + 1.f;
  f32x3 color = mix(n0, n1, mm);
  float var = abs(round(t0 * 2.f - 0.5f) - t0 * 2.f + 0.5f) * (2.f - sqrt(2.f));
  color = mix(color, f32x3(0.5f), var);
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
