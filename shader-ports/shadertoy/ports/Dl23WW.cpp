// Port of https://www.shadertoy.com/view/Dl23WW; author: remaindeer.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Dl23WW.json.
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
f32x2x2 rotmat2(Context &__ctx, float angle) {
  float c = cos(angle), s = sin(angle);
  return f32x2x2(c, -s, s, c);
}
f32x3x3 rotmat3(Context &__ctx, f32x3 angle) {
  float sintht = sin(angle.data[0]), sinpsi = sin(angle.data[1]),
        sinphi = sin(angle.data[2]);
  float costht = cos(angle.data[0]), cospsi = cos(angle.data[1]),
        cosphi = cos(angle.data[2]);
  return f32x3x3(costht * cospsi, sintht * cospsi, -sinpsi,
                 costht * sinpsi * sinphi - sintht * cosphi,
                 sintht * sinpsi * sinphi + costht * cosphi, cospsi * sinphi,
                 costht * sinpsi * cosphi + sintht * sinphi,
                 sintht * sinpsi * cosphi - costht * sinphi, cospsi * cosphi);
}
float ndot(Context &__ctx, f32x2 a, f32x2 b) {
  return a.data[0] * b.data[0] - a.data[1] * b.data[1];
}
float sdSphere(Context &__ctx, f32x3 p, float s) { return length(p) - s; }
float sdRoundCone(Context &__ctx, f32x3 p, float r1, float r2, float h) {
  float b = (r1 - r2) / h;
  float a = sqrt(1.0f - b * b);
  f32x2 q = f32x2(length(p.swizzle<0, 2>()), p.data[1]);
  float k = dot(q, f32x2(-b, a));
  if (k < 0.0f)
    return length(q) - r1;
  if (k > a * h)
    return length(q - f32x2(0.0f, h)) - r2;
  return dot(q, f32x2(a, b)) - r1;
}
float sdCutHollowSphere(Context &__ctx, f32x3 p, float r, float h, float t) {
  f32x2 q = f32x2(length(p.swizzle<0, 2>()), p.data[1]);
  float w = sqrt(r * r - h * h);
  return ((h * q.data[0] < w * q.data[1]) ? length(q - f32x2(w, h))
                                          : abs(length(q) - r)) -
         t;
}
float sdRhombus(Context &__ctx, f32x3 p, float la, float lb, float h,
                float ra) {
  p = abs(p);
  f32x2 b = f32x2(la, lb);
  float f = clamp((ndot(__ctx, b, b - 2.0f * p.swizzle<0, 2>())) / dot(b, b),
                  -1.0f, 1.0f);
  f32x2 q =
      f32x2(length(p.swizzle<0, 2>() - 0.5f * b * f32x2(1.0f - f, 1.0f + f)) *
                    sign(p.data[0] * b.data[1] + p.data[2] * b.data[0] -
                         b.data[0] * b.data[1]) -
                ra,
            p.data[1] - h);
  return min(max(q.data[0], q.data[1]), 0.0f) + length(max(q, 0.0f));
}
float sdVerticalCapsule(Context &__ctx, f32x3 p, float h, float r) {
  p.data[1] -= clamp(p.data[1], 0.0f, h);
  return length(p) - r;
}
float sdCappedTorus(Context &__ctx, f32x3 p, f32x2 sc, float ra, float rb) {
  p.data[0] = abs(p.data[0]);
  float k = (sc.data[1] * p.data[0] > sc.data[0] * p.data[1])
                ? dot(p.swizzle<0, 1>(), sc)
                : length(p.swizzle<0, 1>());
  return sqrt(dot(p, p) + ra * ra - 2.0f * ra * k) - rb;
}
float sdLink(Context &__ctx, f32x3 p, float le, float r1, float r2) {
  f32x3 q = f32x3(p.data[0], max(abs(p.data[1]) - le, 0.0f), p.data[2]);
  return length(f32x2(length(q.swizzle<0, 1>()) - r1, q.data[2])) - r2;
}
float scene(Context &__ctx, f32x3 p) {
  float d;
  f32x3 q;
  q = rotmat3(__ctx, f32x3(0, 0, radians(75.0f))) * p;
  d = sdRoundCone(__ctx, q - f32x3(0), 0.15f, 0.25f, 0.35f);
  q = p;
  q.data[0] = abs(q.data[0]);
  d = min(d, sdSphere(__ctx, q - f32x3(+0.175f, 0.25f, -0.205f), 0.10f));
  q = p;
  q.data[0] = abs(q.data[0]);
  q = rotmat3(__ctx, f32x3(0, 0, radians(90.0f))) * q;
  d = min(d, sdCutHollowSphere(__ctx, q - f32x3(+0.175f, 0.205f, 0.25f), 0.10f,
                               -0.085f, 0.01f));
  q = p;
  q.data[0] = abs(q.data[0]);
  q = rotmat3(__ctx, f32x3(radians(10.0f), radians(15.0f), radians(90.0f))) * q;
  d = min(d, sdRhombus(__ctx, q - f32x3(+0.35f, +0.40f, 0.20f), 0.25f, 0.15f,
                       0.05f, 0.001f));
  q = p;
  q.data[0] = abs(q.data[0]);
  q = rotmat3(__ctx, f32x3(radians(15.0f), 0, 0)) * q;
  d = min(d, sdVerticalCapsule(__ctx, q - f32x3(+0.015f, 0.3f, -0.325f), 0.35f,
                               0.05f));
  q = p;
  q.data[0] = abs(q.data[0]);
  d = min(d, sdCappedTorus(__ctx, q - f32x3(+0.175f, 0.25f, -0.205f),
                           f32x2(0.25f), 0.10f, 0.05f));
  q = p;
  d = min(d, sdCappedTorus(__ctx, q - f32x3(-0.025f, +0.015f, 0.15f), f32x2(0),
                           0.0125f, 0.0075f));
  d = min(d, sdCappedTorus(__ctx, q - f32x3(+0.025f, -0.015f, 0.15f), f32x2(0),
                           0.0125f, 0.0075f));
  q = rotmat3(__ctx, f32x3(radians(30.0f), 0, 0)) * p;
  d = min(d, sdVerticalCapsule(__ctx, q - f32x3(0, -0.030f, 0.15f), 0.060f,
                               0.0075f));
  q = p;
  q = rotmat3(__ctx, f32x3(0, 0, radians(80.0f))) * (q - f32x3(0, -0.075f, 0));
  d = min(d, sdLink(__ctx, q, 0.10f, 0.040f, 0.025f));
  q = rotmat3(__ctx, f32x3(0, 0, radians(30.0f))) *
      (p - f32x3(0, 0.15f, -0.5f));
  d = min(d, sdLink(__ctx, q, 0.15f, 0.045f, 0.025f));
  return d;
}
float march(Context &__ctx, f32x3 ro, f32x3 rd) {
  float depth = 0.0f;
  float d;
  for (int i = 0; i < 500; i++) {
    f32x3 p = ro + depth * rd;
    d = scene(__ctx, p);
    depth += d;
    if (d < 0.001f || depth > 100.0f) {
      break;
    }
  }
  d = depth;
  return d;
}
f32x3 normal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(1, -1) * 0.0005f;
  return normalize(
      e.swizzle<0, 1, 1>() * scene(__ctx, p + e.swizzle<0, 1, 1>()) +
      e.swizzle<1, 1, 0>() * scene(__ctx, p + e.swizzle<1, 1, 0>()) +
      e.swizzle<1, 0, 1>() * scene(__ctx, p + e.swizzle<1, 0, 1>()) +
      e.swizzle<0, 0, 0>() * scene(__ctx, p + e.swizzle<0, 0, 0>()));
}
f32x3x3 camera(Context &__ctx, f32x3 cameraPos, f32x3 lookAtPoint) {
  f32x3 cd = normalize(lookAtPoint - cameraPos);
  f32x3 cr = normalize(cross(f32x3(0, 1, 0), cd));
  f32x3 cu = normalize(cross(cd, cr));
  return f32x3x3(-cr, cu, -cd);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  if (m == f32x2(0.0f))
    m = f32x2(0.5f);
  f32x3 col = f32x3(0);
  f32x3 lp = f32x3(0);
  f32x3 ro = f32x3(0, 0, 1.5f);
  float camRadius = 1.0f;
  ro.set_swizzle<1, 2>(
      ro.swizzle<1, 2>() * camRadius *
      rotmat2(__ctx, mix(-3.1415926535897932384626433f / 2.0f,
                         3.1415926535897932384626433f / 2.0f, m.data[1])));
  ro.set_swizzle<0, 2>(
      ro.swizzle<0, 2>() *
          rotmat2(__ctx, mix(-3.1415926535897932384626433f,
                             3.1415926535897932384626433f, m.data[0])) +
      f32x2(lp.data[0], lp.data[2]));
  f32x3 rd = camera(__ctx, ro, lp) * normalize(f32x3(p, -1));
  float d = march(__ctx, ro, rd);
  if (d > 100.0f) {
    col = f32x3(0, 0, 0);
  } else {
    f32x3 p = ro + rd * d;
    f32x3 norm = normal(__ctx, p);
    f32x3 lightPos = f32x3(0, 1, 1);
    f32x3 lightDir = normalize(lightPos - p) * 0.65f;
    float diff = clamp(dot(norm, lightDir), 0.0f, 1.0f) * 0.5f + 0.5f;
    f32x3 rnd =
        0.5f + 0.5f * cos(iTime + p.swizzle<0, 1, 0>() + f32x3(0, 2, 4));
    rnd.set_swizzle<0, 1>(f32x2(0.0f));
    col = f32x3(diff) + rnd;
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
