// Port of https://www.shadertoy.com/view/NlBcW1; author: ccc9527.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NlBcW1.json.
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
float SphereSDF(Context &__ctx, f32x3 p, f32x4 sphere) {
  return length(p - sphere.swizzle<0, 1, 2>()) - sphere.data[3];
}
float CapsuleSDF(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 ap = p - a;
  f32x3 ab = b - a;
  float t = dot(ab, ap) / dot(ab, ab);
  t = clamp(t, 0.0f, 1.0f);
  f32x3 c = a + t * ab;
  return length(p - c) - r;
}
float TorusSDF(Context &__ctx, f32x3 p, f32x3 a, f32x2 r) {
  float x = length(p.swizzle<0, 2>() - a.swizzle<0, 2>()) - r.data[0];
  return length(f32x2(x, p.data[1] - a.data[1])) - r.data[1];
}
float BoxSDF(Context &__ctx, f32x3 ap, f32x3 size) {
  return length(max(abs(ap) - size, 0.0f));
}
float CylinderSDF(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 ap = p - a;
  f32x3 ab = b - a;
  float t = dot(ap, ab) / dot(ab, ab);
  f32x3 c = a + t * ab;
  float d = length(p - c) - r;
  float y = abs(t - 0.5f) - 0.5f;
  y *= length(ab);
  float dist = length(max(f32x2(d, y), 0.0f));
  return dist + min(max(d, y), 0.0f);
}
f32x2x2 Rot(Context &__ctx, float t) {
  float s = sin(t);
  float c = cos(t);
  return f32x2x2(c, -s, s, c);
}
float GetDist(Context &__ctx, f32x3 p) {
  f32x4 sphere = f32x4(-6, 1, 10, 1);
  f32x3 a = f32x3(-3.0f, 2.0f, 10.0f);
  f32x3 b = f32x3(-3.0f, 0.5f, 10.0f);
  float t = 0.5f;
  f32x3 a1 = f32x3(0, 1.0f, 10.0f);
  f32x2 r = f32x2(1.0f, 0.3f);
  f32x3 a2 = f32x3(3.0f, 1.0f, 10.0f);
  f32x3 a2p = a2 - p;
  f32x3 size = f32x3(1);
  f32x3 a3 = f32x3(6.0f, 2.0f, 10.0f);
  f32x3 b3 = f32x3(6.0f, 0.0f, 10.0f);
  float r3 = 0.5f;
  float sphereDist = SphereSDF(__ctx, p, sphere);
  float planeDist = p.data[1];
  float capsuleDist = CapsuleSDF(__ctx, p, a, b, t);
  float torusDist = TorusSDF(__ctx, p, a1, r);
  float boxDist = BoxSDF(__ctx, a2p, size);
  float cylinderDist = CylinderSDF(__ctx, p, a3, b3, r3);
  float d = min(sphereDist, planeDist);
  d = min(d, capsuleDist);
  d = min(d, torusDist);
  d = min(d, boxDist);
  d = min(d, cylinderDist);
  return d;
}
float RayMarching(Context &__ctx, f32x3 ro, f32x3 rd) {
  float d0 = 0.0f;
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + rd * d0;
    float ds = GetDist(__ctx, p);
    d0 += ds;
    if (d0 > 100.0f || ds < 0.01f)
      break;
  }
  return d0;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p);
  f32x2 e = f32x2(0.001f, 0);
  f32x3 normal = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()),
                           GetDist(__ctx, p - e.swizzle<1, 0, 1>()),
                           GetDist(__ctx, p - e.swizzle<1, 1, 0>()));
  return normalize(normal);
}
float CalcAO(Context &__ctx, f32x3 p, f32x3 n) {
  float res = 0.f;
  float scale = 1.f;
  for (int i = 0; i < 5; i++) {
    float h = 0.01f + 0.03f * float(i);
    float d = GetDist(__ctx, p + n * h);
    res += (h - d) * scale;
    scale *= 0.95f;
    if (res > 0.35f)
      break;
  }
  return clamp(1.f - 3.f * res, 0.f, 1.f);
}
f32x3 GetLight(Context &__ctx, f32x3 p, f32x3 ro) {
  f32x3 lightPos = f32x3(3, 5, 5);
  f32x3 lightCol = f32x3(1.0f);
  lightPos.set_swizzle<0, 2>(lightPos.swizzle<0, 2>() +
                             (f32x2(sin(2.0f * iTime), cos(2.0f * iTime))));
  f32x3 k_a = f32x3(0.1745f, 0.01175f, 0.01175f);
  f32x3 k_d = f32x3(0.61424f, 0.041362f, 0.04136f);
  f32x3 k_s = f32x3(0.727811f, 0.626959f, 0.626959f);
  float shininess = 76.8f;
  f32x3 lightDir = normalize(lightPos - p);
  f32x3 normal = GetNormal(__ctx, p);
  f32x3 ambient = k_a * 0.5f;
  float dif = clamp(dot(lightDir, normal), 0.0f, 1.0f);
  f32x3 diffuse = lightCol * k_d * dif;
  f32x3 viewDir = normalize(ro - p);
  f32x3 halfDir = normalize(viewDir + lightDir);
  float spec = pow(max(dot(halfDir, normal), 0.0f), shininess);
  f32x3 specular = lightCol * k_s * spec;
  f32x3 p2 = p + normal * 0.01f * 2.0f;
  float d = RayMarching(__ctx, p2, lightDir);
  if (d < length(lightPos - p))
    return ambient;
  f32x3 color = ambient + diffuse + specular;
  float ao = CalcAO(__ctx, p, normal);
  return color * ao;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 mo = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 col = f32x3(0.0f);
  float time = iTime;
  f32x3 ro = f32x3(0.0f, 3.0f, 1.0f);
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() * (Rot(__ctx, -mo.data[1] + .4f)));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() *
                       (Rot(__ctx, -mo.data[0] * 6.2831f)));
  f32x3 rd = normalize(f32x3(uv.data[0], uv.data[1], 1.0f));
  float d = RayMarching(__ctx, ro, rd);
  f32x3 p = ro + rd * d;
  col = GetLight(__ctx, p, ro);
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
