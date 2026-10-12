// Port of https://www.shadertoy.com/view/mtjyDw; author: beans_please.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mtjyDw.json.
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
struct Context {
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float sd_segment(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h);
}
float f(Context &__ctx, float x) {
  return .6f * sin(4.f * x + iTime * 3.f) * exp(-x * .25f);
}
const float zoom = 2.8f;
float px2uv;
float max_thickness;
float dist_f(Context &__ctx, f32x2 p) {
  int n = int(.5f * iResolution.data[0]);
  float total_len = iResolution.data[0] * px2uv;
  float step_size = total_len / float(n);
  int idx0 =
      int(floor((p.data[0] - max(max_thickness, step_size)) / step_size));
  int idx1 =
      int(floor((p.data[0] + max(max_thickness, step_size)) / step_size));
  float dist = 1e9f;
  for (int i = idx0; i <= idx1; i++) {
    float ax = float(i) * step_size;
    float ay = f(__ctx, ax);
    float bx = float(i + 1) * step_size;
    float by = f(__ctx, bx);
    dist = min(dist, sd_segment(__ctx, p, f32x2(ax, ay), f32x2(bx, by)));
  }
  return dist;
}
f32x3 render(Context &__ctx, f32x2 frag_coord) {
  f32x2 uv = px2uv * (frag_coord - iResolution.swizzle<0, 1>() * .5f);
  if (iMouse.data[2] > .2f) {
    uv += px2uv * (abs(iMouse.swizzle<2, 3>()) - iMouse.swizzle<0, 1>());
  }
  uv += (texture(
             __ctx.channel0,
             .4f * uv +
                 f32x2(13.4823f * ((floor(iTime / .125f) + .5f) * .125f), 0.f))
             .swizzle<0, 1>() -
         .5f) *
        .02f;
  f32x3 col = f32x3(1.f, .9f, 0.72f);
  f32x3 col_grid = f32x3(0.3f, 0.1f, 0.065f);
  {
    float d = min(abs(uv.data[0] - round(uv.data[0])),
                  abs(uv.data[1] - round(uv.data[1])));
    if (d < .006f)
      col = mix(col, col_grid, .5f);
    f32x2 t = uv * 4.f;
    d = min(abs(t.data[0] - round(t.data[0])),
            abs(t.data[1] - round(t.data[1])));
    if (d < .006f * 4.f)
      col = mix(col, col_grid, .2f);
    d = min(abs(uv.data[0]), abs(uv.data[1]));
    if (d < .012f)
      col = col_grid;
  }
  float d = dist_f(__ctx, uv);
  float thick = max_thickness;
  col = mix(col, f32x3(.03f, .04f, 0.6f), smoothstep(thick, thick - px2uv, d));
  return col;
}
void mainImage(Context &__ctx, f32x4 &frag_col, f32x2 frag_coord) {
  px2uv = 2.f * zoom / min(iResolution.data[0], iResolution.data[1]);
  max_thickness = .03f;
  f32x3 col = f32x3(0);
  {
    col += render(__ctx, frag_coord + f32x2(-.25f, -.25f));
    col += render(__ctx, frag_coord + f32x2(-.25f, +.25f));
    col += render(__ctx, frag_coord + f32x2(+.25f, +.25f));
    col += render(__ctx, frag_coord + f32x2(+.25f, -.25f));
  }
  col /= 4.f;
  col = pow(col, f32x3(1.f / 2.2f));
  frag_col = f32x4(col, 1.f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
               __binding(1, 0) sampler sampling0) {
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
  Context __ctx{{channel0, sampling0}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
