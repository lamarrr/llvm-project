// Port of https://www.shadertoy.com/view/Dlt3Dj; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Dlt3Dj.json.
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
  Texture2D channel1;
  Texture2D channel2;
  Texture2D channel3;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x4 qMult(Context &__ctx, f32x4 qa, f32x4 qb) {
  f32x3 v = cross(qa.swizzle<0, 1, 2>(), qb.swizzle<0, 1, 2>()) +
            (qa.swizzle<0, 1, 2>() * qb.data[3]) +
            (qb.swizzle<0, 1, 2>() * qa.data[3]);
  float scalar = qa.data[3] * qb.data[3] -
                 dot(qa.swizzle<0, 1, 2>(), qb.swizzle<0, 1, 2>());
  return f32x4(v, scalar);
}
f32x4 qFromAxisAngle(Context &__ctx, f32x3 axisNorm, float angle) {
  float halfAngle = angle * 0.5f;
  return f32x4(axisNorm * sin(halfAngle), cos(halfAngle));
}
f32x4 qFromEuler(Context &__ctx, f32x3 rot) {
  f32x4 qx = qFromAxisAngle(__ctx, f32x3(1, 0, 0), rot.data[0]);
  f32x4 qy = qFromAxisAngle(__ctx, f32x3(0, 1, 0), rot.data[1]);
  f32x4 qz = qFromAxisAngle(__ctx, f32x3(0, 0, 1), rot.data[2]);
  f32x4 q = qMult(__ctx, qx, qy);
  return qMult(__ctx, q, qz);
}
f32x3 qRot(Context &__ctx, f32x3 v, f32x4 q) {
  return v + 2.f * cross(cross(v, q.swizzle<0, 1, 2>()) + q.data[3] * v,
                         q.swizzle<0, 1, 2>());
}
float getSlice(Context &__ctx, f32x2 uv) {
  float r = texture(__ctx.channel1, 0.5f * uv).data[0];
  float r2 = texture(__ctx.channel2, uv).data[0];
  float r3 = texture(__ctx.channel3, 0.5f * uv).data[0];
  float t = mod(0.12f * iTime, 3.f);
  float ft = smoothstep(0.4f, 0.6f, fract(t));
  if (t < 1.f)
    r = mix(r, r2, ft);
  else if (t < 2.f)
    r = mix(r2, r3, ft);
  else if (t < 3.f)
    r = mix(r3, r, ft);
  return smoothstep(0.f, 1.f, r);
}
f32x2 getNormal(Context &__ctx, f32x2 uv) {
  f32x2 e = f32x2(1.f / iResolution.swizzle<0, 1>().data[1], 0);
  f32x2 n = f32x2(getSlice(__ctx, uv - e.swizzle<0, 1>()),
                  getSlice(__ctx, uv - e.swizzle<1, 0>()));
  return (getSlice(__ctx, uv) - n);
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 px) {
  f32x2 uv = (px - 0.5f * iResolution.swizzle<0, 1>()) /
             iResolution.swizzle<0, 1>().data[1];
  f32x2 ouv = uv;
  f32x2 ms = iMouse.swizzle<0, 1>();
  float rotX = ms.data[1] / iResolution.swizzle<0, 1>().data[1] - 1.f;
  float rotZ = 0.5f - ms.data[0] / iResolution.swizzle<0, 1>().data[0];
  rotX = -0.15f + 0.07f * tanh(4.f * cos(0.32f * iTime)) / tanh(4.f);
  rotZ = 0.08f * sin(0.4f * iTime) - 0.f;
  f32x3 rot = 3.14159f * f32x3(rotX, 0, rotZ);
  f32x4 quat = qFromEuler(__ctx, rot);
  f32x3 camScene = f32x3(0, 0.15f, -1);
  f32x3 dirScene = normalize(f32x3(uv, 1.2f + 0.1f * cos(0.32f * iTime)));
  f32x3 cam = qRot(__ctx, camScene, quat);
  f32x3 dir = qRot(__ctx, dirScene, quat);
  float aDir = atan(uv.data[1], uv.data[0]);
  float depth = 0.3f;
  float d = -cam.data[1] / dir.data[1];
  uv = (dir.swizzle<0, 2>() * d + cam.swizzle<0, 2>()) + 0.5f;
  f32x3 bg = f32x3(1, 0.96f, 0.98f);
  f32x3 col = f32x3(0);
  if (d > 0.f) {
    float alphaBudget = 1.4f;
    float n = 36.f;
    for (float i = 0.f; i < n; i++) {
      float io = i / n;
      f32x3 dirNorm = normalize(dir);
      dirNorm.set_swizzle<0, 2>(dirNorm.swizzle<0, 2>() / (dir.data[1]));
      f32x2 uv2 = uv + dirNorm.swizzle<0, 2>() * io * depth;
      uv2 = mix(uv, uv2,
                .5f - .5f * tanh(4.f * cos(iTime + 2.f * 3.14159f * io)) /
                          tanh(4.f));
      uv2 += f32x2(0.5f * tanh(2.f * cos(0.32f * iTime)) / tanh(2.f),
                   .32f * iTime);
      float slice = getSlice(__ctx, 0.28f * uv2);
      slice = round(slice * n) / n;
      f32x2 normal = getNormal(__ctx, 0.28f * uv2);
      float aNorm = atan(normal.data[1], normal.data[0]);
      float v = abs(io - slice);
      float alpha = smoothstep(0.03f, 0.f, v);
      alpha = mix(alpha, min(alpha, alphaBudget), 0.75f);
      alpha = max(alpha, 0.f);
      f32x3 tx = texture(__ctx.channel0, 0.5f * uv2).swizzle<0, 1, 2>();
      tx *= 5.f * bg * tx;
      tx *= .85f - .3f * cos(aNorm);
      col += (0.1f + 0.9f * smoothstep(0.f, 1.f, slice)) * tx * alpha;
      alphaBudget -= 0.6f * alpha;
    }
    col = mix(bg, col, exp(-0.4f * d));
  } else
    col = bg;
  O = f32x4(col, 1);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
               __binding(1, 0) sampler sampling0,
               __binding(0, 1) texture_2d<f32x4> channel1,
               __binding(1, 1) sampler sampling1,
               __binding(0, 2) texture_2d<f32x4> channel2,
               __binding(1, 2) sampler sampling2,
               __binding(0, 3) texture_2d<f32x4> channel3,
               __binding(1, 3) sampler sampling3) {
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
  Context __ctx{{channel0, sampling0},
                {channel1, sampling1},
                {channel2, sampling2},
                {channel3, sampling3}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
