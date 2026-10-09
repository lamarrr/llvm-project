// Port of https://www.shadertoy.com/view/mslBRr; author: jsulpis.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/mslBRr.json.
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
  Texture3D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float sphIntersect(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 ce, float ra) {
  f32x3 oc = ro - ce;
  float b = dot(oc, rd);
  float c = dot(oc, oc) - ra * ra;
  float h = b * b - c;
  if (h < 0.0f)
    return 1e10f;
  return -b - sqrt(h);
}
float noise(Context &__ctx, f32x3 p) {
  return texture(__ctx.channel0, p * .05f).data[0];
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
float fbmSphereDist(Context &__ctx, f32x3 ro, f32x3 rd) {
  float smoothSphereDist = sphIntersect(__ctx, ro, rd, f32x3(0, 1, 6), 2.f);
  f32x3 intersection = ro + smoothSphereDist * rd;
  float fresnel = clamp(dot(rd, f32x3(0, 1, 6) - intersection), 0.f, 1.f);
  f32x3 intersectionWithRotation =
      (intersection - f32x3(0, 1, 6)) * rotateY(__ctx, iTime * .1f) +
      f32x3(0, 1, 6);
  float noise =
      fbm(__ctx, intersectionWithRotation, 6, .5f, 2.f, 4.5f) * .5f * fresnel;
  return sphIntersect(__ctx, ro, rd, f32x3(0, 1, 6), 2.f + noise);
}
f32x3 getNormal(Context &__ctx, f32x3 p) {
  f32x3 rd = f32x3(0, 1, 6) - p;
  float dist = fbmSphereDist(__ctx, p, rd);
  f32x2 e = f32x2(.001f, 0);
  f32x3 normal =
      dist - f32x3(fbmSphereDist(__ctx, p - e.swizzle<0, 1, 1>(), rd),
                   fbmSphereDist(__ctx, p - e.swizzle<1, 0, 1>(), rd),
                   fbmSphereDist(__ctx, p + e.swizzle<1, 1, 0>(), rd));
  return normalize(normal);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord / iResolution.swizzle<0, 1>() - 0.5f) *
             iResolution.swizzle<0, 1>() / iResolution.data[1];
  f32x3 ro = f32x3(0, 1, 0);
  f32x3 rd = normalize(f32x3(uv, 1));
  f32x3 color = f32x3(0.01f);
  float distance = fbmSphereDist(__ctx, ro, rd);
  f32x3 position = ro + rd * distance;
  float backgroundMask = step(1e10f, distance);
  float sphereAO = sphOcclusion(__ctx, position, getNormal(__ctx, position),
                                f32x3(0, 1, 6), 2.f);
  f32x3 sphereColor = f32x3(sphereAO);
  color = mix(color, sphereColor, 1.0f - backgroundMask);
  color = pow(color, f32x3(1.0f / 2.2f));
  fragColor = f32x4(color, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_3d<f32x4> channel0,
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
