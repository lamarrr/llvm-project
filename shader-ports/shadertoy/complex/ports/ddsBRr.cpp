// Port of https://www.shadertoy.com/view/ddsBRr; author: jsulpis.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/ddsBRr.json.
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
f32x4 mod289(Context &__ctx, f32x4 x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}
f32x4 permute(Context &__ctx, f32x4 x) {
  return mod289(__ctx, ((x * 34.0f) + 1.0f) * x);
}
float noise(Context &__ctx, f32x3 p) {
  f32x3 a = floor(p);
  f32x3 d = p - a;
  d = d * d * (3.0f - 2.0f * d);
  f32x4 b = a.swizzle<0, 0, 1, 1>() + f32x4(0.0f, 1.0f, 0.0f, 1.0f);
  f32x4 k1 = permute(__ctx, b.swizzle<0, 1, 0, 1>());
  f32x4 k2 = permute(__ctx, k1.swizzle<0, 1, 0, 1>() + b.swizzle<2, 2, 3, 3>());
  f32x4 c = k2 + a.swizzle<2, 2, 2, 2>();
  f32x4 k3 = permute(__ctx, c);
  f32x4 k4 = permute(__ctx, c + 1.0f);
  f32x4 o1 = fract(k3 * (1.0f / 41.0f));
  f32x4 o2 = fract(k4 * (1.0f / 41.0f));
  f32x4 o3 = o2 * d.data[2] + o1 * (1.0f - d.data[2]);
  f32x2 o4 =
      o3.swizzle<1, 3>() * d.data[0] + o3.swizzle<0, 2>() * (1.0f - d.data[0]);
  return o4.data[1] * d.data[1] + o4.data[0] * (1.0f - d.data[1]);
}
float fbm(Context &__ctx, f32x3 p, int octaves, float persistence,
          float lacunarity, float exponentiation) {
  float amplitude = 0.5f;
  float frequency = 2.0f;
  float total = 0.0f;
  float normalization = 0.0f;
  for (int i = 0; i < octaves; ++i) {
    float noiseValue = noise(__ctx, p * frequency);
    total += noiseValue * amplitude;
    normalization += amplitude;
    amplitude *= persistence;
    frequency *= lacunarity;
  }
  total /= normalization;
  total = total * 0.8f + 0.1f;
  total = pow(total, exponentiation);
  return total;
}
float sphOcclusion(Context &__ctx, f32x3 pos, f32x3 nor, f32x3 origin,
                   float radius) {
  f32x3 di = origin - pos;
  float l = length(di);
  float nl = dot(nor, di / l);
  float h = l / radius;
  float h2 = h * h;
  float k2 = 1.0f - h2 * nl * nl;
  float res = max(0.0f, nl) / h2;
  if (k2 > 0.001f) {
    res = nl * acos(-nl * sqrt((h2 - 1.0f) / (1.0f - nl * nl))) -
          sqrt(k2 * (h2 - 1.0f));
    res = res / h2 + atan(sqrt(k2 / (h2 - 1.0f)));
    res /= 3.141593f;
  }
  return res;
}
f32x3x3 rotateY(Context &__ctx, float angle) {
  float c = cos(angle);
  float s = sin(angle);
  return f32x3x3(f32x3(c, 0, s), f32x3(0, 1, 0), f32x3(-s, 0, c));
}
float sphereSDF(Context &__ctx, f32x3 p) {
  f32x3 position =
      (p - f32x3(0, 1, 6)) * rotateY(__ctx, iTime * .1f) + f32x3(0, 1, 6);
  float noise = fbm(__ctx, position, 6, .5f, 2.f, 4.f) * .5f;
  return length(position - f32x3(0, 1, 6)) - (2.f + noise);
}
float rayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.f;
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + rd * dO;
    float dS = sphereSDF(__ctx, p);
    dO += dS;
    if (dO > 100.f || dS < .001f)
      break;
  }
  return dO;
}
f32x3 getNormal(Context &__ctx, f32x3 p) {
  float dist = sphereSDF(__ctx, p);
  f32x2 e = f32x2(.001f, 0);
  f32x3 normal = dist - f32x3(sphereSDF(__ctx, p - e.swizzle<0, 1, 1>()),
                              sphereSDF(__ctx, p - e.swizzle<1, 0, 1>()),
                              sphereSDF(__ctx, p + e.swizzle<1, 1, 0>()));
  return normalize(normal);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord / iResolution.swizzle<0, 1>() - 0.5f) *
             iResolution.swizzle<0, 1>() / iResolution.data[1];
  f32x3 ro = f32x3(0, 1, 0);
  f32x3 rd = normalize(f32x3(uv, 1));
  f32x3 color = f32x3(0.01f);
  float distance = rayMarch(__ctx, ro, rd);
  f32x3 position = ro + rd * distance;
  float backgroundMask = step(100.f, distance);
  float sphereAO = sphOcclusion(__ctx, position, getNormal(__ctx, position),
                                f32x3(0, 1, 6), 2.f);
  f32x3 sphereColor = f32x3(sphereAO);
  color = mix(color, sphereColor, 1.0f - backgroundMask);
  color = pow(color, f32x3(1.0f / 2.2f));
  fragColor = f32x4(color, 1.0f);
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
