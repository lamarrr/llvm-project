// Port of https://www.shadertoy.com/view/mlcXWs; author: silica163.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mlcXWs.json.
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
const int LIGHTS = 4;
const int SPHERES = 3;
const int CUBES = 2;
struct Ray {
  f32x3 o;
  f32x3 p;
  f32x3 dir;
  f32x3 c;
  float dist;
  float rl;
};
struct Sphere {
  f32x4 p;
  f32x3 color;
};
struct Cube {
  f32x3 p;
  f32x3 c;
  f32x3 s;
};
struct Light {
  f32x3 p;
  f32x3 c;
  f32x3 L;
};
Sphere sp[SPHERES];
Cube cu[CUBES];
Light l[LIGHTS];
void init(Context &__ctx) {
  sp[0] = Sphere(f32x4(0, 0, 0, 1.3f), f32x3(0));
  sp[1] = Sphere(f32x4(0, 0, -.5f, .4f), f32x3(0));
  sp[2] = Sphere(f32x4(0, 0, .5f, .4f), f32x3(0));
  cu[0] = Cube(f32x3(0, 0, 0), f32x3(0), f32x3(1));
  cu[1] = Cube(f32x3(0, -1, 0), f32x3(0), f32x3(3.f, .01f, 3.f));
  l[0].p = f32x3(sin(2.f / 3.f * 3.14159265f * 2.f) * 1.5f, 2,
                 cos(2.f / 3.f * 3.14159265f * 2.f) * 1.5f);
  l[0].c = f32x3(0, 0, 1);
  l[1].p = f32x3(sin(1.f / 3.f * 3.14159265f * 2.f) * 1.5f, 2,
                 cos(1.f / 3.f * 3.14159265f * 2.f) * 1.5f);
  l[1].c = f32x3(1, 0, 0);
  l[2].p = f32x3(sin(0.f) * 1.5f, 2, cos(0.f) * 1.5f);
  l[2].c = f32x3(0, 1, 0);
  l[3].p = f32x3(0, 5, 0);
  l[3].c = f32x3(1);
}
f32x3 N = f32x3(0);
f32x3 V = f32x3(0);
float max3(Context &__ctx, f32x3 n) {
  return max(max(n.data[0], n.data[1]), n.data[2]);
}
float cubeSDF(Context &__ctx, f32x3 p, int id) {
  f32x3 c = abs(p - cu[id].p) - cu[id].s;
  return length(max(c, 0.f)) + min(0.f, max3(__ctx, c));
}
float sphereSDF(Context &__ctx, f32x3 p, int id) {
  float d = length(p - sp[id].p.swizzle<0, 1, 2>()) - sp[id].p.data[3];
  return d;
}
float dist(Context &__ctx, f32x3 p) {
  float d = 0.f;
  d = sphereSDF(__ctx, p, 0);
  d = max(cubeSDF(__ctx, p, 0) - .01f, -d);
  d = min(d, cubeSDF(__ctx, p, 1));
  d = min(d, sphereSDF(__ctx, p, 1));
  d = min(d, sphereSDF(__ctx, p, 2));
  return d;
}
float lightDist(Context &__ctx, f32x3 p, float d) {
  for (int i = 0; i < LIGHTS; i++)
    d = min(d, length(p - l[i].p) - .1f);
  return d;
}
void march(Context &__ctx, Ray &r, int t) {
  for (int i = 0; i < 128; i++) {
    r.p = r.o + r.dist * r.dir;
    float d = dist(__ctx, r.p);
    d = t == 0 ? d : min(d, lightDist(__ctx, r.p, d));
    if (d <= .001f || d > 20.f)
      break;
    r.dist += d;
  }
  r.rl += r.dist;
}
f32x3 sNormal(Context &__ctx, f32x3 p) {
  f32x2 t = f32x2(.001f, 0);
  float d = dist(__ctx, p);
  return normalize(d - f32x3(dist(__ctx, p - t.swizzle<0, 1, 1>()),
                             dist(__ctx, p - t.swizzle<1, 0, 1>()),
                             dist(__ctx, p - t.swizzle<1, 1, 0>())));
}
float diffuse(Context &__ctx, f32x3 p, f32x3 L) {
  return max(dot(sNormal(__ctx, p), normalize(L)), 0.f);
}
f32x3 colorDiff(Context &__ctx, Light l, Ray r) {
  f32x3 c = f32x3(0);
  float d = diffuse(__ctx, r.p, l.L);
  c += l.c * d;
  return c;
}
float bPhong(Context &__ctx, Light l, Ray r) {
  float c = 0.f;
  float d = diffuse(__ctx, r.p, l.L);
  c += pow(max(dot(normalize(l.L + V), N), 0.f), 100.f) * 2.f * d;
  return c;
}
float shadowMask(Context &__ctx, f32x3 p, f32x3 l) {
  float m = 1.f;
  float x = 10.f;
  Ray r = Ray(p + N * .001f * x, p + N * .001f * x, normalize(l - p), f32x3(0),
              0.f, 0.f);
  march(__ctx, r, 0);
  if (r.dist < length(l - p))
    m = 0.1f;
  return m;
}
void lNormal(Context &__ctx, Light &l, Ray r) { l.L = normalize(l.p - r.p); }
f32x3 getColor(Context &__ctx, Light l, Ray r) {
  return ((colorDiff(__ctx, l, r) + bPhong(__ctx, l, r) * l.c) *
          f32x3(shadowMask(__ctx, r.p, l.p))) /
         length(r.p - l.p);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>()) * 2.f - 1.f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  uv *= 2.f;
  f32x2 m = f32x2(iTime * .1f, -abs(sin(iTime / 40.f) * 1.f));
  float camz = 3.14159265f;
  f32x3 ca = f32x3(3.14159265f / 2.f + m.data[1] * 3.14159265f / 2.f,
                   3.14159265f / 2.f + m.data[0] * 3.14159265f / 1.f, 0);
  f32x3x3 rotX = f32x3x3(1, 0, 0, 0, sin(ca.data[0]), cos(ca.data[0]), 0,
                         -cos(ca.data[0]), sin(ca.data[0]));
  f32x3x3 rotY = f32x3x3(sin(ca.data[1]), 0, cos(ca.data[1]), 0, 1, 0,
                         -cos(ca.data[1]), 0, sin(ca.data[1]));
  f32x3 camera = f32x3(0, 0, camz) * rotX * rotY;
  f32x3 plane = (f32x3(uv, 0) * rotX * rotY);
  Ray r = Ray(camera, camera, normalize(plane - camera), f32x3(0), 0.f, 0.f);
  init(__ctx);
  march(__ctx, r, 1);
  f32x3 c = f32x3(.2f);
  N = sNormal(__ctx, r.p);
  V = -r.dir;
  Ray fr = r;
  for (int i = 0; i < LIGHTS; i++) {
    lNormal(__ctx, l[i], r);
    c += getColor(__ctx, l[i], r);
  }
  f32x3 fc = c;
  for (int i = 0; i < 3; i++) {
    r.dir = reflect(r.dir, N);
    r.o = r.p + N * .001f;
    r.p = r.o;
    r.dist = 0.f;
    march(__ctx, r, 1);
    N = sNormal(__ctx, r.p);
    if (r.rl > 20.f)
      break;
    V = -r.dir;
    for (int i = 0; i < LIGHTS; i++) {
      lNormal(__ctx, l[i], r);
      c += (getColor(__ctx, l[i], r) * pow(.9f, float(i)) * 2.f / r.rl);
      if (length(r.p - l[i].p) - .1f <= .001f)
        c = l[i].c * 5.f;
    }
  }
  if (fr.dist > 20.f)
    c = f32x3(.1f);
  for (int i = 0; i < LIGHTS; i++) {
    if (length(fr.p - l[i].p) - .1f <= .001f)
      c = l[i].c * 5.f;
  }
  fragColor = f32x4(c, 1.0f);
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
