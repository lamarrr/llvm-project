// Port of https://www.shadertoy.com/view/fdGyR3; author: gehtsiegarnixan.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fdGyR3.json.
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
const f32x2 r = f32x2(1, 1.7320508075f);
const f32x2 h = f32x2(0.5f, 0.8660254037f);
float hexDist(Context &__ctx, f32x2 p) {
  p = abs(p);
  return max(dot(p, h), p.data[0]);
}
f32x4 hexTile_lerp(Context &__ctx, f32x2 uv) {
  f32x2 a = mod(uv, r) - h;
  f32x2 b = mod(uv - h, r) - h;
  float d1 = dot(a, a);
  float d2 = dot(b, b);
  f32x2 gv = mix(a, b, float(d2 < d1));
  f32x2 id = uv - gv;
  return f32x4(gv, id);
}
f32x4 hexCoords_if(Context &__ctx, f32x2 uv) {
  f32x2 a = mod(uv, r) - h;
  f32x2 b = mod(uv - h, r) - h;
  f32x2 gv = dot(a, a) < dot(b, b) ? a : b;
  f32x2 id = uv - gv;
  return f32x4(gv, id);
}
f32x4 HexCoords_vec4(Context &__ctx, f32x2 p) {
  f32x4 ab = f32x4(p, p - f32x2(.5f, 1));
  f32x4 hC = floor(ab / r.swizzle<0, 1, 0, 1>()) + .5f;
  f32x4 h =
      f32x4(p - hC.swizzle<0, 1>() * r, p - (hC.swizzle<2, 3>() + .5f) * r);
  f32x4 uvid = dot(h.swizzle<0, 1>(), h.swizzle<0, 1>()) <
                       dot(h.swizzle<2, 3>(), h.swizzle<2, 3>())
                   ? f32x4(h.swizzle<0, 1>(), hC.swizzle<0, 1>())
                   : f32x4(h.swizzle<2, 3>(), hC.swizzle<2, 3>() + .5f);
  return f32x4(uvid);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - iResolution.swizzle<0, 1>() * .5f) / iResolution.data[1];
  f32x2 subject = (uv * 2.f) + (iTime * 0.2f) + 100.f;
  int repeat = 1;
  f32x4 test;
  for (int i = 0; i < repeat; i++) {
    f32x2 a = subject + float(i);
    f32x4 x = HexCoords_vec4(__ctx, a);
    test += x;
  }
  fragColor = f32x4(fract(test));
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
