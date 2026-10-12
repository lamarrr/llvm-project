// Port of https://www.shadertoy.com/view/NsBfWz; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NsBfWz.json.
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
f32x3 pal(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(6.28318f * (c * t + d));
}
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float sdBox(Context &__ctx, f32x3 p, f32x3 s) {
  p = abs(p) - s;
  return length(max(p, 0.f)) +
         min(max(p.data[0], max(p.data[1], p.data[2])), 0.f);
}
float hash(Context &__ctx, float a, float b) {
  int x = float_bits_to_int(a * a / 7.f) ^ float_bits_to_int(a),
      y = float_bits_to_int(b * b / 7.f) ^ float_bits_to_int(b);
  return float((x * x + y) * (y * y - x) - x) / 2.14e9f;
}
f32x3 erot(Context &__ctx, f32x3 p, f32x3 ax, float ro) {
  return mix(dot(ax, p) * ax, p, cos(ro)) + cross(ax, p) * sin(ro);
}
f32x3 face(Context &__ctx, f32x3 p) {
  f32x3 a = abs(p);
  return step(a.swizzle<1, 2, 0>(), a.swizzle<0, 1, 2>()) *
         step(a.swizzle<2, 0, 1>(), a.swizzle<0, 1, 2>()) * sign(p);
}
f32x3 getRo(Context &__ctx) {
  float t = 0.0f * iTime, o = 2.f * 3.14159f / 3.f;
  return 3.f * f32x3(cos(t - o), cos(t), cos(t + o));
}
f32x3 distort(Context &__ctx, f32x3 p) {
  float o = 2.f * 3.14159f / 3.f;
  float t = 0.f * length(p) - 0.25f * iTime;
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (Rot(__ctx, t - o)));
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (Rot(__ctx, t)));
  p.set_swizzle<2, 0>(p.swizzle<2, 0>() * (Rot(__ctx, t + o)));
  return p;
}
float GetDist(Context &__ctx, f32x3 p) {
  f32x3 ro = getRo(__ctx);
  float cd = length(p - ro) - 0.f;
  p = distort(__ctx, p);
  float r1 = 1.2f;
  float r2 = 0.3f;
  float d0 = length(p.swizzle<0, 2>()) - r1;
  float d1 = length(f32x2(d0, p.data[1])) - r2;
  p *= 1.8f;
  p *= log(d1 - 0.f);
  f32x3 center = floor(p) + 0.5f;
  f32x3 neighbour = center + face(__ctx, p - center);
  f32x3 pos = p - center;
  f32x3 npos = p - neighbour;
  float h = hash(__ctx, hash(__ctx, neighbour.data[0], neighbour.data[1]),
                 neighbour.data[2]);
  float o = 2.f * 3.14159f / 3.f;
  float t = 2.f * 3.14159f * h + 0.f * iTime;
  f32x3 ax = f32x3(cos(t - o), cos(t), cos(t + o));
  ax = normalize(ax);
  float e = 0.1f;
  float worst = sdBox(__ctx, npos, f32x3(0.75f)) - 0.f;
  float sq = 0.25f * sqrt(3.f);
  worst = sdBox(__ctx, npos, f32x3(0.4f)) - 0.3798f;
  float me = sdBox(__ctx, pos, f32x3(0.2f)) - 0.f;
  float k = 0.4f;
  float d = worst;
  d = -min(-d, cd);
  return k * d + 0.14f;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd, float z) {
  float dO = 0.f;
  for (int i = 0; i < 80; i++) {
    f32x3 p = ro + rd * dO;
    float dS = GetDist(__ctx, p);
    if (abs(dS) < 0.0001f || dO > 50.f)
      break;
    dO += dS * z;
  }
  return min(dO, 50.f);
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p);
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()));
  return normalize(n);
}
f32x3 GetRayDir(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i);
  return d;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 ro = getRo(__ctx);
  f32x3 rd = GetRayDir(__ctx, uv, ro, f32x3(0), 0.95f);
  f32x3 col = f32x3(0);
  float d = RayMarch(__ctx, ro, rd, 1.f);
  if (d < 50.f) {
    f32x3 p = ro + rd * d;
    f32x3 n = GetNormal(__ctx, p);
    f32x3 dp = distort(__ctx, p);
    float dif = dot(n, normalize(f32x3(1, 2, 3))) * .5f + .5f;
    float v = exp(-0.3f * pow(dot(p, p), 0.25f));
    v = smoothstep(0.f, 1.f, v);
    v = clamp(1.2f * v * v, 0.f, 1.f);
    f32x3 e = f32x3(1);
    col = v * pal(__ctx, 0.32f + 1.f * v, 0.8f * e, 0.5f * e, 0.5f * e,
                  0.8f * f32x3(0, 1, 2) / 3.f);
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
