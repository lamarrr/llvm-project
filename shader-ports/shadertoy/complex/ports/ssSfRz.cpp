// Port of https://www.shadertoy.com/view/ssSfRz; author: Michel_Rouzic.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ssSfRz.json.
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
float sq(Context &__ctx, float x) { return x * x; }
float gaussian(Context &__ctx, float x) { return exp(-x * x); }
float erf_fast(Context &__ctx, float x) {
  float y, xa = abs(x);
  y = ((-0.06388f * xa - 0.66186f) * xa - 1.123613f) * xa;
  y = 1.f - exp(y);
  y *= sign(x);
  return y;
}
float lsrgb(Context &__ctx, float l) {
  float x, line, curve;
  line = l * 12.92f;
  x = sqrt(l);
  curve = ((((0.455f * x - 1.48f) * x + 1.92137f) * x - 1.373254f) * x +
           1.51733216f) *
              x -
          0.0404733783f;
  return l <= 0.0031308f ? line : curve;
}
f32x2 point_rot(Context &__ctx, f32x2 p, float costh, float sinth) {
  f32x2 rp;
  rp.data[0] = p.data[0] * costh - p.data[1] * sinth;
  rp.data[1] = p.data[0] * sinth + p.data[1] * costh;
  return rp;
}
float thin_line_weight(Context &__ctx, float r1x, float r1y, float r2x,
                       float costh, float sinth, f32x2 pf) {
  float v;
  f32x2 rp;
  rp = point_rot(__ctx, pf, costh, sinth);
  v = (erf_fast(__ctx, rp.data[0] - r1x) - erf_fast(__ctx, rp.data[0] - r2x)) *
      0.5f;
  v *= gaussian(__ctx, rp.data[1] - r1y);
  return v;
}
float point_weight(Context &__ctx, f32x2 dp, float rad, f32x2 pf) {
  float d = distance(dp, pf) * rad;
  return gaussian(__ctx, d);
}
float circle_full_weight(Context &__ctx, f32x2 pc, float circrad, float rad,
                         f32x2 pf) {
  float dc, dn, df;
  dc = distance(pf, pc);
  dn = (circrad - dc) * rad;
  df = -(circrad + dc) * rad;
  return (erf_fast(__ctx, dn) - erf_fast(__ctx, df)) * 0.5f;
}
float circle_hollow_weight(Context &__ctx, f32x2 pc, float circrad, float rad,
                           f32x2 pf) {
  float dc, dn, df;
  dc = distance(pf, pc);
  dn = (circrad - dc) * rad;
  df = -(circrad + dc) * rad;
  return gaussian(__ctx, dn) + gaussian(__ctx, df);
}
float rect_full_weight(Context &__ctx, f32x2 p0, f32x2 p1, float rad,
                       f32x2 pf) {
  float d;
  f32x2 d0, d1, gv;
  d0 = (pf - p0) * rad;
  d1 = (pf - p1) * rad;
  gv.data[0] = erf_fast(__ctx, d0.data[0]) - erf_fast(__ctx, d1.data[0]);
  gv.data[1] = erf_fast(__ctx, d0.data[1]) - erf_fast(__ctx, d1.data[1]);
  return gv.data[0] * gv.data[1] * 0.25f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float scrscale = 18.f / iResolution.data[1];
  f32x2 uv = (fragCoord - iResolution.swizzle<0, 1>() * 0.5f) * scrscale;
  float th = iTime * 6.2831853f / 40.f;
  float costh = cos(th);
  float sinth = sin(-th);
  float rad = sq(__ctx, 1.f - abs(cos(iTime * 6.2831853f / 16.f))) * 6.f;
  float min_rad = 0.8f * scrscale;
  rad = length(f32x2(rad, min_rad));
  f32x3 pv = f32x3(0.f);
  pv += thin_line_weight(__ctx, -3.5f / rad, 0.f / rad, 5.f / rad, costh / rad,
                         -sinth / rad, uv) *
        min_rad / rad;
  pv += f32x3(0.f, 0.44f, 1.f) *
        circle_full_weight(__ctx,
                           point_rot(__ctx, f32x2(-5.f, 0.f), costh, sinth),
                           1.5f, 1.f / rad, uv);
  pv += f32x3(0.5f, 1.f, 0.f) *
        circle_hollow_weight(__ctx,
                             point_rot(__ctx, f32x2(5.f, 0.f), costh, sinth),
                             1.5f, 1.f / rad, uv) *
        min_rad / rad;
  for (float dt = -3.25f; dt <= 3.25f; dt += 0.5f)
    pv += mix(f32x3(1.f, 1.f, 0.f), f32x3(1.f, 0.f, 0.f), (dt + 3.5f) / 7.f) *
          point_weight(__ctx, point_rot(__ctx, f32x2(0.f, dt), costh, sinth),
                       1.f / rad, uv) *
          sq(__ctx, min_rad / rad);
  pv += f32x3(1.f, 0.f, 0.06f) *
        rect_full_weight(__ctx, f32x2(-1.5f, 3.5f), f32x2(1.5f, 6.5f),
                         1.f / rad, point_rot(__ctx, uv, costh, -sinth));
  pv += f32x3(1.f, 1.f, 0.f) *
        (rect_full_weight(__ctx, f32x2(-1.5f, -6.5f), f32x2(1.5f, -3.5f),
                          1.f / rad, point_rot(__ctx, uv, costh, -sinth)) -
         rect_full_weight(__ctx, f32x2(-1.f, -6.f), f32x2(1.f, -4.f), 1.f / rad,
                          point_rot(__ctx, uv, costh, -sinth)));
  fragColor = f32x4(lsrgb(__ctx, pv.data[0]), lsrgb(__ctx, pv.data[1]),
                    lsrgb(__ctx, pv.data[2]), 1.f);
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
