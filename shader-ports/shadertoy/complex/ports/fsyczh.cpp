// Port of https://www.shadertoy.com/view/fsyczh; author: amoser.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fsyczh.json.
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
const bool julia = false;
const float iterations = 100.0f;
const float escapeDistance = 60.0f;
const float pi = 3.1415926f;
f32x3 hue(Context &__ctx, float x) {
  return f32x3(0.6f + 0.6f * cos(2.0f * pi * (x) + f32x4(0, -2.0f * pi / 3.f,
                                                         2.0f * pi / 3.0f, 0)));
}
f32x2x2 rot(Context &__ctx, float x) {
  return f32x2x2(cos(x), -sin(x), sin(x), cos(x));
}
f32x2 cmul(Context &__ctx, f32x2 za, f32x2 zb) {
  return za * f32x2x2(zb.data[0], -zb.data[1], zb.swizzle<1, 0>());
}
f32x2 cinv(Context &__ctx, f32x2 z) { return z * f32x2(1, -1) / dot(z, z); }
f32x2 cexp(Context &__ctx, f32x2 z) {
  return exp(z.data[0]) * f32x2(cos(z.data[1]), sin(z.data[1]));
}
f32x2 clog(Context &__ctx, f32x2 z) {
  return f32x2(log(length(z)), atan(z.data[1], z.data[0]));
}
f32x2 cpow(Context &__ctx, f32x2 z, float n) {
  return cexp(__ctx, n * clog(__ctx, z));
}
f32x3 magma(Context &__ctx, float t) {
  const f32x3 c0 = f32x3(-0.002136485053939582f, -0.000749655052795221f,
                         -0.005386127855323933f);
  const f32x3 c1 =
      f32x3(0.2516605407371642f, 0.6775232436837668f, 2.494026599312351f);
  const f32x3 c2 =
      f32x3(8.353717279216625f, -3.577719514958484f, 0.3144679030132573f);
  const f32x3 c3 =
      f32x3(-27.66873308576866f, 14.26473078096533f, -13.64921318813922f);
  const f32x3 c4 =
      f32x3(52.17613981234068f, -27.94360607168351f, 12.94416944238394f);
  const f32x3 c5 =
      f32x3(-50.76852536473588f, 29.04658282127291f, 4.23415299384598f);
  const f32x3 c6 =
      f32x3(18.65570506591883f, -11.48977351997711f, -5.601961508734096f);
  return c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * c6)))));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float escaped = iterations;
  f32x2 coord =
      f32x2(-2.0f, 0) +
      3.0f * (2.0f * fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
          (iResolution.data[0]);
  f32x2 mouse =
      f32x2(-2.0f, 0) +
      3.0f * (2.0f * iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
          (iResolution.data[0]);
  f32x2 B = julia ? mouse : coord;
  f32x2 A = cpow(__ctx, -B, 0.5f);
  f32x2 z = julia ? coord : f32x2(-1, 0);
  f32x2 zStop = f32x2(0);
  float i = 0.0f;
  for (; i < iterations; i++) {
    if (length(z) > escapeDistance && escaped >= iterations) {
      escaped = i;
      zStop = z;
    }
    z = z + cinv(__ctx, z) + A;
  }
  if (zStop != f32x2(0)) {
    escaped += 1.0f - 2.0f * escapeDistance *
                          log(log(length(zStop)) / log(escapeDistance));
    escaped = max(escaped, 0.0f);
  }
  float escapeTime = escaped / iterations;
  f32x2 gridOrigin =
      z + 50.0f * mouse + f32x2(0, 15) * rot(__ctx, iTime / 4.0f);
  float grid = 0.5f * (0.5f * sin(0.4f * gridOrigin.data[0]) + 0.5f) +
               0.5f * (0.5f * sin(0.4f * gridOrigin.data[1]) + 0.5f);
  grid = pow(grid + 0.4975f, 200.0f);
  grid = clamp(grid, 0.0f, 1.0f);
  f32x3 col =
      magma(__ctx, mix(escapeTime, grid * escapeTime,
                       0.05f * smoothstep(0.0f, 10.0f, length(z) - 5.0f)));
  col = mix(col,
            hue(__ctx, 0.2f * iTime + -0.5f * mouse.data[1] +
                           4.0f / pi * atan(z.data[1], z.data[0]))
                .swizzle<0, 1, 2>(),
            0.1f);
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
