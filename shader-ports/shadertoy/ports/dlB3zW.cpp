// Port of https://www.shadertoy.com/view/dlB3zW; author: raymarchingenthusiast.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dlB3zW.json.
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
struct Context {
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
struct Triangle {
  f32x3 a;
  f32x3 b;
  f32x3 c;
};
struct TriDat {
  Triangle Vp;
  Triangle Vuv;
  bool flipNorm;
  int colMode;
};
void rotX(Context &__ctx, f32x3 &p, f32x2 at) {
  p.data[2] = at.data[1] * p.data[1] - at.data[0] * p.data[2];
  p.data[1] = at.data[0] * p.data[1] + at.data[1] * p.data[2];
}
void rotY(Context &__ctx, f32x3 &p, f32x2 at) {
  p.data[0] = at.data[1] * p.data[2] - at.data[0] * p.data[0];
  p.data[2] = at.data[0] * p.data[2] - at.data[1] * p.data[0];
}
void rotZ(Context &__ctx, f32x3 &p, f32x2 at) {
  p.data[1] = at.data[1] * p.data[0] - at.data[0] * p.data[1];
  p.data[0] = at.data[0] * p.data[0] - at.data[1] * p.data[1];
}
f32x3 crd(Context &__ctx, f32x2 iMouse, f32x2 uv) {
  f32x3 rd = normalize(f32x3(uv, 1.f));
  f32x2 ra = 3.14159265f * (f32x2(1.f, 0.f) - 3.f * iMouse);
  rotY(__ctx, rd, f32x2(cos(ra.data[0]), sin(ra.data[0])));
  rotX(__ctx, rd, f32x2(cos(ra.data[1]), sin(ra.data[1])));
  return rd;
}
void SetTri(Context &__ctx, TriDat &t[2], int i, f32x3 v1, f32x3 v2, f32x3 v3,
            f32x2 uv1, f32x2 uv2, f32x2 uv3, bool fn, int txtr) {
  t[i] = TriDat(Triangle(v1, v2, v3),
                Triangle(f32x3(uv1, 1.f), f32x3(uv2, 1.f), f32x3(uv3, 1.f)), fn,
                txtr);
}
void SetTriData(Context &__ctx, TriDat &t[2], float iTime) {
  SetTri(__ctx, t, 0, f32x3(-1.f, -1.f, 1.f), f32x3(-1.f, 2.f, 1.f),
         f32x3(2.f, -1.f, 1.f), f32x2(0.f, 0.f), f32x2(0.f, 1.f),
         f32x2(1.f, 0.f), false, 0);
  SetTri(__ctx, t, 1, f32x3(2.f, -1.f, 1.f), f32x3(2.f, 2.f, 1.f),
         f32x3(-1.f, 2.f, 1.f), f32x2(1.f, 0.f), f32x2(1.f, 1.f),
         f32x2(0.f, 1.f), true, 0);
}
void SplitTriDat(Context &__ctx, TriDat t[2], Triangle &tp[2],
                 Triangle &tuv[2]) {
  for (int i = 0; i < 2; i++) {
    tp[i] = t[i].Vp;
    tuv[i] = t[i].Vuv;
  }
}
void RecalibTris(Context &__ctx, Triangle &tso[2], f32x3 ro) {
  for (int i = 0; i < 2; i++) {
    f32x3 a =
        f32x3(tso[i].data[3].swizzle<0, 1>() - .5f, tso[i].data[3].data[2]);
    f32x3 b =
        f32x3(tso[i].data[2].swizzle<0, 1>() - .5f, tso[i].data[2].data[2]);
    f32x3 c = f32x3(tso[i].c.swizzle<0, 1>() - .5f, tso[i].c.data[2]);
    a -= ro;
    b -= ro;
    c -= ro;
    tso[i] = Triangle(a, b, c);
  }
}
f32x3 CalcNorm(Context &__ctx, Triangle t) {
  f32x3 U = t.data[2] - t.data[3];
  f32x3 V = t.c - t.data[3];
  f32x3 n;
  n.data[0] = U.data[1] * V.data[2] - U.data[2] * V.data[1];
  n.data[1] = U.data[2] * V.data[0] - U.data[0] * V.data[2];
  n.data[2] = U.data[0] * V.data[1] - U.data[1] * V.data[0];
  return normalize(n);
}
f32x2 CalcUv(Context &__ctx, f32x2 p, Triangle tp, Triangle tuv, Triangle top,
             f32x3 &pos) {
  float w1p1 =
      (tp.data[2].data[1] - tp.c.data[1]) * (p.data[0] - tp.c.data[0]) +
      (tp.c.data[0] - tp.data[2].data[0]) * (p.data[1] - tp.c.data[1]);
  float w1p2 =
      (tp.data[2].data[1] - tp.c.data[1]) *
          (tp.data[3].data[0] - tp.c.data[0]) +
      (tp.c.data[0] - tp.data[2].data[0]) * (tp.data[3].data[1] - tp.c.data[1]);
  float w1 = w1p1 / w1p2;
  float w2p1 =
      (tp.c.data[1] - tp.data[3].data[1]) * (p.data[0] - tp.c.data[0]) +
      (tp.data[3].data[0] - tp.c.data[0]) * (p.data[1] - tp.c.data[1]);
  float w2p2 =
      (tp.data[2].data[1] - tp.c.data[1]) *
          (tp.data[3].data[0] - tp.c.data[0]) +
      (tp.c.data[0] - tp.data[2].data[0]) * (tp.data[3].data[1] - tp.c.data[1]);
  float w2 = w2p1 / w2p2;
  float w3 = 1.f - w1 - w2;
  f32x3 puv = w1 * tuv.data[3] / tp.data[3].data[2] +
              w2 * tuv.data[2] / tp.data[2].data[2] + w3 * tuv.c / tp.c.data[2];
  f32x4 p0a = f32x4(top.data[3], 1.f);
  f32x4 p0b = f32x4(top.data[2], 1.f);
  f32x4 p0c = f32x4(top.c, 1.f);
  f32x4 p1 =
      w1 * p0a / p0a.data[2] + w2 * p0b / p0b.data[2] + w3 * p0c / p0c.data[2];
  pos = p1.swizzle<0, 1, 2>() / p1.data[3];
  return puv.swizzle<0, 1>() / puv.data[2];
}
bool isInside(Context &__ctx, f32x2 p, Triangle tri) {
  float s =
      (tri.data[3].data[0] - tri.c.data[0]) * (p.data[1] - tri.c.data[1]) -
      (tri.data[3].data[1] - tri.c.data[1]) * (p.data[0] - tri.c.data[0]);
  float t = (tri.data[2].data[0] - tri.data[3].data[0]) *
                (p.data[1] - tri.data[3].data[1]) -
            (tri.data[2].data[1] - tri.data[3].data[1]) *
                (p.data[0] - tri.data[3].data[0]);
  if ((s < 0.f) != (t < 0.f) && s != 0.f && t != 0.f)
    return false;
  float d =
      (tri.c.data[0] - tri.data[2].data[0]) *
          (p.data[1] - tri.data[2].data[1]) -
      (tri.c.data[1] - tri.data[2].data[1]) * (p.data[0] - tri.data[2].data[0]);
  return d == 0.f || (d < 0.f) == (s + t <= 0.f);
}
Triangle Project(Context &__ctx, Triangle to, f32x2 Mouse) {
  f32x2 ra = 3.14159265f * (f32x2(1.f, 0.f) - 3.f * Mouse);
  f32x3 a = to.data[3];
  rotY(__ctx, a, f32x2(cos(-ra.data[0]), sin(-ra.data[0])));
  rotX(__ctx, a, f32x2(cos(-ra.data[1]), sin(-ra.data[1])));
  f32x3 b = to.data[2];
  rotY(__ctx, b, f32x2(cos(-ra.data[0]), sin(-ra.data[0])));
  rotX(__ctx, b, f32x2(cos(-ra.data[1]), sin(-ra.data[1])));
  f32x3 c = to.c;
  rotY(__ctx, c, f32x2(cos(-ra.data[0]), sin(-ra.data[0])));
  rotX(__ctx, c, f32x2(cos(-ra.data[1]), sin(-ra.data[1])));
  a.set_swizzle<0, 1>(a.swizzle<0, 1>() / (a.data[2]));
  b.set_swizzle<0, 1>(b.swizzle<0, 1>() / (b.data[2]));
  c.set_swizzle<0, 1>(c.swizzle<0, 1>() / (c.data[2]));
  return Triangle(a, b, c);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  uv -= .5f;
  f32x2 MOUSE = (iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>()) - .5f;
  if (iMouse.swizzle<0, 1>() == f32x2(0.f))
    MOUSE = f32x2(0.f);
  TriDat tris[2];
  SetTriData(__ctx, tris, iTime);
  Triangle tos[2];
  Triangle uvs[2];
  SplitTriDat(__ctx, tris, tos, uvs);
  f32x3 rd = crd(
      __ctx, (iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>()) - .5f, uv);
  f32x3 centrRd = crd(__ctx, MOUSE, f32x2(0.f));
  f32x3 ro = f32x3(.5f, .5f, 0.f) - centrRd * 8.f;
  RecalibTris(__ctx, tos, ro);
  f32x3 ld = f32x3(.5f, .5f, 5.f * sin(iTime) + 1.f);
  fragColor = f32x4(0.f, 0.f, .3f, 100.f);
  for (int i = 0; i < 2; i++) {
    Triangle ts = Project(__ctx, tos[i], MOUSE);
    if (isInside(__ctx, uv, ts) &&
        !(ts.data[3].data[2] <= 0.f || ts.data[2].data[2] <= 0.f ||
          ts.c.data[2] <= 0.f)) {
      f32x3 p;
      f32x2 textureUv = CalcUv(__ctx, uv, ts, uvs[i], tos[i], p);
      f32x3 col = texture(__ctx.channel0, textureUv).swizzle<0, 1, 2>();
      f32x3 n = CalcNorm(__ctx, tos[i]);
      if (tris[i].flipNorm)
        n *= -1.f;
      float dif = clamp(dot(n, normalize(ld - p)) * .5f + .5f, 0.f, 1.f);
      col *= dif;
      f32x3 h = normalize(rd + normalize(ld - p));
      col += dif * f32x3(pow(clamp(dot(h, n), 0.f, 1.f), 30.f));
      if (p.data[2] <= fragColor.data[3])
        fragColor = f32x4(col, p.data[2]);
      if (textureUv.data[0] > 1.f || textureUv.data[0] < 0.f ||
          textureUv.data[1] > 1.f || textureUv.data[1] < 0.f)
        fragColor = f32x4(.15f, .15f, 3.f, 1.f);
    }
  }
  fragColor.data[3] = 1.f;
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
