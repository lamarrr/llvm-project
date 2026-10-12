// Port of https://www.shadertoy.com/view/dtGyWw; author: fishy.
// Dataset license metadata: mit; original retained in upstream/dtGyWw.json.
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
float sdSegment(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 ba = b - a;
  f32x2 pa = p - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - h * ba);
}
float sdEllipse(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  float la = length(a);
  float lb = length(b);
  p *= f32x2x2(a / la, b / lb);
  f32x2 ab = f32x2(la, lb);
  p = abs(p);
  f32x2 q = ab * (p - ab);
  float w = (q.data[0] < q.data[1]) ? 1.570796327f : 0.0f;
  for (int i = 0; i < 4; i++) {
    f32x2 cs = f32x2(cos(w), sin(w));
    f32x2 u = ab * f32x2(cs.data[0], cs.data[1]);
    f32x2 v = ab * f32x2(-cs.data[1], cs.data[0]);
    w = w + dot(p - u, v) / (dot(p - u, u) + dot(v, v));
  }
  float d = length(p - ab * f32x2(cos(w), sin(w)));
  return (dot(p / ab, p / ab) > 1.0f) ? d : -d;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = (2.0f * iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
            iResolution.data[1];
  f32x2 a = f32x2(sin(iTime * 2.5f) * 0.1f + 0.5f, 0);
  f32x2 b = f32x2(0, sin(iTime * 1.5f) * 0.1f + 0.5f);
  a *= f32x2x2(cos(iTime * 0.25f), -sin(iTime * 0.25f), sin(iTime * 0.25f),
               cos(iTime * 0.25f));
  b *= f32x2x2(cos(iTime * 0.25f), -sin(iTime * 0.25f), sin(iTime * 0.25f),
               cos(iTime * 0.25f));
  if (iMouse.data[2] > 0.001f) {
    a = m;
    b = (normalize(a) * (sin(iTime * 1.5f) * 0.1f + 0.5f)) *
        f32x2x2(cos(1.57f), -sin(1.57f), sin(1.57f), cos(1.57f));
  }
  float d = sdEllipse(__ctx, p, a, b);
  f32x3 col = f32x3(1.0f) - sign(d) * f32x3(0.1f, 0.4f, 0.7f);
  col *= 1.0f - exp(-2.0f * abs(d));
  col *= 0.8f + 0.2f * cos(120.0f * d);
  col = mix(col, f32x3(1.0f), 1.0f - smoothstep(0.0f, 0.01f, abs(d)));
  col = mix(col, f32x3(1.0f, 1.0f, 0.0f),
            1.0f - smoothstep(0.0f, 0.01f, sdSegment(__ctx, p, f32x2(0), a)));
  col = mix(col, f32x3(0.0f, 1.0f, 1.0f),
            1.0f - smoothstep(0.0f, 0.01f, sdSegment(__ctx, p, f32x2(0), b)));
  fragColor = f32x4(col, 1.0f);
  ;
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
