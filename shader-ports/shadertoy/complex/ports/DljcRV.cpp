// Port of https://www.shadertoy.com/view/DljcRV; author: 01000001.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DljcRV.json.
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
const float pi = 3.1415926f;
const float maxDist = 80.f;
const float horizon = 1.f;
const float disk = 3.f;
const float diskT = 0.05f;
f32x3 col = f32x3(0);
float stepSize;
const int maxSteps = 1024;
const float focalLength = 9.1f;
float frame(Context &__ctx, f32x3 p, f32x3 b, float e) {
  p = abs(p) - b;
  f32x3 q = abs(p + e) - e;
  return min(min(length(max(f32x3(p.data[0], q.data[1], q.data[2]), 0.0f)) +
                     min(max(p.data[0], max(q.data[1], q.data[2])), 0.0f),
                 length(max(f32x3(q.data[0], p.data[1], q.data[2]), 0.0f)) +
                     min(max(q.data[0], max(p.data[1], q.data[2])), 0.0f)),
             length(max(f32x3(q.data[0], q.data[1], p.data[2]), 0.0f)) +
                 min(max(q.data[0], max(q.data[1], p.data[2])), 0.0f));
}
float box(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
float df(Context &__ctx, f32x3 p) {
  if (max(abs(p.data[0]), max(abs(p.data[1]), abs(p.data[2]))) < 6.02f) {
    p += 2.f;
    p = mod(p, f32x3(4)) - f32x3(2);
    float df = frame(__ctx, p, f32x3(2), 0.05f);
    return df;
  } else {
    return box(__ctx, p, f32x3(6));
  }
}
const unsigned k = 1103515245U;
f32x3 hash(Context &__ctx, u32x3 x) {
  x = ((x >> 8U) ^ x.swizzle<1, 2, 0>()) * k;
  x = ((x >> 8U) ^ x.swizzle<1, 2, 0>()) * k;
  x = ((x >> 8U) ^ x.swizzle<1, 2, 0>()) * k;
  return f32x3(x) * (1.0f / float(0xffffffffU));
}
f32x2 ps(Context &__ctx, u32x3 x) {
  return hash(__ctx, x).swizzle<0, 1>() * 2.f - 1.f;
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 U) {
  f32x2 ri = iResolution.swizzle<0, 1>();
  int i = 0;
  f32x2 muv = iMouse.data[2] > 0.f
                  ? (2.f * iMouse.swizzle<0, 1>() - ri.swizzle<0, 1>()) /
                        ri.swizzle<0, 1>() * f32x2(3, 7)
                  : f32x2(.5f, 2.f);
  f32x3 dirV;
  f32x3 odir;
  float sDist = 500.f;
  int ssaa = 1;
  for (int m = 0; m < ssaa; m++) {
    f32x2 offset =
        ps(__ctx, u32x3(iFrame + m, int(U.data[1]) - m, int(U.data[0]) + m)) *
        .5f;
    float c = 1.f / (sqrt(length(offset)) + 0.3f);
    f32x2 uv = (2.f * U - ri.swizzle<0, 1>() + offset) / ri.data[1];
    f32x3 camDir = normalize(
        f32x3(sin(muv.swizzle<0, 0>() + f32x2(0, 11)), -muv.data[1] * 0.085f));
    f32x3 r = cross(camDir, f32x3(0, 0, 1));
    f32x3 u = cross(r, camDir);
    dirV = normalize(camDir * focalLength + r * uv.data[0] + u * uv.data[1]);
    odir = dirV;
    float depth = 0.f;
    f32x3 camPos = f32x3(sin(-muv.swizzle<0, 0>() - f32x2(0, 11)) * 65.f,
                         muv.data[1] * 5.f);
    f32x3 shift =
        f32x3(sin(iTime) * 4.f, cos(iTime) * 4.f, sin(iTime * pi) * 0.f);
    float g = 0.f;
    f32x3 p = camPos + shift;
    for (i = 0; i < maxSteps; i++) {
      stepSize = min(df(__ctx, p - shift), .5f);
      p += stepSize * dirV;
      depth += stepSize;
      sDist = min(sDist, length(p) * length(p));
      g = (stepSize / pow(length(p), 2.f));
      dirV = normalize(dirV + p * 0.1f * g);
      if (stepSize < 0.01f) {
        col += c * f32x3(2.f / (depth - 56.f));
        i = maxSteps;
      }
      if (depth > maxDist) {
        i = maxSteps;
      }
    }
  }
  col = col /= float(ssaa);
  O = f32x4(pow(col.data[0], 1.f / 2.2f));
  O *= 0.5f + 0.5f * sin(10.f / (sDist + (10.f / pi * .5f)) +
                         f32x4(0, 33, 11, 0) - 2.f);
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
