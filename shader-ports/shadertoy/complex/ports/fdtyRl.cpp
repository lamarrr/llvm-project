// Port of https://www.shadertoy.com/view/fdtyRl; author: NEAR_DREAM_UNITY.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fdtyRl.json.
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
f32x4 Building(Context &__ctx, f32x2 p, f32x2 corner1, f32x2 corner2,
               i32x2 windows) {
  f32x2 bp = (p - corner1) / (corner2 - corner1);
  if (bp.data[0] >= 0.f && bp.data[1] >= 0.f && bp.data[0] <= 1.f &&
      bp.data[1] <= 1.f) {
    i32x2 wslots = windows * 2 + i32x2(1, 2);
    i32x2 wloc = i32x2(floor(bp * f32x2(wslots)));
    f32x3 c = ((wloc.data[0] & 1) == 1 && (wloc.data[1] & 1) == 0 &&
               wloc.data[1] != 0)
                  ? f32x3(0.f, 0.8f, 0.6f)
                  : f32x3(0.8f, 0.6f, 0.4f);
    return f32x4(c, 1.f);
  } else
    return f32x4(0.f);
}
f32x3 Sky(Context &__ctx, f32x2 sun_p) {
  float d = length(sun_p);
  f32x3 c = (f32x3(0.38f, 0.30f, 0.48f) * 0.8f);
  c += f32x3(0.f, 0.7f, 0.7f) * pow(1.f - min(1.f, d * (1.f / 1.8f)), 1.6f);
  c += f32x3(2.f, -0.35f, -0.5f) * pow(1.f - min(1.f, d * (1.f / 0.85f)), 1.8f);
  c += f32x3(0.f, 0.35f, 0.1f) * pow(1.f - min(1.f, d * (1.f / 0.45f)), 1.2f);
  c += 1.5f * pow(1.f - min(1.f, d * (1.f / 0.15f)), 2.8f);
  c -= sun_p.data[1] * 0.15f;
  return c;
}
f32x3 Ground(Context &__ctx, f32x2 sun_p) {
  return f32x3(0.2f, 0.3f, 0.2f) + 0.1f - pow(-sun_p.data[1], 1.5f) * 0.4f -
         dot(sun_p, sun_p) * 0.05f;
}
f32x3 SunsetScene(Context &__ctx, f32x2 p) {
  p.data[1] += 0.3f;
  f32x2 sun_p = p - f32x2(0.f, 0.1f);
  f32x3 c = (p.data[1] > 0.f) ? Sky(__ctx, sun_p) : Ground(__ctx, sun_p);
  return c;
}
bool OneSquare(Context &__ctx, f32x2 moved, float size) {
  float hlt = 17.f / 32.f;
  return (moved.data[0] >= -hlt && moved.data[1] >= -hlt &&
          moved.data[0] <= size + hlt && moved.data[1] <= size + hlt &&
          (moved.data[0] < hlt || moved.data[1] < hlt ||
           moved.data[0] > size - hlt || moved.data[1] > size - hlt));
}
bool NDULogo(Context &__ctx, f32x2 p) {
  f32x2 zoom = p * 17.f;
  f32x2 tilt =
      f32x2(zoom.data[0] * 15.f - zoom.data[1] * 8.f, zoom.data[1] * 17.f) /
      15.f;
  return (OneSquare(__ctx, tilt - f32x2(0.f, -8.f), 8.f) ||
          OneSquare(__ctx, tilt - f32x2(-4.f, -12.f), 4.f) ||
          OneSquare(__ctx, tilt - f32x2(8.f, 0.f), 4.f) ||
          OneSquare(__ctx, tilt - f32x2(8.f, -12.f), 4.f) ||
          OneSquare(__ctx, tilt - f32x2(0.f, 8.f), 4.f) ||
          OneSquare(__ctx, tilt - f32x2(-12.f, -4.f), 4.f) ||
          OneSquare(__ctx, tilt - f32x2(-12.f, 8.f), 4.f) ||
          OneSquare(__ctx, tilt - f32x2(-8.f, 0.f), 8.f));
}
f32x3 Composition(Context &__ctx, f32x2 p) {
  f32x3 c = f32x3(0.f);
  c += SunsetScene(__ctx, p * (0.9f + 0.1f * cos(iTime * 0.5f)));
  c += NDULogo(__ctx, p * (0.7f - 0.2f * cos(iTime * 0.5f)))
           ? (0.02f * (1.f - cos(iTime * 0.5f)))
           : 0.f;
  return c;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 accum = f32x3(0.f);
  float resmin = min(iResolution.data[0], iResolution.data[1]);
  for (float aax = 0.5f; aax < 3.f; ++aax)
    for (float aay = 0.5f; aay < 3.f; ++aay) {
      f32x2 aacoord = fragCoord + f32x2(aax, aay) * (1.f / 3.f) - 0.5f;
      accum += Composition(
          __ctx, (aacoord * 2.f - iResolution.swizzle<0, 1>()) / resmin);
    }
  accum /= (3.f * 3.f);
  float noise_p = 9.87654321f * fragCoord.data[0] +
                  8.79546213f * fragCoord.data[1] +
                  7913.84625f * fract(iTime * 0.123456789f);
  float noise_q = 6.93582471f * fragCoord.data[0] +
                  5.26841397f * fragCoord.data[1] +
                  4175.28639f * fract(iTime * 0.321654987f);
  float noise_v = fract(sin(noise_p) * 1024.f) - fract(sin(noise_q) * 1024.f);
  f32x3 noise_c = sin(noise_v * f32x3(123456.f, 456789.f, 789123.f));
  accum += noise_c * 0.25f;
  fragColor = f32x4(accum, 1.f);
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
