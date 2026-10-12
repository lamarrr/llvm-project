// Port of https://www.shadertoy.com/view/sltfRs; author: iq.
// Dataset license metadata: mit; original retained in upstream/sltfRs.json.
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
float mp(Context &__ctx, float x, int &n) {
  n = int(min(floor(1.0f / (1.0f - x)), 256.0f));
  float c = 0.0f;
  for (int i = 1; i < n; i++) {
    float a = float(i + 0);
    float b = float(i + 1);
    c += pow(a / b, a) - pow(a / b, b);
  }
  return pow(x, float(n)) + c;
}
float helper(Context &__ctx, float x, int n) {
  float c = 0.0f;
  for (int i = 1; i < n; i++) {
    float a = float(i + 0);
    float b = float(i + 1);
    float d = a / b;
    c += pow(d, a) - pow(d, b);
  }
  return c + pow(x, float(n));
}
float sdLine(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p = fragCoord / iResolution.data[0];
  float px = 1.0f / iResolution.data[0];
  p.data[0] *= 0.995f;
  px *= 0.995f;
  float time = mod(iTime, 16.0f);
  int n = int(floor(time));
  float demo = min(time, 1.0f) * (1.0f - clamp(time - 14.0f, 0.0f, 1.0f));
  if (n > 0)
    n = n - 1;
  f32x3 col = f32x3(0.0f);
  {
    float n = floor(1.0f / (1.0f - p.data[0]));
    float d =
        min(abs(p.data[0] - n / (n + 1.0f)), abs(p.data[0] - (n - 1.0f) / n));
    float al = (1.0f - smoothstep(0.0f, 1.0f * px, d));
    al *= 1.0f - 0.95f * p.data[0];
    al *= demo;
    col = mix(col, f32x3(0.85f), al);
  }
  if (n > 0) {
    float d = 1e20f;
    for (int i = -2; i <= 2; i++) {
      float x0 = p.data[0] + px * (float(i + 0));
      float x1 = p.data[0] + px * (float(i + 1));
      float y0 = 0.25f * helper(__ctx, x0, n);
      float y1 = 0.25f * helper(__ctx, x1, n);
      d = min(d, sdLine(__ctx, p, f32x2(x0, y0), f32x2(x1, y1)));
    }
    float al = 1.0f - smoothstep(0.0f, 2.0f * px, d);
    al *= demo;
    col = mix(col, f32x3(0.5f), al);
  }
  {
    f32x4 min_dis_col = f32x4(1e20f, 0.0f, 0.0f, 0.0f);
    for (int i = -2; i <= 2; i++) {
      int e = 0;
      int f = 0;
      float x0 = p.data[0] + px * (float(i + 0));
      float x1 = p.data[0] + px * (float(i + 1));
      float y0 = 0.25f * mp(__ctx, x0, e);
      float y1 = 0.25f * mp(__ctx, x1, f);
      float d = sdLine(__ctx, p, f32x2(x0, y0), f32x2(x1, y1));
      if (e == n)
        d -= 4.0f * px * demo;
      if (d < min_dis_col.data[0]) {
        f32x3 tmp = mix(f32x3(1.0f),
                        0.5f + 0.5f * cos(float(e) + f32x3(0, 2, 4)), demo);
        min_dis_col = f32x4(d, tmp);
      }
    }
    col = mix(col, min_dis_col.swizzle<1, 2, 3>(),
              1.0f - smoothstep(1.0f * px, 3.0f * px, min_dis_col.data[0]));
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
