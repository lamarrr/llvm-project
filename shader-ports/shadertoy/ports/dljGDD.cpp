// Port of https://www.shadertoy.com/view/dljGDD; author: brainsandwich.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dljGDD.json.
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
float rand(Context &__ctx, f32x2 co) {
  return fract(sin(dot(co, f32x2(12.9898f, 78.233f))) * 43758.5453f);
}
float sdf_rect(Context &__ctx, f32x2 uv, f32x2 position, f32x2 halfsize) {
  f32x2 componentWiseEdgeDistance = abs(uv - position) - halfsize;
  return min(
      max(componentWiseEdgeDistance.data[0], componentWiseEdgeDistance.data[1]),
      1.f);
}
float sdf_rect_rounded(Context &__ctx, f32x2 uv, f32x2 position,
                       f32x2 halfsize) {
  f32x2 componentWiseEdgeDistance = abs(uv - position) - halfsize;
  float outsideDistance = length(max(componentWiseEdgeDistance, 0.f));
  float insideDistance = min(
      max(componentWiseEdgeDistance.data[0], componentWiseEdgeDistance.data[1]),
      0.f);
  return outsideDistance + insideDistance;
}
struct rect {
  f32x3 color;
  f32x2 center;
  f32x2 halfsize;
};
f32x3 mixfn(Context &__ctx, f32x3 pixel, f32x3 color, float mixer) {
  f32x3 cp = color;
  return mix(pixel, cp, mixer);
}
float antialias(Context &__ctx, float dist) {
  return smoothstep(-1.f, 1.f, (dist - .04f) / fwidth(dist));
}
void draw_rect(Context &__ctx, f32x3 &pixel, f32x2 position, rect r) {
  float dist = sdf_rect_rounded(__ctx, position, r.center, r.halfsize);
  float clampd = 1.f - antialias(__ctx, dist);
  pixel = mixfn(__ctx, pixel, r.color, clampd);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.data[0];
  f32x3 col = f32x3(1.f);
  for (int i = 0; i < 32; i++) {
    float fi = float(i) * sin(0.00000001f * iTime + 0.001f);
    rect r;
    r.color =
        f32x3(rand(__ctx, 10.f * f32x2(fi)), rand(__ctx, f32x2(4.501f * fi)),
              rand(__ctx, f32x2(-fi * .99f)));
    r.center = 1.f * f32x2(rand(__ctx, f32x2(fi, fi + 0.1f)),
                           rand(__ctx, f32x2(0.1f * fi)));
    r.halfsize = 0.2f * f32x2(rand(__ctx, f32x2(fi, fi + 0.1f)),
                              rand(__ctx, f32x2(2.f * fi)));
    draw_rect(__ctx, col, uv, r);
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
