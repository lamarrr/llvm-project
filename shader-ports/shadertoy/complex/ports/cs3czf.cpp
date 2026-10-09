// Port of https://www.shadertoy.com/view/cs3czf; author: alphardex.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cs3czf.json.
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

const float PI = 3.14159265359f;
const float TWO_PI = 6.28318530718f;
f32x2x2 rotation2d(Context &__ctx, float angle) {
  float s = sin(angle);
  float c = cos(angle);
  return f32x2x2(c, -s, s, c);
}
f32x4x4 rotation3d(Context &__ctx, f32x3 axis, float angle) {
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.f - c;
  return f32x4x4(oc * axis.data[0] * axis.data[0] + c,
                 oc * axis.data[0] * axis.data[1] - axis.data[2] * s,
                 oc * axis.data[2] * axis.data[0] + axis.data[1] * s, 0.f,
                 oc * axis.data[0] * axis.data[1] + axis.data[2] * s,
                 oc * axis.data[1] * axis.data[1] + c,
                 oc * axis.data[1] * axis.data[2] - axis.data[0] * s, 0.f,
                 oc * axis.data[2] * axis.data[0] - axis.data[1] * s,
                 oc * axis.data[1] * axis.data[2] + axis.data[0] * s,
                 oc * axis.data[2] * axis.data[2] + c, 0.f, 0.f, 0.f, 0.f, 1.f);
}
f32x2 rotate(Context &__ctx, f32x2 v, float angle) {
  return rotation2d(__ctx, angle) * v;
}
f32x3 rotate(Context &__ctx, f32x3 v, f32x3 axis, float angle) {
  return (rotation3d(__ctx, axis, angle) * f32x4(v, 1.f)).swizzle<0, 1, 2>();
}
f32x3 mod289(Context &__ctx, f32x3 x) {
  return x - floor(x * (1.f / 289.f)) * 289.f;
}
f32x4 mod289(Context &__ctx, f32x4 x) {
  return x - floor(x * (1.f / 289.f)) * 289.f;
}
f32x4 permute(Context &__ctx, f32x4 x) {
  return mod289(__ctx, ((x * 34.f) + 1.f) * x);
}
f32x4 taylorInvSqrt(Context &__ctx, f32x4 r) {
  return 1.79284291400159f - .85373472095314f * r;
}
f32x3 fade(Context &__ctx, f32x3 t) {
  return t * t * t * (t * (t * 6.f - 15.f) + 10.f);
}
float cnoise(Context &__ctx, f32x3 P) {
  f32x3 Pi0 = floor(P);
  f32x3 Pi1 = Pi0 + f32x3(1.f);
  Pi0 = mod289(__ctx, Pi0);
  Pi1 = mod289(__ctx, Pi1);
  f32x3 Pf0 = fract(P);
  f32x3 Pf1 = Pf0 - f32x3(1.f);
  f32x4 ix = f32x4(Pi0.data[0], Pi1.data[0], Pi0.data[0], Pi1.data[0]);
  f32x4 iy = f32x4(Pi0.swizzle<1, 1>(), Pi1.swizzle<1, 1>());
  f32x4 iz0 = Pi0.swizzle<2, 2, 2, 2>();
  f32x4 iz1 = Pi1.swizzle<2, 2, 2, 2>();
  f32x4 ixy = permute(__ctx, permute(__ctx, ix) + iy);
  f32x4 ixy0 = permute(__ctx, ixy + iz0);
  f32x4 ixy1 = permute(__ctx, ixy + iz1);
  f32x4 gx0 = ixy0 * (1.f / 7.f);
  f32x4 gy0 = fract(floor(gx0) * (1.f / 7.f)) - .5f;
  gx0 = fract(gx0);
  f32x4 gz0 = f32x4(.5f) - abs(gx0) - abs(gy0);
  f32x4 sz0 = step(gz0, f32x4(0.f));
  gx0 -= sz0 * (step(0.f, gx0) - .5f);
  gy0 -= sz0 * (step(0.f, gy0) - .5f);
  f32x4 gx1 = ixy1 * (1.f / 7.f);
  f32x4 gy1 = fract(floor(gx1) * (1.f / 7.f)) - .5f;
  gx1 = fract(gx1);
  f32x4 gz1 = f32x4(.5f) - abs(gx1) - abs(gy1);
  f32x4 sz1 = step(gz1, f32x4(0.f));
  gx1 -= sz1 * (step(0.f, gx1) - .5f);
  gy1 -= sz1 * (step(0.f, gy1) - .5f);
  f32x3 g000 = f32x3(gx0.data[0], gy0.data[0], gz0.data[0]);
  f32x3 g100 = f32x3(gx0.data[1], gy0.data[1], gz0.data[1]);
  f32x3 g010 = f32x3(gx0.data[2], gy0.data[2], gz0.data[2]);
  f32x3 g110 = f32x3(gx0.data[3], gy0.data[3], gz0.data[3]);
  f32x3 g001 = f32x3(gx1.data[0], gy1.data[0], gz1.data[0]);
  f32x3 g101 = f32x3(gx1.data[1], gy1.data[1], gz1.data[1]);
  f32x3 g011 = f32x3(gx1.data[2], gy1.data[2], gz1.data[2]);
  f32x3 g111 = f32x3(gx1.data[3], gy1.data[3], gz1.data[3]);
  f32x4 norm0 = taylorInvSqrt(__ctx, f32x4(dot(g000, g000), dot(g010, g010),
                                           dot(g100, g100), dot(g110, g110)));
  g000 *= norm0.data[0];
  g010 *= norm0.data[1];
  g100 *= norm0.data[2];
  g110 *= norm0.data[3];
  f32x4 norm1 = taylorInvSqrt(__ctx, f32x4(dot(g001, g001), dot(g011, g011),
                                           dot(g101, g101), dot(g111, g111)));
  g001 *= norm1.data[0];
  g011 *= norm1.data[1];
  g101 *= norm1.data[2];
  g111 *= norm1.data[3];
  float n000 = dot(g000, Pf0);
  float n100 = dot(g100, f32x3(Pf1.data[0], Pf0.swizzle<1, 2>()));
  float n010 = dot(g010, f32x3(Pf0.data[0], Pf1.data[1], Pf0.data[2]));
  float n110 = dot(g110, f32x3(Pf1.swizzle<0, 1>(), Pf0.data[2]));
  float n001 = dot(g001, f32x3(Pf0.swizzle<0, 1>(), Pf1.data[2]));
  float n101 = dot(g101, f32x3(Pf1.data[0], Pf0.data[1], Pf1.data[2]));
  float n011 = dot(g011, f32x3(Pf0.data[0], Pf1.swizzle<1, 2>()));
  float n111 = dot(g111, Pf1);
  f32x3 fade_xyz = fade(__ctx, Pf0);
  f32x4 n_z = mix(f32x4(n000, n100, n010, n110), f32x4(n001, n101, n011, n111),
                  fade_xyz.data[2]);
  f32x2 n_yz = mix(n_z.swizzle<0, 1>(), n_z.swizzle<2, 3>(), fade_xyz.data[1]);
  float n_xyz = mix(n_yz.data[0], n_yz.data[1], fade_xyz.data[0]);
  return 2.2f * n_xyz;
}
float pow5(Context &__ctx, const float v) {
  float v2 = v * v;
  return v2 * v2 * v;
}
f32x3 schlick(Context &__ctx, const f32x3 f0, const float f90,
              const float VoH) {
  float f = pow5(__ctx, 1.f - VoH);
  return f + f0 * (f90 - f);
}
f32x3 fresnel(Context &__ctx, f32x3 f0, f32x3 normal, f32x3 view) {
  return schlick(__ctx, f0, 1.f, dot(view, normal));
}
float sdSphere(Context &__ctx, f32x3 p, float r) { return length(p) - r; }
float opUnion(Context &__ctx, float d1, float d2) { return min(d1, d2); }
f32x2 opUnion(Context &__ctx, f32x2 d1, f32x2 d2) {
  return (d1.data[0] < d2.data[0]) ? d1 : d2;
}
f32x3 distort(Context &__ctx, f32x3 p) {
  float frequency = 1.7f;
  float offset = cnoise(__ctx, p / frequency + iTime * .5f);
  float t = (p.data[1] + offset) * PI * 12.f;
  float noise = (sin(t) * p.data[0] + cos(t) * p.data[2]) * 2.f;
  p += noise * .01f;
  return p;
}
f32x2 map(Context &__ctx, f32x3 p) {
  p -= f32x3(0.f, 0.f, -2.f);
  f32x2 mouse = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  p.set_swizzle<1, 2>(
      rotate(__ctx, p.swizzle<1, 2>(), -mouse.data[1] * PI + 1.f));
  p.set_swizzle<0, 2>(
      rotate(__ctx, p.swizzle<0, 2>(), -mouse.data[0] * TWO_PI));
  p = distort(__ctx, p);
  f32x2 d = f32x2(1e10f, 0.f);
  float d1 = sdSphere(__ctx, p, 1.5f);
  d = opUnion(__ctx, d, f32x2(d1, 2.f));
  return d * .5f;
}
f32x3 calcNormal(Context &__ctx, f32x3 p) {
  const float h = .0001f;
  const f32x2 k = f32x2(1, -1);
  return normalize(
      k.swizzle<0, 1, 1>() * map(__ctx, p + k.swizzle<0, 1, 1>() * h).data[0] +
      k.swizzle<1, 1, 0>() * map(__ctx, p + k.swizzle<1, 1, 0>() * h).data[0] +
      k.swizzle<1, 0, 1>() * map(__ctx, p + k.swizzle<1, 0, 1>() * h).data[0] +
      k.swizzle<0, 0, 0>() * map(__ctx, p + k.swizzle<0, 0, 0>() * h).data[0]);
}
f32x3 render(Context &__ctx, f32x2 uv) {
  f32x3 col = f32x3(0.f);
  uv = (uv - .5f) * 2.f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 ro = f32x3(0.f, 0.f, 1.f);
  f32x3 rd = normalize(f32x3(uv, 0.f) - ro);
  float depth = 0.f;
  for (int i = 0; i < 64; i++) {
    f32x3 p = ro + rd * depth;
    f32x2 t = map(__ctx, p);
    float d = t.data[0];
    float m = t.data[1];
    depth += d;
    if (d < .01f) {
      f32x3 normal = calcNormal(__ctx, p);
      f32x3 viewDir = normalize(ro - p);
      col = fresnel(__ctx, f32x3(0.f), normal, viewDir);
      break;
    }
  }
  return col;
}
f32x3 getSceneColor(Context &__ctx, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x3 col = render(__ctx, uv);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 tot = f32x3(0.f);
  float AA_size = 2.f;
  float count = 0.f;
  for (float aaY = 0.f; aaY < AA_size; aaY++) {
    for (float aaX = 0.f; aaX < AA_size; aaX++) {
      tot += getSceneColor(__ctx, fragCoord + f32x2(aaX, aaY) / AA_size);
      count += 1.f;
    }
  }
  tot /= count;
  fragColor = f32x4(tot, 1.f);
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
