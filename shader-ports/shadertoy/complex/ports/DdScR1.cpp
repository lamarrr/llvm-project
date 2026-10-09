// Port of https://www.shadertoy.com/view/DdScR1; author: leosalvatierra.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DdScR1.json.
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
f32x4 HexCoords(Context &__ctx, f32x2 uv) {
  f32x2 s = f32x2(1, 1.732051f);
  f32x2 h = .5f * s;
  f32x2 gv = s * uv;
  f32x2 a = mod(gv, s) - h;
  f32x2 b = mod(gv + h, s) - h;
  f32x2 ab = dot(a, a) < dot(b, b) ? a : b;
  f32x2 st = ab;
  f32x2 id = gv - ab;
  st = ab;
  return f32x4(st, id);
}
float GetSize(Context &__ctx, f32x2 id, float seed) {
  float d = length(id);
  float t = iTime * .5f;
  float a = sin(d * seed + t) + sin(d * seed * seed * 10.f + t * 2.f);
  return a / 2.f + .5f;
}
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a);
  float c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float Hexagon(Context &__ctx, f32x2 uv, float r, f32x2 offs) {
  uv *= Rot(__ctx, mix(0.f, 3.1415f, r));
  r /= 1.f / sqrt(2.f);
  uv = f32x2(-uv.data[1], uv.data[0]);
  uv.data[0] *= 1.732051f;
  uv = abs(uv);
  f32x2 n = normalize(f32x2(1, 1));
  float d = dot(uv, n) - r;
  d = max(d, uv.data[1] - r * .707f);
  d = smoothstep(.06f, .02f, abs(d));
  d += smoothstep(.1f, .09f, abs(r - .5f)) * sin(iTime);
  return d;
}
float Xor(Context &__ctx, float a, float b) { return a + b; }
float ShapePattern(Context &__ctx, f32x2 uv) {
  float r = 0.0f;
  for (int i = 0; i < 8; i++) {
    f32x2 pos = f32x2(fract(sin(float(i) * 123.456f + iTime) * 234.567f),
                      fract(cos(float(i) * 234.567f + iTime) * 345.678f));
    r += length(uv - pos) < 0.1f ? 1.0f : 0.0f;
  }
  return r;
}
float Layer(Context &__ctx, f32x2 uv, float s) {
  f32x4 hu = HexCoords(__ctx, uv * 2.f);
  float d = Hexagon(__ctx, hu.swizzle<0, 1>(),
                    GetSize(__ctx, hu.swizzle<2, 3>(), s), f32x2(0));
  f32x2 offs = f32x2(1, 0);
  d = Xor(__ctx, d,
          Hexagon(__ctx, hu.swizzle<0, 1>() - offs,
                  GetSize(__ctx, hu.swizzle<2, 3>() + offs, s), offs));
  d = Xor(__ctx, d,
          Hexagon(__ctx, hu.swizzle<0, 1>() + offs,
                  GetSize(__ctx, hu.swizzle<2, 3>() - offs, s), -offs));
  offs = f32x2(.5f, .8725f);
  d = Xor(__ctx, d,
          Hexagon(__ctx, hu.swizzle<0, 1>() - offs,
                  GetSize(__ctx, hu.swizzle<2, 3>() + offs, s), offs));
  d = Xor(__ctx, d,
          Hexagon(__ctx, hu.swizzle<0, 1>() + offs,
                  GetSize(__ctx, hu.swizzle<2, 3>() - offs, s), -offs));
  offs = f32x2(-.5f, .8725f);
  d = Xor(__ctx, d,
          Hexagon(__ctx, hu.swizzle<0, 1>() - offs,
                  GetSize(__ctx, hu.swizzle<2, 3>() + offs, s), offs));
  d = Xor(__ctx, d,
          Hexagon(__ctx, hu.swizzle<0, 1>() + offs,
                  GetSize(__ctx, hu.swizzle<2, 3>() - offs, s), -offs));
  d = Xor(__ctx, d, ShapePattern(__ctx, uv));
  return d;
}
float N(Context &__ctx, float p) { return fract(sin(p * 123.34f) * 345.456f); }
f32x3 Col(Context &__ctx, float p, float offs) {
  float n = N(__ctx, p) * 1234.34f;
  return sin(n * f32x3(12.23f, 45.23f, 56.2f) + offs * 3.f) * .5f + .5f;
}
f32x3 GetRayDir(Context &__ctx, f32x2 uv, f32x3 p, f32x3 lookat, float zoom) {
  f32x3 f = normalize(lookat - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = p + f * zoom,
        i = c + uv.data[0] * r + uv.data[1] * u, d = normalize(i - p);
  return d;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 UV = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - .5f;
  float duv = dot(UV, UV);
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - .5f;
  float t = iTime * .2f + m.data[0] * 10.f + 5.f;
  float y = sin(t * .5f);
  f32x3 ro = f32x3(0, 20.f * y, -5);
  f32x3 lookat = f32x3(0, 0, -10);
  f32x3 rd = GetRayDir(__ctx, uv, ro, lookat, 1.f);
  f32x3 col = f32x3(0);
  f32x3 p = ro + rd * (ro.data[1] / rd.data[1]);
  float dp = length(p.swizzle<0, 2>());
  if ((ro.data[1] / rd.data[1]) > 0.f)
    col *= 0.f;
  else {
    uv = p.swizzle<0, 2>() * .1f;
    uv *= mix(1.f, 5.f, sin(t * .5f) * .5f + .5f);
    uv *= Rot(__ctx, t);
    m *= Rot(__ctx, t);
    uv.data[0] *= 1.732051f;
    for (float i = 0.f; i < 1.f; i += 1.f / 3.f) {
      float id = floor(i + t);
      float t = fract(i + t);
      float z = mix(5.f, .1f, t);
      float fade = smoothstep(0.f, .3f, t) * smoothstep(1.f, .7f, t);
      col += fade * t * Layer(__ctx, uv * z, N(__ctx, i + id)) *
             Col(__ctx, id, duv);
    }
  }
  col *= 2.f;
  if (ro.data[1] < 0.f)
    col = 1.f - col;
  col *= smoothstep(18.f, 5.f, dp);
  col *= 1.f - duv * 2.f;
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
