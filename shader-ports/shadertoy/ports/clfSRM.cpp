// Port of https://www.shadertoy.com/view/clfSRM; author: eo.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/clfSRM.json.
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
  Texture2D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x3 angle_to_rgb(Context &__ctx, float num_0_to_1) {
  f32x3 brightness = f32x3(0.6f);
  f32x3 saturation = f32x3(0.525f);
  f32x3 rgb_phase = f32x3(0.0f, 0.33f, 0.67f);
  return brightness +
         saturation * cos(6.28318530718f * (num_0_to_1 + rgb_phase));
}
f32x2 get_cartesian_coords(Context &__ctx, f32x2 xy_px, f32x3 res) {
  return (2.0f * xy_px - res.swizzle<0, 1>()) / res.data[1];
}
bool get_key_toggle(Context &__ctx, sampler2D keyboard_channel, int key_code) {
  return bool(texelFetch(keyboard_channel, i32x2(key_code, 2)).data[0]);
}
float get_mouse_x_norm(Context &__ctx, f32x4 mouse, f32x3 res) {
  float is_mouse_down = clamp(mouse.data[2], 0.f, 1.f);
  float mouse_x_norm =
      clamp(2.0f * mouse.data[0] / res.data[0] - 0.5f, 0.f, 1.f);
  float default_mouse_not_down = 1.0f;
  return mix(default_mouse_not_down, mouse_x_norm, is_mouse_down);
}
f32x3 sample_video_warped(Context &__ctx, sampler2D channel, f32x2 warp_uv,
                          f32x3 res) {
  f32x2 aspect_correction = f32x2(res.data[1] / res.data[0], 1.0f);
  f32x2 sample_coords = 0.5f * (1.0f + warp_uv * aspect_correction);
  return texture(channel, fract(sample_coords)).swizzle<0, 1, 2>();
}
f32x2 mult2d(Context &__ctx, f32x2 a, f32x2 b) {
  return f32x2(a.data[0] * b.data[0] - a.data[1] * b.data[1],
               a.data[0] * b.data[1] + a.data[1] * b.data[0]);
}
f32x2 div2d(Context &__ctx, f32x2 a, f32x2 b) {
  if (abs(b.data[0]) < 0.0001f && abs(b.data[1]) < 0.0001f) {
    return f32x2(0, 0);
  }
  float numer_real = a.data[0] * b.data[0] + a.data[1] * b.data[1];
  float numer_imag = a.data[1] * b.data[0] - a.data[0] * b.data[1];
  float denom = b.data[0] * b.data[0] + b.data[1] * b.data[1];
  return f32x2(numer_real, numer_imag) / denom;
}
f32x2 get_polar(Context &__ctx, f32x2 xy) {
  float magnitude = sqrt(length(xy));
  float angle_minus_pi_to_pi = atan(-xy.data[1], xy.data[0]);
  float angle_norm = angle_minus_pi_to_pi / 6.28318530718f;
  return f32x2(magnitude, fract(angle_norm));
}
f32x2 calculate_f_of_z(Context &__ctx, f32x2 xy, float zoom) {
  float t =
      smoothstep(0.0f, 1.0f, get_mouse_x_norm(__ctx, iMouse, iResolution));
  f32x2 z = xy / zoom;
  f32x2 numer = z - f32x2(1, 0) * t;
  f32x2 z_squared = mult2d(__ctx, z, z);
  f32x2 denom = (z_squared + z) * t + f32x2(1, 0);
  return div2d(__ctx, numer, denom);
}
float get_level_scale(Context &__ctx, f32x2 polar_mag_angle, bool enable_mag,
                      bool enable_phase) {
  float mag_level = fract(float(4) * polar_mag_angle.data[0]);
  float phase_level = fract(float(12) * polar_mag_angle.data[1]);
  mag_level = mix(1.0f, mag_level, float(enable_mag));
  phase_level = mix(1.0f, phase_level, float(enable_phase));
  float leveling = mag_level * phase_level;
  return (1.0f - 0.25f) + 0.25f * leveling;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  bool show_video = get_key_toggle(__ctx, __ctx.channel0, 9);
  bool show_mag_levels = !get_key_toggle(__ctx, __ctx.channel0, 77);
  bool show_phase_levels = !get_key_toggle(__ctx, __ctx.channel0, 80);
  f32x2 cartesian_xy =
      get_cartesian_coords(__ctx, fragCoord.swizzle<0, 1>(), iResolution);
  f32x2 f_of_z = calculate_f_of_z(__ctx, cartesian_xy, 0.5f);
  f32x2 polar = get_polar(__ctx, f_of_z);
  float level_scale =
      get_level_scale(__ctx, polar, show_mag_levels, show_phase_levels);
  f32x3 out_rgb;
  if (show_video) {
    out_rgb =
        sample_video_warped(__ctx, __ctx.channel1, f_of_z * 0.5f, iResolution);
  } else {
    f32x3 phase_color = angle_to_rgb(__ctx, polar.data[1]);
    out_rgb = phase_color * level_scale;
  }
  fragColor = f32x4(out_rgb, 1.0f);
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
