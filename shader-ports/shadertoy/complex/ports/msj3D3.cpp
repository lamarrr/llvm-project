// Port of https://www.shadertoy.com/view/msj3D3; author: kishimisu.
// Dataset license metadata: cc-by-nc-sa-4.0; original retained in
// upstream/msj3D3.json.
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
float hash12(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(f32x3(p.swizzle<0, 1, 0>()) * .1031f);
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
f32x3 getLight(Context &__ctx, f32x3 p, f32x3 color) {
  return max(
      f32x3(0.f),
      color / (1.f + pow(abs((length(f32x2(length(p.swizzle<0, 2>()) - .6f,
                                           p.data[1])) -
                              .06f) *
                             22.f),
                         1.3f)) -
          .001f);
}
f32x3 geo(Context &__ctx, f32x3 po, float &d, f32x2 &f) {
  float r = hash12(__ctx, floor(po.swizzle<1, 2>() / 7.f + f32x2(.5f))) - .5f;
  f32x3 p = (mod(po + f32x3((iTime * .7f) * r * 4.f, 0.f, 0.f) +
                     f32x3(.5f, 7.f, 7.f) / 2.f,
                 f32x3(.5f, 7.f, 7.f)) -
             f32x3(.5f, 7.f, 7.f) / 2.f);
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (f32x2x2(cos(1.57f), -sin(1.57f),
                                                   sin(1.57f), cos(1.57f))));
  d = min(d,
          (length(f32x2(length(p.swizzle<0, 2>()) - .6f, p.data[1])) - .06f));
  f = floor(po.swizzle<1, 2>() / (7.f * 2.f) - f32x2(.5f));
  r = hash12(__ctx, f) - .5f;
  if (r > -.45f)
    p = (mod(po + f32x3((iTime * .7f) * 30.f * r, 0.f, 0.f) +
                 7.f * 2.f * f32x3(r + .54f, 1.f, 1.f) / 2.f,
             7.f * 2.f * f32x3(r + .54f, 1.f, 1.f)) -
         7.f * 2.f * f32x3(r + .54f, 1.f, 1.f) / 2.f);
  else
    p = (mod(po +
                 f32x3((iTime * .7f) * 30.f * .5f *
                           (1.f +
                            r * 0.003f *
                                hash12(__ctx, floor(po.swizzle<1, 2>() * 7.f))),
                       0.f, 0.f) +
                 7.f * 2.f / 2.f,
             7.f * 2.f) -
         7.f * 2.f / 2.f);
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (f32x2x2(cos(1.57f), -sin(1.57f),
                                                   sin(1.57f), cos(1.57f))));
  f = (cos(f.swizzle<0, 1>()) * .5f + .5f) * .4f;
  return p;
}
f32x4 map(Context &__ctx, f32x3 p) {
  float d = 1e6f;
  f32x3 po, col = f32x3(0.f);
  f32x2 f;
  po = geo(__ctx, p, d, f);
  col += getLight(__ctx, po, f32x3(1.f, f));
  p.data[2] += 7.f / 2.f;
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (f32x2x2(cos(1.57f), -sin(1.57f),
                                                   sin(1.57f), cos(1.57f))));
  po = geo(__ctx, p, d, f);
  col += getLight(__ctx, po, f32x3(f.data[0], .5f, f.data[1]));
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() + (7.f / 2.f));
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (f32x2x2(cos(1.57f), -sin(1.57f),
                                                   sin(1.57f), cos(1.57f))));
  po = geo(__ctx, p, d, f);
  col += getLight(__ctx, po, f32x3(f, 1.f));
  return f32x4(col, d);
}
f32x3 getOrigin(Context &__ctx, float t) {
  t = (t + 35.f) * -.05f * 1.f;
  float rad = mix(50.f, 80.f, cos(t * 1.24f) * .5f + .5f);
  return f32x3(rad * sin(t * .97f), rad * cos(t * 1.11f), rad * sin(t * 1.27f));
}
void initRayOriginAndDirection(Context &__ctx, f32x2 uv, f32x3 &ro, f32x3 &rd) {
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>() * 2.f - 1.f;
  ro = getOrigin(__ctx, (iTime * .7f) + m.data[0] * 10.f);
  f32x3 f =
      normalize(getOrigin(__ctx, (iTime * .7f) + m.data[0] * 10.f + .5f) - ro);
  f32x3 r = normalize(cross(normalize(ro), f));
  rd = normalize(f + uv.data[0] * r + uv.data[1] * cross(f, r));
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 F) {
  f32x2 uv = (2.f * F - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 p, ro, rd, col;
  initRayOriginAndDirection(__ctx, uv, ro, rd);
  float t = 2.f;
  for (float i = 0.f; i < 50.f; i++) {
    p = ro + t * rd;
    f32x4 res = map(__ctx, p);
    col += res.swizzle<0, 1, 2>();
    t += abs(res.data[3]);
    if (abs(res.data[3]) < 0.005f)
      t += 0.005f;
    if (col.data[0] >= 1.f && col.data[1] >= 1.f && col.data[2] >= 1.f)
      break;
    if (t > 80.f)
      break;
  }
  col = pow(col, f32x3(.45f));
  O = f32x4(col, 1.0f);
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
