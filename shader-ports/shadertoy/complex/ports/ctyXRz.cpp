// Port of https://www.shadertoy.com/view/ctyXRz; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctyXRz.json.
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
struct Context {
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x2 solve(Context &__ctx, f32x2 p, float r1, float r2, float side) {
  float h = dot(p, p);
  float w = h + r1 * r1 - r2 * r2;
  float s = max(4.0f * r1 * r1 * h - w * w, 0.0f);
  return (w * p + side * f32x2(-p.data[1], p.data[0]) * sqrt(s)) * 0.5f / h;
}
float seg(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h);
}
float cap(Context &__ctx, f32x2 uv, f32x2 p, f32x2 q, float r1, float r2) {
  float h = length(p - q);
  uv -= p;
  float th = atan(q.data[1] - p.data[1], q.data[0] - p.data[0]);
  uv *= f32x2x2(cos(-th + 3.14159f / 2.f), -sin(-th + 3.14159f / 2.f),
                sin(-th + 3.14159f / 2.f), cos(-th + 3.14159f / 2.f));
  uv.data[0] = abs(uv.data[0]);
  float b = (r1 - r2) / h;
  float a = sqrt(1.0f - b * b);
  float k = dot(uv, f32x2(-b, a));
  if (k < 0.0f)
    return length(uv) - r1;
  if (k > a * h)
    return length(uv - f32x2(0.0f, h)) - r2;
  return dot(uv, f32x2(a, b)) - r1;
}
float g(Context &__ctx, float x) {
  return pow((27.f / 32.f) * x * x * (2.f - x), 8.f);
}
f32x2 getPos(Context &__ctx, float t, float a, float w, float h) {
  return f32x2(w, h) * f32x2(cos(3.14159f * (t + a)), g(__ctx, t));
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 px) {
  f32x2 uv = (px - 0.5f * iResolution.swizzle<0, 1>()) /
             iResolution.swizzle<0, 1>().data[1];
  f32x2 ouv = uv;
  uv.data[1] += 0.48f;
  uv *= 1.2f;
  float t = 0.75f * iTime;
  t += 0.5f *
       ((exp2((2.f) * cos(0.2f * iTime)) - 1.f) /
        (exp2((2.f) * cos(0.2f * iTime)) + 1.f)) /
       ((exp2(2.f) - 1.f) / (exp2(2.f) + 1.f));
  float a = 0.5f;
  float w = 0.31f;
  float h = 0.2f;
  float n = 40.f;
  float t2 = mod(t, 2.f);
  f32x2 p = getPos(__ctx, t2, a, w, h);
  f32x2 q = getPos(__ctx, mod(t - 2.f / n, 2.f), a, w, h);
  float d = seg(__ctx, uv, p, q);
  for (float i = 1.f; i < n; i++) {
    float t2 = mod(t + 2.f * i / n, 2.f);
    f32x2 q = getPos(__ctx, t2, a, w, h);
    d = min(d, seg(__ctx, uv, p, q));
    p = q;
  }
  float s = smoothstep(-1.f / iResolution.swizzle<0, 1>().data[1],
                       1.f / iResolution.swizzle<0, 1>().data[1], 0.005f - d);
  f32x2 pLegL = getPos(__ctx, mod(t, 2.f), a + 0.f * sin(0.5f * t), w, h);
  f32x2 pLegR = getPos(__ctx, mod(t + 1.f, 2.f), a + 0.f * cos(0.5f * t), w, h);
  f32x2 pHip = f32x2(0.1f, 0.4f + 0.05f * cos(2.f * 3.14159f * t));
  f32x2 pKneeL = pLegL + solve(__ctx, pHip - pLegL, 0.26f, 0.28f, -1.f);
  f32x2 pKneeR = pLegR + solve(__ctx, pHip - pLegR, 0.26f, 0.28f, -1.f);
  f32x2 pFootL =
      pLegL +
      0.05f * f32x2(cos(-0.5f * 3.14159f * g(__ctx, mod(t + 0.1f, 2.f))),
                    sin(-0.5f * 3.14159f * g(__ctx, mod(t + 0.1f, 2.f))));
  f32x2 pFootR =
      pLegR +
      0.05f * f32x2(cos(-0.5f * 3.14159f * g(__ctx, mod(t + 1.1f, 2.f))),
                    sin(-0.5f * 3.14159f * g(__ctx, mod(t + 1.1f, 2.f))));
  f32x2 pBody =
      pHip + 0.34f * f32x2(cos(3.14159f / 2.f +
                               0.3f * cos(2.f * 3.14159f * t + 3.14159f / 4.f)),
                           sin(3.14159f / 2.f + 0.3f * cos(2.f * 3.14159f * t +
                                                           3.14159f / 4.f)));
  f32x2 pArmL =
      pBody + 0.2f * f32x2(cos(-3.14159f / 8.f +
                               .3f * sin(2.f * 3.14159f * t + 3.14159f / 4.f) +
                               g(__ctx, mod(t + 1.2f, 2.f))),
                           sin(-3.14159f / 8.f +
                               .3f * sin(2.f * 3.14159f * t + 3.14159f / 4.f) +
                               g(__ctx, mod(t + 1.2f, 2.f))));
  f32x2 pArmR =
      pBody - 0.2f * f32x2(cos(3.14159f / 8.f +
                               .3f * sin(2.f * 3.14159f * t + 3.14159f / 4.f) +
                               g(__ctx, mod(t + 0.2f, 2.f))),
                           sin(3.14159f / 8.f +
                               .3f * sin(2.f * 3.14159f * t + 3.14159f / 4.f) +
                               g(__ctx, mod(t + 0.2f, 2.f))));
  f32x2 pHandL =
      pArmL + 0.2f * f32x2(cos(0.5f * cos(2.f * 3.14159f * t + 3.14159f)),
                           sin(0.5f * cos(2.f * 3.14159f * t + 3.14159f)));
  f32x2 pHandR =
      pArmR + 0.2f * f32x2(cos(0.5f * cos(2.f * 3.14159f * t + 3.14159f)),
                           sin(0.5f * cos(2.f * 3.14159f * t + 3.14159f)));
  f32x2 pHead =
      pBody +
      0.1f * f32x2(cos(3.f * 3.14159f / 8.f +
                       0.3f * cos(2.f * 3.14159f * t + 3.f * 3.14159f / 4.f)),
                   sin(3.f * 3.14159f / 8.f +
                       0.3f * cos(2.f * 3.14159f * t + 3.f * 3.14159f / 4.f)));
  f32x2 pFingL =
      pHandL +
      0.02f * f32x2(cos(-0.5f * cos(2.f * 3.14159f * t - 3.14159f / 4.f)),
                    sin(-0.5f * cos(2.f * 3.14159f * t - 3.14159f / 4.f)));
  f32x2 pEye = pHead + f32x2(0.04f, 0);
  f32x2 pEye2 =
      pEye +
      0.01f * f32x2(cos(0.3f * cos(2.f * 3.14159f * t - 3.14159f / 4.f)),
                    sin(0.3f * cos(2.f * 3.14159f * t - 3.14159f / 4.f)));
  float dL, dC, dR;
  dL = dR = dC = 1e5f;
  float d3 = 1e5f;
  dL = min(dL, cap(__ctx, uv, pHip, pKneeL, .02f, .01f));
  ;
  dL = min(dL, cap(__ctx, uv, pLegL, pKneeL, .005f, .01f));
  ;
  dL = min(dL, cap(__ctx, uv, pLegL, pFootL, .005f, .00f));
  ;
  dL = min(dL, cap(__ctx, uv, pBody, pArmL, .02f, .01f));
  ;
  dL = min(dL, cap(__ctx, uv, pArmL, pHandL, .01f, .0f));
  ;
  dL = min(dL, cap(__ctx, uv, pHandL, pFingL, .0f, .0f));
  ;
  dR = min(dR, cap(__ctx, uv, pHip, pKneeR, .02f, .01f));
  ;
  dR = min(dR, cap(__ctx, uv, pLegR, pKneeR, .005f, .01f));
  ;
  dR = min(dR, cap(__ctx, uv, pLegR, pFootR, .005f, .00f));
  ;
  dR = min(dR, cap(__ctx, uv, pBody, pArmR, .02f, .01f));
  ;
  dR = min(dR, cap(__ctx, uv, pArmR, pHandR, .01f, .0f));
  ;
  dC = min(dC, cap(__ctx, uv, pHip, pBody, .02f, .015f));
  ;
  dC = min(dC, cap(__ctx, uv, pBody, pHead, .015f, .0f));
  ;
  dC = min(dC, length(uv - pHead) - 0.045f);
  float dEye = length(uv - pEye);
  float sEye =
      smoothstep(-1.f / iResolution.swizzle<0, 1>().data[1],
                 1.f / iResolution.swizzle<0, 1>().data[1], 0.03f - dEye);
  float sEye0 =
      smoothstep(-1.f / iResolution.swizzle<0, 1>().data[1],
                 1.f / iResolution.swizzle<0, 1>().data[1], 0.02f - dEye);
  float dEye2 = length(uv - pEye2);
  float sEye2 =
      smoothstep(-1.f / iResolution.swizzle<0, 1>().data[1],
                 1.f / iResolution.swizzle<0, 1>().data[1], 0.015f - dEye2);
  float mx =
      0.5f + 0.5f *
                 ((exp2((4.f) * cos(30.f * uv.data[1] + 0.f * iTime)) - 1.f) /
                  (exp2((4.f) * cos(30.f * uv.data[1] + 0.f * iTime)) + 1.f)) /
                 ((exp2(4.f) - 1.f) / (exp2(4.f) + 1.f));
  float rL = mix(0.01f, 0.02f, mx);
  float rC = mix(0.008f, 0.018f, mx);
  float rR = mix(0.01f, 0.02f, mx);
  float rO = mix(0.025f, 0.035f, mx);
  float sL = smoothstep(-1.f / iResolution.swizzle<0, 1>().data[1],
                        1.f / iResolution.swizzle<0, 1>().data[1], rL - dL);
  float sC = smoothstep(-1.f / iResolution.swizzle<0, 1>().data[1],
                        1.f / iResolution.swizzle<0, 1>().data[1], rC - dC);
  float sR = smoothstep(-1.f / iResolution.swizzle<0, 1>().data[1],
                        1.f / iResolution.swizzle<0, 1>().data[1], rR - dR);
  float sLO =
      smoothstep(-1.f / iResolution.swizzle<0, 1>().data[1],
                 1.f / iResolution.swizzle<0, 1>().data[1], 0.01f + rL - dL);
  float sCO =
      smoothstep(-1.f / iResolution.swizzle<0, 1>().data[1],
                 1.f / iResolution.swizzle<0, 1>().data[1], 0.01f + rC - dC);
  float sRO =
      smoothstep(-1.f / iResolution.swizzle<0, 1>().data[1],
                 1.f / iResolution.swizzle<0, 1>().data[1], 0.01f + rR - dR);
  f32x3 col = f32x3(0.98f, 0.95f, 0.8f);
  col = mix(col, f32x3(0), 1.f - exp(-0.12f * length(ouv)));
  float bgLines = smoothstep(
      -1.f / iResolution.swizzle<0, 1>().data[1],
      1.f / iResolution.swizzle<0, 1>().data[1],
      0.2f -
          abs(fract(30.f *
                    (uv * f32x2x2(cos(-3.14159f / 4.f), -sin(-3.14159f / 4.f),
                                  sin(-3.14159f / 4.f), cos(-3.14159f / 4.f)))
                        .data[0]) -
              0.5f));
  col *= 1.f - 0.005f * bgLines;
  ouv *= f32x2x2(cos(0.1f *
                     ((exp2((4.f) * cos(iTime)) - 1.f) /
                      (exp2((4.f) * cos(iTime)) + 1.f)) /
                     ((exp2(4.f) - 1.f) / (exp2(4.f) + 1.f))),
                 -sin(0.1f *
                      ((exp2((4.f) * cos(iTime)) - 1.f) /
                       (exp2((4.f) * cos(iTime)) + 1.f)) /
                      ((exp2(4.f) - 1.f) / (exp2(4.f) + 1.f))),
                 sin(0.1f *
                     ((exp2((4.f) * cos(iTime)) - 1.f) /
                      (exp2((4.f) * cos(iTime)) + 1.f)) /
                     ((exp2(4.f) - 1.f) / (exp2(4.f) + 1.f))),
                 cos(0.1f *
                     ((exp2((4.f) * cos(iTime)) - 1.f) /
                      (exp2((4.f) * cos(iTime)) + 1.f)) /
                     ((exp2(4.f) - 1.f) / (exp2(4.f) + 1.f))));
  float tx =
      texture(__ctx.channel0, .2f * ouv + 0.05f * f32x2(iTime, 0)).data[0];
  tx = smoothstep(-1.f / iResolution.swizzle<0, 1>().data[1],
                  1.f / iResolution.swizzle<0, 1>().data[1], 0.2f - tx);
  col -= 0.1f * tx;
  col = mix(col, f32x3(0.32f), sLO);
  col = mix(col, f32x3(1, .5f, .5f), sL);
  col = mix(col, f32x3(0.32f), sCO);
  col = mix(col, f32x3(.5f, .8f, 1.f), sC);
  col = mix(col, f32x3(0.32f), sRO);
  col = mix(col, f32x3(.4f), sR);
  col = mix(col, f32x3(.32f), sEye);
  col = mix(col, f32x3(0.99f), sEye0);
  col = mix(col, f32x3(.32f), sEye2);
  col *= 1.f - 0.005f * bgLines;
  O = f32x4(col, 1);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
  Context __ctx{{channel0, sampling0}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
