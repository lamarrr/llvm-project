// Port of https://www.shadertoy.com/view/ctBGzd; author: mrange.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctBGzd.json.
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
struct Context {};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
const float w_0 = 0.11f;
const float w_1 = 0.195f;
float hash(Context &__ctx, f32x2 co) {
  return fract(sin(dot(co.swizzle<0, 1>(), f32x2(12.9898f, 58.233f))) *
               13758.5453f);
}
float circle(Context &__ctx, f32x2 p, float r) { return length(p) - r; }
f32x2 mod2_1(Context &__ctx, f32x2 &p) {
  p += 0.5f;
  f32x2 c = floor(p);
  p = fract(p) - 0.5f;
  return c;
}
float df_0_0(Context &__ctx, f32x2 p) {
  float d0 = circle(__ctx, p - 0.5f, 0.5f);
  float d1 = circle(__ctx, p + 0.5f, 0.5f);
  float d = d0;
  d = min(d, d1);
  d = abs(d) - w_0;
  d = -d;
  return d;
}
float df_0_1(Context &__ctx, f32x2 p) {
  f32x2 ap = abs(p);
  float d0 = ap.data[0];
  float d1 = ap.data[1];
  float d2 = circle(__ctx, p, 2.0f * w_0);
  float d = d0;
  d = min(d, d1);
  d = min(d, d2);
  d = abs(d) - w_0;
  d = -d;
  return d;
}
float df_0_2(Context &__ctx, f32x2 p) {
  f32x2 ap = abs(p);
  float d0 = length(ap - f32x2(0.5f, 0.0f));
  float d1 = length(ap - f32x2(0.0f, 0.5f));
  float d = d0;
  d = min(d, d1);
  d = abs(d) - w_0;
  d = -d;
  return d;
}
float df_0_3(Context &__ctx, f32x2 p) {
  float d0 = length((p)-f32x2(0.5f, 0.0f));
  float d1 = length((p)-f32x2(0.0f, 0.5f));
  float d2 = circle(__ctx, p + 0.5f, 0.5f);
  float d = d0;
  d = min(d, d1);
  d = min(d, d2);
  d = abs(d) - w_0;
  d = -d;
  return d;
}
float df_0_4(Context &__ctx, f32x2 p) {
  f32x2 ap = p;
  ap.data[0] = abs(ap.data[0]);
  float d0 = length(ap - f32x2(0.5f, 0.0f));
  float d1 = ap.data[0];
  float d = d0;
  d = min(d, d1);
  d = abs(d) - w_0;
  d = -d;
  return d;
}
float df_1_0(Context &__ctx, f32x2 p) {
  f32x2 ap = abs(p);
  float d0 = circle(__ctx, p - 0.5f, 0.5f);
  float d1 = circle(__ctx, p + 0.5f, 0.5f);
  float d2 = circle(__ctx, ap - 0.5f, w_1);
  float d = d0;
  d = min(d, d1);
  d = abs(d) - w_1;
  d = min(d, d2);
  return d;
}
float df_1_1(Context &__ctx, f32x2 p) {
  f32x2 ap = abs(p);
  float d0 = circle(__ctx, ap - f32x2(0.5f, 0.0f), (0.25f));
  float d1 = circle(__ctx, ap - f32x2(0.0f, 0.5f), (0.25f));
  float d = d0;
  d = min(d, d1);
  d = abs(d) - 0.5f * w_0;
  d = -d;
  return d;
}
float df_1_2(Context &__ctx, f32x2 p) {
  f32x2 ap = (p);
  ap.data[0] = abs(p.data[0]);
  float d0 = circle(__ctx, ap - f32x2(0.5f, 0.0f), (0.25f));
  float d1 = abs(ap.data[0] - 0.25f);
  float d = d0;
  d = min(d, d1);
  d = abs(d) - 0.5f * w_0;
  d = -d;
  return d;
}
float df_1_3(Context &__ctx, f32x2 p) {
  float d0 = (circle(__ctx, p - 0.5f, 0.75f));
  float d1 = (circle(__ctx, p - f32x2(-0.5f, 0.0f), (0.25f)));
  float d2 = (circle(__ctx, p - f32x2(0.0f, -0.5f), (0.25f)));
  float d3 = abs(circle(__ctx, p - 0.5f, 0.25f)) - 0.5f * w_0;
  float d = d0;
  d = min(d, d1);
  d = min(d, d2);
  d = abs(d) - 0.5f * w_0;
  d = min(d, d3);
  d = -d;
  return d;
}
const f32x2x2 rots[4] =
    f32x2x2[4](f32x2x2(cos(0.0f * (2.0f * 3.141592654f) / 4.0f),
                       sin(0.0f * (2.0f * 3.141592654f) / 4.0f),
                       -sin(0.0f * (2.0f * 3.141592654f) / 4.0f),
                       cos(0.0f * (2.0f * 3.141592654f) / 4.0f)),
               f32x2x2(cos(1.0f * (2.0f * 3.141592654f) / 4.0f),
                       sin(1.0f * (2.0f * 3.141592654f) / 4.0f),
                       -sin(1.0f * (2.0f * 3.141592654f) / 4.0f),
                       cos(1.0f * (2.0f * 3.141592654f) / 4.0f)),
               f32x2x2(cos(2.0f * (2.0f * 3.141592654f) / 4.0f),
                       sin(2.0f * (2.0f * 3.141592654f) / 4.0f),
                       -sin(2.0f * (2.0f * 3.141592654f) / 4.0f),
                       cos(2.0f * (2.0f * 3.141592654f) / 4.0f)),
               f32x2x2(cos(3.0f * (2.0f * 3.141592654f) / 4.0f),
                       sin(3.0f * (2.0f * 3.141592654f) / 4.0f),
                       -sin(3.0f * (2.0f * 3.141592654f) / 4.0f),
                       cos(3.0f * (2.0f * 3.141592654f) / 4.0f)));
