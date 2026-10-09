// Port of https://www.shadertoy.com/view/ssGcWz; author: ccc9527.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ssGcWz.json.
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
f32x3 DrawLine(Context &__ctx, f32x2 uv, f32x2 a, f32x2 b, float size,
               f32x3 color) {
  f32x2 ap = uv - a;
  f32x2 ab = b - a;
  float t = dot(ap, ab) / dot(ab, ab);
  t = clamp(t, 0.0f, 1.0f);
  float d = length(ap - ab * t);
  float s = smoothstep(size, size - 1.5f / iResolution.data[1], d);
  return s * color;
}
f32x3 DrawPoint(Context &__ctx, f32x2 uv, f32x2 p, float size, f32x3 color) {
  float d = length(uv - p);
  d = smoothstep(size, size - 1.5f / iResolution.data[1], d);
  return d * color;
}
f32x3 Bezier(Context &__ctx, f32x2 uv, f32x2 a, f32x2 b, f32x2 c, float size,
             f32x3 color) {
  f32x2 p1 = f32x2(0.0f);
  f32x2 p2 = f32x2(0.0f);
  f32x2 p = f32x2(0.0f);
  f32x3 col = f32x3(0.0f);
  for (float t = 0.0f; t <= 1.0f; t += 0.003f) {
    p1 = mix(a, b, t);
    p2 = mix(b, c, t);
    p = mix(p1, p2, t);
    col += DrawPoint(__ctx, uv, p, size, color);
  }
  return col;
}
f32x3 Bezier(Context &__ctx, f32x2 uv, f32x2 a, f32x2 b, f32x2 c, f32x2 d,
             float size, f32x3 color) {
  f32x2 p1 = f32x2(0.0f);
  f32x2 p2 = f32x2(0.0f);
  f32x2 p3 = f32x2(0.0f);
  f32x2 p11 = f32x2(0.0f);
  f32x2 p12 = f32x2(0.0f);
  f32x2 p = f32x2(0.0f);
  f32x3 col = f32x3(0.0f);
  for (float t = 0.0f; t <= 1.0f; t += 0.003f) {
    p1 = mix(a, b, t);
    p2 = mix(b, c, t);
    p3 = mix(c, d, t);
    p11 = mix(p1, p2, t);
    p12 = mix(p2, p3, t);
    p = mix(p11, p12, t);
    col += DrawPoint(__ctx, uv, p, size, color);
  }
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.data[1];
  float w = iResolution.data[0] / iResolution.data[1];
  f32x3 col = f32x3(0.0f);
  float size = 0.015f;
  if (false) {
    f32x2 a = f32x2(0.1f * w, 0.15f);
    f32x2 b = f32x2(0.5f * w, 0.8f);
    f32x2 c = f32x2(0.8f * w, 0.25f);
    col += DrawPoint(__ctx, uv, a, size, f32x3(1.0f));
    col += DrawPoint(__ctx, uv, b, size, f32x3(1.0f));
    col += DrawPoint(__ctx, uv, c, size, f32x3(1.0f));
    col += DrawLine(__ctx, uv, a, b, size / 3.0f, f32x3(1.0f, 0.0f, 1.0f));
    col += DrawLine(__ctx, uv, b, c, size / 3.0f, f32x3(1.0f, 0.0f, 1.0f));
    float t = fract(iTime / 5.0f);
    f32x2 p1 = mix(a, b, t);
    col += DrawPoint(__ctx, uv, p1, size, f32x3(1.0f));
    f32x2 p2 = mix(b, c, t);
    col += DrawPoint(__ctx, uv, p2, size, f32x3(1.0f));
    col += DrawLine(__ctx, uv, p1, p2, size / 3.0f, f32x3(1.0f, 1.0f, 0.0f));
    f32x2 p = mix(p1, p2, t);
    col += DrawPoint(__ctx, uv, p, size, f32x3(1.0f));
    col += Bezier(__ctx, uv, a, b, c, size / 3.0f, f32x3(1.0f));
  } else {
    f32x2 a = f32x2(0.1f * w, 0.15f);
    f32x2 b = f32x2(0.3f * w, 0.85f);
    f32x2 c = f32x2(0.65f * w, 0.8f);
    f32x2 d = f32x2(0.8f * w, 0.25f);
    col += DrawPoint(__ctx, uv, a, size, f32x3(1.0f));
    col += DrawPoint(__ctx, uv, b, size, f32x3(1.0f));
    col += DrawPoint(__ctx, uv, c, size, f32x3(1.0f));
    col += DrawPoint(__ctx, uv, d, size, f32x3(1.0f));
    col += DrawLine(__ctx, uv, a, b, size / 3.0f, f32x3(1.0f, 0.0f, 1.0f));
    col += DrawLine(__ctx, uv, b, c, size / 3.0f, f32x3(1.0f, 0.0f, 1.0f));
    col += DrawLine(__ctx, uv, c, d, size / 3.0f, f32x3(1.0f, 0.0f, 1.0f));
    float t = fract(iTime / 5.0f);
    f32x2 p1 = mix(a, b, t);
    col += DrawPoint(__ctx, uv, p1, size, f32x3(1.0f));
    f32x2 p2 = mix(b, c, t);
    col += DrawPoint(__ctx, uv, p2, size, f32x3(1.0f));
    f32x2 p3 = mix(c, d, t);
    col += DrawPoint(__ctx, uv, p3, size, f32x3(1.0f));
    col += DrawLine(__ctx, uv, p1, p2, size / 3.0f, f32x3(1.0f, 1.0f, 0.0f));
    col += DrawLine(__ctx, uv, p2, p3, size / 3.0f, f32x3(1.0f, 1.0f, 0.0f));
    f32x2 p11 = mix(p1, p2, t);
    col += DrawPoint(__ctx, uv, p11, size, f32x3(1.0f));
    f32x2 p12 = mix(p2, p3, t);
    col += DrawPoint(__ctx, uv, p12, size, f32x3(1.0f));
    col += DrawLine(__ctx, uv, p11, p12, size / 3.0f, f32x3(0.0f, 1.0f, 1.0f));
    f32x2 p = mix(p11, p12, t);
    col += DrawPoint(__ctx, uv, p, size, f32x3(1.0f));
    col += Bezier(__ctx, uv, a, b, c, d, size / 3.0f, f32x3(1.0f));
  }
  col = pow(col, f32x3(1.0f / 2.2f));
  fragColor = f32x4(col, 1.0f);
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
