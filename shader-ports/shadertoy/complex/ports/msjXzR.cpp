// Port of https://www.shadertoy.com/view/msjXzR; author: kishimisu.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/msjXzR.json.
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
float box(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.f);
}
float rect(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.f)) + min(max(d.data[0], d.data[1]), 0.f);
}
float light(Context &__ctx, float d, float att, float n) {
  return 1.f / (1.f + pow(abs(d * att), n));
}
float map(Context &__ctx, f32x3 p, float t, f32x3 &lgt) {
  float d, b1, b2, lb2, b3, b4, r;
  f32x2 id0, id1;
  f32x3 id3, q, c = cos(p) * .25f + .75f;
  d = rect(__ctx, p.swizzle<0, 1>() + f32x2(0, 2), f32x2(1.4f, 1));
  d = min(d, abs(abs(p.data[1]) - 4.f));
  if (abs(p.data[0]) < 1.5f) {
    q = p;
    q.data[2] = (q.data[2] - .7f * clamp(round(q.data[2] / .7f),
                                         -floor(iTime * 8.f - 14.f),
                                         floor(iTime * 8.f - 14.f)));
    b1 = box(__ctx, q - f32x3(0, -1.1f, 0), f32x3(1.f, .1f, .08f));
    d = min(d, b1);
    lgt += c * f32x3(.1f, 1, .8f) * light(__ctx, b1, 60.f, 2.f);
    q = p;
    q.data[0] = abs(q.data[0]);
    q.data[2] = (mod(q.data[2] + 1.5f / 2.f, 1.5f) - 1.5f / 2.f);
    id3 = floor((p + 1.5f / 2.f) / 1.5f);
    b2 = rect(__ctx, q.swizzle<0, 2>() - f32x2(1.2f, 0), f32x2(.1f));
    q.data[2] =
        (p.data[2] - 1.5f * clamp(round(p.data[2] / 1.5f), -floor(iTime * 8.f),
                                  floor(iTime * 8.f)));
    lb2 = box(__ctx, q - f32x3(1.2f, 2.f + sin(iTime + id3.data[2]) * .5f, 0),
              f32x3(.1f, .3f, .1f));
    d = min(d, b2);
    lgt += c * f32x3(1, .7f, .4f) * light(__ctx, lb2, 80.f, 2.f) * t * .5f;
  }
  if (abs(p.data[1]) > 2.f) {
    const float br = 1.5f;
    q = p;
    id1 = floor((p.swizzle<0, 2>() + br / 2.f) / br);
    r = fract(dot(sin(id1), (id1 + 41.21f) * 24.77f));
    q.set_swizzle<0, 2>((mod(p.swizzle<0, 2>() + br / 2.f, br) - br / 2.f));
    q.data[1] = abs(q.data[1]) - 4.f;
    q.set_swizzle<0, 2>(q.swizzle<0, 2>() *
                        (f32x2x2(cos(1.570796f * (.5f + floor(r * 2.f))),
                                 -sin(1.570796f * (.5f + floor(r * 2.f))),
                                 sin(1.570796f * (.5f + floor(r * 2.f))),
                                 cos(1.570796f * (.5f + floor(r * 2.f))))));
    q.data[0] = abs(q.data[0]) - br * .35f;
    b3 = box(__ctx, q, f32x3(.01f, .2f, br * .4f));
    d = min(d, b3);
    lgt +=
        c * f32x3(.8f, .6f, .3f) * light(__ctx, b3, 20.f, 2.f) *
        exp(-t * .1f - (cos(max(0.f, iTime - 8.f) * .5f) * .5f + .5f) * 10.f);
  }
  q = p;
  id0 = floor((p.swizzle<0, 2>() + 7.f / 2.f) / 7.f);
  q.set_swizzle<0, 2>((mod(p.swizzle<0, 2>() + 7.f / 2.f, 7.f) - 7.f / 2.f));
  b3 = rect(__ctx, q.swizzle<0, 2>(), f32x2(.5f));
  b3 = max(b3, -abs(p.data[0]) + 2.f);
  q.data[1] = (mod(q.data[1] + .5f / 2.f, .5f) - .5f / 2.f);
  b4 = box(__ctx, q, f32x3(.5f, .04f, .5f));
  b4 = max(b4, -abs(p.data[0]) + 2.f);
  d = min(d, b3);
  if (fract((id0.data[0] + 41.11f) * sin(id0.data[1] * 44.7f)) <
      smoothstep(5.f, 8.f, iTime))
    lgt += c * f32x3(1, .1f, .1f) * light(__ctx, b4, 40.f, 2.f) *
           (sin(iTime +
                p.data[1] * (2.f * fract(id0.data[0] * id0.data[1] * 47.44f)) +
                id0.data[0] * id0.data[1]) *
                .5f +
            .5f);
  return d;
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 F) {
  f32x2 uv = (2.f * F - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 lgt = f32x3(0);
  f32x3 ro = f32x3(0, 0, iTime);
  f32x3 rd = normalize(f32x3(uv, 1.f));
  float t = 0.f;
  for (int i = 0; i < 40; i++) {
    f32x3 p = ro + t * rd;
    float d = map(__ctx, p, t, lgt);
    t += d;
  }
  O = f32x4(pow(lgt, f32x3(.45f)), 1.0f);
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
