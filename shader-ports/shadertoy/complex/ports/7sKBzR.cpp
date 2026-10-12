// Port of https://www.shadertoy.com/view/7sKBzR; author: BeardThings.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7sKBzR.json.
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
int factorial(Context &__ctx, int v) {
  int t = 1;
  for (int i = 2; i < v; i++) {
    t *= i;
  }
  return t;
}
float P(Context &__ctx, int l, int m, float x) {
  float pmm = 1.0f;
  float somx2 = sqrt((1.0f - x) * (1.0f + x));
  float fact = 1.0f;
  for (int i = 1; i <= m; i++) {
    pmm *= (-fact) * somx2;
    fact += 2.0f;
  }
  if (l == m)
    return pmm;
  float pmmp1 = x * (2.0f * float(m) + 1.0f) * pmm;
  if (l == m + 1)
    return pmmp1;
  float pll = 0.0f;
  for (float ll = float(m + 2); ll <= float(l); ll += 1.0f) {
    pll = ((2.0f * ll - 1.0f) * x * pmmp1 - (ll + float(m) - 1.0f) * pmm) /
          (ll - float(m));
    pmm = pmmp1;
    pmmp1 = pll;
  }
  return pll;
}
float K(Context &__ctx, int l, int m) {
  float n = float((2 * l + 1) * factorial(__ctx, l - m));
  float d = 4.0f * 3.1415926f * float(factorial(__ctx, l + m));
  return sqrt(n / d);
}
float y(Context &__ctx, int l, int m, float cosTheta, float phi) {
  float v = K(__ctx, l, abs(m)) * P(__ctx, l, abs(m), cosTheta);
  if (m != 0)
    v *= sqrt(2.0f);
  if (m > 0)
    v *= cos(float(m) * phi);
  if (m < 0)
    v *= sin(float(-m) * phi);
  return v;
}
f32x3 RayDirection(Context &__ctx, f32x3 forward, float focalDistance,
                   f32x3 right, f32x3 up, f32x2 screenSpaceCoord,
                   float aspectRatioXOverY) {
  return normalize(forward * focalDistance +
                   right * screenSpaceCoord.data[0] * aspectRatioXOverY +
                   up * screenSpaceCoord.data[1]);
}
float Sphere(Context &__ctx, f32x3 p, float r) { return length(p) - r; }
float SDF(Context &__ctx, f32x3 p, f32x3 &c) {
  f32x3 d = normalize(p);
  float cosTheta = d.data[1];
  float phi = atan(d.data[2], d.data[0]);
  float r = y(__ctx, 2, 0, cosTheta, phi);
  float sphereIntersect = Sphere(__ctx, p, abs(r));
  c = mix(f32x3(1.0f, 0.0f, 0.0f), f32x3(0.0f, 1.0f, 0.0f),
          sign(r) * 0.5f + 0.5f);
  return sphereIntersect;
}
f32x3 Norm(Context &__ctx, f32x3 p) {
  const float cEps = 0.01f;
  f32x3 c;
  return normalize(f32x3(SDF(__ctx, p + f32x3(cEps, 0, 0), c) -
                             SDF(__ctx, p - f32x3(cEps, 0, 0), c),
                         SDF(__ctx, p + f32x3(0, cEps, 0), c) -
                             SDF(__ctx, p - f32x3(0, cEps, 0), c),
                         SDF(__ctx, p + f32x3(0, 0, cEps), c) -
                             SDF(__ctx, p - f32x3(0, 0, cEps), c)));
}
f32x4 Sample(Context &__ctx, f32x3 o, f32x3 direction) {
  f32x4 fc;
  float d = 0.0f;
  for (float i = 0.f; i < 64.f; ++i) {
    f32x3 pos = o + direction * d;
    f32x3 c;
    float sdf = SDF(__ctx, pos, c);
    d += sdf * 0.25f;
    fc = f32x4(direction.swizzle<0, 1>() * 0.5f + 0.5f, 1.0f, 1.0f) * 0.5f;
    const float EPSILON = 0.01f;
    if (sdf < EPSILON) {
      fc.set_swizzle<0, 1, 2>(
          c *
          (dot(Norm(__ctx, pos), f32x3(0.707f, 0.707f, 0.0f)) * 0.5f + 0.5f));
      break;
    }
    if (d > 30.f) {
      break;
    }
  }
  return fc;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 screenSpace = uv * 2.0f - 1.0f;
  const float height = 0.0f;
  f32x3 o = f32x3(sin(iTime), 0.0f, -cos(iTime)) * 2.0f;
  f32x3 f = normalize(-o);
  f32x3 u = f32x3(0, 1, 0);
  f32x3 r = normalize(cross(f, u));
  f32x3 direction = RayDirection(__ctx, f, 2.0f, r, u, screenSpace,
                                 iResolution.data[0] / iResolution.data[1]);
  fragColor = Sample(__ctx, o, direction);
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
