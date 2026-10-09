// Port of https://www.shadertoy.com/view/DtX3Dl; author: raymarchingenthusiast.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtX3Dl.json.
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

struct Vert {
  f32x3 col;
};
struct Triangle2 {
  f32x2 a;
  f32x2 b;
  f32x2 c;
};
struct Triangle3 {
  f32x3 a;
  f32x3 b;
  f32x3 c;
};
f32x2 CalcUv(Context &__ctx, f32x2 p, Triangle2 tp, Triangle2 tuv) {
  float w1p1 =
      (tp.data[2].data[1] - tp.c.data[1]) * (p.data[0] - tp.c.data[0]) +
      (tp.c.data[0] - tp.data[2].data[0]) * (p.data[1] - tp.c.data[1]);
  float w1p2 =
      (tp.data[2].data[1] - tp.c.data[1]) *
          (tp.data[3].data[0] - tp.c.data[0]) +
      (tp.c.data[0] - tp.data[2].data[0]) * (tp.data[3].data[1] - tp.c.data[1]);
  float w1 = w1p1 / w1p2;
  float w2p1 =
      (tp.c.data[1] - tp.data[3].data[1]) * (p.data[0] - tp.c.data[0]) +
      (tp.data[3].data[0] - tp.c.data[0]) * (p.data[1] - tp.c.data[1]);
  float w2p2 =
      (tp.data[2].data[1] - tp.c.data[1]) *
          (tp.data[3].data[0] - tp.c.data[0]) +
      (tp.c.data[0] - tp.data[2].data[0]) * (tp.data[3].data[1] - tp.c.data[1]);
  float w2 = w2p1 / w2p2;
  float w3 = 1.f - w1 - w2;
  return w1 * tuv.data[3] + w2 * tuv.data[2] + w3 * tuv.c;
}
float area(Context &__ctx, Triangle2 tri) {
  return (tri.data[3].data[0] * (tri.data[2].data[1] - tri.c.data[1]) +
          tri.data[2].data[0] * (tri.c.data[1] - tri.data[3].data[1]) +
          tri.c.data[0] * (tri.data[3].data[1] - tri.data[2].data[1])) /
         2.f;
}
bool isInside(Context &__ctx, f32x2 p, Triangle2 tri) {
  float s =
      (tri.data[3].data[0] - tri.c.data[0]) * (p.data[1] - tri.c.data[1]) -
      (tri.data[3].data[1] - tri.c.data[1]) * (p.data[0] - tri.c.data[0]);
  float t = (tri.data[2].data[0] - tri.data[3].data[0]) *
                (p.data[1] - tri.data[3].data[1]) -
            (tri.data[2].data[1] - tri.data[3].data[1]) *
                (p.data[0] - tri.data[3].data[0]);
  if ((s < 0.f) != (t < 0.f) && s != 0.f && t != 0.f)
    return false;
  float d =
      (tri.c.data[0] - tri.data[2].data[0]) *
          (p.data[1] - tri.data[2].data[1]) -
      (tri.c.data[1] - tri.data[2].data[1]) * (p.data[0] - tri.data[2].data[0]);
  return d == 0.f || (d < 0.f) == (s + t <= 0.f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord +
              (iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>() / 2.f)) /
             iResolution.swizzle<0, 1>();
  if (iMouse.swizzle<0, 1>() == f32x2(0.f))
    uv = fragCoord / iResolution.swizzle<0, 1>();
  Triangle2 ts[2];
  Triangle2 uvs[2];
  ts[0] = Triangle2(f32x2(0.f, 0.f), f32x2(0.f, 1.f), f32x2(1.f, 0.f));
  uvs[0] = Triangle2(f32x2(0.f, 0.f), f32x2(0.f, 1.f), f32x2(1.f, 0.f));
  ts[1] = Triangle2(f32x2(1.f, 0.f), f32x2(1.f, 1.f), f32x2(0.f, 1.f));
  uvs[1] = Triangle2(f32x2(1.f, 0.f), f32x2(1.f, 1.f), f32x2(0.f, 1.f));
  fragColor = f32x4(0.f, 0.f, .3f, 1.f);
  for (int i = 0; i < ts.length(); i++) {
    if (isInside(__ctx, uv, ts[i])) {
      f32x2 textureUv = CalcUv(__ctx, uv, ts[i], uvs[i]);
      f32x3 col = texture(__ctx.channel0, textureUv).swizzle<0, 1, 2>();
      fragColor = f32x4(col, 1.f);
      if (textureUv.data[0] > 1.f || textureUv.data[0] < 0.f ||
          textureUv.data[1] > 1.f || textureUv.data[1] < 0.f)
        fragColor = f32x4(.15f, .15f, 3.f, 1.f);
    }
  }
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
