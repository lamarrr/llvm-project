// Port of https://www.shadertoy.com/view/7tdBz7; author: PiGIon.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7tdBz7.json.
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
float antiAlias;
f32x2 rotAround(Context &__ctx, f32x2 p, f32x2 o, float ang);
f32x4 circle(Context &__ctx, f32x2 uv, f32x2 origin, float size, f32x3 col) {
  return f32x4(
      col, smoothstep(size + antiAlias, size - antiAlias, length(uv - origin)));
}
f32x4 eye(Context &__ctx, f32x2 uv, f32x2 origin, float size, float ang) {
  f32x4 eyeBall = circle(__ctx, uv, origin, size, f32x3(1.f));
  f32x2 pupilO = f32x2(origin.data[0] + size / 2.f, origin.data[1]);
  pupilO = rotAround(__ctx, origin, pupilO, ang);
  f32x4 pupil = circle(__ctx, uv, pupilO, size / 2.f, f32x3(0.f));
  return mix(eyeBall, pupil, pupil.data[3]);
}
f32x2x2 rot2D(Context &__ctx, float ang) {
  return f32x2x2(cos(ang), -sin(ang), sin(ang), cos(ang));
}
f32x2 rotAround(Context &__ctx, f32x2 p, f32x2 o, float ang) {
  f32x2 v = (o - p);
  return (rot2D(__ctx, ang) * v) + p;
}
float angle(Context &__ctx, f32x2 v) { return -atan(v.data[1] / v.data[0]); }
f32x3 rgb(Context &__ctx, int r, int g, int b) {
  return f32x3(float(r) / 255.f, float(g) / 255.f, float(b) / 255.f);
}
float cycle(Context &__ctx, float perSecond) {
  float d = fract((iTime + iTimeDelta) * perSecond);
  return 2.f * (d > .5f ? 1.f - d : d);
}
float rangeCycle(Context &__ctx, float size, float perSec) {
  return mod(iTime * perSec - size, size) - .5f * size;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (2.f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  antiAlias = 1.5f / iResolution.data[1];
  f32x3 col = f32x3(0.f);
  float size = floor(50.f + (200.f * cycle(__ctx, 0.1f)));
  float thick = 0.15f + (0.25f * cycle(__ctx, 0.2f));
  f32x2 o = f32x2(-1.3f, 0.f);
  f32x3 initCol = rgb(__ctx, 186, 98, 245);
  f32x3 destCol = rgb(__ctx, 228, 237, 62);
  f32x3 incCol = (destCol - initCol) / size;
  f32x2 lastO;
  for (float i = 1.f; i <= size; i += 1.f) {
    lastO = f32x2(o);
    o.data[0] += 0.01f;
    o.data[1] = cos(o.data[0] + iTime * 5.12f) / (0.5f / thick);
    initCol += incCol;
    f32x4 circleColor = circle(__ctx, uv, o, thick, initCol);
    col = mix(col, circleColor.swizzle<0, 1, 2>(), circleColor.data[3]);
    if (i == size) {
      float eyeSize = thick / 4.f;
      f32x2 eyeVec = f32x2(eyeSize, eyeSize * 2.f);
      f32x2 angVec = o - lastO;
      float ang = angle(__ctx, angVec);
      f32x2 lEyeO = o + eyeVec;
      lEyeO = rotAround(__ctx, o, lEyeO, ang);
      f32x4 lEye = eye(__ctx, uv, lEyeO, eyeSize, ang);
      col = mix(col, lEye.swizzle<0, 1, 2>(), lEye.data[3]);
      eyeVec.data[1] = -eyeVec.data[1];
      f32x2 rEyeO = o + eyeVec;
      rEyeO = rotAround(__ctx, o, rEyeO, ang);
      f32x4 rEye = eye(__ctx, uv, rEyeO, eyeSize, ang);
      col = mix(col, rEye.swizzle<0, 1, 2>(), rEye.data[3]);
    }
  }
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
