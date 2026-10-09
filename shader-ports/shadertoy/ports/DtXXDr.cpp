// Port of https://www.shadertoy.com/view/DtXXDr; author: ArmandB.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtXXDr.json.
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

float pixelation = 7.0f;
f32x2 pixelate(Context &__ctx, f32x2 a, float k) {
  float p = pow(2.0f, k);
  return floor(a * p) / p;
}
f32x4 pixelate(Context &__ctx, f32x4 a, float k) {
  float p = pow(2.0f, k);
  return floor(a * p) / p;
}
f32x4 getcol(Context &__ctx, f32x2 p, f32x2 o) {
  return pixelate(__ctx,
                  textureLod(__ctx.channel0,
                             p + o / pow(2.0f, pixelation) + iTime * 0.005f,
                             0.0f),
                  3.0f);
}
f32x4 mincol(Context &__ctx, f32x4 a, f32x4 b) {
  float A = (0.299f * a.data[0] + 0.587f * a.data[1] + 0.114f * a.data[2]);
  float B = (0.299f * b.data[0] + 0.587f * b.data[1] + 0.114f * b.data[2]);
  return A < B ? a
               : (B < A ? b
                        : (a.data[0] + a.data[1] + a.data[2] >
                                   b.data[0] + b.data[1] + b.data[2]
                               ? a
                               : b));
}
f32x4 mincol4(Context &__ctx, f32x4 a, f32x4 b, f32x4 c, f32x4 d) {
  return mincol(__ctx, mincol(__ctx, a, b), mincol(__ctx, c, d));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.data[1] / 2.0f;
  f32x2 uvpx = pixelate(__ctx, uv, pixelation);
  float pxunit = 1.0f / pow(2.0f, pixelation);
  f32x4 c = getcol(__ctx, uvpx, f32x2(0, 0));
  f32x4 c0 = getcol(__ctx, uvpx, f32x2(1, 0));
  f32x4 c1 = getcol(__ctx, uvpx, f32x2(1, 1));
  f32x4 c2 = getcol(__ctx, uvpx, f32x2(0, 1));
  f32x4 c3 = getcol(__ctx, uvpx, f32x2(-1, 1));
  f32x4 c4 = getcol(__ctx, uvpx, f32x2(-1, 0));
  f32x4 c5 = getcol(__ctx, uvpx, f32x2(-1, -1));
  f32x4 c6 = getcol(__ctx, uvpx, f32x2(0, -1));
  f32x4 c7 = getcol(__ctx, uvpx, f32x2(1, -1));
  bool u = c2 == c;
  bool d = c6 == c;
  bool l = c4 == c;
  bool r = c0 == c;
  f32x2 relcoord = uv - uvpx - pxunit / 2.0f;
  float dist = length(relcoord);
  if (!(u || l) && relcoord.data[1] > 0.0f && relcoord.data[0] < 0.0f &&
      dist > pxunit / 2.0f) {
    c = c2 == c4 ? c2 : (c == c3 ? c : mincol4(__ctx, c, c2, c3, c4));
  } else if (!(u || r) && relcoord.data[1] > 0.0f && relcoord.data[0] > 0.0f &&
             dist > pxunit / 2.0f) {
    c = c == c1 ? c : (c2 == c0 ? c2 : mincol4(__ctx, c, c0, c1, c2));
  } else if (!(d || l) && relcoord.data[1] < 0.0f && relcoord.data[0] < 0.0f &&
             dist > pxunit / 2.0f) {
    c = c == c5 ? c : (c6 == c4 ? c6 : mincol4(__ctx, c, c4, c5, c6));
  } else if (!(d || r) && relcoord.data[1] < 0.0f && relcoord.data[0] > 0.0f &&
             dist > pxunit / 2.0f) {
    c = c6 == c0 ? c6 : (c == c7 ? c : mincol4(__ctx, c, c0, c6, c7));
  }
  fragColor = c;
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
