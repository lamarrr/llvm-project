// Port of https://www.shadertoy.com/view/stGXDw; author: ersteller.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/stGXDw.json.
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
float sdCapsule(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 ab = b - a;
  f32x3 ap = p - a;
  float t = dot(ab, ap) / dot(ab, ab);
  t = clamp(t, 0.f, 1.f);
  f32x3 c = a + t * ab;
  return length(p - c) - r;
}
float dot2(Context &__ctx, f32x3 v) { return dot(v, v); }
float sdFace(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, f32x3 c) {
  f32x3 ba = b - a;
  f32x3 pa = p - a;
  f32x3 cb = c - b;
  f32x3 pb = p - b;
  f32x3 ac = a - c;
  f32x3 pc = p - c;
  f32x3 nor = cross(ba, ac);
  return sqrt(
      (sign(dot(cross(ba, nor), pa)) + sign(dot(cross(cb, nor), pb)) +
           sign(dot(cross(ac, nor), pc)) <
       2.0f)
          ? min(min(dot2(__ctx,
                         ba * clamp(dot(ba, pa) / dot2(__ctx, ba), 0.0f, 1.0f) -
                             pa),
                    dot2(__ctx,
                         cb * clamp(dot(cb, pb) / dot2(__ctx, cb), 0.0f, 1.0f) -
                             pb)),
                dot2(__ctx,
                     ac * clamp(dot(ac, pc) / dot2(__ctx, ac), 0.0f, 1.0f) -
                         pc))
          : dot(nor, pa) * dot(nor, pa) / dot2(__ctx, nor));
}
f32x3 rotateY(Context &__ctx, f32x3 center, f32x3 p, float rot) {
  f32x3 cp = p - center;
  return f32x3(cp.data[0] * cos(rot) - cp.data[2] * sin(rot), cp.data[1] * 1.f,
               cp.data[0] * sin(rot) + cp.data[2] * cos(rot)) +
         center;
}
f32x4 minD(Context &__ctx, f32x4 rgbdA, f32x4 rgbdB) {
  return rgbdA.data[3] < rgbdB.data[3] ? rgbdA : rgbdB;
}
f32x4 GetDist(Context &__ctx, f32x3 p) {
  float d = 100.f;
  f32x4 colD = f32x4(0, 0, 0, d);
  float thic = 0.06f;
  float rot = iTime * 0.5f;
  float scale = 1.5f;
  f32x3 tr = f32x3(0, 1, 6.f);
  f32x3 A = f32x3(1, 0, 0) * scale + tr;
  f32x3 B = f32x3(0, -1.2f, 0) * scale + tr;
  f32x3 C = f32x3(0, 1.2f, 0) * scale + tr;
  f32x3 D = f32x3(0.5f, 0, 0.866f) * scale + tr;
  for (float i = rot; i <= 2.f * 3.1415f + rot; i += 3.1415f / 3.f) {
    float ADC =
        sdFace(__ctx, p, rotateY(__ctx, tr, A, i), rotateY(__ctx, tr, D, i), C);
    ;
    colD = minD(__ctx, colD, f32x4(1, 0.7f, 0.7f, ADC));
    float ADB =
        sdFace(__ctx, p, rotateY(__ctx, tr, A, i), rotateY(__ctx, tr, D, i), B);
    ;
    colD = minD(__ctx, colD, f32x4(0.7f, 0.7f, 1, ADB));
    float AB = sdCapsule(__ctx, p, rotateY(__ctx, tr, A, i),
                         rotateY(__ctx, tr, B, i), thic);
    colD = minD(__ctx, colD, f32x4(0.f, 0.9f, 0.7f, AB));
    float AC = sdCapsule(__ctx, p, rotateY(__ctx, tr, A, i),
                         rotateY(__ctx, tr, C, i), thic);
    colD = minD(__ctx, colD, f32x4(0.f, 0.7f, 1, AC));
    float AD = sdCapsule(__ctx, p, rotateY(__ctx, tr, A, i),
                         rotateY(__ctx, tr, D, i), thic);
    colD = minD(__ctx, colD, f32x4(1, 1, 0.f, AD));
  }
  return colD;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p).data[3];
  if (d > .01f)
    return f32x3(0);
  f32x2 e = f32x2(.01f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()).data[3],
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()).data[3],
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()).data[3]);
  return normalize(n);
}
f32x4 RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.f;
  f32x4 colD = f32x4(0);
  ;
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + rd * dO;
    colD = GetDist(__ctx, p);
    float dS = colD.data[3];
    dO += dS;
    if (dO > 100.f) {
      dO = 100.f;
      break;
    }
    if (dS < .01f) {
      break;
    }
  }
  colD.data[3] = dO;
  return colD;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col = f32x3(0);
  f32x3 cam = f32x3(0, 2, 0);
  f32x3 ro = cam;
  f32x3 rd = normalize(f32x3(uv.data[0], uv.data[1] - 0.2f, 1));
  f32x4 resRM = RayMarch(__ctx, ro, rd);
  float d = resRM.data[3];
  if (d > 10.1f)
    d = 20.f;
  d /= 4.f;
  d = 2.f - d;
  col = f32x3(resRM.data[0] * d, resRM.data[1] * d, resRM.data[2] * d);
  if (col.data[0] <= 0.01f && col.data[1] <= 0.01f && col.data[2] <= 0.01f)
    col = f32x3(0.25f, uv.data[1] * 0.4f + .3f, uv.data[1] * 0.8f + .3f);
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
