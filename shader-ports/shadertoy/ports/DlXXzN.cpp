// Port of https://www.shadertoy.com/view/DlXXzN; author: ManuManu.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DlXXzN.json.
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
struct Context {};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x2x2 Rot(Context &__ctx, float angle) {
  float s = sin(angle);
  float c = cos(angle);
  return f32x2x2(c, -s, s, c);
}
float hash21(Context &__ctx, f32x2 p) {
  p = fract(p * f32x2(789.123f, 456.987f));
  p += dot(p, p + f32x2(12.34f));
  return fract(p.data[0] * p.data[1]);
}
float circle(Context &__ctx, f32x2 pos, f32x2 center, float radius, float width,
             float smooth_width) {
  float l = length(pos - center);
  float low_val = width * .5f - smooth_width;
  float high_val = width * .5f;
  float tore = smoothstep(high_val, low_val, abs(l - radius));
  return tore;
}
float dirty_circle(Context &__ctx, f32x2 pos, f32x2 center, float radius,
                   float width, float smooth_width) {
  float rand_val = hash21(__ctx, pos);
  pos = pos + f32x2(rand_val, fract(rand_val * 11.f)) * 0.01f;
  return circle(__ctx, pos, center, radius, width, smooth_width);
}
f32x3 planet_text(Context &__ctx, f32x2 uv) {
  float nb_circles = 5.f;
  float val = 0.f;
  f32x3 col = f32x3(.2f, .6f, .4f) * .28f;
  for (float i = 0.001f; i < nb_circles; i += 1.f) {
    float rand_val = hash21(__ctx, f32x2(i, i * .123f));
    float rand_val1 = fract(rand_val * 13.f);
    float rand_val2 = fract(rand_val * 53.f);
    float rand_val3 = fract(rand_val * 111.f);
    f32x2 center = f32x2(rand_val - .5f, rand_val1 - .5f);
    center.data[0] *= 1.7f;
    float radius = .3f - (rand_val2 - .5f) * 0.3f;
    float width = .2f - (rand_val3 - .5f) * 0.05f;
    float tore = dirty_circle(__ctx, uv, center, radius, width, 0.1f);
    val = max(tore, val);
  }
  nb_circles = 40.f;
  for (float i = 0.00001f; i < nb_circles; i += 1.f) {
    float rand_val = hash21(__ctx, f32x2(i * .321f, i * .456f));
    float rand_val1 = fract(rand_val * 13.f);
    float rand_val2 = fract(rand_val * 53.f);
    float rand_val3 = fract(rand_val * 111.f);
    f32x2 center = f32x2(rand_val - .5f, rand_val1 - .5f);
    center.data[0] *= 1.7f;
    center.data[1] *= 2.1f;
    float radius = .1f - (rand_val2 - .5f) * 0.1f;
    float width = .1f - (rand_val3 - .5f) * 0.2f;
    float tore = dirty_circle(__ctx, uv, center, radius, width, 0.1f);
    val = max(tore, val);
  }
  return f32x3(val) + col;
}
f32x3 planet(Context &__ctx, f32x2 uv, f32x3 local_y, f32x3 local_z) {
  f32x3 local_x = cross(local_y, local_z);
  float dist_from_center = length(uv);
  float dist_from_circle_plane =
      sqrt(1.f - dist_from_center * dist_from_center);
  f32x3 pos_on_sphere = f32x3(uv.data[0], uv.data[1], dist_from_circle_plane);
  f32x2 proj_on_equator_plane =
      f32x2(dot(pos_on_sphere, local_x), dot(pos_on_sphere, local_z));
  float long_angle =
      atan(proj_on_equator_plane.data[1], proj_on_equator_plane.data[0]);
  f32x3 proj_eq_plane = dot(pos_on_sphere, local_x) * local_x +
                        dot(pos_on_sphere, local_z) * local_z;
  float lat_angle =
      atan(dot(pos_on_sphere, local_y), dot(pos_on_sphere, proj_eq_plane));
  f32x2 text_uv = f32x2((long_angle + 3.14159265359f) / (2.f * 3.14159265359f),
                        (lat_angle + 3.14159265359f / 2.f) / 3.14159265359f);
  text_uv = 2.f * text_uv - 1.f;
  f32x3 col = f32x3(0.f);
  const float nb_div = 6.f;
  float inc = 2.f * 3.14159265359f / nb_div;
  for (float i = 0.f; i < 2.f * 3.14159265359f; i += inc) {
    float moved_ux = fract((text_uv.data[0] + i) * .5f + .5f) * 2.f - 1.f;
    f32x2 cur_uv = f32x2(moved_ux, text_uv.data[1]);
    col += planet_text(__ctx, cur_uv);
  }
  const float nb_div2 = 3.f;
  inc = 2.f * 3.14159265359f / nb_div2;
  float wind = .02f * iTime + sin(iTime * .1f) * .09f;
  for (float i = 0.f; i < 2.f * 3.14159265359f; i += inc) {
    float moved_ux =
        fract((0.456f + text_uv.data[0] + i + wind * i) * .5f + .5f) * 2.f -
        1.f;
    f32x2 cur_uv = f32x2(moved_ux, text_uv.data[1]);
    col += planet_text(__ctx, cur_uv);
  }
  return col / (nb_div + nb_div2);
}
f32x3 add_planet(Context &__ctx, f32x2 uv) {
  f32x3 col = f32x3(0.2f);
  f32x3 axis = f32x3(0.3f, 1.0f, 0.f);
  axis = normalize(axis);
  f32x3 right_vect = f32x3(1.f, 0.f, 0.f);
  float rotation_speed = 0.35f;
  float angle = mod(iTime * rotation_speed, 2.f * 3.14159265359f);
  right_vect = f32x3(cos(angle), 0.f, sin(angle));
  f32x3 dir = normalize(cross(right_vect, axis));
  col = planet(__ctx, uv, axis, dir);
  return col;
}
float star(Context &__ctx, f32x2 uv, float flare) {
  float d = length(uv);
  float col = 0.f;
  col = .01f / d;
  col += max(0.f, 1.f - abs(uv.data[0] * uv.data[1]) * 2000.f) * flare;
  uv = Rot(__ctx, 3.14f / 4.f) * uv;
  col += max(0.f, 1.f - abs(uv.data[0] * uv.data[1]) * 2000.f) * .4f * flare;
  col *= smoothstep(.5f, .3f, d);
  return col;
}
f32x3 starfield(Context &__ctx, f32x2 uv, float flare, f32x3 color1,
                f32x3 color2) {
  f32x2 gv = fract(uv) - .5f;
  f32x2 n = floor(uv);
  float flare_effect = flare * 1.f / 5.f;
  f32x3 col = f32x3(0.f);
  for (int i = -1; i <= 1; i++)
    for (int j = -1; j <= 1; j++) {
      f32x2 offset = f32x2(i, j);
      float rand_val = hash21(__ctx, n + offset);
      float flare_rand = flare * (fract(rand_val * 789.f) * .5f + .5f);
      float starcol = star(
          __ctx,
          gv - offset + f32x2(rand_val - .5f, fract(rand_val * 10.f) - .5f),
          flare_rand);
      float freq = (fract(rand_val * 100.f) + 3.f) / 2.f;
      float phase = fract(rand_val * 1000.f) * 15.f;
      float power =
          fract(rand_val * 55.f) * (1.f - flare_effect) + flare_effect;
      float sin_amplitude = (1.f - power);
      starcol *= abs(sin(iTime * freq + phase)) * sin_amplitude + power +
                 sin_amplitude / 2.f;
      float colRand = fract(rand_val * 123456.f);
      col += starcol * mix(color1, color2, colRand) * fract(rand_val * 111.f);
    }
  return col;
}
f32x3 starry_sky(Context &__ctx, f32x2 uv) {
  f32x3 col = f32x3(0.f, 0.f, 0.1f);
  uv *= 12.f;
  col += starfield(__ctx, uv, .1f, f32x3(.97f, .95f, .77f),
                   f32x3(.97f, .77f, .22f));
  uv /= 2.f;
  uv = Rot(__ctx, .5f) * uv;
  col += starfield(__ctx, uv, .1f, f32x3(.65f, .32f, .60f),
                   f32x3(.18f, .50f, .63f));
  uv /= 2.f;
  uv = Rot(__ctx, 1.f) * uv;
  col += starfield(__ctx, uv, .4f, f32x3(.34f, .09f, .39f),
                   f32x3(.18f, .43f, .52f));
  uv /= 2.f;
  uv += f32x2(5.f, 10.f);
  uv = Rot(__ctx, 6.2f) * uv;
  col += starfield(__ctx, uv, .4f, f32x3(.18f, .43f, .52f),
                   f32x3(.08f, .40f, .51f));
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col = starry_sky(__ctx, uv);
  uv *= 2.f;
  const float planet_factor = .6f;
  if (length(uv) < planet_factor) {
    col = add_planet(__ctx, uv / planet_factor);
  }
  fragColor = f32x4(col, 1.0f);
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
