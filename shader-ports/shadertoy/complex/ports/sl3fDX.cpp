// Port of https://www.shadertoy.com/view/sl3fDX; author: fishy.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/sl3fDX.json.
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

struct material {
  f32x3 baseColor;
  float roughness;
};
f32x3 aces_tonemap(Context &__ctx, f32x3 color) {
  f32x3x3 m1 = f32x3x3(0.59719f, 0.07600f, 0.02840f, 0.35458f, 0.90834f,
                       0.13383f, 0.04823f, 0.01566f, 0.83777f);
  f32x3x3 m2 = f32x3x3(1.60475f, -0.10208f, -0.00327f, -0.53108f, 1.10813f,
                       -0.07276f, -0.07367f, -0.00605f, 1.07602f);
  f32x3 v = m1 * color;
  f32x3 a = v * (v + 0.0245786f) - 0.000090537f;
  f32x3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
  return pow(clamp(m2 * (a / b), 0.0f, 1.0f), f32x3(1.0f / 2.2f));
}
float calcSpecular(Context &__ctx, f32x3 e, f32x3 n, f32x3 l, float p) {
  return pow(clamp(dot(l, reflect(e, n)), 0.0f, 1.0f), p);
}
float join(Context &__ctx, float &a, float b, material bm, material &m) {
  if (a < b) {
    return a;
  } else {
    m = bm;
    return b;
  }
}
float sdPlane(Context &__ctx, f32x3 p, float y) { return p.data[1] - y; }
float sdSphere(Context &__ctx, f32x3 p, f32x3 pos, float r) {
  return length(p - pos) - r;
}
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
float sdBox(Context &__ctx, f32x3 p, f32x3 b, f32x3 bPos) {
  return sdBox(__ctx, p - bPos, b);
}
float sceneDist(Context &__ctx, f32x3 p, material &m) {
  m = material(f32x3(0.0f, 0.0f, 1.0f), 1.0f);
  float dist = sdSphere(__ctx, p, f32x3(0.3f, 0.0f, 2.0f), 0.2f);
  dist = join(__ctx, dist, sdSphere(__ctx, p, f32x3(-0.3f, 0.0f, 2.0f), 0.2f),
              material(f32x3(0.0f, 0.5f, 1.0f), 8.0f), m);
  dist =
      join(__ctx, dist,
           sdBox(__ctx, p, f32x3(0.5f, 0.1f, 0.5f), f32x3(0.0f, -0.3f, 2.0f)),
           material(f32x3(1.0f, 0.0f, 0.0f), 8.0f), m);
  dist = join(__ctx, dist, sdPlane(__ctx, p, -0.3f),
              material(f32x3(0.0f, 1.0f, 0.0f), 8.0f), m);
  return dist;
}
f32x3 calcNormal(Context &__ctx, f32x3 p) {
  const float h = 0.0001f;
  const f32x2 k = f32x2(1, -1);
  material m;
  return normalize(
      k.swizzle<0, 1, 1>() * sceneDist(__ctx, p + k.swizzle<0, 1, 1>() * h, m) +
      k.swizzle<1, 1, 0>() * sceneDist(__ctx, p + k.swizzle<1, 1, 0>() * h, m) +
      k.swizzle<1, 0, 1>() * sceneDist(__ctx, p + k.swizzle<1, 0, 1>() * h, m) +
      k.swizzle<0, 0, 0>() * sceneDist(__ctx, p + k.swizzle<0, 0, 0>() * h, m));
}
void march(Context &__ctx, f32x3 dir, f32x3 &p, int maxSteps, float minDist,
           bool &hit, float &depth, f32x3 &normal, material &m) {
  f32x3 oldP = p;
  depth = 0.0f;
  for (int i = 0; i < maxSteps; i++) {
    depth = sceneDist(__ctx, p, m);
    p += depth * dir;
    hit = depth < minDist;
    if (hit) {
      normal = calcNormal(__ctx, p);
      depth = distance(oldP, p);
      return;
    }
  }
  normal = calcNormal(__ctx, p);
}
void marchShadow(Context &__ctx, f32x3 dir, f32x3 p, int maxSteps,
                 float minDist, float &inShadow, float k) {
  f32x3 oldP = p;
  float dist = 0.002f;
  float totalDist = dist;
  p += dist * dir;
  inShadow = 1.0f;
  material m = material(f32x3(0.0f, 0.0f, 0.0f), 0.0f);
  for (int i = 0; i < maxSteps; i++) {
    dist = sceneDist(__ctx, p, m);
    totalDist += dist;
    if (dist < minDist) {
      inShadow = 1.0f;
      return;
    }
    p += dist * dir;
    inShadow = min(inShadow, k * dist / totalDist);
  }
  inShadow = 1.0f - inShadow;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  uv -= 0.5f;
  uv.data[1] *= iResolution.data[1] / iResolution.data[0];
  f32x3 eyeDir = normalize(f32x3(uv * f32x2(1.0f, 1.0f), 1.0f));
  f32x3 eyePos = f32x3(0.0f, 0.0f, 0.0f);
  f32x2 mouse =
      iMouse.data[2] > 0.0f
          ? iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - 0.5f
          : f32x2(sin(iTime), cos(iTime) + 2.0f);
  f32x3 lightingDir = normalize(f32x3(
      (mouse)*f32x2(1.0f, iResolution.data[1] / iResolution.data[0]) * 10.0f,
      -1.0f));
  bool hit = false;
  float depth = 0.0f;
  f32x3 normal = f32x3(0.0f);
  material hitMat;
  march(__ctx, eyeDir, eyePos, 512, 0.0001f, hit, depth, normal, hitMat);
  float inShadow = 0.0f;
  marchShadow(__ctx, lightingDir, eyePos, 64, 0.00001f, inShadow, 4.0f);
  float lighting = max(0.0f, dot(normal, lightingDir));
  float specular =
      calcSpecular(__ctx, eyeDir, normal, lightingDir, 8.0f * hitMat.roughness);
  specular = clamp(specular, 0.0f, 1.0f);
  inShadow *= float(hit);
  f32x3 color =
      (f32x3(lighting + specular) * hitMat.baseColor) * (1.0f - inShadow);
  color = aces_tonemap(__ctx, color);
  fragColor = f32x4(hit ? color : f32x3(0.0f), 1.0f);
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
