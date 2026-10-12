// Port of https://www.shadertoy.com/view/mlBSWc; author: mrange.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mlBSWc.json.
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
const float MaxIter = 12.0f;
float hash(Context &__ctx, float co) {
  return fract(sin(co * 12.9898f) * 13758.5453f);
}
float hash(Context &__ctx, f32x2 co) {
  return fract(sin(dot(co.swizzle<0, 1>(), f32x2(12.9898f, 58.233f))) *
               13758.5453f);
}
f32x2 mod2(Context &__ctx, f32x2 &p, f32x2 size) {
  f32x2 c = floor((p + size * 0.5f) / size);
  p = mod(p + size * 0.5f, size) - size * 0.5f;
  return c;
}
f32x4 plane(Context &__ctx, f32x2 p, float i, float zf, float z, f32x3 bgcol) {
  float sz = 0.5f * zf;
  f32x2 cp = p;
  f32x2 cn = mod2(__ctx, cp, f32x2(2.0f * sz, sz));
  float h0 = hash(__ctx, cn + i + 123.4f);
  float h1 = fract(4483.0f * h0);
  float h2 = fract(8677.0f * h0);
  float h3 = fract(9677.0f * h0);
  float h4 = fract(7877.0f * h0);
  float h5 = fract(9967.0f * h0);
  if (h4 < 0.5f) {
    return f32x4(0.0f);
  }
  float fi = exp(-0.25f * max(z - 2.0f, 0.0f));
  float aa = mix(0.0125f, 2.0f / iResolution.data[1], fi);
  float r = sz * mix(0.1f, 0.475f, h0 * h0);
  float amp = mix(0.5f, 0.5f, h3) * r;
  cp.data[0] -=
      amp * sin(mix(3.0f, 0.25f, h0) * iTime + (2.0f * 3.141592654f) * h2);
  cp.data[0] += 0.95f * (sz - r - amp) * sign(h3 - 0.5f) * h3;
  cp.data[1] += 0.475f * (sz - 2.0f * r) * sign(h5 - 0.5f) * h5;
  float d = length(cp) - r;
  if (d > aa) {
    return f32x4(0.0f);
  }
  f32x3 ocol =
      (0.5f + 0.5f * sin(f32x3(0.0f, 1.0f, 2.0f) + h1 * (2.0f * 3.141592654f)));
  f32x3 icol = sqrt(ocol);
  ocol *= 1.5f;
  icol *= 2.0f;
  const f32x3 lightDir = normalize(f32x3(1.0f, 1.5f, 2.0f));
  float z2 = (r * r - dot(cp, cp));
  f32x3 col = ocol;
  float t = smoothstep(aa, -aa, d);
  if (z2 > 0.0f) {
    float z = sqrt(z2);
    t *= mix(1.0f, 0.8f, z / r);
    f32x3 pp = f32x3(cp, z);
    f32x3 nn = normalize(pp);
    float dd = max(dot(lightDir, nn), 0.0f);
    col = mix(ocol, icol, dd * dd * dd);
  }
  col *= mix(0.8f, 1.0f, h0);
  col = mix(bgcol, col, fi);
  return f32x4(col, t);
}
float tanh_approx(Context &__ctx, float x) {
  float x2 = x * x;
  return clamp(x * (27.0f + x2) / (27.0f + 9.0f * x2), -1.0f, 1.0f);
}
f32x3 effect(Context &__ctx, f32x2 p, f32x2 pp) {
  const f32x3 bgcol0 = f32x3(0.1f, 0.0f, 1.0f) * 0.1f;
  const f32x3 bgcol1 = f32x3(0.0f, 0.4f, 1.0f) * 0.6f;
  f32x3 bgcol = mix(bgcol1, bgcol0, tanh_approx(__ctx, 1.5f * length(p)));
  f32x3 col = bgcol;
  for (float i = 0.0f; i < MaxIter; ++i) {
    const float Near = 4.0f;
    float z = MaxIter - i;
    float zf = Near / (Near + MaxIter - i);
    f32x2 sp = p;
    float h = hash(__ctx, i + 1234.5f);
    sp.data[1] += -mix(0.2f, 0.3f, h * h) * iTime * zf;
    sp += h;
    f32x4 pcol = plane(__ctx, sp, i, zf, z, bgcol);
    col = mix(col, pcol.swizzle<0, 1, 2>(), pcol.data[3]);
  }
  col *= smoothstep(1.5f, 0.5f, length(pp));
  col = clamp(col, 0.0f, 1.0f);
  col = sqrt(col);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 q = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 p = -1.f + 2.f * q;
  f32x2 pp = p;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 col = effect(__ctx, p, pp);
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
