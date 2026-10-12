// Port of https://www.shadertoy.com/view/mddcR4; author: Dem0g.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mddcR4.json.
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
float rand(Context &__ctx, float x) {
  return fract(sin(dot(f32x2(x, x), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
f32x2 random2(Context &__ctx, f32x2 st) {
  st = f32x2(dot(st, f32x2(127.1f, 311.7f)), dot(st, f32x2(269.5f, 183.3f)));
  return -1.0f + 2.0f * fract(sin(st) * 43758.5453123f);
}
float noise(Context &__ctx, f32x2 st) {
  f32x2 i = floor(st);
  f32x2 f = fract(st);
  f32x2 u = f * f * (3.0f - 2.0f * f);
  return mix(
      mix(dot(random2(__ctx, i + f32x2(0.0f, 0.0f)), f - f32x2(0.0f, 0.0f)),
          dot(random2(__ctx, i + f32x2(1.0f, 0.0f)), f - f32x2(1.0f, 0.0f)),
          u.data[0]),
      mix(dot(random2(__ctx, i + f32x2(0.0f, 1.0f)), f - f32x2(0.0f, 1.0f)),
          dot(random2(__ctx, i + f32x2(1.0f, 1.0f)), f - f32x2(1.0f, 1.0f)),
          u.data[0]),
      u.data[1]);
}
f32x2x2 rotate2d(Context &__ctx, float _angle) {
  return f32x2x2(cos(_angle), -sin(_angle), sin(_angle), cos(_angle));
}
f32x2 cubic_hermite(Context &__ctx, f32x2 p0, f32x2 p1, f32x2 v0, f32x2 v1,
                    float t) {
  float sq = t * t;
  float cu = t * t * t;
  float h1 = 2.f * cu - 3.f * sq + 1.f;
  float h2 = -2.f * cu + 3.f * sq;
  float h3 = cu - 2.f * sq + t;
  float h4 = cu - sq;
  return h1 * p0 + h2 * p1 + h3 * v0 + h4 * v1;
}
float comb(Context &__ctx, float phase, float density, float sharpness) {
  float signed_mod = mod(phase, density) - density / 2.0f;
  float sinc = sin(signed_mod * sharpness) / (signed_mod * sharpness);
  return pow(sinc, 1.5f);
}
f32x2 random_path(Context &__ctx, float t, float seed) {
  f32x2 pos0 = random2(__ctx, f32x2(seed, floor(t)));
  f32x2 pos1 = random2(__ctx, f32x2(seed, floor(t + 1.f)));
  f32x2 vel0 = random2(__ctx, f32x2(floor(t), seed));
  f32x2 vel1 = random2(__ctx, f32x2(floor(t + 1.f), seed));
  return cubic_hermite(__ctx, pos0, pos1, vel0, vel1, fract(t));
}
f32x2 combed_path(Context &__ctx, float t, float seed, float comb) {
  float offset = max(0.5f, 2.0f * comb);
  f32x2 pos0 = random2(__ctx, f32x2(seed, floor(t)));
  f32x2 pos1 = random2(__ctx, f32x2(seed, floor(t + 1.f)));
  float a0 = atan(pos0.data[0], pos0.data[1]);
  float a1 = atan(pos1.data[0], pos1.data[1]);
  float r0 = length(pos0) * offset;
  float r1 = length(pos1) * offset;
  pos0.data[0] = r0 * cos(a0);
  pos0.data[1] = r0 * sin(a0);
  pos1.data[0] = r1 * cos(a1);
  pos1.data[1] = r1 * sin(a1);
  f32x2 vel0 = random2(__ctx, f32x2(floor(t), seed));
  f32x2 vel1 = random2(__ctx, f32x2(floor(t + 1.f), seed));
  return cubic_hermite(__ctx, pos0, pos1, vel0, vel1, fract(t));
}
float blob(Context &__ctx, f32x2 uv, f32x2 pos, float brightness,
           float spotlight, float cutoff) {
  float d = distance(uv, pos);
  float mul = 2.0f * (d * d * d) / (cutoff * cutoff * cutoff) -
              3.0f * (d * d) / (cutoff * cutoff) + 1.0f;
  float res = mul * brightness * exp(-spotlight * d);
  return d < cutoff ? res : 0.0f;
}
float clean_blob(Context &__ctx, f32x2 uv, f32x2 pos, float brightness,
                 float spotlight, float cutoff) {
  float res = blob(__ctx, uv, pos, brightness, spotlight, cutoff);
  return res > 0.8f ? 0.8f : res;
}
f32x3 mass(Context &__ctx, f32x2 _uv, float metablobs, float subblobs,
           f32x3 col, float ring_strength, float dispersion) {
  float a = 1.3f;
  float b = 4.0f;
  float c = 10.0f;
  float seed =
      rand(__ctx, 3.0f * col.data[0] + 5.0f * col.data[1] + 7.0f * col.data[2]);
  float blob_field = 0.0f;
  f32x2 uv = _uv * rotate2d(__ctx, iTime * (0.5f - seed) * 0.1f);
  for (float i = 0.0f; i < metablobs; i = i + 1.0f) {
    float t = iTime * 0.03f * 1.0f + (i) * 1.0f / (metablobs);
    f32x2 pos =
        combed_path(__ctx, t, i + seed,
                    comb(__ctx, iTime * 0.05f * 1.0f + i, metablobs, 0.3f));
    for (float j = 0.0f; j < subblobs; j = j + 1.0f) {
      float index = (seed + i * metablobs + j);
      col = mix(col, f32x3(0.0f), 0.12f);
      f32x2 offset = dispersion * random_path(__ctx, t, index);
      float blob = clean_blob(__ctx, uv, pos + offset, a, b, c);
      blob_field += blob;
    }
  }
  float ring = (blob_field < 1.f && blob_field > 1.f - 0.299f)
                   ? (1.f - blob_field) / 0.299f
                   : 0.0f;
  float blob = (blob_field > 1.f - 0.299f) ? blob_field : 0.0f;
  blob = blob < 0.2f && blob != 0.0f ? 0.2f : blob;
  float blob_count = metablobs + subblobs;
  f32x3 res = col * (ring * ring_strength + blob);
  return res;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  uv -= .5f;
  uv *= 3.f;
  uv *= rotate2d(__ctx, 0.08f * iTime);
  float field = 0.0f;
  f32x3 color_field = f32x3(1.0f, 0.8f, 0.1f);
  f32x2 light_pos = f32x2(0.0f);
  float slow = iTime * 0.1f;
  f32x3 light_color = f32x3(1.0f + 1.0f * noise(__ctx, f32x2(slow)),
                            1.0f + 1.0f * noise(__ctx, f32x2(slow, -3.0f)),
                            1.0f + 1.0f * noise(__ctx, f32x2(5.0f, slow)));
  light_color = mix(light_color, f32x3(0.0f),
                    0.5f * noise(__ctx, uv * 1.0f + f32x2(0.25f * iTime)));
  color_field = light_color * blob(__ctx, uv, light_pos, 0.9f, 0.0f, 100.0f);
  color_field -=
      1.0f * mass(__ctx, uv, 4.f, 4.f, f32x3(1.0f, 0.0f, 0.0f), 2.0f, 0.3f);
  color_field -=
      10.0f * mass(__ctx, uv, 4.f, 10.f, f32x3(0.0f, 1.0f, 0.0f), 2.0f, 0.3f);
  color_field -=
      30.0f * mass(__ctx, uv, 4.f, 10.f, f32x3(0.0f, 0.0f, 1.0f), 5.0f, 0.5f);
  f32x3 col = color_field;
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
