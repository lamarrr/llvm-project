// Port of https://www.shadertoy.com/view/7stcR4; author: iq.
// Dataset license metadata: mit; original retained in upstream/7stcR4.json.
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
float sdSquircle(Context &__ctx, f32x2 p, float n) {
  p = abs(p);
  if (p.data[1] > p.data[0])
    p = p.swizzle<1, 0>();
  n = 2.0f / n;
  float xa = 0.0f, xb = 6.283185f / 8.0f;
  for (int i = 0; i < 6; i++) {
    float x = 0.5f * (xa + xb);
    float c = cos(x);
    float s = sin(x);
    float cn = pow(c, n);
    float sn = pow(s, n);
    float y = (p.data[0] - cn) * cn * s * s - (p.data[1] - sn) * sn * c * c;
    if (y < 0.0f)
      xa = x;
    else
      xb = x;
  }
  f32x2 qa = pow(f32x2(cos(xa), sin(xa)), f32x2(n));
  f32x2 qb = pow(f32x2(cos(xb), sin(xb)), f32x2(n));
  f32x2 pa = p - qa, ba = qb - qa;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h) *
         sign(pa.data[0] * ba.data[1] - pa.data[1] * ba.data[0]);
}
float approx_sdSquircle(Context &__ctx, f32x2 p, float n) {
  p = abs(p);
  if (p.data[1] > p.data[0])
    p = p.swizzle<1, 0>();
  float w = pow(p.data[0], n) + pow(p.data[1], n);
  float b = 2.0f * n - 2.0f;
  float a = 1.0f - 1.0f / n;
  return (w - pow(w, a)) * inverse_sqrt(pow(p.data[0], b) + pow(p.data[1], b));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = (2.0f * iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
            iResolution.data[1];
  p *= 1.4f;
  m *= 1.4f;
  float n = 3.0f + 2.5f * sin(6.283185f * iTime / 3.0f);
  float d = (p.data[1] < p.data[0]) ? sdSquircle(__ctx, p, n)
                                    : approx_sdSquircle(__ctx, p, n);
  f32x3 col = (d > 0.0f) ? f32x3(0.9f, 0.6f, 0.3f) : f32x3(0.65f, 0.85f, 1.0f);
  col *= 1.0f - exp(-8.0f * abs(d));
  col *= 0.6f + 0.4f * smoothstep(-0.5f, 0.5f, cos(90.0f * abs(d)));
  col = mix(col, f32x3(1.0f), 1.0f - smoothstep(0.0f, 0.015f, abs(d)));
  if (iMouse.data[2] > 0.001f) {
    d = (m.data[1] < m.data[0]) ? sdSquircle(__ctx, m, n)
                                : approx_sdSquircle(__ctx, m, n);
    col = mix(
        col, f32x3(1.0f, 1.0f, 0.0f),
        1.0f - smoothstep(0.0f, 0.005f, abs(length(p - m) - abs(d)) - 0.0025f));
    col = mix(col, f32x3(1.0f, 1.0f, 0.0f),
              1.0f - smoothstep(0.0f, 0.005f, length(p - m) - 0.015f));
  }
  col *= smoothstep(0.005f, 0.010f, abs(p.data[1] - p.data[0]));
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