float df(Context &__ctx, f32x2 p) {
  f32x2 pp0 = p;
  f32x2 np0 = mod2_1(__ctx, pp0);
  float h0_0 = hash(__ctx, np0);
  float h1_0 = fract(8677.0f * h0_0);
  float h2_0 = fract(3677.0f * h0_0);
  if (h2_0 > 0.5f) {
    f32x2 pp1 = p * 2.0f;
    pp1 += 0.5f;
    f32x2 np1 = mod2_1(__ctx, pp1);
    float h1_0 = hash(__ctx, np1);
    float h1_1 = fract(8677.0f * h1_0);
    pp1 *= rots[int(h1_0 * 4.0f)];
    float d1 = 0.0f;
    if (h1_1 > 0.85f) {
      d1 = df_0_4(__ctx, pp1);
    } else if (h1_1 > 0.75f) {
      d1 = df_0_3(__ctx, pp1);
    } else if (h1_1 > 0.55f) {
      d1 = df_0_2(__ctx, pp1);
    } else if (h1_1 > 0.4f) {
      d1 = df_0_1(__ctx, pp1);
    } else {
      d1 = df_0_0(__ctx, pp1);
    }
    d1 *= 0.5f;
    return d1;
  }
  pp0 *= rots[int(h0_0 * 4.0f)];
  float d0 = 0.0f;
  if (h1_0 > 0.9f) {
    d0 = df_1_3(__ctx, pp0);
  } else if (h1_0 > 0.7f) {
    d0 = df_1_2(__ctx, pp0);
  } else if (h1_0 > 0.5f) {
    d0 = df_1_1(__ctx, pp0);
  } else {
    d0 = df_1_0(__ctx, pp0);
  }
  return d0;
}
f32x3 effect(Context &__ctx, f32x2 p, f32x2 pp) {
  float ll = length(pp);
  float aa = 2.0f / iResolution.data[1];
  const float sz = 0.333f;
  const float a = 10.0f;
  p += sin(0.25f * f32x2(1.0f, sqrt(0.5f)) * iTime / a) * a;
  float d = df(__ctx, p / sz) * sz;
  f32x3 col = f32x3(0.0f);
  col = mix(col, f32x3(0.5f), smoothstep(aa, -aa, d));
  col *= smoothstep(1.5f, 0.5f, ll);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 q = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 p = -1.f + 2.f * q;
  f32x2 pp = p;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 col = effect(__ctx, p, pp);
  col = sqrt(col);
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
