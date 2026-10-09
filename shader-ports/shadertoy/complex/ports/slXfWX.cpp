// Port of https://www.shadertoy.com/view/slXfWX; author: Spi3lot.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/slXfWX.json.
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
float Hash21(Context &__ctx, f32x2 p) {
  return fract(43697.62f * sin(dot(p, f32x2(52.83f, 823.17f))));
}
f32x3 Hash23(Context &__ctx, f32x2 p) {
  float r = Hash21(__ctx, p);
  float g = Hash21(__ctx, f32x2(p.data[1] * 59.84f, r * 81.53f));
  float b = Hash21(__ctx, f32x2(g * 124.21f, p.data[0] * 62.27f));
  return f32x3(r, g, b);
}
const float PI = 3.141592f;
const float TAU = PI + PI;
float sdGrid(Context &__ctx, f32x3 p) {
  float plane = p.data[1];
  plane += 0.025f * smoothstep(0.05f, 0.0f, abs(p.data[0] - round(p.data[0])));
  plane += 0.025f * smoothstep(0.05f, 0.0f, abs(p.data[2] - round(p.data[2])));
  return plane;
}
float sdSpheres(Context &__ctx, f32x3 p) {
  float off = TAU * Hash21(__ctx, round(p.swizzle<0, 2>()));
  p.set_swizzle<0, 2>(mod(p.swizzle<0, 2>(), 2.0f) - 1.0f);
  p.data[1] -= 0.2f * sin(iTime + off) + 1.0f;
  return length(p) - 0.4f;
}
float GetDist(Context &__ctx, f32x3 p) {
  float grid = sdGrid(__ctx, p);
  float spheres = sdSpheres(__ctx, p);
  return min(grid, spheres);
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p);
  f32x2 e = f32x2(0.001f, 0);
  return normalize(d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()),
                             GetDist(__ctx, p - e.swizzle<1, 0, 1>()),
                             GetDist(__ctx, p - e.swizzle<1, 1, 0>())));
}
float GetLight(Context &__ctx, f32x3 p, f32x3 n) {
  f32x3 l = normalize(
      f32x3(10.0f, 10.0f, 10.0f) * f32x3(cos(iTime), 1, sin(iTime)) - p);
  float diff = max(0.0f, dot(n, l));
  return diff;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd, float side) {
  float dO = 0.0f;
  for (int i = 0; i < 1000; i++) {
    f32x3 p = ro + rd * dO;
    float dS = side * GetDist(__ctx, p);
    if (abs(dS) <= 0.001f || dO > 1000.0f)
      break;
    dO += dS;
  }
  return dO;
}
f32x3 offset(Context &__ctx, f32x3 n) { return 0.001f * 2.0f * n; }
void image(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord - iResolution.swizzle<0, 1>() * 0.5f) /
             min(iResolution.data[0], iResolution.data[1]);
  f32x2 mouse;
  if (iMouse.data[0] == 0.0f && iMouse.data[1] == 0.0f)
    mouse = f32x2(0, 0);
  else
    mouse = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - 0.5f;
  float yaw = mouse.data[0] * TAU;
  float pitch = mouse.data[1] * PI;
  f32x4 cs = f32x4(cos(yaw), sin(yaw), cos(pitch), sin(pitch));
  f32x3 ro = f32x3(0, 1, 0),
        p = cs.swizzle<0, 3, 1>() * f32x3(cs.data[2], 1, cs.data[2]),
        f = normalize(p), r = normalize(cross(f, f32x3(0, 1, 0))),
        u = cross(r, f),
        rd = normalize(uv.data[0] * r + uv.data[1] * u + f * 1.0f);
  f32x3 col = f32x3(0);
  f32x3 n = f32x3(0);
  f32x3 reflOri = ro;
  f32x3 reflDir = rd;
  for (float r = 0.f; r <= float(10); r++) {
    f32x3 off = offset(__ctx, n);
    reflOri += off;
    float d = RayMarch(__ctx, reflOri, reflDir, 1.0f);
    reflOri += reflDir * d;
    f32x3 spec = f32x3(0);
    if (sdGrid(__ctx, reflOri) <= 0.001f)
      spec += mod(floor(reflOri.data[0]) + floor(reflOri.data[2]), 2.0f);
    else if (sdSpheres(__ctx, reflOri) <= 0.001f)
      spec += Hash23(__ctx, round(reflOri.swizzle<0, 2>()));
    else
      break;
    n = GetNormal(__ctx, reflOri);
    float diff = GetLight(__ctx, reflOri, n);
    spec += smoothstep(0.975f, 1.0f, diff);
    col += diff * spec * pow(0.4f, r);
    reflDir = reflect(reflDir, n);
  }
  fragColor = f32x4(col, 1.0f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  fragColor = f32x4(0);
  for (int j = 0; j < 2; j++)
    for (int i = 0; i < 2; i++) {
      f32x4 color;
      f32x2 co = fragCoord - 0.5f;
      f32x2 off = (f32x2(i, j) + 0.5f) / float(2);
      image(__ctx, color, co + off);
      fragColor += color;
    }
  fragColor /= float(2 * 2);
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
