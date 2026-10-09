// Port of https://www.shadertoy.com/view/7tBfD3; author: iq.
// Dataset license metadata: mit; original retained in upstream/7tBfD3.json.
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
float fun_smt(Context &__ctx, float x, float k) {
  return pow(x, k) / (pow(x, k) + pow(1.0f - x, k));
}
float inv_smt(Context &__ctx, float x, float k) {
  return fun_smt(__ctx, x, 1.0f / k);
}
float fun_pow(Context &__ctx, float x, float k) { return pow(x, k); }
float inv_pow(Context &__ctx, float x, float k) {
  return fun_pow(__ctx, x, 1.0f / k);
}
float function(Context &__ctx, float x, float t) {
  float k = exp2(3.0f * (0.5f - 0.5f * cos(t * 6.283185f / 3.0f)));
  return (t < 3.0f) ? fun_smt(__ctx, x, k) : fun_pow(__ctx, x, k);
}
float inverse_function(Context &__ctx, float x, float t) {
  float k = exp2(3.0f * (0.5f - 0.5f * cos(t * 6.283185f / 3.0f)));
  return (t < 3.0f) ? inv_smt(__ctx, x, k) : inv_pow(__ctx, x, k);
}
float cantor(Context &__ctx, float x, float t) {
  float y = 0.0f;
  float sc = 0.5f;
  float bi = 0.0f;
  float xa = 0.0f;
  float xb = 1.0f;
  for (int i = 0; i < 9; i++) {
    float ya = function(__ctx, xa, t);
    float yb = function(__ctx, xb, t);
    float wa = inverse_function(__ctx, ya + (yb - ya) / 3.0f, t);
    float wb = inverse_function(__ctx, yb - (yb - ya) / 3.0f, t);
    if (x < wa) {
      bi += 0.0f * sc;
      y = bi + sc * (x - xa) / (wa - xa);
      xb = wa;
    } else if (x > wb) {
      bi += 1.0f * sc;
      y = bi + sc * (x - wb) / (xb - wb);
      xa = wb;
    } else {
      bi += 0.0f * sc;
      y = bi + sc;
      break;
    }
    sc *= 0.5f;
  }
  return y;
}
float sdLine(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 pa = p - a, ba = b - a;
  return length(pa - ba * clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float px = 1.0f / iResolution.data[1];
  f32x2 p = (f32x2((iResolution.data[1] - iResolution.data[0]) / 2.0f, 0.0f) +
             fragCoord) *
            px;
  float t = mod(iTime, 6.0f);
  f32x3 col = f32x3(0.0f);
  if (p.data[0] > 0.0f && p.data[0] < 1.0f) {
    col = f32x3(0.04f + 0.008f * mod(floor(p.data[0] * 10.0f) +
                                         floor(p.data[1] * 10.0f),
                                     2.0f));
    f32x2 d = f32x2(1e20f);
    for (int i = -2; i < 2; i++) {
      float x0 = p.data[0] + px * float(i + 0);
      float x1 = p.data[0] + px * float(i + 1);
      d.data[0] =
          min(d.data[0], sdLine(__ctx, p, f32x2(x0, function(__ctx, x0, t)),
                                f32x2(x1, function(__ctx, x1, t))));
      d.data[1] =
          min(d.data[1], sdLine(__ctx, p, f32x2(x0, cantor(__ctx, x0, t)),
                                f32x2(x1, cantor(__ctx, x1, t))));
    }
    col = mix(col, f32x3(0.25f, 0.25f, 0.25f),
              1.0f - smoothstep(0.0007f, 0.0007f + px, d.data[0]));
    col = mix(col, f32x3(1.00f, 0.36f, 0.04f),
              1.0f - smoothstep(0.0007f, 0.0007f + px, d.data[1]));
  }
  col = sqrt(col);
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
