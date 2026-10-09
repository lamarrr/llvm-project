// Port of https://www.shadertoy.com/view/fs2SRm; author: loryruta.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fs2SRm.json.
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
float hash(Context &__ctx, float n) { return fract(sin(n) * 1e4f); }
float hash(Context &__ctx, f32x2 p) {
  return fract(1e4f * sin(17.0f * p.data[0] + p.data[1] * 0.1f) *
               (0.1f + abs(sin(p.data[1] * 13.0f + p.data[0]))));
}
float noise(Context &__ctx, float x) {
  float i = floor(x);
  float f = fract(x);
  float u = f * f * (3.0f - 2.0f * f);
  return mix(hash(__ctx, i), hash(__ctx, i + 1.0f), u);
}
float noise(Context &__ctx, f32x2 x) {
  f32x2 i = floor(x);
  f32x2 f = fract(x);
  float a = hash(__ctx, i);
  float b = hash(__ctx, i + f32x2(1.0f, 0.0f));
  float c = hash(__ctx, i + f32x2(0.0f, 1.0f));
  float d = hash(__ctx, i + f32x2(1.0f, 1.0f));
  f32x2 u = f * f * (3.0f - 2.0f * f);
  return mix(a, b, u.data[0]) + (c - a) * u.data[1] * (1.0f - u.data[0]) +
         (d - b) * u.data[0] * u.data[1];
}
float fbm(Context &__ctx, f32x2 x) {
  float v = 0.0f;
  float a = 0.5f;
  f32x2 shift = f32x2(100);
  f32x2x2 rot = f32x2x2(cos(0.5f), sin(0.5f), -sin(0.5f), cos(0.50f));
  for (int i = 0; i < 8; ++i) {
    v += a * noise(__ctx, x);
    x = rot * x * 2.0f + shift;
    a *= 0.5f;
  }
  return v;
}
f32x3 surf_norm(Context &__ctx, f32x2 pos) {
  float h = fbm(__ctx, pos);
  f32x3 d1 =
      normalize(f32x3(0.001f, 0, fbm(__ctx, pos + f32x2(0.001f, 0)) - h));
  f32x3 d2 =
      normalize(f32x3(0, 0.001f, fbm(__ctx, pos + f32x2(0, 0.001f)) - h));
  return normalize(cross(d1, d2));
}
f32x3 get_scene_color(Context &__ctx, f32x3 ray_dir) { return f32x3(1.0f); }
float FresnelReflectAmount(Context &__ctx, float n1, float n2, f32x3 normal,
                           f32x3 incident) {
  float r0 = (n1 - n2) / (n1 + n2);
  r0 *= r0;
  float cosX = -dot(normal, incident);
  if (n1 > n2) {
    float n = n1 / n2;
    float sinT2 = n * n * (1.0f - cosX * cosX);
    if (sinT2 > 1.0f)
      return 1.0f;
    cosX = sqrt(1.0f - sinT2);
  }
  float x = 1.0f - cosX;
  float ret = r0 + (1.0f - r0) * x * x * x * x * x;
  ret = (0.1f + (1.0f - 0.1f) * ret);
  return ret;
}
f32x3 apply_light(Context &__ctx, f32x3 surf_norm, f32x3 view_dir) {
  f32x3 diff = f32x3(1.0f, 0.0f, 0.0f) * f32x3(0.1f);
  float diff_w =
      clamp(dot(surf_norm, -normalize(f32x3(-1.0f, -1.0f, -1.0f))), 0.0f, 1.0f);
  diff += f32x3(1.0f, 0.0f, 0.0f) * diff_w * f32x3(1.0f);
  f32x3 refl_light_dir =
      reflect(normalize(f32x3(-1.0f, -1.0f, -1.0f)), surf_norm);
  float spec_w = max(dot(-view_dir, refl_light_dir), 0.0f);
  diff += pow(spec_w, 2.0f) * f32x3(1.0f);
  float reflect_mult =
      FresnelReflectAmount(__ctx, 1.0f, 1.1f, surf_norm, view_dir);
  f32x3 view_refl_dir = reflect(view_dir, surf_norm);
  f32x3 ret = f32x3(0);
  ret += diff * (1.0f - reflect_mult);
  ret += get_scene_color(__ctx, view_refl_dir) * reflect_mult;
  return ret;
}
float gamma = 2.2f;
f32x3 lumaBasedReinhardToneMapping(Context &__ctx, f32x3 color) {
  float luma = dot(color, f32x3(0.2126f, 0.7152f, 0.0722f));
  float toneMappedLuma = luma / (1.f + luma);
  color *= toneMappedLuma / luma;
  color = pow(color, f32x3(1.f / gamma));
  return color;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord / iResolution.swizzle<0, 1>()) * 2.0f - 1.0f;
  f32x3 norm = surf_norm(__ctx, uv);
  f32x3 view_dir = normalize(f32x3(uv, -1.0f));
  f32x3 col = apply_light(__ctx, norm, view_dir);
  fragColor = f32x4(col, 1.0f);
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
