// Port of https://www.shadertoy.com/view/DtXSzs; author: djancool.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtXSzs.json.
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
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x3 reinhard(Context &__ctx, f32x3 x) { return x / (1.0f + x); }
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.data[0];
  f32x2 cam = f32x2(sin(iTime * 0.8f), cos(iTime * 0.4f)) * 0.3f;
  float heightmap = texture(__ctx.channel0, uv + cam).data[0];
  f32x3 viewDir_ts = f32x3(0.0f);
  viewDir_ts.data[2] = 1.0f - distance(uv, f32x2(0.5f));
  viewDir_ts.set_swizzle<0, 1>(uv - f32x2(0.5f));
  viewDir_ts = normalize(viewDir_ts);
  f32x2 parallax = viewDir_ts.swizzle<0, 1>() * viewDir_ts.data[2] *
                   (heightmap - 0.5f) * 0.15f;
  float tex = texture(__ctx.channel0, uv - parallax + cam).data[0];
  f32x2 tex_res = f32x2(textureSize(__ctx.channel0, 0));
  float tex_x1 =
      texture(__ctx.channel0,
              uv - parallax + cam + f32x2(1.0f / tex_res.data[0], 0.0f))
          .data[0];
  float tex_y1 =
      texture(__ctx.channel0,
              uv - parallax + cam + f32x2(0.0f, 1.0f / tex_res.data[1]))
          .data[0];
  float tex_ddx = tex - tex_x1;
  float tex_ddy = tex - tex_y1;
  f32x3 normal = normalize(f32x3(tex_ddx, tex_ddy, 0.004f));
  f32x3 lightDir = normalize(f32x3(
      (iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>()) - f32x2(0.5f),
      1.0f));
  f32x3 light_direct =
      max(0.0f, dot(normal, lightDir)) * f32x3(1.0f, 0.89f, 0.81f);
  float tex_shadow =
      texture(__ctx.channel0, uv + cam - parallax +
                                  lightDir.swizzle<0, 1>() * lightDir.data[2] *
                                      heightmap * 0.5f)
          .data[0];
  float shadow = pow(max(heightmap - tex_shadow, 0.0f), 0.25f);
  f32x3 fake_gi = tex * f32x3(0.8f, 0.9f, 1.0f) * 1.0f;
  f32x3 col = light_direct * 10.0f * shadow + fake_gi;
  col = reinhard(__ctx, col);
  col *= pow(sin(uv.data[0] * 3.141592f) * sin(uv.data[1] * 3.141592f), 0.1f);
  fragColor = f32x4(col, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
               __binding(1, 0) sampler sampling0) {
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
  Context __ctx{{channel0, sampling0}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
