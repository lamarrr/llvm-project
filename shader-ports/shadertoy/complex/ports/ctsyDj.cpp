// Port of https://www.shadertoy.com/view/ctsyDj; author: jackjackdev.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctsyDj.json.
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
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float sdBox(Context &__ctx, f32x3 p, f32x3 s) {
  p = abs(p) - s;
  return length(max(p, 0.f)) +
         min(max(p.data[0], max(p.data[1], p.data[2])), 0.f);
}
float sdGyroid(Context &__ctx, f32x3 p, float scale, float thickness,
               float bias) {
  p *= scale;
  return abs(dot(sin(p), cos(p.swizzle<2, 0, 1>())) - bias) / scale - thickness;
}
f32x3 Transform(Context &__ctx, f32x3 p) {
  p.data[2] -= iTime * .1f;
  p.data[1] -= .3f;
  return p;
}
float GetDist(Context &__ctx, f32x3 p) {
  p = Transform(__ctx, p);
  float box = sdBox(__ctx, p, f32x3(1));
  float g1 = sdGyroid(__ctx, p, 5.23f, .03f, 1.4f);
  float g2 = sdGyroid(__ctx, p, 10.76f, .03f, .3f);
  float g3 = sdGyroid(__ctx, p, 20.76f, .03f, .3f);
  float g4 = sdGyroid(__ctx, p, 35.76f, .03f, .3f);
  float g5 = sdGyroid(__ctx, p, 60.76f, .03f, .3f);
  float g6 = sdGyroid(__ctx, p, 101.72f, .03f, .3f);
  float g7 = sdGyroid(__ctx, p, 185.56f, .03f, .3f);
  g1 -= g2 * .3f;
  g1 -= g3 * .2f;
  g1 += g4 * .1f;
  g1 += g5 * .1f;
  g1 += g6 * .8f;
  g1 += g7 * .5f;
  float d = g1 * .9f;
  return d;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.f;
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + rd * dO;
    float dS = GetDist(__ctx, p);
    dO += dS;
    if (dO > 100.f || abs(dS) < .001f)
      break;
  }
  return dO;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(.005f, 0);
  f32x3 n = GetDist(__ctx, p) - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()),
                                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()),
                                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()));
  return normalize(n);
}
f32x3 GetRayDir(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u;
  return normalize(i);
}
f32x3 Background(Context &__ctx, f32x3 rd) {
  f32x3 col = f32x3(0);
  float t = iTime;
  float y = rd.data[1] * .5f + .5f;
  col += (1.f - y) * f32x3(0.4f, 0.8f, 1.f) * 1.5f;
  float a = atan(rd.data[0], rd.data[2]);
  float beams = (sin(a * 10.f + t) * sin(a * 7.f - t) * sin(a * 3.f + t));
  beams *= smoothstep(.8f, .5f, y);
  col += beams;
  col = max(col, 0.f);
  col += smoothstep(.5f, .0f, y);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float t = iTime;
  f32x3 ro = f32x3(0, 0, -.03f);
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() *
                       (Rot(__ctx, -m.data[1] * 3.141592f + 1.f)));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() *
                       (Rot(__ctx, -m.data[0] * 6.283185f)));
  f32x3 lookat = f32x3(0, 0, 0);
  f32x3 rd = GetRayDir(__ctx, uv, ro, lookat, .8f);
  f32x3 col = f32x3(0);
  float d = RayMarch(__ctx, ro, rd);
  if (d < 100.f) {
    f32x3 p = ro + rd * d;
    f32x3 n = GetNormal(__ctx, p);
    p = Transform(__ctx, p);
    float dif = n.data[1] * .5f + .5f;
    col += dif * dif;
    float g2 = sdGyroid(__ctx, p, 10.76f, .03f, .3f);
    col *= smoothstep(-.1f, .1f, g2);
    float crackWidth = -0.02f + smoothstep(0.f, -.5f, n.data[1]) * .04f;
    float cracks = smoothstep(crackWidth, -.03f, g2);
    float g3 = sdGyroid(__ctx, p + t * .1f, 5.76f, .03f, .0f);
    float g4 = sdGyroid(__ctx, p + t * .05f, 4.76f, .03f, .0f);
    cracks *= g3 * g4 * 20.f + .2f * smoothstep(.2f, .0f, n.data[1]);
    col += cracks * f32x3(0.1f, 0.2f, 1.f) * 3.f;
  }
  col = mix(col, Background(__ctx, rd), smoothstep(0.f, 7.f, d));
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
