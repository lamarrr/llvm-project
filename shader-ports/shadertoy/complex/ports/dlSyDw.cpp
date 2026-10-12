// Port of https://www.shadertoy.com/view/dlSyDw; author: gehtsiegarnixan.
// Dataset license metadata: mit; original retained in upstream/dlSyDw.json.
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
  Texture3D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x3 cubeGrid(Context &__ctx, f32x3 uvw, float lineThickness) {
  f32x3 sideDistBig = abs(fract(uvw + 0.5f) - 0.5f);
  float detailScale = 4.0f;
  uvw *= detailScale;
  f32x3 sideDistDetail = abs(fract(uvw + 0.5f) - 0.5f);
  f32x3 joined = min(sideDistBig, sideDistDetail);
  return step(joined, f32x3(lineThickness));
}
f32x3 flowPlot(Context &__ctx, f32x3 uvw) {
  float gridScale = 1.3f;
  float lineThickness = 0.02f;
  float time = iTime * 0.25f;
  float vectorScale = 0.5f;
  f32x3 noise = texture(__ctx.channel0, uvw * 0.05f).swizzle<0, 1, 2>();
  f32x3 direction = noise * 2.0f - 1.0f;
  direction *= vectorScale;
  uvw *= gridScale;
  f32x3 grid = cubeGrid(__ctx, uvw, lineThickness);
  int count = 7;
  for (int index = 1; index < count; index += 1) {
    float percent = fract(float(index) / float(count - 1) + time);
    float brightness = 1.0f - percent;
    f32x3 offset = percent * direction;
    f32x3 aniGrid = cubeGrid(__ctx, uvw - offset, lineThickness);
    aniGrid *= brightness;
    grid = max(grid, aniGrid);
  }
  return grid;
}
f32x2x2 mm2(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, -s, s, c);
}
float iSphere(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x3 oc = ro;
  float b = dot(oc, rd);
  float c = dot(oc, oc) - 1.f;
  float h = b * b - c;
  if (h < 0.0f)
    return -1.f;
  return -b - sqrt(h);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - 0.5f;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x2 um = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - .5f;
  um.data[0] *= iResolution.data[0] / iResolution.data[1];
  p *= 1.5f;
  f32x3 ro = f32x3(0.f, 0.f, 2.4f);
  f32x3 rd = normalize(f32x3(p, -1.5f));
  f32x2x2 mx;
  f32x2x2 my;
  if (iMouse.data[0] > 0.0f) {
    mx = mm2(__ctx, um.data[0] * 5.f);
    my = mm2(__ctx, um.data[1] * 5.f);
  } else {
    mx = mm2(__ctx, iTime * .1f + um.data[0] * 5.f);
    my = mm2(__ctx, iTime * 0.05f + um.data[1] * 5.f);
  }
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (mx));
  rd.set_swizzle<0, 2>(rd.swizzle<0, 2>() * (mx));
  ro.set_swizzle<0, 1>(ro.swizzle<0, 1>() * (my));
  rd.set_swizzle<0, 1>(rd.swizzle<0, 1>() * (my));
  float t = iSphere(__ctx, ro, rd);
  f32x3 col = f32x3(0);
  col = flowPlot(__ctx, rd) * 0.5f;
  if (t > 0.f) {
    f32x3 pos = ro + rd * t;
    col = flowPlot(__ctx, pos);
  }
  fragColor = f32x4(col, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_3d<f32x4> channel0,
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
