// Port of https://www.shadertoy.com/view/fllczs; author: oatmealine.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fllczs.json.
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
f32x4 permute(Context &__ctx, f32x4 x) {
  return mod(((x * 34.0f) + 1.0f) * x, 289.0f);
}
f32x4 taylorInvSqrt(Context &__ctx, f32x4 r) {
  return 1.79284291400159f - 0.85373472095314f * r;
}
float snoise(Context &__ctx, f32x3 v) {
  const f32x2 C = f32x2(1.0f / 6.0f, 1.0f / 3.0f);
  const f32x4 D = f32x4(0.0f, 0.5f, 1.0f, 2.0f);
  f32x3 i = floor(v + dot(v, C.swizzle<1, 1, 1>()));
  f32x3 x0 = v - i + dot(i, C.swizzle<0, 0, 0>());
  f32x3 g = step(x0.swizzle<1, 2, 0>(), x0.swizzle<0, 1, 2>());
  f32x3 l = 1.0f - g;
  f32x3 i1 = min(g.swizzle<0, 1, 2>(), l.swizzle<2, 0, 1>());
  f32x3 i2 = max(g.swizzle<0, 1, 2>(), l.swizzle<2, 0, 1>());
  f32x3 x1 = x0 - i1 + 1.0f * C.swizzle<0, 0, 0>();
  f32x3 x2 = x0 - i2 + 2.0f * C.swizzle<0, 0, 0>();
  f32x3 x3 = x0 - 1.f + 3.0f * C.swizzle<0, 0, 0>();
  i = mod(i, 289.0f);
  f32x4 p = permute(
      __ctx,
      permute(__ctx,
              permute(__ctx,
                      i.data[2] + f32x4(0.0f, i1.data[2], i2.data[2], 1.0f)) +
                  i.data[1] + f32x4(0.0f, i1.data[1], i2.data[1], 1.0f)) +
          i.data[0] + f32x4(0.0f, i1.data[0], i2.data[0], 1.0f));
  float n_ = 1.0f / 7.0f;
  f32x3 ns = n_ * D.swizzle<3, 1, 2>() - D.swizzle<0, 2, 0>();
  f32x4 j = p - 49.0f * floor(p * ns.data[2] * ns.data[2]);
  f32x4 x_ = floor(j * ns.data[2]);
  f32x4 y_ = floor(j - 7.0f * x_);
  f32x4 x = x_ * ns.data[0] + ns.swizzle<1, 1, 1, 1>();
  f32x4 y = y_ * ns.data[0] + ns.swizzle<1, 1, 1, 1>();
  f32x4 h = 1.0f - abs(x) - abs(y);
  f32x4 b0 = f32x4(x.swizzle<0, 1>(), y.swizzle<0, 1>());
  f32x4 b1 = f32x4(x.swizzle<2, 3>(), y.swizzle<2, 3>());
  f32x4 s0 = floor(b0) * 2.0f + 1.0f;
  f32x4 s1 = floor(b1) * 2.0f + 1.0f;
  f32x4 sh = -step(h, f32x4(0.0f));
  f32x4 a0 = b0.swizzle<0, 2, 1, 3>() +
             s0.swizzle<0, 2, 1, 3>() * sh.swizzle<0, 0, 1, 1>();
  f32x4 a1 = b1.swizzle<0, 2, 1, 3>() +
             s1.swizzle<0, 2, 1, 3>() * sh.swizzle<2, 2, 3, 3>();
  f32x3 p0 = f32x3(a0.swizzle<0, 1>(), h.data[0]);
  f32x3 p1 = f32x3(a0.swizzle<2, 3>(), h.data[1]);
  f32x3 p2 = f32x3(a1.swizzle<0, 1>(), h.data[2]);
  f32x3 p3 = f32x3(a1.swizzle<2, 3>(), h.data[3]);
  f32x4 norm = taylorInvSqrt(
      __ctx, f32x4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
  p0 *= norm.data[0];
  p1 *= norm.data[1];
  p2 *= norm.data[2];
  p3 *= norm.data[3];
  f32x4 m = max(
      0.6f - f32x4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0f);
  m = m * m;
  return 42.0f *
         dot(m * m, f32x4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}
float drawLine(Context &__ctx, f32x2 p1, f32x2 p2, f32x2 uv, float a) {
  float r = 0.f;
  float one_px = 0.8f / 256.0f;
  float d = distance(p1, p2);
  float duv = distance(p1, uv);
  r = 1.f - floor(1.f - (a * one_px) +
                  distance(mix(p1, p2, clamp(duv / d, 0.f, 1.f)), uv));
  return r;
}
f32x2 gridpos(Context &__ctx, int x, int y, float o) {
  float fracx = 1.0f / (256.0f / 20.0f);
  float fracy =
      1.0f / (((256.0f / iResolution.data[0]) * iResolution.data[1]) / 20.0f);
  f32x2 offset =
      f32x2(snoise(__ctx, f32x3(float(x) - floor((iTime * 0.1f) / fracx),
                                float(y) - floor((iTime * 0.05f) / fracy),
                                iTime * 1.2f)),
            snoise(__ctx, f32x3(float(x) - floor((iTime * 0.1f) / fracx),
                                float(y) - floor((iTime * 0.05f) / fracy),
                                -iTime * 1.2f)));
  return f32x2(float(x) / (256.0f / 20.0f) + mod(iTime * 0.1f, fracx),
               float(y) /
                       (((256.0f / iResolution.data[0]) * iResolution.data[1]) /
                        20.0f) +
                   mod(iTime * 0.05f, fracy)) +
         offset * o;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  uv = f32x2(floor(uv.data[0] * 256.0f) / 256.0f,
             floor(uv.data[1] *
                   ((256.0f / iResolution.data[0]) * iResolution.data[1])) /
                 ((256.0f / iResolution.data[0]) * iResolution.data[1]));
  float amp = (sin(iTime) * 0.5f + 0.5f) * 0.2f;
  float line = 0.0f;
  f32x2 lowerBound = uv - f32x2(amp * 1.1f, amp * 1.1f);
  f32x2 higherBound = uv + f32x2(amp * 1.1f, amp * 1.1f);
  int width = int(256.0f / 20.0f);
  int height =
      int(((256.0f / iResolution.data[0]) * iResolution.data[1]) / 20.0f);
  for (int x = int(lowerBound.data[0] * float(width)) - 1;
       x <= int(higherBound.data[0] * float(width)) + 1; x++) {
    for (int y = int(lowerBound.data[1] * float(height)) - 1;
         y <= int(higherBound.data[1] * float(height)) + 1; y++) {
      line += max(drawLine(__ctx, gridpos(__ctx, x, y, amp),
                           gridpos(__ctx, x + 1, y, amp), uv, 1.f),
                  0.0f);
      line += max(drawLine(__ctx, gridpos(__ctx, x, y, amp),
                           gridpos(__ctx, x, y + 1, amp), uv, 1.f),
                  0.0f);
      if (line >= 1.0f)
        break;
    }
  }
  f32x3 col = mix(f32x3(0.07f, 0.05f, 0.08f), f32x3(0.45f, 0.01f, 0.7f),
                  min(line, 1.0f));
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
