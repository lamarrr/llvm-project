// Port of https://www.shadertoy.com/view/mdVcWK; author: mokasiri.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mdVcWK.json.
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
float random(Context &__ctx, f32x2 st) {
  return fract(sin(dot(st.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
float GetDist(Context &__ctx, f32x3 p) {
  f32x4 sphere = f32x4(0.0f, 1.f, 6.0f, 1.0f);
  float sphereDist = length(p - sphere.swizzle<0, 1, 2>()) - sphere.data[3];
  float planeDist = p.data[1];
  float dis = min(sphereDist, planeDist);
  return dis;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float d0 = 0.0f;
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + d0 * rd;
    float ds = GetDist(__ctx, p);
    d0 += ds;
    if (ds < 0.01f || d0 > 1000.f)
      break;
  }
  return d0;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p);
  f32x2 e = f32x2(0.01f, 0.0f);
  f32x3 dists = f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()));
  f32x3 n = d - dists;
  return normalize(n);
}
f32x3 GetLight(Context &__ctx, f32x3 p, f32x3 ro) {
  f32x3 MainlightDirection = f32x3(.0f, 1.0f, .0f);
  f32x3 rgb = f32x3(0.925f, 0.714f, 0.514f);
  f32x4 sphere = f32x4(0.0f, 1.0f, 6.0f, 1.0f);
  f32x3 lightPos = f32x3(0.f, 4.f, 5.f);
  lightPos.set_swizzle<0, 2>(lightPos.swizzle<0, 2>() +
                             (f32x2(sin(iTime), cos(iTime))));
  float distance2light = length(lightPos - p);
  f32x3 lightDirection = normalize(lightPos - p);
  ro = f32x3(ro.data[0], ro.data[1], ro.data[2]);
  f32x3 h = normalize((lightDirection + ro) / length(lightDirection + ro));
  f32x3 n = GetNormal(__ctx, p);
  float dif = clamp(dot(n, lightDirection), 0.f, 1.0f);
  float d = RayMarch(__ctx, p + n * 0.01f * 2.0f, lightDirection);
  float kc = 0.1f;
  float kl = 0.1f;
  float kq = 0.01f;
  float spec = max(pow(dot(h, n), 30.f), 0.01f);
  dif += spec;
  float kWide = 1.2f;
  dif = dif * max(dot(MainlightDirection, lightDirection) * kWide, 0.0f) /
        max(kc + kl * distance2light + kq * distance2light * distance2light,
            1.0f);
  if (d < length(lightPos - p))
    dif *= 0.05f;
  if (length(p) < 1000.f)
    dif += 0.1f;
  return dif * rgb;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (2.f * gl_FragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
      iResolution.data[1];
  f32x3 col = f32x3(0);
  f32x3 ro = f32x3(0.0f, 1.f, 0.0f);
  f32x3 rd = normalize(f32x3(uv.data[0], uv.data[1], 1.f));
  float d = RayMarch(__ctx, ro, rd);
  f32x3 p = ro + rd * d;
  f32x3 dif = GetLight(__ctx, p, ro);
  col = dif;
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
