// Port of https://www.shadertoy.com/view/7lSyDV; author: Spi3lot.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7lSyDV.json.
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
float f(Context &__ctx, float x) {
  return 15.0f * (sin(x * 2.0f) + sin(1.1f * (x + acos(-1.0f) * 0.5f)));
}
float sdf_by_Spi3lot(Context &__ctx, f32x2 p) {
  float y0 = f(__ctx, p.data[0]);
  float minDist = p.data[1] - y0;
  float sgn = sign(minDist);
  minDist *= sgn;
  for (float i = 1.f; i < (iTime * 5.f); i++) {
    float dx = minDist * i / (iTime * 5.f);
    float xL = p.data[0] - dx;
    float yL = f(__ctx, xL);
    float xR = p.data[0] + dx;
    float yR = f(__ctx, xR);
    float distL, distR, mini;
    {
      distL = distance(p, f32x2(xL, yL));
      distR = distance(p, f32x2(xR, yR));
      mini = min(distL, distR);
    }
    if (mini < minDist) {
      minDist = mini;
    }
  }
  return sgn * minDist;
}
float sdf_by_Spi3lot_pure_no_comments(Context &__ctx, f32x2 p) {
  float y0 = f(__ctx, p.data[0]);
  float minDist = p.data[1] - y0;
  float sgn = sign(minDist);
  minDist *= sgn;
  for (float i = 1.f; i < (iTime * 5.f); i++) {
    float dx = minDist * i / (iTime * 5.f);
    float xL = p.data[0] - dx;
    float yL = f(__ctx, xL);
    float xR = p.data[0] + dx;
    float yR = f(__ctx, xR);
    float distL = distance(p, f32x2(xL, yL));
    float distR = distance(p, f32x2(xR, yR));
    float mini = min(distL, distR);
    minDist = min(minDist, mini);
  }
  return sgn * minDist;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 pos = 100.0f * (uv - f32x2(0.0f, 0.5f));
  float dist = sdf_by_Spi3lot(__ctx, pos);
  f32x3 col = f32x3(abs(dist));
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
