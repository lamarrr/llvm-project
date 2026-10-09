// Port of https://www.shadertoy.com/view/fllfzS; author: MrNissenDK.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fllfzS.json.
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
unsigned seed = 455245u;
float modF(Context &__ctx, float x, float y) { return x - y * floor(x / y); }
f32x3 modF(Context &__ctx, f32x3 a, float y) {
  a = a;
  return a - y * floor(a / y);
}
int modI(Context &__ctx, int x, int y) { return x - y * (x / y); }
f32x3 modI(Context &__ctx, f32x3 a, int y) {
  a = floor(a);
  return a - float(y) * floor(a / float(y));
}
unsigned modU(Context &__ctx, unsigned x, unsigned y) {
  return x - (x / y) * y;
}
int random(Context &__ctx, unsigned i) {
  unsigned si = i + seed;
  si *= i;
  si += seed;
  si *= si;
  si ^= seed;
  si += i + seed;
  si *= si;
  si *= si;
  si *= si;
  si ^= seed;
  return abs(modI(__ctx, int(si), 255));
}
int random(Context &__ctx, float i) { return random(__ctx, unsigned(i)); }
int random(Context &__ctx, int i) { return random(__ctx, unsigned(i)); }
int random(Context &__ctx, f32x3 i) {
  return random(__ctx, unsigned(random(
                           __ctx, unsigned(random(__ctx, unsigned(i.data[0]))) +
                                      unsigned(i.data[1]))) +
                           unsigned(i.data[2]));
}
float fade(Context &__ctx, float t) {
  return t * t * t * (t * (t * 6.f - 15.f) + 10.f);
}
f32x3 fade(Context &__ctx, f32x3 t) {
  return f32x3(fade(__ctx, t.data[0]), fade(__ctx, t.data[1]),
               fade(__ctx, t.data[2]));
}
int inc(Context &__ctx, int num) {
  num++;
  return num;
}
int inc(Context &__ctx, float num) { return inc(__ctx, int(num)); }
float grad(Context &__ctx, int hash, f32x3 pos) {
  int h = hash & 15;
  float u = (h < 8) ? pos.data[0] : pos.data[1];
  float v;
  if (h < 4) {
    v = pos.data[1];
  } else if (h == 12 || h == 14) {
    v = pos.data[0];
  } else {
    v = pos.data[2];
  }
  return (((h & 1) == 0) ? u : -u) + ((h & 2) == 0 ? v : -v);
}
float lerp(Context &__ctx, float a, float b, float x) {
  return a + x * (b - a);
}
float noise(Context &__ctx, f32x3 pos) {
  f32x3 posI = floor(pos);
  f32x3 posF = pos - posI;
  posI.data[0] = float(int(posI.data[0]) & 255);
  posI.data[1] = float(int(posI.data[1]) & 255);
  posI.data[2] = float(int(posI.data[2]) & 255);
  f32x3 uvw = fade(__ctx, posF);
  int aaa = random(__ctx, f32x3(posI.data[0], posI.data[1], posI.data[2]));
  int aba = random(__ctx,
                   f32x3(posI.data[0], inc(__ctx, posI.data[1]), posI.data[2]));
  int aab = random(__ctx,
                   f32x3(posI.data[0], posI.data[1], inc(__ctx, posI.data[2])));
  int abb = random(__ctx, f32x3(posI.data[0], inc(__ctx, posI.data[1]),
                                inc(__ctx, posI.data[2])));
  int baa = random(__ctx,
                   f32x3(inc(__ctx, posI.data[0]), posI.data[1], posI.data[2]));
  int bba = random(__ctx, f32x3(inc(__ctx, posI.data[0]),
                                inc(__ctx, posI.data[1]), posI.data[2]));
  int bab = random(__ctx, f32x3(inc(__ctx, posI.data[0]), posI.data[1],
                                inc(__ctx, posI.data[2])));
  int bbb =
      random(__ctx, f32x3(inc(__ctx, posI.data[0]), inc(__ctx, posI.data[1]),
                          inc(__ctx, posI.data[2])));
  float x1, x2, y1, y2;
  x1 = lerp(
      __ctx, grad(__ctx, aaa, posF),
      grad(__ctx, baa, f32x3(posF.data[0] - 1.f, posF.data[1], posF.data[2])),
      uvw.data[0]);
  x2 = lerp(
      __ctx,
      grad(__ctx, aba, f32x3(posF.data[0], posF.data[1] - 1.f, posF.data[2])),
      grad(__ctx, bba,
           f32x3(posF.data[0] - 1.f, posF.data[1] - 1.f, posF.data[2])),
      uvw.data[0]);
  y1 = lerp(__ctx, x1, x2, uvw.data[1]);
  x1 = lerp(
      __ctx,
      grad(__ctx, aab, f32x3(posF.data[0], posF.data[1], posF.data[2] - 1.f)),
      grad(__ctx, bab,
           f32x3(posF.data[0] - 1.f, posF.data[1], posF.data[2] - 1.f)),
      uvw.data[0]);
  x2 = lerp(
      __ctx,
      grad(__ctx, abb,
           f32x3(posF.data[0], posF.data[1] - 1.f, posF.data[2] - 1.f)),
      grad(__ctx, bbb,
           f32x3(posF.data[0] - 1.f, posF.data[1] - 1.f, posF.data[2] - 1.f)),
      uvw.data[0]);
  y2 = lerp(__ctx, x1, x2, uvw.data[1]);
  return (lerp(__ctx, y1, y2, uvw.data[2]) + 1.f) / 2.f;
}
float perlin(Context &__ctx, f32x3 pos, int octaves, float persistence) {
  float total = 0.f;
  float frequency = 1.f;
  float amplitude = 1.f;
  float maxValue = 0.f;
  for (int i = 0; i < octaves; i++) {
    total += noise(__ctx, pos * frequency) * amplitude;
    maxValue += amplitude;
    amplitude *= persistence;
    frequency *= 2.f;
  }
  return total / maxValue;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  uv *= 10.f;
  float time = iTime * 0.5f;
  float r =
      1.f -
      abs(perlin(__ctx, f32x3(-uv - f32x2(time + 058.f, time * 0.5f), time), 1,
                 .5f) *
              2.f -
          1.f);
  float g = 1.f - abs(perlin(__ctx,
                             f32x3(uv + f32x2(time + 255.f, time * 0.2f), time),
                             1, .5f) *
                          2.f -
                      1.f);
  float b = 1.f - abs(perlin(__ctx,
                             f32x3(uv + f32x2(time + 954.f, time * 0.1f), time),
                             1, .5f) *
                          2.f -
                      1.f);
  f32x3 col = f32x3(r, g, b);
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
