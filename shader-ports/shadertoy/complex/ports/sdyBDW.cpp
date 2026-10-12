// Port of https://www.shadertoy.com/view/sdyBDW; author: NEAR_DREAM_UNITY.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/sdyBDW.json.
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
float hash(Context &__ctx, u32x2 x) {
  u32x2 q = 1103515245U * ((x >> 1U) ^ (x.swizzle<1, 0>()));
  unsigned n = 1103515245U * ((q.data[0]) ^ (q.data[1] >> 3U));
  return float(n) * (1.0f / float(0xffffffffU));
}
float AA_Recip = 1.f / 6.f;
float AA_Sq = 6.f * 6.f;
float Building(Context &__ctx, i32x3 id, f32x2 bq) {
  f32x2 pos = f32x2(
      hash(__ctx, u32x2(1 * 1000 + id.data[0], id.data[2] * 1000 + id.data[1])),
      hash(__ctx,
           u32x2(2 * 1000 + id.data[0], id.data[2] * 1000 + id.data[1])));
  f32x2 size = f32x2(
      (0.75f + 0.25f * sin(6.1414f *
                           (hash(__ctx, u32x2(3 * 1000 + id.data[0],
                                              id.data[2] * 1000 + id.data[1])) +
                            2.f * iTime))),
      (0.75f + 0.25f * sin(6.1414f *
                           (hash(__ctx, u32x2(4 * 1000 + id.data[0],
                                              id.data[2] * 1000 + id.data[1])) +
                            2.f * iTime))));
  float height =
      0.25f + 0.75f * hash(__ctx, u32x2(5 * 1000 + id.data[0],
                                        id.data[2] * 1000 + id.data[1]));
  bool hit = (pos.data[0] <= bq.data[0] && pos.data[1] <= bq.data[1] &&
              bq.data[0] <= pos.data[0] + size.data[0] &&
              bq.data[1] <= pos.data[1] + size.data[1]);
  return hit ? height : 0.f;
}
float AllBuildings(Context &__ctx, f32x2 q) {
  i32x2 q_ipart = i32x2(floor(q));
  f32x2 q_fpart = q - f32x2(q_ipart);
  float v = 0.f;
  for (int xi = -1; xi <= 0; ++xi)
    for (int yi = -1; yi <= 0; ++yi) {
      i32x2 quad_ipart = q_ipart + i32x2(xi, yi);
      f32x2 quad_fpart = q - f32x2(quad_ipart);
      for (int bi = 0; bi < 2; ++bi)
        v = max(v, Building(__ctx, i32x3(quad_ipart, bi), quad_fpart));
    }
  return v;
}
f32x3 HotColour(Context &__ctx, float value) {
  return 0.5f + 0.5f * cos(3.141f * (f32x3(1.5f, 1.f, 0.f) + value));
}
void mainImage(Context &__ctx, f32x4 &o, f32x2 p) {
  o = f32x4(0.f);
  for (float aax = 0.5f; aax < 6.f; ++aax)
    for (float aay = 0.5f; aay < 6.f; ++aay) {
      f32x2 aap = p + f32x2(aax, aay) * AA_Recip - 0.5f;
      f32x2 q = (aap * 2.f - iResolution.swizzle<0, 1>()) / iResolution.data[1];
      q *= 502.f - 500.f * cos(0.5f * iTime);
      q += 100.f * iTime * normalize(iResolution.swizzle<0, 1>());
      float v = AllBuildings(__ctx, q);
      o += f32x4(HotColour(__ctx, v), 1.f);
    }
  o /= AA_Sq;
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
