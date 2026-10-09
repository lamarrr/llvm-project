// Port of https://www.shadertoy.com/view/mdVSRV; author: mpusch88.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mdVSRV.json.
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

float snoise(Context &__ctx, f32x3 v);
float sdSphere(Context &__ctx, f32x3 p, float r);
float sdScene(Context &__ctx, f32x3 p, float time);
f32x3 calcNormal(Context &__ctx, f32x3 p, float time);
f32x4 permute(Context &__ctx, f32x4 x) {
  return mod(((x * 34.0f) + 1.0f) * x, 289.0f);
}
f32x4 taylorInvSqrt(Context &__ctx, f32x4 r) {
  return 1.79284291400159f - 0.85373472095314f * r;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  uv = uv * 2.0f - 1.0f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  float time = iTime * 0.5f;
  f32x3 camPos = f32x3(0, 0, -4.0f);
  f32x3 camTarget = f32x3(0, 0, 0);
  f32x3 camUp = f32x3(0, 1, 0);
  f32x3 camDir = normalize(camTarget - camPos);
  f32x3 camRight = normalize(cross(camDir, camUp));
  f32x3 camForward = normalize(cross(camRight, camUp));
  f32x3 rayOrigin = camPos;
  f32x3 rayDirection =
      normalize(uv.data[0] * camRight + uv.data[1] * camUp + 2.0f * camForward);
  float t = 0.0f;
  float raymarchStep = 0.05f;
  const int maxSteps = 100;
  f32x3 pos = rayOrigin;
  for (int i = 0; i < maxSteps; i++) {
    pos = rayOrigin + t * rayDirection;
    float sdf = sdScene(__ctx, pos, time);
    if (sdf < 0.001f) {
      break;
    }
    t += sdf * raymarchStep;
  }
  f32x3 normal = calcNormal(__ctx, pos, time);
  f32x3 light = f32x3(1.0f, 1.0f, 1.0f);
  f32x3 lightDirection = normalize(light - pos);
  float diffuse = max(0.0f, dot(normal, lightDirection));
  fragColor = f32x4(f32x3(diffuse), 1.0f);
}
float sdSphere(Context &__ctx, f32x3 p, float r) { return length(p) - r; }
float sdScene(Context &__ctx, f32x3 p, float time) {
  float displacement = snoise(__ctx, p * 1.5f + time) * 0.2f;
  return sdSphere(__ctx, p, 2.0f + displacement);
}
f32x3 calcNormal(Context &__ctx, f32x3 p, float time) {
  float eps = 0.001f;
  f32x3 ex = f32x3(eps, 0, 0);
  f32x3 ey = f32x3(0, eps, 0);
  f32x3 ez = f32x3(0, 0, eps);
  float x = sdScene(__ctx, p + ex, time) - sdScene(__ctx, p - ex, time);
  float y = sdScene(__ctx, p + ey, time) - sdScene(__ctx, p - ey, time);
  float z = sdScene(__ctx, p + ez, time) - sdScene(__ctx, p - ez, time);
  return normalize(f32x3(x, y, z));
}
float snoise(Context &__ctx, f32x3 v) {
  const f32x2 C = f32x2(1.0f / 6.0f, 1.0f / 3.0f);
  const f32x4 D = f32x4(0.0f, 0.5f, 1.0f, 2.0f);
  f32x3 i = floor(v + dot(v, C.swizzle<1, 1, 1>()));
  f32x3 x0 = v - i + dot(i, C.swizzle<0, 0, 0>());
  f32x3 g = step(x0.swizzle<1, 2, 0>(), x0.swizzle<0, 1, 2>());
  f32x3 l = 1.0f - g;
  f32x3 i1 = min(g.swizzle<0, 1, 2>(), l.swizzle<2, 0, 1>());
  f32x3 i2 = max(g.swizzle<0, 1, 2>(), l.swizzle<2, 0, 1>());
  f32x3 x1 = x0 - i1 + C.swizzle<0, 0, 0>();
  f32x3 x2 = x0 - i2 + C.swizzle<1, 1, 1>();
  f32x3 x3 = x0 - D.swizzle<1, 1, 1>();
  i = mod(i, 289.0f);
  f32x4 p0 =
      permute(__ctx, i.data[2] + f32x4(0.0f, i1.data[2], i2.data[2], 1.0f));
  f32x4 p1 = permute(__ctx, p0 + i.data[1] +
                                f32x4(0.0f, i1.data[1], i2.data[1], 1.0f));
  f32x4 p = permute(__ctx,
                    p1 + i.data[0] + f32x4(0.0f, i1.data[0], i2.data[0], 1.0f));
  float n_ = 0.142857142857f;
  f32x3 ns = n_ * D.swizzle<3, 1, 2>() - D.swizzle<0, 2, 0>();
  f32x4 j = p - 49.0f * floor(p * ns.data[2] * ns.data[2]);
  f32x4 x_ = floor(j * ns.data[2]);
  f32x4 y_ = floor(j - 7.0f * x_);
  f32x4 x = x_ * ns.data[0] + ns.swizzle<1, 1, 1, 1>();
  f32x4 y = y_ * ns.data[0] + ns.swizzle<1, 1, 1, 1>();
  f32x4 h = 1.0f - abs(x) - abs(y);
  f32x4 b0 = f32x4(x.swizzle<0, 1>(), y.swizzle<0, 1>());
  f32x4 b1 = f32x4(x.swizzle<2, 3>(), y.swizzle<2, 3>());
  f32x4 s0 = floor(b0) * 2.0f + 1.0f;
  f32x4 s1 = floor(b1) * 2.0f + 1.0f;
  f32x4 sh = -step(h, f32x4(0.0f));
  f32x4 a0 = b0.swizzle<0, 2, 1, 3>() +
             s0.swizzle<0, 2, 1, 3>() * sh.swizzle<0, 0, 1, 1>();
  f32x4 a1 = b1.swizzle<0, 2, 1, 3>() +
             s1.swizzle<0, 2, 1, 3>() * sh.swizzle<2, 2, 3, 3>();
  f32x3 p0_new = f32x3(a0.swizzle<0, 1>(), h.data[0]);
  f32x3 p1_new = f32x3(a0.swizzle<2, 3>(), h.data[1]);
  f32x3 p2 = f32x3(a1.swizzle<0, 1>(), h.data[2]);
  f32x3 p3 = f32x3(a1.swizzle<2, 3>(), h.data[3]);
  f32x4 norm =
      taylorInvSqrt(__ctx, f32x4(dot(p0_new, p0_new), dot(p1_new, p1_new),
                                 dot(p2, p2), dot(p3, p3)));
  p0_new *= norm.data[0];
  p1_new *= norm.data[1];
  p2 *= norm.data[2];
  p3 *= norm.data[3];
  f32x4 m = max(
      0.6f - f32x4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0f);
  m = m * m;
  return 42.0f * dot(m * m, f32x4(dot(p0_new, x0), dot(p1_new, x1), dot(p2, x2),
                                  dot(p3, x3)));
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
