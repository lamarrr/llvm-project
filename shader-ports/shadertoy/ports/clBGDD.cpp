// Port of https://www.shadertoy.com/view/clBGDD; author: fishy.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/clBGDD.json.
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
  TextureCube channel0;
  Texture3D channel2;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x4 displayVar(Context &__ctx, bool a) { return f32x4(f32x3(a), 1.0f); }
f32x4 displayVar(Context &__ctx, float a) { return f32x4(f32x3(a), 1.0f); }
f32x4 displayVar(Context &__ctx, f32x2 a) { return f32x4(a, 0.0f, 1.0f); }
f32x4 displayVar(Context &__ctx, f32x3 a) { return f32x4(a, 1.0f); }
f32x4 displayVar(Context &__ctx, f32x4 a) {
  return f32x4(a.swizzle<0, 1, 2>(), 1.0f);
}
void join(Context &__ctx, float &a, float b) { a = min(a, b); }
void subt(Context &__ctx, float &a, float b) { a = max(a, -b); }
float box(Context &__ctx, f32x2 p, f32x2 s) {
  return max((abs(p) - s).data[0], (abs(p) - s).data[1]);
}
float digit(Context &__ctx, f32x2 p, int d) {
  float r = 100000000.0f;
  switch (d) {
  case (0):
    join(__ctx, r, box(__ctx, p, f32x2(0.1f, 0.15f)));
    subt(__ctx, r, box(__ctx, p, f32x2(0.05f, 0.1f)));
    break;
  case (1):
    join(__ctx, r, box(__ctx, p, f32x2(0.025f, 0.15f)));
    break;
  case (2):
    join(__ctx, r, box(__ctx, p, f32x2(0.1f, 0.15f)));
    subt(__ctx, r, box(__ctx, p - f32x2(0.15f, -0.06f), f32x2(0.2f, 0.035f)));
    subt(__ctx, r, box(__ctx, p - f32x2(-0.15f, 0.06f), f32x2(0.2f, 0.035f)));
    break;
  case (3):
    join(__ctx, r, box(__ctx, p, f32x2(0.1f, 0.15f)));
    subt(__ctx, r, box(__ctx, p - f32x2(-0.15f, -0.06f), f32x2(0.2f, 0.035f)));
    subt(__ctx, r, box(__ctx, p - f32x2(-0.15f, 0.06f), f32x2(0.2f, 0.035f)));
    break;
  case (4):
    join(__ctx, r, box(__ctx, p - f32x2(0.075f, 0.0f), f32x2(0.025f, 0.15f)));
    join(__ctx, r, box(__ctx, p - f32x2(0.0f, 0.05f), f32x2(0.1f, 0.1f)));
    subt(__ctx, r, box(__ctx, p - f32x2(0.0f, 0.09f), f32x2(0.05f, 0.085f)));
    break;
  case (5):
    join(__ctx, r, box(__ctx, p, f32x2(0.1f, 0.15f)));
    subt(__ctx, r, box(__ctx, p - f32x2(0.15f, 0.06f), f32x2(0.2f, 0.035f)));
    subt(__ctx, r, box(__ctx, p - f32x2(-0.15f, -0.06f), f32x2(0.2f, 0.035f)));
    break;
  case (6):
    p = -p;
    join(__ctx, r, box(__ctx, p - f32x2(0.075f, 0.0f), f32x2(0.025f, 0.15f)));
    join(__ctx, r, box(__ctx, p - f32x2(0.0f, 0.05f), f32x2(0.1f, 0.1f)));
    subt(__ctx, r, box(__ctx, p - f32x2(0.0f, 0.05f), f32x2(0.05f, 0.05f)));
    break;
  case (7):
    join(__ctx, r, box(__ctx, p - f32x2(0.0f), f32x2(0.1f, 0.15f)));
    subt(__ctx, r, box(__ctx, p - f32x2(-0.05f), f32x2(0.1f, 0.15f)));
    break;
  case (8):
    join(__ctx, r, box(__ctx, p - f32x2(0.0f), f32x2(0.1f, 0.15f)));
    subt(__ctx, r, box(__ctx, p - f32x2(0.0f, 0.05f), f32x2(0.05f, 0.05f)));
    subt(__ctx, r, box(__ctx, p - f32x2(0.0f, -0.05f), f32x2(0.05f, 0.05f)));
    join(__ctx, r, box(__ctx, p - f32x2(0.0f), f32x2(0.1f, 0.025f)));
    break;
  case (9):
    join(__ctx, r, box(__ctx, p - f32x2(0.075f, 0.0f), f32x2(0.025f, 0.15f)));
    join(__ctx, r, box(__ctx, p - f32x2(0.0f, 0.05f), f32x2(0.1f, 0.1f)));
    subt(__ctx, r, box(__ctx, p - f32x2(0.0f, 0.05f), f32x2(0.05f, 0.05f)));
    break;
  }
  return r;
}
float getDig(Context &__ctx, float num, float d) {
  float a = floor(num / pow(10.f, d));
  return a - floor(a / 10.f) * 10.f;
}
float print(Context &__ctx, f32x2 p, float num, float spacing) {
  float r = 10000.0f;
  int fig = int(ceil(log2(num) / 3.32192809489f));
  for (int i = 0; i < fig; i++) {
    int di = int(getDig(__ctx, num, float(i)));
    join(__ctx, r, digit(__ctx, p + f32x2(i, 0) * spacing, di));
  }
  return r;
}
f32x3 printFR(Context &__ctx, f32x2 p, float iFrameRate) {
  return 1.f - f32x3(step(0.f, print(__ctx, (p - f32x2(0.474f, 0.25f)) * 9.f,
                                     iFrameRate, 0.3f)));
}
f32x2 hash12(Context &__ctx, float p) {
  f32x3 p3 = fract(f32x3(p, p, p) * f32x3(.1031f, .1030f, .0973f));
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.swizzle<0, 0>() + p3.swizzle<1, 2>()) * p3.swizzle<2, 1>());
}
f32x2 hash22(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(f32x3(p.swizzle<0, 1, 0>()) * f32x3(.1031f, .1030f, .0973f));
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.swizzle<0, 0>() + p3.swizzle<1, 2>()) * p3.swizzle<2, 1>());
}
float mRange(Context &__ctx, float ai, float aa, float bi, float ba, float x) {
  return (x - ai) / (aa - ai) * (ba - bi) + bi;
}
float mRange(Context &__ctx, f32x2 a, f32x2 b, float x) {
  return mRange(__ctx, a.data[0], a.data[1], b.data[0], b.data[1], x);
}
float sRange(Context &__ctx, float i, float a, float x) {
  return x * (a - i) + i;
}
float sRange(Context &__ctx, f32x2 a, float x) {
  return sRange(__ctx, a.data[0], a.data[1], x);
}
float eRange(Context &__ctx, float i, float a, float x) {
  return (x - i) / (a - i);
}
float eRange(Context &__ctx, f32x2 a, float x) {
  return eRange(__ctx, a.data[0], a.data[1], x);
}
float sharpstep(Context &__ctx, float a, float b, float x) {
  return clamp(0.0f, 1.0f, (x - a) / (b - a));
}
float peak(Context &__ctx, float a, float b, float x) {
  return clamp(0.0f, 1.0f, 1.f - abs((x - b) / a));
}
float peak2(Context &__ctx, float a, float b, float m, float x) {
  x = clamp(0.0f, 1.0f, (x - a) / (b - a));
  return clamp(0.0f, 1.0f,
               1.f - abs((x - m) / mix(m, 1.f - m, step(0.f, x - m))));
}
f32x2x2 rotMat(Context &__ctx, float r) {
  return f32x2x2(cos(r), -sin(r), sin(r), cos(r));
}
f32x3 rotX(Context &__ctx, f32x3 p, float r) {
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (rotMat(__ctx, r)));
  return p;
}
f32x3 rotY(Context &__ctx, f32x3 p, float r) {
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (rotMat(__ctx, r)));
  return p;
}
f32x3 rotZ(Context &__ctx, f32x3 p, float r) {
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (rotMat(__ctx, r)));
  return p;
}
f32x3 rot(Context &__ctx, f32x3 v, f32x3 k, float t) {
  k = normalize(k);
  return mix(k * dot(k, v), v, cos(t)) + cross(k, v) * sin(t);
}
struct marchMat {
  f32x3 color;
  int id;
};
struct marchRes {
  bool hit;
  float depth;
  f32x3 pos;
  marchMat mat;
};
void sub(Context &__ctx, float &a, float b) { a = max(a, -b); }
void intersect(Context &__ctx, float &a, float b) { a = max(a, b); }
void sjoin(Context &__ctx, float &a, float b, float k) {
  float h = max(k - abs(a - b), 0.0f) / k;
  a = min(a, b) - h * h * k * (1.0f / 4.0f);
}
void ssub(Context &__ctx, float &d2, float d1, float k) {
  float h = clamp(0.5f - 0.5f * (d2 + d1) / k, 0.0f, 1.0f);
  d2 = mix(d2, -d1, h) + k * h * (1.0f - h);
}
void sintersect(Context &__ctx, float &d2, float d1, float k) {
  float h = clamp(0.5f - 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  d2 = mix(d2, d1, h) + k * h * (1.0f - h);
}
void marginIntersect(Context &__ctx, float &a, float b, float k) {
  a = max(a - k, b);
}
void smarginIntersect(Context &__ctx, float &a, float b, float k1, float k2) {
  a = a - k1;
  sintersect(__ctx, a, b, k2);
}
f32x2 opRepLim2(Context &__ctx, f32x2 p, float c, f32x2 l) {
  return p - c * clamp(round(p / c), -l, l);
}
f32x3 opRepLim3(Context &__ctx, f32x3 p, float c, f32x3 l) {
  return p - c * clamp(round(p / c), -l + 1.f, l);
}
float sdSphere(Context &__ctx, f32x3 p, float r) { return length(p) - r; }
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
float sdBox(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 q = abs(p) - b;
  return length(max(q, 0.0f)) + min(max(q.data[0], q.data[1]), 0.0f);
}
float sdBox(Context &__ctx, f32x3 p, f32x3 b, float r) {
  return sdBox(__ctx, p, b - r) - r;
}
float sdCylinder(Context &__ctx, f32x3 p, f32x2 r) {
  return sdBox(__ctx, f32x2(length(p.swizzle<0, 2>()), p.data[1]), r);
}
float sdCylinder(Context &__ctx, f32x3 p, f32x2 r, float b) {
  return sdBox(__ctx, f32x2(length(p.swizzle<0, 2>()), p.data[1]), r - b) - b;
}
float sdFloor(Context &__ctx, f32x3 p, float y) { return p.data[1] - y; }
float sdPlane(Context &__ctx, f32x3 p, float y) { return abs(p.data[1] - y); }
f32x3 planeIntersect(Context &__ctx, f32x3 ro, f32x3 rd, float y, bool &hit) {
  hit = sign(-rd.data[1]) == sign(ro.data[1] - y);
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() +
                       rd.swizzle<0, 2>() * (y - ro.data[1]) / rd.data[1]);
  return f32x3(ro.data[0], y, ro.data[2]);
}
float sdFbmBase(Context &__ctx, f32x3 p, float j, f32x2 r, float s) {
  float res = 1e20f;
  for (f32x2 x = f32x2(-2.f); x.data[0] <= 2.f; x.data[0]++)
    for (x.data[1] = -2.f; x.data[1] <= 2.f; x.data[1]++) {
      f32x2 currentCell = (floor(p.swizzle<0, 2>() / s) + x) * s;
      f32x2 currentCellOffset = (hash22(__ctx, currentCell) - 0.5f) * j;
      float rad = sRange(__ctx, r, hash22(__ctx, currentCell + 10.f).data[0]);
      float currentCellDist = sdSphere(
          __ctx,
          p - f32x3(currentCell - currentCellOffset, 0.f).swizzle<0, 2, 1>(),
          rad);
      sjoin(__ctx, res, currentCellDist, 1.5f);
    }
  return res;
}
float sdFbmBase(Context &__ctx, f32x3 p, float j, f32x2 r, float s,
                float scale) {
  return sdFbmBase(__ctx, p / scale, j, r, s) * scale;
}
float sdFbm(Context &__ctx, f32x3 p, float j, f32x2 r, float s, float o,
            float b) {
  float res = sdPlane(__ctx, p, 0.f);
  float ex = 1.f;
  for (float i = 0.f; i < o; i++) {
    sjoin(__ctx, res, sdFbmBase(__ctx, p, j, r, s, ex * 2.f), b);
    ex *= 0.5f;
  }
  return res;
}
float sceneDist(Context &__ctx, f32x3 p, marchMat &outMat) {
  float scale = 0.05f;
  p /= scale;
  float d = 1e20f;
  join(__ctx, d, sdFbm(__ctx, p, 5.f, f32x2(0.f, 5.f), 5.f, 2.f, 1.f));
  outMat = marchMat(f32x3(0.9f), 0);
  d *= scale;
  return d;
}
float sceneDist(Context &__ctx, f32x3 p) {
  marchMat m;
  return sceneDist(__ctx, p, m);
}
marchRes march(Context &__ctx, f32x3 ro, f32x3 rd, float margin, float maxDist,
               int maxIt) {
  float depth = 0.0f;
  f32x3 cp;
  marchRes mres;
  for (int i = 0; i < maxIt && depth < maxDist; i++) {
    cp = ro + depth * rd;
    float cd = sceneDist(__ctx, cp, mres.mat);
    if (cd < margin) {
      return marchRes(true, depth, cp, mres.mat);
    }
    depth += cd;
  }
  return marchRes(false, depth, ro + depth * rd, mres.mat);
}
f32x3 calcNormal(Context &__ctx, f32x3 p) {
  const float h = 0.00001f;
  f32x3 n = f32x3(0.0f);
  for (int i = (min(iFrame, 0)); i < 4; i++) {
    f32x3 e =
        0.5773f *
        (2.0f * f32x3((((i + 3) >> 1) & 1), ((i >> 1) & 1), (i & 1)) - 1.0f);
    n += e * sceneDist(__ctx, p + e * h);
  }
  return normalize(n);
}
f32x4 getColorAtPix(Context &__ctx, f32x2 fc, f32x4 m) {
  f32x2 uv = (fc - iResolution.swizzle<0, 1>() * 0.5f) / iResolution.data[1];
  f32x2 mouse = (m.swizzle<0, 1>() - iResolution.swizzle<0, 1>() * 0.5f) /
                iResolution.data[1] * step(0.01f, m.data[2]) * 3.0f;
  f32x2 cr =
      f32x2(0.6154729f, 0.7853982f + iTime * 0.2f) - mouse.swizzle<1, 0>();
  f32x3 ro =
      rotY(__ctx, rotX(__ctx, f32x3(0.1f, 0.05f, -2.f * 1.f), cr.data[0]),
           cr.data[1]);
  f32x3 rd = rotY(__ctx, rotX(__ctx, normalize(f32x3(uv, 1.f)), cr.data[0]),
                  cr.data[1]);
  marchRes res = march(__ctx, ro, rd, 0.001f, 100.0f, 512);
  f32x3 n = calcNormal(__ctx, res.pos);
  float lighting =
      dot(max(rot(__ctx, n, f32x3(1, 1, 0) * 0.707106781187f, 0.96f), 0.0f),
          f32x3(0.33f)) +
      0.1f * max(0.f, n.data[2]);
  float ao =
      1.f - pow(0.9f - clamp(0.0f, 1.0f,
                             sceneDist(__ctx, res.pos + 0.05f * n) / 0.05f),
                2.f);
  f32x3 envMap =
      texture(__ctx.channel0, reflect(rd, n),
              texture(__ctx.channel2, res.pos * 0.5f).data[0] * 10.0f)
          .swizzle<0, 1, 2>();
  envMap *= envMap;
  res.mat.color = res.mat.color + envMap * 1.0f;
  return sqrt(displayVar(
      __ctx, mix(f32x3(0.9f), res.mat.color * lighting * mix(1.f, ao, 0.2f),
                 f32x3(res.hit))));
}
f32x4 simpleAA(Context &__ctx, f32x2 fc, f32x4 m) {
  f32x4 res = f32x4(0);
  float st = 2.0f / 1.0f;
  for (float x = -1.f; x < 1.f; x += st) {
    for (float y = -1.f; y < 1.f; y += st) {
      res += getColorAtPix(__ctx, fc + (f32x2(x, y) + 0.5f), m);
    }
  }
  return res / (1.0f * 1.0f);
}
f32x4 stochasticAA(Context &__ctx, f32x2 fc, f32x4 m) {
  f32x4 res = f32x4(0);
  float st = 1.0f / (1.0f - 1.f);
  res += getColorAtPix(__ctx, fc, m);
  if (1.0f > 1.f) {
    for (float x = 0.f; x < 1.f; x += st) {
      res += getColorAtPix(
          __ctx, fc + (hash22(__ctx, (x + iTime + fc) * 25.f) - 0.5f), m);
    }
  }
  return res / 1.0f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x4 col = stochasticAA(__ctx, fragCoord, iMouse);
  fragColor =
      col +
      displayVar(__ctx,
                 printFR(__ctx,
                         (fragCoord - iResolution.swizzle<0, 1>() * 0.5f) /
                             iResolution.data[1] * 0.55f,
                         iFrameRate));
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_cube<f32x4> channel0,
               __binding__(1, 0) sampler sampling0,
               __binding__(0, 2) texture_3d<f32x4> channel2,
               __binding__(1, 2) sampler sampling2) {
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
  Context __ctx{{channel0, sampling0}, {channel2, sampling2}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
