// Port of https://www.shadertoy.com/view/DlXSRs; author: EvilRyu.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/DlXSRs.json.
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
bool reverse = false;
float rnd = 0.f;
float hash12(Context &__ctx, f32x2 p) {
  p = fract(p * f32x2(5.3983f, 5.4427f));
  p += dot(p.swizzle<1, 0>(), p.swizzle<0, 1>() + f32x2(21.5351f, 14.3137f));
  return fract(p.data[0] * p.data[1] * 95.4337f);
}
float smin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
float tri(Context &__ctx, f32x2 p, float r) {
  return abs(max(-2.f * p.data[1], max(p.data[1] - 1.732050808f * p.data[0],
                                       p.data[1] + 1.732050808f * p.data[0])) -
             r);
}
float noise(Context &__ctx, f32x2 p) {
  if (reverse)
    p.data[1] = -p.data[1];
  return abs(tri(
      __ctx, p + .55f * tri(__ctx, p.swizzle<1, 0>() * 2.f, 0.5f + .5f * rnd),
      0.1f));
}
f32x3 drawTri(Context &__ctx, f32x2 p, float r) {
  return 1.f -
         smoothstep(0.0f, 0.015f,
                    abs(tri(__ctx, p, r)) - 0.03f + 0.02f * noise(__ctx, p)) *
             f32x3(1.f);
}
f32x2 symm(Context &__ctx, float a, float b, float c, f32x2 p) {
  float x =
      ((b * b - a * a) * p.data[0] - 2.0f * a * b * p.data[1] - 2.0f * a * c) /
      (a * a + b * b);
  float y =
      ((a * a - b * b) * p.data[1] - 2.0f * a * b * p.data[0] - 2.0f * b * c) /
      (a * a + b * b);
  return f32x2(x, y);
}
float distToEdge(Context &__ctx, f32x2 p1, f32x2 p2, f32x2 p) {
  f32x2 d = p2 - p1;
  f32x2 v = f32x2(1, -1);
  return abs(dot(d.swizzle<1, 0>() * v, p) + dot(d * v, p1.swizzle<1, 0>())) /
         length(d);
}
f32x2 p0, p1, p2;
float distToEdges(Context &__ctx, f32x2 p) {
  float d = distToEdge(__ctx, p0, p2, p);
  float d2 = distToEdge(__ctx, p0, p1, p);
  float d3 = distToEdge(__ctx, p1, p2, p);
  return smin(__ctx, d, smin(__ctx, d2, d3, .05f), .05f);
}
float height(Context &__ctx, f32x2 p) {
  float d = min(0.13f, distToEdges(__ctx, p));
  return min(d, 0.03f * noise(__ctx, p) + 0.1f);
}
f32x3 getNormal(Context &__ctx, f32x2 p) {
  f32x2 eps = f32x2(0.03f, 0.0f);
  f32x3 n = f32x3(height(__ctx, p - eps.swizzle<0, 1>()) -
                      height(__ctx, p + eps.swizzle<0, 1>()),
                  height(__ctx, p - eps.swizzle<1, 0>()) -
                      height(__ctx, p + eps.swizzle<1, 0>()),
                  2.0f * eps.data[0]);
  return normalize(n);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 p = uv * 2.f - 1.f;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  p.data[0] += iTime * 0.1f;
  f32x2 i = floor(p + f32x2(0.5f * p.data[1], 0.f));
  f32x2 a = p - i + f32x2(0.5f * i.data[1], 0.f);
  float m = step(a.data[1], 2.f * a.data[0]);
  f32x2 o = f32x2(m, 1.f - m);
  p0 = f32x2(0);
  p1 = o - f32x2(.5f * (1.f - m), 0.f);
  p2 = f32x2(.5f, 1.f);
  f32x2 center = (p0 + p1 + p2) / 3.f;
  p0 -= center;
  p1 -= center;
  p2 -= center;
  a -= center;
  float d = distToEdges(__ctx, a);
  f32x3 col = f32x3(1);
  reverse = m > 0.f;
  rnd = hash12(__ctx, i);
  d = smoothstep(0.f, 0.1f, d);
  f32x3 n = getNormal(__ctx, a), ld = normalize(f32x3(1.f, 1.f, 2.f));
  col *=
      1.1f * max(0.f, dot(n, ld)) * f32x3(1.f, .8f, 0.5f) +
      pow(max(0.0f, dot(reflect(normalize(f32x3(3, 0, -1.f)), n), ld)), 32.f);
  col *= .5f + .5f * cos(6.28318f * (f32x3(6) * d + f32x3(0.23f, 0.2f, 0.15f)));
  col *= smoothstep(.0f, 0.1f, d);
  if (reverse)
    a.data[1] = -a.data[1];
  p = a * f32x2(2.5f, 2.2f) * 1.35f;
  float s = 1.0f;
  float iter = mod(floor(iTime), 9.0f);
  for (int i = 0; i < 4; ++i) {
    if (i < int(iter + 0.5f)) {
      s *= 0.5f;
      col += drawTri(__ctx, p, s) * .1f;
      if (p.data[1] + 1.732050808f * p.data[0] > s)
        p = symm(__ctx, 1.732050808f / 3.f, 1.0f, 0.f, p);
      if (p.data[1] - 1.732050808f * p.data[0] > s)
        p = symm(__ctx, -1.732050808f / 3.f, 1.0f, 0.f, p);
      p.data[1] += s;
    } else {
      float a = -2.0f * fract(iTime);
      col += drawTri(__ctx,
                     f32x2(p.data[0] * cos(a) - p.data[1] * sin(a),
                           p.data[1] * cos(a) + p.data[0] * sin(a)),
                     fract(iTime) * s * 0.5f) *
             .05f;
    }
  }
  col *= pow(16.0f * uv.data[0] * uv.data[1] * (1.0f - uv.data[0]) *
                 (1.0f - uv.data[1]),
             0.1f);
  fragColor.set_swizzle<0, 1, 2>(col);
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
