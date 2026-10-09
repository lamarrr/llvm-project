// Port of https://www.shadertoy.com/view/cdscz2; author: Mitsync.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cdscz2.json.
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
float on_hand(Context &__ctx, float hand_angle, f32x3 hand_size, f32x2 uv,
              float aa_size) {
  f32x2 hand = f32x2(cos(hand_angle), sin(hand_angle));
  f32x2 hand_normal =
      f32x2(cos(hand_angle + .5f * 3.1415926535897932384626433832795f),
            sin(hand_angle + .5f * 3.1415926535897932384626433832795f));
  float d_hand = dot(uv, hand_normal);
  float d_norm = dot(uv, hand);
  float lum_width = 1.f - smoothstep(hand_size.data[2] - aa_size,
                                     hand_size.data[2], abs(d_hand));
  float lum_length =
      smoothstep(hand_size.data[0] - aa_size, hand_size.data[0], d_norm) -
      smoothstep(hand_size.data[1] - aa_size, hand_size.data[1], d_norm);
  return lum_width * lum_length;
}
float on_tick(Context &__ctx, f32x3 tick_size, f32x2 uv, float pix_siz) {
  float num_ticks = 12.f;
  float sum = 0.f;
  for (float i = 0.f; i < num_ticks; i++)
    sum +=
        on_hand(__ctx, 2.f * 3.1415926535897932384626433832795f * i / num_ticks,
                tick_size, uv, pix_siz);
  return sum;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      ((2.0f * fragCoord) - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  float aa_size = 3.f / iResolution.data[1];
  float t = iDate.data[3];
  f32x3 angles = f32x3(0.0f);
  angles.data[0] =
      0.5f * 3.1415926535897932384626433832795f -
      (2.f * 3.1415926535897932384626433832795f * fract(t / (12.f * 3600.f)));
  angles.data[1] =
      0.5f * 3.1415926535897932384626433832795f -
      (2.f * 3.1415926535897932384626433832795f * fract(t / 3600.f));
  float jerk_alignment = 1.f / 0.50f;
  float jerk_length = 1.20f / 2.f;
  float jerky_t = (floor(jerk_alignment * t) +
                   smoothstep(0.5f - jerk_length, 0.5f + jerk_length,
                              fract(jerk_alignment * t))) /
                  jerk_alignment;
  angles.data[2] =
      0.5f * 3.1415926535897932384626433832795f -
      (2.f * 3.1415926535897932384626433832795f * fract(jerky_t / 60.f));
  angles.data[0] =
      angles.data[0] < 0.f
          ? angles.data[0] + 2.f * 3.1415926535897932384626433832795f
          : angles.data[0];
  angles.data[1] =
      angles.data[1] < 0.f
          ? angles.data[1] + 2.f * 3.1415926535897932384626433832795f
          : angles.data[1];
  angles.data[2] =
      angles.data[2] < 0.f
          ? angles.data[2] + 2.f * 3.1415926535897932384626433832795f
          : angles.data[2];
  float uv_dist = length(uv);
  float edge = smoothstep(0.9f - aa_size, 0.9f, uv_dist) -
               smoothstep(0.915f - aa_size, 0.915f, uv_dist);
  float hub = 1.f - smoothstep(0.03f - aa_size, 0.03f, uv_dist);
  float tick = on_tick(__ctx, f32x3(0.83f, 0.88f, 0.008f), uv, aa_size);
  float hour_hand =
      on_hand(__ctx, angles.data[0], f32x3(0.075f, 0.45f, 0.010f), uv, aa_size);
  float minute_hand =
      on_hand(__ctx, angles.data[1], f32x3(0.075f, 0.7f, 0.008f), uv, aa_size);
  float second_hand =
      on_hand(__ctx, angles.data[2], f32x3(-0.075f, 0.8f, 0.005f), uv, aa_size);
  f32x3 edge_col = (edge + tick) * f32x3(0.322f, 0.322f, 0.322f);
  f32x3 hand_col = (hub + hour_hand + minute_hand + second_hand) *
                   f32x3(1.000f, 1.000f, 1.000f);
  fragColor = f32x4(edge_col + hand_col, 1.0f);
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
