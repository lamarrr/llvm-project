// Port of https://www.shadertoy.com/view/css3R8; author: mla.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/css3R8.json.
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
float scale = 6.0f;
float lwidth = 0.02f;
float pwidth = 0.1f;
float eps = 1.0f / 32.0f;
float N = 8.0f;
float g(Context &__ctx, float a, float b, float x) {
  return x - a + sin(x) * cos(x) - b * cos(x);
}
float h(Context &__ctx, float a, float b, float x) {
  return 0.5f * distance(f32x2(a, b), f32x2(x, sin(x)));
}
float graphdist(Context &__ctx, float y, float eps, float f0, float f1,
                float f2) {
  return abs(y - f1) / length(f32x2((f2 - f0) / (2.0f * eps), 1.0f));
}
float segment(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  p -= a;
  b -= a;
  float h = dot(p, b) / dot(b, b);
  h = clamp(h, 0.0f, 1.0f);
  return length(p - b * h);
}
float line(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  p -= a;
  b -= a;
  float h = dot(p, b) / dot(b, b);
  return length(p - b * h);
}
f32x2 map(Context &__ctx, f32x2 coords) {
  f32x2 uv =
      (2.0f * coords - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  uv.data[1] += 0.5f;
  uv *= scale;
  return uv;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = map(__ctx, fragCoord);
  float t = 0.4f * iTime;
  f32x2 mouse = f32x2(6.5f * sin(t), 4.5f * cos(t) + 2.0f);
  if (iMouse.data[2] > 0.0f)
    mouse = map(__ctx, iMouse.swizzle<0, 1>());
  float x = uv.data[0], y = uv.data[1];
  float px = fwidth(x);
  float a = mouse.data[0], b = mouse.data[1];
  f32x3 col = f32x3(0.5f);
  float pdist = 1e8f, ldist = 1e8f, ldist2 = 1e8f;
  float span = abs(b) + 0.5f;
  float x0 = a - span;
  float y0 = g(__ctx, a, b, x0);
  for (float i = -N; i < N; i++) {
    float x1 = a + span / N * (i + 1.0f);
    float y1 = g(__ctx, a, b, x1);
    if (y0 * y1 <= 0.0f) {
      for (int j = 0; j < 6; j++) {
        float x2 = 0.5f * (x0 + x1);
        float y2 = g(__ctx, a, b, x2);
        if (y0 * y2 <= 0.0f) {
          x1 = x2;
          y1 = y2;
        } else {
          x0 = x2;
          y0 = y2;
        }
      }
      f32x2 zero = f32x2(x0, 0);
      f32x2 foot = f32x2(x0, sin(x0));
      pdist = min(pdist, distance(uv, zero));
      pdist = min(pdist, distance(uv, foot));
      pdist = min(pdist, distance(uv, f32x2(x0, h(__ctx, a, b, x0))));
      ldist2 = min(ldist2, line(__ctx, uv, zero, foot));
      ldist = min(ldist, segment(__ctx, uv, mouse, foot));
    }
    x0 = x1;
    y0 = y1;
  }
  col =
      mix(f32x3(0.6f), col, smoothstep(0.0f, px, abs(ldist2) - 0.5f * lwidth));
  col = mix(f32x3(0.1f), col, smoothstep(0.0f, px, abs(y) - 0.75f * lwidth));
  float d = graphdist(__ctx, y, eps, g(__ctx, a, b, x - eps), g(__ctx, a, b, x),
                      g(__ctx, a, b, x + eps));
  col = mix(f32x3(1, 1, 0), col, smoothstep(0.0f, px, d - lwidth));
  d = graphdist(__ctx, y, eps, h(__ctx, a, b, x - eps), h(__ctx, a, b, x),
                h(__ctx, a, b, x + eps));
  col = mix(f32x3(0, 0, 1), col, smoothstep(0.0f, px, d - lwidth));
  {
    float y0 = (sin(a));
    float y1 = (sin(a - eps));
    float y2 = (sin(a + eps));
    y1 -= y0;
    y2 -= y0;
    float A = 0.5f * (y2 + y1) / (eps * eps);
    float B = 0.5f * (y2 - y1) / eps;
    float x0 = x - a - eps;
    float x1 = x - a;
    float x2 = x - a + eps;
    float f0 = A * x0 * x0 + B * x0;
    float f1 = A * x1 * x1 + B * x1;
    float f2 = A * x2 * x2 + B * x2;
    float d = graphdist(__ctx, y - y0, eps, f0, f1, f2);
    col = mix(f32x3(0), col, smoothstep(0.0f, px, d - 0.66f * lwidth));
  }
  d = graphdist(__ctx, y, eps, sin(x - eps), sin(x), sin(x + eps));
  col = mix(f32x3(1, 0, 0), col, smoothstep(0.0f, px, d - lwidth));
  col =
      mix(f32x3(0.25f), col, smoothstep(0.0f, px, abs(ldist) - 0.5f * lwidth));
  col = mix(f32x3(0.0f), col, smoothstep(0.0f, px, abs(pdist) - pwidth));
  f32x2 xseg0 = f32x2(0.2f, 0);
  f32x2 xseg1 = xseg0.swizzle<1, 0>();
  d = min(segment(__ctx, uv, mouse + xseg0, mouse - xseg0),
          segment(__ctx, uv, mouse + xseg1, mouse - xseg1));
  col = mix(f32x3(0), col, smoothstep(0.0f, px, d - lwidth));
  if (x < a - span || x > a + span)
    col *= 0.8f;
  col = pow(col, f32x3(0.4545f));
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
