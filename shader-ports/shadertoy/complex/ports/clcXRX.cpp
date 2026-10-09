// Port of https://www.shadertoy.com/view/clcXRX; author: chenxianming.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/clcXRX.json.
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
float smin(Context &__ctx, float a, float b, float r) {
  float h = clamp(.5f + .5f * (b - a) / r, 0.f, 1.f);
  return mix(b, a, h) - r * h * (1.f - h);
}
f32x2x2 rotation(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, s, -s, c);
}
float deg2Rad(Context &__ctx, float deg) { return radians(deg); }
float calcSquare(Context &__ctx, f32x3 pos) {
  f32x2 uv = pos.swizzle<0, 1>();
  uv *= rotation(__ctx, fract(iTime * .05f) * 3.1415926f * 4.f);
  uv.data[0] += cos(pos.data[2]) * .3f;
  uv.data[1] += sin(pos.data[2]) * .3f;
  float p = abs(uv.data[0]) - .05f;
  p = min(-p, -(abs(uv.data[1]) - .05f));
  p *= -1.f;
  uv *= 1.5f;
  float s2 = abs(uv.data[0]) - .05f;
  s2 = min(-s2, -(abs(uv.data[1]) - .05f));
  s2 *= -1.f;
  p = max(p, -s2);
  return max(p, abs(pos.data[2]) - 11.5f);
}
float map(Context &__ctx, f32x3 p) {
  float d = 5.0f;
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (rotation(__ctx, 3.1415926f * .05f)));
  p.data[2] += fract(iTime * .05f) * 5.f;
  p.data[2] = p.data[2] - 5.f * clamp(round(p.data[2] / 5.f), -1.f, 1.f);
  float count = 10.f;
  for (float i = 0.f; i < count; i++) {
    f32x3 pp = p;
    pp.set_swizzle<0, 1>(pp.swizzle<0, 1>() *
                         (rotation(__ctx, deg2Rad(__ctx, (360.f / count) * i) +
                                              3.1415926f * .5f)));
    d = smin(__ctx, d, calcSquare(__ctx, pp), .12f);
  }
  return d;
}
f32x3 calcNormal(Context &__ctx, f32x3 pos) {
  f32x2 e = f32x2(1.0f, -1.0f) * 0.5773f;
  const float eps = 0.0005f;
  return normalize(
      e.swizzle<0, 1, 1>() * map(__ctx, pos + e.swizzle<0, 1, 1>() * eps) +
      e.swizzle<1, 1, 0>() * map(__ctx, pos + e.swizzle<1, 1, 0>() * eps) +
      e.swizzle<1, 0, 1>() * map(__ctx, pos + e.swizzle<1, 0, 1>() * eps) +
      e.swizzle<0, 0, 0>() * map(__ctx, pos + e.swizzle<0, 0, 0>() * eps));
}
f32x3 raymarching(Context &__ctx, f32x3 ro, f32x3 rd) {
  float t = 0.f, dist = 0.f;
  f32x3 lig = normalize(f32x3(.57703f));
  lig.set_swizzle<0, 2>(
      lig.swizzle<0, 2>() *
      (rotation(__ctx, fract(iTime * .05f) * 3.1415926f * 4.f)));
  f32x3 hal = normalize(lig - rd);
  f32x3 pos = f32x3(0.f);
  for (int i = 0; i < 32; i++) {
    pos = ro + rd * t;
    dist = map(__ctx, pos);
    if (t > 1e3f) {
      break;
    }
    t += dist * .777778f;
  }
  if (dist < 1e-3f) {
    f32x3 nor = calcNormal(__ctx, pos);
    float dif = clamp(dot(nor, f32x3(.4f, 1.f, -.5f)), 0.0f, 1.0f);
    float spe = pow(clamp(dot(nor, hal), 0.0f, 1.0f), 50.0f) * .6f;
    spe *= dif;
    float amb = dot(nor, f32x3(0.f, -0.3f, 0.f));
    return sqrt(spe + f32x3(.9f) * amb + f32x3(.9f) * dif - f32x3(.2f));
  }
  return f32x3(0.f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  uv = (uv - .5f) * 2.f;
  uv.data[0] *= iResolution.data[0] / -iResolution.data[1];
  f32x3 col = f32x3(0.f);
  f32x3 ro = f32x3(0.f, 0.f, -0.5f);
  f32x3 rd = f32x3(uv, 2.f);
  col = raymarching(__ctx, ro, rd);
  col = mix(col, f32x3(0.f),
            1.f - smoothstep(0.f, .3f, length(uv - f32x2(0.f, -.25f)) - .1f));
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
