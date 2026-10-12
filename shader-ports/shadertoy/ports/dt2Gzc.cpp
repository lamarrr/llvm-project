// Port of https://www.shadertoy.com/view/dt2Gzc; author: koerriva.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dt2Gzc.json.
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
float np = 100.f;
float snp = 20.f;
float R = 0.032f;
float R_RATIO = 0.04f;
float ACC_RATIO = 0.03f;
float ANG = 90.f;
f32x2 N22(Context &__ctx, f32x2 p) {
  f32x3 a = fract(p.swizzle<0, 1, 0>() * f32x3(123.34f, 234.34f, 345.65f));
  a += dot(a, a + 34.45f);
  return fract(f32x2(a.data[0] * a.data[1], a.data[1] * a.data[2]));
}
f32x3 burst(Context &__ctx, f32x2 st, f32x2 pos, float r, f32x3 col) {
  st -= pos;
  r = 0.6f * r * r;
  return (r / dot(st, st)) * 0.6f * col;
}
f32x2 get_pos(Context &__ctx, f32x2 u, f32x2 a, f32x2 p0, float t, float ang) {
  f32x2 d = p0 + f32x2(u.data[0] * cos(ang), u.data[1] * sin(ang)) * t +
            0.5f * a * t * t;
  return d;
}
f32x2 get_velocity(Context &__ctx, f32x2 u, f32x2 a, float t, float ang) {
  return f32x2(u.data[0] * cos(ang), u.data[1] * sin(ang)) + a * t;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (2.f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  float aspect = iResolution.data[0] / iResolution.data[1];
  f32x3 col = f32x3(0.0f);
  float t = mod(iTime, 10.f);
  f32x2 u = f32x2(0.f);
  const f32x2 acc = f32x2(0.0f, -9.8f);
  float ang = radians(ANG);
  f32x3 particles = f32x3(0.0f);
  for (float i = 0.f; i < np; i++) {
    float r = R;
    f32x2 rand = N22(__ctx, f32x2(i));
    f32x2 ip = f32x2(sin(30.f * rand.data[0]) * aspect, -1.f + r);
    u = f32x2(sin(5.f * rand.data[0]), 5.f + sin(4.f * rand.data[1]));
    float t_i = t - i / 5.f;
    f32x2 s = get_pos(__ctx, u, acc, ip, t_i, ang);
    f32x2 v = get_velocity(__ctx, u, acc, t_i, ang);
    float t_up = u.data[1] * sin(ang) / abs(acc.data[1]);
    f32x2 h_max = get_pos(__ctx, u, acc, ip, t_up, ang);
    f32x3 pcol = f32x3(cos(i), cos(iTime), sin(iTime));
    if (v.data[1] < -0.5f) {
      r = 0.0f;
    }
    particles += burst(__ctx, uv, s, r, pcol);
    if (v.data[1] > -6.5f && v.data[1] < 0.0f && t_i >= t_up) {
      float unit = (360.f / snp);
      for (float j = 0.0f; j < snp; j++) {
        float ang = radians(j * unit);
        float r = 0.035f;
        r -= (t_i - t_up) * R_RATIO;
        float x = cos(ang);
        float y = sin(ang);
        y = y + abs(x) * sqrt((8.f - abs(x)) / 50.0f);
        f32x2 heart = f32x2(x * x + y * y) * (0.4f / (t_i * sqrt(t_i)));
        f32x2 S =
            get_pos(__ctx, heart, acc * ACC_RATIO, h_max, t_i - (t_up), ang);
        particles += burst(__ctx, uv, S, max(0.0f, r), pcol);
      }
    }
  }
  col = particles;
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
