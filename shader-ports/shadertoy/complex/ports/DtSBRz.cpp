// Port of https://www.shadertoy.com/view/DtSBRz; author: misol101.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtSBRz.json.
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
const int NOF_TRANSITIONS = 9;
const float EDGE_SMOOTH = 0.0f;
const bool BOUNCE_TIME = true;
float getTime(Context &__ctx, float time, float interval) {
  if (!BOUNCE_TIME)
    time = mod(time, interval);
  else {
    time = mod(time, interval * 2.f);
    if (time >= interval)
      time = max(interval * 2.f - time, 0.f);
  }
  return time;
}
const float interval = 2.0f;
const float speed = 0.33f;
float tmod(Context &__ctx, int index, f32x2 uv) {
  switch (index) {
  case 0:
    return sin(((uv.data[0] + 0.53f) / (uv.data[1] + 0.53f) * 200.f +
                uv.data[1] * 100.f) *
               0.2f) *
           0.25f;
  case 1:
    return sin(uv.data[0] * 200.f) * 0.125f + cos(uv.data[1] * 200.f) * 0.125f;
  case 2:
    return sin(uv.data[0] * 200.f * 0.166f) * 0.125f +
           cos(uv.data[1] * 200.f * 0.25f) * 0.125f;
  case 3:
    return sin((uv.data[0] * 100.f * uv.data[1] * 100.f) / 3.f) * 0.15f *
           cos((uv.data[1] * 200.f * uv.data[1] * 200.f) / 5.f) * 1.5f;
  case 4:
    return sin(uv.data[0] * 200.f) * 0.25f;
  case 5:
    return sin((uv.data[0] * 200.f + uv.data[1] * 100.f) * 0.1f) * 0.25f;
  case 6:
    return sin((uv.data[0] * 200.f - uv.data[1] * 300.f) * 0.35f) * 0.14f +
           cos((uv.data[1] * 66.f) * 0.3f) * 0.09f;
  case 7:
    return sin((uv.data[0] * 200.f + uv.data[1] * 100.f) * 0.2f) * 0.06f;
  case 8:
    return sin((uv.data[0] * 100.f * uv.data[1] * 100.f) * 0.1f) * 0.25f;
  default:
    return 0.f;
  }
}
void fx1(Context &__ctx, f32x4 &O, f32x2 u) {
  f32x2 uv = u / iResolution.swizzle<0, 1>() - .5f;
  float l = length(uv) / 3.f;
  float a = atan(uv.data[1], uv.data[0]);
  float b1 =
      sin(100.f * (sqrt(l) - 0.02f * a -
                   0.3f * ((iTime * 50.f +
                            uv.data[1] * (0.f + cos(iTime * 0.72f) * 150.9f) +
                            uv.data[0] * (0.f + cos(iTime * 0.14f) * 100.9f)) /
                           140.f))) *
      2.54f;
  float b2 = l * 2.5f;
  float g = max(b1 / 16.f, 0.09f) +
            sin((a / 2.5f + iTime / 2.5f) * 25.f) * 13.f * (b2 * b2);
  float b =
      sin((a / 5.f + iTime / 3.5f) * 25.f) * 0.71f * b2 * b2 * 42.f + 0.62f;
  f32x3 col = f32x3(b1 * b1 * b1 * 0.1f + .62f,
                    b1 / 3.5f + b1 * b * 0.2f - g / 2.f - .32f,
                    b1 / 8.f + b1 * g * 0.26f + b / 3.f + .45f);
  col = col * min(l * l * 940000.f, 1.f);
  float c1 = max(col.data[0], max(col.data[1], col.data[2]));
  c1 = c1 * c1 * 0.27f;
  O = f32x4(c1, c1, c1, 1.0f);
}
void fx2(Context &__ctx, f32x4 &O, f32x2 u) {
  f32x2 R = f32x2(0.f, iResolution.data[1] + sin(iTime / 25.0f) * 8.24f),
        p = 5.f *
            (u + u + f32x2(800.f, cos((iTime + 5.f) / 10.0f) * 1000.f) - R) /
            R.data[1],
        r = p - p;
  for (float f = 1.f; f < 2.5f; f *= 1.05f)
    r += sin(p * f + iTime * 0.85f) / f,
        p = p * f32x2x2(8, 6, -8, 6) * .1f + r * .4f;
  float l = length(r);
  f32x3 c = f32x3(l * 0.29f, l * l * 0.024f, l * l * l * 0.0016f);
  f32x2 uv = u / iResolution.swizzle<0, 1>();
  f32x3 c2 = -0.3f + 0.5f * cos(iTime + uv.swizzle<0, 1, 0>() + f32x3(0, 2, 4));
  O = f32x4(c + c2, 0.f);
}
float transition(Context &__ctx, f32x2 uv, int index, bool distinct) {
  float t = getTime(__ctx, iTime, interval / speed);
  t = mod(t * speed, interval) - 0.25f;
  t = clamp(t, 0.f, interval - 0.25f);
  float a = length(uv) + 0.25f;
  float t2 = t + tmod(__ctx, index, uv);
  if (distinct) {
    if (a >= t2 - EDGE_SMOOTH && a < t2)
      t2 = smoothstep(0.f, 1.f, (t2 - a) / EDGE_SMOOTH);
    else
      t2 = step(a, t2);
  } else
    t2 = clamp(t2 - 0.25f, 0.f, 1.f);
  return t2;
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 u) {
  f32x4 px1, px2;
  fx1(__ctx, px1, u);
  fx2(__ctx, px2, u);
  int index = (int(iTime / (interval * (BOUNCE_TIME ? 2.f : 1.f) / speed))) %
              NOF_TRANSITIONS;
  if (!BOUNCE_TIME && (index & 1) == 1) {
    f32x4 tmp = px1;
    px1 = px2;
    px2 = tmp;
  }
  f32x2 uv = u / iResolution.data[1] - .5f;
  uv.data[0] += (1.f - iResolution.data[0] / iResolution.data[1]) * 0.5f;
  float t = transition(__ctx, uv, index, iMouse.data[2] < 1.f);
  O = mix(px1, px2, t);
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
