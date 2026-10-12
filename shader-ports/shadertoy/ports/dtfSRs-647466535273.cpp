// Port of https://www.shadertoy.com/view/dtfSRs; author: fishy.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtfSRs-647466535273.json.
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
float f(Context &__ctx, float x) {
  int s = int(mod(iTime * 0.5f, 4.f));
  switch (s) {
  case 0:
    return sin(x * 5.f + iTime) * 0.5f;
    break;
  case 1:
    x -= 0.3f;
    return -x * x - x * x * x;
    break;
  case 2:
    x += 0.5f;
    return 3.f * x * x - 2.f * x * x * x - 0.5f;
    break;
  case 3:
    return (sin(x * 5.f) + cos(x * 10.f + iTime)) * 0.25f;
    break;
  }
}
float graph(Context &__ctx, f32x2 p) {
  float f = f(__ctx, p.data[0]);
  return abs(p.data[1] - f) - abs(ddx(f));
}
float map(Context &__ctx, f32x2 uv) { return graph(__ctx, uv); }
f32x3 colorSdf(Context &__ctx, f32x2 uv, f32x4 m, float px) {
  float d = map(__ctx, uv);
  float md = map(__ctx, m.swizzle<0, 1>());
  f32x3 col = mix(f32x3(0.094f, 0.494f, 0.988f), f32x3(1.000f, 0.082f, 0.980f),
                  sign(d) * 0.5f + 0.5f);
  col *=
      smoothstep(-px * 200.f, px * 200.f, cos(d * 200.f)) * 0.2f + 1.f - 0.2f;
  col = mix(col, f32x3(1), smoothstep(0.005f + px, 0.005f - px, abs(d)));
  if (m.data[2] > 0.f) {
    float mdc = smoothstep(
        px, -px,
        min(abs(distance(m.swizzle<0, 1>(), uv) - md + 0.005f) - 0.005f,
            distance(m.swizzle<0, 1>(), uv) - 0.005f * 2.f));
    col = mix(col, f32x3(1.000f, 0.875f, 0.000f), mdc);
  }
  col /= d + 1.f;
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord * 2.f - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x4 m =
      (iMouse * 2.f - iResolution.swizzle<0, 1, 0, 1>()) / iResolution.data[1];
  float px = 2.f / iResolution.data[1];
  float g = graph(__ctx, uv);
  float fw = fwidth(g);
  f32x3 col = f32x3(smoothstep(0.01f + fw, 0.01f - fw, g));
  fragColor = f32x4(
      mix(col, colorSdf(__ctx, uv, m, px), clamp(iMouse.data[2], 0.f, 1.f)),
      1.0f);
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
