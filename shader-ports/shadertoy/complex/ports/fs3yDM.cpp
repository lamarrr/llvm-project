// Port of https://www.shadertoy.com/view/fs3yDM; author: iq.
// Dataset license metadata: mit; original retained in upstream/fs3yDM.json.
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
f32x3 sdSquircle(Context &__ctx, f32x2 p, float n) {
  f32x2 k = sign(p);
  p = abs(p);
  bool m = p.data[1] > p.data[0];
  if (m)
    p = p.swizzle<1, 0>();
  const int num = 16;
  float s = 1.0f;
  float d = 1e20f;
  f32x2 oq = f32x2(1.0f, 0.0f);
  f32x2 g = f32x2(0.0f, 0.0f);
  for (int i = 1; i <= num; i++) {
    float h = (6.283185f / 8.0f) * float(i) / float(num);
    f32x2 q = pow(f32x2(cos(h), sin(h)), f32x2(2.0f / n));
    f32x2 pa = p - oq;
    f32x2 ba = q - oq;
    f32x2 z = pa - ba * clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
    float d2 = dot(z, z);
    if (d2 < d) {
      d = d2;
      s = pa.data[0] * ba.data[1] - pa.data[1] * ba.data[0];
      g = z;
    }
    oq = q;
  }
  if (m)
    g = g.swizzle<1, 0>();
  g *= k;
  d = sign(s) * sqrt(d);
  return f32x3(d, g / d);
}
float incorrect_sdSquircle(Context &__ctx, f32x2 p, float n) {
  return pow(pow(abs(p.data[0]), n) + pow(abs(p.data[1]), n), 1.0f / n) - 1.0f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = (2.0f * iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
            iResolution.data[1];
  p *= 1.4f;
  m *= 1.4f;
  float n = 3.0f + 2.5f * sin(6.283185f * iTime / 3.0f);
  f32x3 dg = sdSquircle(__ctx, p, n);
  float d = dg.data[0];
  f32x2 g = dg.swizzle<1, 2>();
  f32x3 col = (d > 0.0f) ? f32x3(0.9f, 0.6f, 0.3f) : f32x3(0.4f, 0.7f, 0.85f);
  col *= 1.0f + f32x3(0.5f * g, 0.0f);
  col *= 1.0f - 0.5f * exp(-8.0f * abs(d));
  col *= 0.9f + 0.1f * cos(90.0f * d);
  col = mix(col, f32x3(1.0f), 1.0f - smoothstep(0.0f, 0.015f, abs(d)));
  if (iMouse.data[2] > 0.001f) {
    d = sdSquircle(__ctx, m, n).data[0];
    col = mix(
        col, f32x3(1.0f, 1.0f, 0.0f),
        1.0f - smoothstep(0.0f, 0.010f, abs(length(p - m) - abs(d)) - 0.0025f));
    col = mix(col, f32x3(1.0f, 1.0f, 0.0f),
              1.0f - smoothstep(0.0f, 0.010f, length(p - m) - 0.015f));
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
