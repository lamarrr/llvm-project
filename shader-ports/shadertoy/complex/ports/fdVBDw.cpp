// Port of https://www.shadertoy.com/view/fdVBDw; author: iq.
// Dataset license metadata: mit; original retained in upstream/fdVBDw.json.
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
float sdBox(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 q = abs(p) - b;
  return min(max(q.data[0], q.data[1]), 0.0f) + length(max(q, 0.0f));
}
float sdBase(Context &__ctx, f32x2 p, f32x2 id, float sp, float time) {
  float d;
  if (sin(time / 2.0f) > 0.0f) {
    d = sdBox(__ctx, p, f32x2(0.1f, 0.1f) * sp) - 0.2f * sp;
  } else {
    if (mod(id.data[0] + id.data[1], 2.0f) > 0.5f)
      d = sdBox(__ctx, p, f32x2(0.2f * sp));
    else
      d = sdBox(__ctx, p, f32x2(0.2f, 0.02f) * sp) - 0.3f * sp;
  }
  return d;
}
float sdCircularRepetition(Context &__ctx, f32x2 p, float ra, float sp,
                           float time) {
  f32x2 id0 = round(p / sp);
  if (dot(id0, id0) > ra * ra)
    id0 = round(normalize(id0) * ra);
  float d = 1e20f;
  for (int j = -2; j <= 2; j++)
    for (int i = -2; i <= 2; i++) {
      f32x2 id = id0 + f32x2(i, j);
      if (dot(id, id) <= ra * ra) {
        f32x2 q = p - sp * id;
        d = min(d, sdBase(__ctx, q, id, sp, time));
      }
    }
  return d;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = (2.0f * iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
            iResolution.data[1];
  float time = iTime + 0.001f;
  float ra = floor(3.0f + 8.0f * (0.5f - 0.5f * cos(time)));
  float sp = 0.8f / ra;
  float d = sdCircularRepetition(__ctx, p, ra, sp, time);
  f32x3 col = (d > 0.0f) ? f32x3(0.9f, 0.6f, 0.3f) : f32x3(0.65f, 0.85f, 1.0f);
  col *= 1.0f - exp(-32.0f * abs(d));
  col *= 0.8f + 0.2f * cos(120.0f * d);
  col = mix(col, f32x3(1.0f), 1.0f - smoothstep(0.0f, 0.009f, abs(d)));
  if (iMouse.data[2] > 0.001f) {
    d = sdCircularRepetition(__ctx, m, ra, sp, time);
    col = mix(
        col, f32x3(1.0f, 1.0f, 0.0f),
        1.0f - smoothstep(0.0f, 0.005f, abs(length(p - m) - abs(d)) - 0.0025f));
    col = mix(col, f32x3(1.0f, 1.0f, 0.0f),
              1.0f - smoothstep(0.0f, 0.005f, length(p - m) - 0.015f));
  }
  fragColor = f32x4(col, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output) {
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
