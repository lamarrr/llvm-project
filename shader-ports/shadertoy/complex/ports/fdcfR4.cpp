// Port of https://www.shadertoy.com/view/fdcfR4; author: lanx06.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fdcfR4.json.
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

const float EPSILON = .1f;
const int max_step = 255;
const float not_hit = 10000.0f;
const float max_len = 100.f;
float sp(Context &__ctx, f32x3 sample_point, f32x3 center, float size) {
  return length(sample_point - center) - size;
}
float cube(Context &__ctx, f32x3 sample_point, f32x3 center, float size) {
  f32x3 d = abs(sample_point - center) - f32x3(size);
  float in_d = min(max(d.data[0], max(d.data[1], d.data[2])), 0.f);
  float out_d = length(max(d, 0.0f));
  return out_d * (1.f - in_d) + in_d;
}
float scene(Context &__ctx, f32x3 sample_point) {
  float cube1 = cube(__ctx, sample_point, f32x3(0.f), .5f);
  float sp1 = sp(__ctx, sample_point, f32x3(.5f), .5f);
  float dep = min(cube1, sp1);
  return dep;
}
f32x3 ray_d(Context &__ctx, float fov, f32x2 size, f32x2 fargcoord) {
  f32x2 xy = fargcoord - size / 2.f;
  float z = size.data[1] / tan(radians(fov / 2.f));
  return normalize(f32x3(xy, -z));
}
float ray_march(Context &__ctx, f32x3 eye, f32x3 d) {
  float depth = 0.f;
  for (int i = 0; i < max_step; i++) {
    float dis = scene(__ctx, eye + d * depth);
    if (dis < EPSILON)
      return depth;
    depth += dis;
    if (depth >= not_hit)
      return not_hit;
  }
  return not_hit;
}
f32x3 culc_normal(Context &__ctx, f32x3 p) {
  return normalize(f32x3(
      scene(__ctx, f32x3(p.data[0] + EPSILON, p.data[1], p.data[2])) -
          scene(__ctx, f32x3(p.data[0] - EPSILON, p.data[1], p.data[2])),
      scene(__ctx, f32x3(p.data[0], p.data[1] + EPSILON, p.data[2])) -
          scene(__ctx, f32x3(p.data[0], p.data[1] - EPSILON, p.data[2])),
      scene(__ctx, f32x3(p.data[0], p.data[1], p.data[2] + EPSILON)) -
          scene(__ctx, f32x3(p.data[0], p.data[1], p.data[2] - EPSILON))));
}
f32x3 culc_light(Context &__ctx, f32x3 diff_c, f32x3 spec_c, float al,
                 f32x3 pos, f32x3 eye, f32x3 light_pos, f32x3 power) {
  f32x3 n = culc_normal(__ctx, pos);
  f32x3 l = normalize(light_pos - pos);
  f32x3 v = normalize(eye - pos);
  f32x3 r = normalize(reflect(-l, n));
  float dot_ln = dot(l, n);
  float dot_rv = dot(r, v);
  if (dot_ln < 0.f)
    return f32x3(0.0f, 0.0f, 0.0f);
  if (dot_rv < 0.f)
    return power * diff_c * dot_ln;
  return power * (diff_c * dot_ln + spec_c * pow(dot_rv, al));
}
f32x3 phong_light(Context &__ctx, float al, f32x3 light_pos, f32x3 pos,
                  f32x3 eye, f32x3 am_c, f32x3 diff_c, f32x3 spec_c) {
  f32x3 amb_light = .5f * am_c;
  f32x3 color = amb_light;
  f32x3 light_power = f32x3(.4f);
  color +=
      culc_light(__ctx, am_c, spec_c, al, pos, eye, light_pos, light_power);
  return color;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 st = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 color = f32x3(0.0f);
  f32x2 ps = f32x2(cos(iTime), sin(iTime)) * 10.f;
  f32x3 eye_postion = f32x3(0, 0, +6.f);
  f32x3 ray_di = ray_d(__ctx, 50.f, iResolution.swizzle<0, 1>(),
                       gl_FragCoord.swizzle<0, 1>());
  float hit = ray_march(__ctx, eye_postion, ray_di);
  f32x3 pos = eye_postion + hit * ray_di;
  color = f32x3(ray_di);
  if (hit != not_hit) {
    f32x3 light_pos = f32x3(ps / 1.f, 1);
    f32x3 am_c = f32x3(0.8353f, 0.8275f, 1.0f);
    f32x3 diff_c = f32x3(1.0f, 0.0f, 0.0f);
    f32x3 spec_c = f32x3(1.0f, 1.0f, 1.0f);
    float max_shine = 100.0f;
    color = phong_light(__ctx, max_shine, light_pos, pos, eye_postion, am_c,
                        diff_c, spec_c);
  }
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
