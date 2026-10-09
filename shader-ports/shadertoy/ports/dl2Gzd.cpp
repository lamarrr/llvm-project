// Port of https://www.shadertoy.com/view/dl2Gzd; author: cast3.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dl2Gzd.json.
#include "../support.h"
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
float opExtrusion(Context &__ctx, f32x3 p, float d, float h) {
  f32x2 w = f32x2(d, abs(p.data[2]) - h);
  return min(max(w.data[0], w.data[1]), 0.0f) + length(max(w, 0.0f));
}
float truchet(Context &__ctx, f32x2 uv, float w) {
  f32x2 id = floor(uv);
  uv = fract(uv) - .5f;
  if (fract(sin(dot(id, f32x2(164.5f, 231.8f))) * 2164.5f) < .5f)
    uv.data[0] *= -1.f;
  float s = (uv.data[0] > -uv.data[1]) ? 1.f : -1.f;
  uv -= .5f * s;
  float contour = abs((length(uv) - .5f)) - w;
  return contour;
}
float SDF(Context &__ctx, f32x3 p) {
  p.data[2] += iTime;
  float per = 1.5f;
  float id = floor(p.data[2] / per);
  p.data[2] = mod(p.data[2], per) - per * .5f;
  f32x2 pp = p.swizzle<0, 1>() + id * .5f;
  float width = sin(length(p.swizzle<0, 1>()) -
                    fract((iTime + -id * .2f) * .3f) * (2.f * acos(-1.f))) *
                .1f;
  return opExtrusion(__ctx, p, truchet(__ctx, pp, width), .05f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (2.f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  float dither = fract(sin(dot(uv, f32x2(164.5f, 231.8f))) * 2164.5f);
  f32x3 ro = f32x3(0.001f, 0.01f, -2.f), rd = normalize(f32x3(uv, 1.f)), p = ro,
        col = f32x3(0.f);
  float shad;
  bool hit = false;
  for (float i = 0.f; i < 64.f; i++) {
    float d = SDF(__ctx, p);
    if (d < 0.01f) {
      hit = true;
      shad = i / 64.f;
      break;
    }
    d *= .45f + 0.01f * dither;
    p += d * rd;
  }
  if (hit) {
    col = f32x3(1.f - shad);
  }
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
