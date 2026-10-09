// Port of https://www.shadertoy.com/view/dtXGRr; author: MaxShaders.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtXGRr.json.
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

struct Circle {
  f32x3 center;
  float radius;
  f32x4 color;
};
float Clamp(Context &__ctx, float a, float b, float x) {
  return max(a, min(b, x));
}
f32x4 DrawCircle(Context &__ctx, f32x2 uv, Circle c, bool AA) {
  float d = length(c.center.swizzle<0, 1>() - uv) - c.center.data[2];
  float blur =
      AA ? smoothstep(1.0f, 0.985f,
                      1.f - clamp(0.f, 1.f, length(d - c.radius) / c.radius))
         : 1.f;
  return d <= c.radius
             ? f32x4(c.color.swizzle<0, 1, 2>(), blur * c.color.data[3])
             : f32x4(0.f);
}
float N(Context &__ctx, f32x2 i) {
  float t = tan(sin(i.data[0])) + cos(i.data[1]);
  return fract(sin(t * 3456.f) * 6547.f);
}
f32x4 depthSort(Context &__ctx, f32x4 c1, float d1, f32x4 c2, float d2) {
  c1.data[3] = clamp(0.f, 1.f, c1.data[3]);
  c2.data[3] = clamp(0.f, 1.f, c2.data[3]);
  if (length(c1) != 0.f && length(c2) != 0.f) {
    return d1 < d2 ? c1 + c2 * (1.f - c1.data[3])
                   : c2 + c1 * (1.f - c2.data[3]);
  }
  return c1 + c2;
}
f32x4 FresnelCircle(Context &__ctx, f32x2 uv, Circle c, float ammount) {
  float d = length(c.center.swizzle<0, 1>() - uv);
  float r = c.radius + c.center.data[2];
  if (d > r)
    return f32x4(0.f);
  return f32x4(pow(d / (r), ammount));
}
f32x4 bloomCircle(Context &__ctx, f32x2 uv, Circle c, float scale,
                  float intensity, bool hollow) {
  float d = length(c.center.swizzle<0, 1>() - uv);
  if (hollow && d < c.radius)
    return f32x4(0.f);
  d = clamp(0.f, 1.f,
            1.f - min(c.radius * scale, d - c.radius) / (c.radius * scale));
  return f32x4(f32x3(1.f) * intensity * d * d, intensity * d * d);
}
f32x4 combineColors(Context &__ctx, f32x4 c1, f32x4 c2) {
  return f32x4(c1.swizzle<0, 1, 2>() * c1.data[3] +
                   c2.swizzle<0, 1, 2>() * c2.data[3],
               clamp(0.f, 1.f, c1.data[3] + c2.data[3]));
}
f32x3 PlanetTexture(Context &__ctx, f32x2 uv, float distToEdge, float tscale) {
  float speed = 0.5f;
  f32x2 scale = f32x2(6.f, 2.f) * tscale;
  f32x2 p = uv * scale / iResolution.swizzle<0, 1>();
  for (float i = 1.f; i < 10.f; i += 1.f) {
    p.data[0] += 0.3f / i * sin(i * 3.f * p.data[1] + iTime * speed) +
                 iMouse.data[0] / 1000.f;
    p.data[1] += 0.3f / i * cos(i * 3.f * p.data[0] + iTime * speed) +
                 iMouse.data[1] / 1000.f;
  }
  float r = cos(p.data[0] + p.data[1] + 1.f) * .3f + .5f;
  float g = sin(p.data[0] + p.data[1] + 1.f) * .5f + .5f;
  float b =
      (sin(p.data[0] + p.data[1]) + cos(p.data[0] + p.data[1])) * .8f + .5f;
  f32x3 color = 0.4f + f32x3(r, g, b) * 0.25f;
  return color * 3.f * distToEdge;
}
float shadeCircle(Context &__ctx, f32x2 uv, Circle c, f32x2 dir, float scale,
                  float offset) {
  dir = normalize(dir);
  float dist = c.radius;
  f32x2 center = f32x2(c.center.swizzle<0, 1>() - dist * -offset * dir);
  return (scale / 2.f) +
         max(1.f, scale) *
             max(0.0f,
                 (1.f / dist - (length(uv - center) *
                                dot(normalize(uv - center), dir) / dist)));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  bool KeepLightDir = false;
  float t = mod(iTime / 2.f, 3.f);
  f32x2 uv = fragCoord;
  float Z = iResolution.data[1] / 10.f;
  f32x2 lightDir = f32x2(-1.f, -1.f);
  lightDir *= iMouse.data[2] > 0.f || KeepLightDir
                  ? normalize(f32x2(iMouse.data[0] - iResolution.data[0] / 2.f,
                                    iMouse.data[1] - iResolution.data[1] / 2.f))
                  : f32x2(1.f);
  float t_moon = t * 0.5f;
  f32x3 g = f32x3(0.f, -1.f, 0.f) * iResolution.data[1];
  f32x3 A = g;
  f32x3 Vi = f32x3(1.0f, 2.2f, -0.1f) * iResolution.data[1];
  f32x3 Pi = f32x3(0.1f, -0.1f, 0.5f) * f32x3(iResolution.swizzle<0, 1>(), Z);
  f32x3 Vel = Vi + A * t_moon;
  f32x3 Pos = f32x3(Pi + Vel * t_moon + 0.5f * A * t_moon * t_moon);
  Circle m;
  m.center = Pos.swizzle<0, 1, 2>();
  m.radius = 0.1f * iResolution.data[1];
  float moonD = -m.center.data[2] / Z;
  m.color = f32x4(0.55f, 0.5f, 0.7f, 1.f) * (clamp(0.f, 1.f, 1.4f - moonD));
  f32x4 moon = DrawCircle(__ctx, uv, m, false);
  float moonR = m.radius + m.center.data[2];
  moon = combineColors(__ctx, moon,
                       1.5f * FresnelCircle(__ctx, uv, m, 5.f) *
                           f32x4(0.2f, 0.4f, 0.5f, 0.5f));
  float distToEdge1 = 2.f * FresnelCircle(__ctx, uv, m, 4.f).data[3];
  moon.set_swizzle<0, 1, 2>(
      moon.swizzle<0, 1, 2>() *
      (0.5f + PlanetTexture(__ctx, uv - m.center.swizzle<0, 1>(), distToEdge1,
                            10.f * distToEdge1)
                      .swizzle<0, 2, 1>() /
                  2.f));
  m.radius = moonR;
  moon.set_swizzle<0, 1, 2>(
      moon.swizzle<0, 1, 2>() *
      (shadeCircle(__ctx, uv, m, lightDir, 0.0f, 0.2f) / 2.f));
  moon += bloomCircle(__ctx, uv, m, 0.3f, 0.9f, true) *
          f32x4(0.1f, 0.15f, 0.25f, 0.2f) *
          pow(shadeCircle(__ctx, uv, m, lightDir, 0.0f, 0.8f), 2.f);
  Circle w;
  w.center = f32x3(0.5f, -1.f, 0.0f) * f32x3(iResolution.swizzle<0, 1>(), 65.f);
  float planetD = -w.center.data[2] / Z;
  w.radius = 1.5f * iResolution.data[1];
  w.color = f32x4(0.5f, 0.4f, 0.3f, 1.f) * 1.f;
  f32x4 planet = DrawCircle(__ctx, uv, w, true);
  Circle Atmosphere;
  Atmosphere.center = w.center - f32x3(0.f, -2.f, 0.f);
  Atmosphere.radius = w.radius;
  f32x4 planetAtmosphere =
      FresnelCircle(__ctx, uv, Atmosphere, 80.f) +
      (bloomCircle(__ctx, uv, Atmosphere, 0.015f, 1.f, true));
  planetAtmosphere *= f32x4(0.4f, 0.6f, 0.9f, 1.f);
  planet.set_swizzle<0, 1, 2>(
      planet.swizzle<0, 1, 2>() *
      (shadeCircle(__ctx, uv, w, lightDir, 0.2f, 0.2f)));
  planet.data[3] = sign(planet.data[0]);
  f32x4 planetBloom = bloomCircle(__ctx, uv, w, 0.4f, 0.5f, false) *
                      f32x4(0.4f, 0.6f, 0.9f, 0.1f);
  planetBloom *= shadeCircle(__ctx, uv, Atmosphere, lightDir, 0.0f, 0.9f) * 1.f;
  float distToEdge = FresnelCircle(__ctx, uv, w, 12.f).data[3];
  planet.set_swizzle<0, 1, 2>(
      planet.swizzle<0, 1, 2>() *
      (PlanetTexture(__ctx, uv.swizzle<1, 0>(), distToEdge, 2.f * distToEdge) /
       1.f));
  planet +=
      planetAtmosphere *
      max(0.1f, shadeCircle(__ctx, uv, Atmosphere, lightDir, 0.1f, 0.0f)) *
      1.8f;
  moon += planetBloom * moon / 2.f;
  float shadow = Clamp(
      __ctx, 0.15f, 1.f,
      1.f - dot(normalize(f32x3(0, 1.f, 0.f)), f32x3(lightDir, 0.f)) / 1.f);
  moon.set_swizzle<0, 1, 2>(moon.swizzle<0, 1, 2>() * (shadow));
  f32x4 stars = f32x4(0.0f);
  float n = N(__ctx, uv);
  if (n >= 0.999f) {
    stars += f32x4(
        n * (.8f + sin(4.f * iTime * N(__ctx, uv.swizzle<1, 0>())) * 0.2f));
  }
  f32x4 planet_moon = depthSort(__ctx, moon, moonD, planet, planetD);
  f32x4 planets_bloom = depthSort(__ctx, planet_moon, 0.0f, planetBloom, 1.f);
  f32x4 planets_stars = depthSort(__ctx, planets_bloom, 0.0f, stars, 1.f);
  fragColor = planets_stars;
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
