// Port of https://www.shadertoy.com/view/DtsGWs; author: lanx06.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtsGWs.json.
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
f32x2 dir4[4] = f32x2[](f32x2(1.0f, 1.0f), f32x2(1.0f, -1.0f),
                        f32x2(-1.0f, -1.0f), f32x2(-1.0f, 1.0f));
float shape(Context &__ctx, f32x2 uv, f32x2 dir) {
  f32x2 l = uv - 0.5f;
  l += dir;
  float thin = 0.15f;
  float radius = 0.5f;
  float a = smoothstep(radius + 1.8f / iResolution.data[1],
                       radius - 1.8f / iResolution.data[1], length(l));
  return a;
}
float shape4(Context &__ctx, f32x2 uv) {
  float o = 0.0f;
  for (int i = 0; i < 4; i++) {
    if (i % 2 == 0)
      continue;
    o += shape(__ctx, uv, dir4[i] * 0.5f);
  }
  o = shape(__ctx, uv, f32x2(0.0f));
  return o;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float n = 500.0f;
  f32x2 st = (fragCoord / iResolution.data[1]);
  f32x2 uv = fract(st);
  float r = 0.1f;
  f32x2 p = f32x2(cos(iTime * 0.9f), sin(iTime * 0.9f * 0.333f)) * 0.5f +
            f32x2(iResolution.data[0] / iResolution.data[1] * 0.5f, 0.5f);
  f32x2 origin = 0.5f + floor(st);
  float l;
  f32x3 col;
  for (int i = 0; i < 5; i++) {
    float size = pow(0.5f, float(i + 1));
    l = distance(p, origin) - size - r;
    if (l > 0.01f)
      break;
    origin += (step(f32x2(0.5f), uv) - 0.5f) * size;
    uv = fract((uv - 0.5f) * 2.0f);
  }
  col.set_swizzle<0, 1>(f32x2(shape4(__ctx, uv)));
  col.data[2] = smoothstep(1.8f / iResolution.data[1],
                           -1.8f / iResolution.data[1], length(st - p) - r);
  ;
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
