// Port of https://www.shadertoy.com/view/ssjyDG; author: copperbotte.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ssjyDG.json.
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
f32x3 optical_illusion(Context &__ctx, f32x2 uv) {
  const f32x3 color_bg = f32x3(1, 0.5f, 0);
  const f32x3 color_fg = f32x3(0, 0, 1);
  const f32x3 color_forw = f32x3(0);
  const f32x3 color_back = f32x3(1);
  const float r_o = 0.9f;
  const float r_i = 0.7f;
  f32x2 xy = mod(uv, 1.0f) * 2.0f - 1.0f;
  float r2 = dot(xy, xy);
  float r_o2 = r_o * r_o;
  float r_i2 = r_i * r_i;
  const float dx = 0.05f;
  float r2smooth_out = smoothstep(r_o2 - dx, r_o2 + dx, r2);
  float r2smooth_in = smoothstep(r_i2 - dx, r_i2 + dx, r2);
  float xsmooth = smoothstep(-dx, dx, xy.data[0]);
  f32x3 xcolor = mix(color_back, color_forw, xsmooth);
  f32x3 forecolor = mix(color_fg, xcolor, r2smooth_in);
  return mix(forecolor, color_bg, r2smooth_out);
}
void generate_ray(Context &__ctx, f32x2 xy, f32x3 &rPos, f32x3 &rDir) {
  rPos = f32x3(0, 0, 2);
  f32x3 dir = f32x3(xy, -1);
  rDir = normalize(dir);
}
float Sdf(Context &__ctx, f32x3 wPos, float inner_rad, float outer_rad) {
  f32x2 rPos = f32x2(length(wPos.swizzle<0, 2>()), wPos.data[1]);
  rPos -= f32x2(outer_rad, 0);
  float sdf = length(rPos) - inner_rad;
  return sdf;
}
f32x3 raymarch(Context &__ctx, f32x3 rPos, f32x3 rDir, float inner_rad,
               float outer_rad) {
  f32x3 sPos = rPos;
  float dir = sign(Sdf(__ctx, sPos, inner_rad, outer_rad));
  for (int i = 0; i < 1000; ++i) {
    float sdf = Sdf(__ctx, sPos, inner_rad, outer_rad);
    sPos += rDir * dir * sdf;
    if (abs(sdf) < 1e-4f)
      return sPos;
  }
  return sPos;
}
f32x3 color_wpos(Context &__ctx, f32x3 wPos, float inner_rad, float outer_rad,
                 float speed, f32x2 scale) {
  float phi = atan(wPos.data[2], wPos.data[0]);
  float r = length(wPos.swizzle<0, 2>());
  float theta = atan(wPos.data[1], r - outer_rad);
  phi = phi - speed * iTime;
  return optical_illusion(__ctx, scale * f32x2(phi, theta) / 3.141592f);
}
f32x3 sample_image(Context &__ctx, f32x2 uv) {
  f32x2 xy = uv * 2.0f - 1.0f;
  xy.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 col = f32x3(0);
  float inner_rad = 0.75f;
  float outer_rad = 2.1f - inner_rad;
  float speed = 2.5e-3f;
  f32x2 scale = f32x2(25.0f, 10.0f);
  f32x3 rPos, rDir;
  generate_ray(__ctx, xy, rPos, rDir);
  f32x3 sPos = raymarch(__ctx, rPos, rDir, inner_rad, outer_rad);
  if (abs(Sdf(__ctx, sPos, inner_rad, outer_rad)) < 0.01f)
    col = color_wpos(__ctx, sPos, inner_rad, outer_rad, speed, scale);
  return col;
}
float seed;
float rnd(Context &__ctx) { return fract(sin(seed++) * 43758.5453123f); }
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 col = f32x3(0);
  const int samples = 4;
  const float s_f = float(samples);
  for (int i = 0; i < samples; ++i) {
    f32x2 rng = f32x2(rnd(__ctx), rnd(__ctx)) - 0.5f;
    f32x2 uv = (fragCoord + rng) / iResolution.swizzle<0, 1>();
    f32x3 C = sample_image(__ctx, uv);
    C = pow(C, f32x3(2.2f));
    col += C;
  }
  col /= s_f;
  col = pow(col, f32x3(1.0f / 2.2f));
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
