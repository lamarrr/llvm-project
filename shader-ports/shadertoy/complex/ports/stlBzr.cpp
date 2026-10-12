// Port of https://www.shadertoy.com/view/stlBzr; author: nuudl3.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/stlBzr.json.
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
float rhombusSDF(Context &__ctx, f32x2 p, float scale) {
  p = abs(p);
  float d = p.data[0] + p.data[1];
  float f1 = d - scale;
  float f2 = d - scale + 0.01f;
  return max(f1, -f2);
}
float squareSDF(Context &__ctx, f32x2 p, float scale) {
  p = abs(p);
  float f1 = max(p.data[0], p.data[1]) - scale;
  float f2 = max(p.data[0], p.data[1]) - scale + 0.005f;
  return max(f1, -f2);
}
f32x2 rotate(Context &__ctx, f32x2 v, float a) {
  float s = sin(a);
  float c = cos(a);
  f32x2x2 m = f32x2x2(c, -s, s, c);
  return m * v;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  uv = rotate(__ctx, uv, 3.1415f / 16.f * iTime);
  float px = 2.5f / iResolution.data[1];
  f32x3 col = f32x3(0.f);
  col = mix(f32x3(1, 1, 1), col,
            smoothstep(0.f, px, rhombusSDF(__ctx, uv, 0.025f)));
  col = mix(f32x3(1, 1, 1), col,
            smoothstep(0.f, px, squareSDF(__ctx, uv, 0.025f)));
  col = mix(f32x3(1, 1, 1), col,
            smoothstep(0.f, px, rhombusSDF(__ctx, uv, 0.05f)));
  col = mix(f32x3(1, 1, 1), col,
            smoothstep(0.f, px, squareSDF(__ctx, uv, 0.05f)));
  col = mix(f32x3(1, 1, 1), col,
            smoothstep(0.f, px, rhombusSDF(__ctx, uv, 0.1f)));
  col =
      mix(f32x3(1, 1, 1), col, smoothstep(0.f, px, squareSDF(__ctx, uv, 0.1f)));
  col = mix(f32x3(1, 1, 1), col,
            smoothstep(0.f, px, rhombusSDF(__ctx, uv, 0.2f)));
  col =
      mix(f32x3(1, 1, 1), col, smoothstep(0.f, px, squareSDF(__ctx, uv, 0.2f)));
  col = mix(f32x3(1, 1, 1), col,
            smoothstep(0.f, px, rhombusSDF(__ctx, uv, 0.4f)));
  col =
      mix(f32x3(1, 1, 1), col, smoothstep(0.f, px, squareSDF(__ctx, uv, 0.4f)));
  col = mix(f32x3(1, 1, 1), col,
            smoothstep(0.f, px, rhombusSDF(__ctx, uv, 0.8f)));
  col =
      mix(f32x3(1, 1, 1), col, smoothstep(0.f, px, squareSDF(__ctx, uv, 0.8f)));
  col *= cos(iTime + uv.swizzle<0, 1, 0>() + f32x3(0, 2, 4));
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
