// Port of https://www.shadertoy.com/view/ctXXR2; author: Meleshin.
// Dataset license metadata: mit; original retained in upstream/ctXXR2.json.
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
f32x3 ro0;
f32x3 ro;
struct varsStruct {
  float iTime;
  float eA;
  float eB;
  float pA;
  float pD;
  float it;
  float s0;
  float pB;
  float pC;
  f32x3 colBase;
  float colFactA;
  float colFactB;
  float fadeIn;
  float fadeOut;
  float fade;
  float clipTime;
};
varsStruct varsSeq[] = varsStruct[](
    varsStruct(0.f, -2.5f, 30.f, -1.5f, -0.7f, 8.f, 2.f, 1.f, 1.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(23.75f, -2.5f, 30.f, -1.5f, -0.7f, 8.f, 2.f, 1.f, 1.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(23.75f, -2.5f, 30.f, -1.5f, -0.7f, 8.f, 2.f, 0.f, 0.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(47.75f, -2.5f, 30.f, -1.5f, -0.7f, 8.f, 2.f, 0.f, 0.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(47.75f, -3.8f, 20.f, -1.f, -0.7f, 8.f, 2.f, 1.f, 1.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(71.75f, -3.8f, 20.f, -1.f, -0.7f, 8.f, 2.f, 1.f, 1.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(71.75f, -2.f, 25.f, -1.9f, -0.7f, 12.f, 2.5f, 1.f, 1.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(95.75f, -2.f, 25.f, -1.9f, -0.7f, 12.f, 2.5f, 1.f, 1.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(95.75f, -2.5f, 20.f, -1.5f, -0.8f, 8.f, 2.f, 1.f, 1.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(119.75f, -2.5f, 20.f, -1.5f, -0.8f, 8.f, 2.f, 1.f, 1.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(119.75f, -2.5f, 20.f, -1.5f, -0.8f, 10.f, 2.f, 1.f, 1.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(143.75f, -2.5f, 20.f, -1.5f, -0.8f, 10.f, 2.f, 1.f, 1.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(143.75f, -2.5f, 20.f, -1.5f, -0.8f, 8.f, 2.f, 0.f, 0.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(167.75f, -2.5f, 20.f, -1.5f, -0.8f, 8.f, 2.f, 0.f, 0.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(167.75f, 1.9f, 11.f, -1.7f, 0.7f, 10.f, 2.f, 0.f, 0.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(191.75f, 1.9f, 11.f, -1.7f, 0.7f, 10.f, 2.f, 0.f, 0.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(191.75f, 1.9f, 8.f, 1.7f, 0.7f, 8.f, 3.f, 0.f, 0.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(215.75f, 1.9f, 8.f, 1.7f, 0.7f, 8.f, 3.f, 0.f, 0.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(215.75f, -2.5f, 30.f, 1.5f, -0.7f, 6.f, 3.f, 1.f, 1.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(233.75f, -2.5f, 30.f, 1.5f, -0.7f, 6.f, 3.f, 1.f, 1.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(233.75f, -2.4f, 20.f, -1.5f, -0.62f, 6.f, 3.f, 1.f, 1.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(251.75f, -2.4f, 20.f, -1.5f, -0.62f, 6.f, 3.f, 1.f, 1.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(251.75f, -2.3f, 30.f, -1.5f, -0.7f, 8.f, 2.f, 0.f, 0.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(275.75f, -2.3f, 30.f, -1.5f, -0.7f, 8.f, 2.f, 0.f, 0.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(275.75f, -2.5f, 30.f, -1.5f, -0.7f, 10.f, 2.f, 0.f, 0.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(299.75f, -2.5f, 30.f, -1.5f, -0.7f, 10.f, 2.f, 0.f, 0.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(299.75f, -3.8f, 20.f, -1.f, -0.7f, 10.f, 2.f, 1.f, 1.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(323.75f, -3.8f, 20.f, -1.f, -0.7f, 10.f, 2.f, 1.f, 1.f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(323.75f, -2.5f, 30.f, -1.5f, -0.7f, 8.f, 2.f, 0.25f, 0.25f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(347.75f, -2.5f, 30.f, -1.5f, -0.7f, 8.f, 2.f, 0.25f, 0.25f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(347.75f, -2.5f, 30.f, -1.5f, -0.7f, 6.f, 2.f, 0.25f, 0.25f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(365.75f, -2.5f, 30.f, -1.5f, -0.7f, 6.f, 2.f, 0.25f, 0.25f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(365.75f, -2.5f, 30.f, -1.5f, -0.7f, 6.f, 1.f, 0.25f, 0.25f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(389.75f, -2.5f, 30.f, -1.5f, -0.7f, 6.f, 1.f, 0.25f, 0.25f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(389.75f, -2.5f, 30.f, -1.5f, -0.7f, 8.f, 1.f, 0.25f, 0.25f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(413.75f, -2.5f, 30.f, -1.5f, -0.7f, 8.f, 1.f, 0.25f, 0.25f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(413.75f, -2.5f, 20.f, -2.f, -0.7f, 8.f, 1.f, 0.25f, 0.25f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(437.75f, -2.5f, 20.f, -2.f, -0.7f, 8.f, 1.f, 0.25f, 0.25f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(437.75f, -2.4f, 20.f, -2.f, -0.7f, 8.f, 1.f, 0.25f, 0.25f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(461.75f, -2.4f, 20.f, -2.f, -0.7f, 8.f, 1.f, 0.25f, 0.25f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(461.75f, -2.4f, 30.f, -2.f, -0.7f, 8.f, 1.f, 0.25f, 0.25f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(485.75f, -2.4f, 30.f, -2.f, -0.7f, 8.f, 1.f, 0.25f, 0.25f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 6.f, 1.f, 0.f),
    varsStruct(485.75f, -2.4f, 30.f, -2.f, -0.6f, 8.f, 1.f, 0.25f, 0.25f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 24.f, 1.f, 0.f),
    varsStruct(528.f, -2.4f, 30.f, -2.f, -0.6f, 8.f, 1.f, 0.25f, 0.25f,
               f32x3(0.2f, 0.9f, 0.3f), 3.f, 0.00005f, 3.f, 24.f, 1.f, 0.f));
varsStruct vars;
void varsInit(Context &__ctx) {
  const float beatTime = 60.f / 210.f;
  float loopLength = varsSeq[varsSeq.length() - 1].iTime * beatTime;
  float loopT = mod(iTime, loopLength);
  float clipT;
  float clipL;
  varsStruct a;
  varsStruct b;
  float t;
  for (int i = 0; i < varsSeq.length() - 1; i++) {
    float sTimeNext = varsSeq[i + 1].iTime * beatTime;
    if (sTimeNext > loopT) {
      float sTime = varsSeq[i].iTime * beatTime;
      a = varsSeq[i];
      b = varsSeq[i + 1];
      clipT = loopT - sTime;
      clipL = sTimeNext - sTime;
      t = clipT / clipL;
      break;
    }
  }
  vars.iTime = loopT;
  vars.clipTime = t;
  vars.eA = mix(a.eA, b.eA, t);
  vars.eB = mix(a.eB, b.eB, t);
  vars.pA = mix(a.pA, b.pA, t);
  vars.pD = mix(a.pD, b.pD, t);
  vars.it = mix(a.it, b.it, t);
  vars.s0 = mix(a.s0, b.s0, t);
  vars.pB = mix(a.pB, b.pB, t);
  vars.pC = mix(a.pC, b.pC, t);
  vars.colBase = mix(a.colBase, b.colBase, t);
  vars.colFactA = mix(a.colFactA, b.colFactA, t);
  vars.colFactB = mix(a.colFactB, b.colFactB, t);
  vars.fade = 1.f;
  if (a.fadeIn != 0.f) {
    vars.fade *= clamp(clipT / (a.fadeIn * beatTime), 0.f, 1.f);
  }
  if (a.fadeOut != 0.f) {
    vars.fade *= clamp((clipL - clipT) / (a.fadeOut * beatTime), 0.f, 1.f);
  }
  vars.clipTime = mix(a.clipTime, b.clipTime, t);
}
f32x3x3 rotMat(Context &__ctx, f32x3 v, float c, float s) {
  float k = (1.f - c);
  return f32x3x3(
      k * v.data[0] * v.data[0] + c, k * v.data[0] * v.data[1] - s * v.data[2],
      k * v.data[0] * v.data[2] + s * v.data[1],
      k * v.data[0] * v.data[1] + s * v.data[2], k * v.data[1] * v.data[1] + c,
      k * v.data[1] * v.data[2] - s * v.data[0],
      k * v.data[0] * v.data[2] - s * v.data[1],
      k * v.data[1] * v.data[2] + s * v.data[0], k * v.data[2] * v.data[2] + c);
}
f32x3x3 rotMat(Context &__ctx, f32x3 v, float angle) {
  return rotMat(__ctx, v, cos(angle), sin(angle));
}
f32x3x3 rotMat(Context &__ctx, f32x3 z, f32x3 d) {
  f32x3 cr = cross(z, d);
  return rotMat(__ctx, cr, dot(z, d), length(cr));
}
f32x3 opRepeat(Context &__ctx, f32x3 p, float repW) {
  float repWD = repW / 2.f;
  return mod(p + repWD, repW) - repWD;
}
void tune(Context &__ctx) {
  if (vars.eA > 0.f) {
    vars.pB = abs(vars.pB);
  } else if (vars.eA < 0.f) {
    vars.it = ceil(vars.it / 2.f) * 2.f;
  }
}
f32x4 deFractal(Context &__ctx, f32x3 p) {
  float s = vars.s0;
  p = abs(p);
  p += vars.pA;
  for (float i = 0.f; i < vars.it; i++) {
    p -= vars.pB;
    p = abs(p);
    p = vars.pC - p;
    float e = vars.eA / min(dot(p, p), vars.eB);
    s *= e;
    p = p * e + vars.pD;
  }
  float dist = abs(p.data[2]) / s;
  dist += .001f;
  f32x3 col = vars.colBase * 10.f + log(s * vars.colFactA);
  col = vars.colFactB * abs(cos(col)) / dot(p, p) / dist;
  return f32x4(col, dist);
}
f32x4 map(Context &__ctx, f32x3 p) {
  p -= ro0;
  p = opRepeat(__ctx, p, 2.f);
  return deFractal(__ctx, p);
}
f32x3 march(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x4 res = f32x4(0.f);
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + res.data[3] * rd;
    f32x4 m = map(__ctx, p);
    res += m;
  }
  return res.swizzle<0, 1, 2>();
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  varsInit(__ctx);
  const float beatTime = 60.f / 210.f;
  const float barTime = 3.f * beatTime;
  tune(__ctx);
  const float foc = 1.f;
  ro0 = f32x3(0, 0, 0);
  ro = ro0 + f32x3(0, 0, iTime / barTime / 4.f);
  f32x3 col = f32x3(0.f);
  f32x2 uv = (fragCoord.swizzle<0, 1>() - .5f * iResolution.swizzle<0, 1>()) /
             iResolution.data[1];
  f32x3 rd = normalize(f32x3(uv.data[0], uv.data[1], foc));
  col += march(__ctx, ro, rd);
  fragColor = f32x4(col * vars.fade, 1);
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
