// Port of https://www.shadertoy.com/view/msjGzz; author: fishy.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/msjGzz.json.
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

void join(Context &__ctx, float &a, float b) { a = min(a, b); }
void subt(Context &__ctx, float &a, float b) { a = max(a, -b); }
float box(Context &__ctx, f32x2 p, f32x2 s) {
  return max((abs(p) - s).data[0], (abs(p) - s).data[1]);
}
float digit(Context &__ctx, f32x2 p, int d) {
  float r = 100000000.0f;
  switch (d) {
  case (0):
    join(__ctx, r, box(__ctx, p, f32x2(0.1f, 0.15f)));
    subt(__ctx, r, box(__ctx, p, f32x2(0.05f, 0.1f)));
    break;
  case (1):
    join(__ctx, r, box(__ctx, p, f32x2(0.025f, 0.15f)));
    break;
  case (2):
    join(__ctx, r, box(__ctx, p, f32x2(0.1f, 0.15f)));
    subt(__ctx, r, box(__ctx, p - f32x2(0.15f, -0.06f), f32x2(0.2f, 0.035f)));
    subt(__ctx, r, box(__ctx, p - f32x2(-0.15f, 0.06f), f32x2(0.2f, 0.035f)));
    break;
  case (3):
    join(__ctx, r, box(__ctx, p, f32x2(0.1f, 0.15f)));
    subt(__ctx, r, box(__ctx, p - f32x2(-0.15f, -0.06f), f32x2(0.2f, 0.035f)));
    subt(__ctx, r, box(__ctx, p - f32x2(-0.15f, 0.06f), f32x2(0.2f, 0.035f)));
    break;
  case (4):
    join(__ctx, r, box(__ctx, p - f32x2(0.075f, 0.0f), f32x2(0.025f, 0.15f)));
    join(__ctx, r, box(__ctx, p - f32x2(0.0f, 0.05f), f32x2(0.1f, 0.1f)));
    subt(__ctx, r, box(__ctx, p - f32x2(0.0f, 0.09f), f32x2(0.05f, 0.085f)));
    break;
  case (5):
    join(__ctx, r, box(__ctx, p, f32x2(0.1f, 0.15f)));
    subt(__ctx, r, box(__ctx, p - f32x2(0.15f, 0.06f), f32x2(0.2f, 0.035f)));
    subt(__ctx, r, box(__ctx, p - f32x2(-0.15f, -0.06f), f32x2(0.2f, 0.035f)));
    break;
  case (6):
    p = -p;
    join(__ctx, r, box(__ctx, p - f32x2(0.075f, 0.0f), f32x2(0.025f, 0.15f)));
    join(__ctx, r, box(__ctx, p - f32x2(0.0f, 0.05f), f32x2(0.1f, 0.1f)));
    subt(__ctx, r, box(__ctx, p - f32x2(0.0f, 0.05f), f32x2(0.05f, 0.05f)));
    break;
  case (7):
    join(__ctx, r, box(__ctx, p - f32x2(0.0f), f32x2(0.1f, 0.15f)));
    subt(__ctx, r, box(__ctx, p - f32x2(-0.05f), f32x2(0.1f, 0.15f)));
    break;
  case (8):
    join(__ctx, r, box(__ctx, p - f32x2(0.0f), f32x2(0.1f, 0.15f)));
    subt(__ctx, r, box(__ctx, p - f32x2(0.0f, 0.05f), f32x2(0.05f, 0.05f)));
    subt(__ctx, r, box(__ctx, p - f32x2(0.0f, -0.05f), f32x2(0.05f, 0.05f)));
    join(__ctx, r, box(__ctx, p - f32x2(0.0f), f32x2(0.1f, 0.025f)));
    break;
  case (9):
    join(__ctx, r, box(__ctx, p - f32x2(0.075f, 0.0f), f32x2(0.025f, 0.15f)));
    join(__ctx, r, box(__ctx, p - f32x2(0.0f, 0.05f), f32x2(0.1f, 0.1f)));
    subt(__ctx, r, box(__ctx, p - f32x2(0.0f, 0.05f), f32x2(0.05f, 0.05f)));
    break;
  }
  return r;
}
float getDig(Context &__ctx, float num, float d) {
  float a = floor(num / pow(10.f, d));
  return a - floor(a / 10.f) * 10.f;
}
float print(Context &__ctx, f32x2 p, float num, float spacing) {
  float r = 10000.0f;
  int fig = int(ceil(log2(num) / 3.32192809489f));
  for (int i = 0; i < fig; i++) {
    int di = int(getDig(__ctx, num, float(i)));
    join(__ctx, r, digit(__ctx, p + f32x2(i, 0) * spacing, di));
  }
  return r;
}
f32x3 printFR(Context &__ctx, f32x2 p) {
  return 1.f - f32x3(step(0.f, print(__ctx, (p - f32x2(0.474f, 0.25f)) * 9.f,
                                     iFrameRate, 0.3f)));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 mouse =
      (iMouse.data[0] == 0.0f
           ? f32x2(0.0f)
           : iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - 0.5f) *
      f32x2(1.f, iResolution.data[1] / iResolution.data[0]);
  f32x2 uv = (fragCoord / iResolution.swizzle<0, 1>() - 0.5f) *
             f32x2(1.f, iResolution.data[1] / iResolution.data[0]);
  f32x3 col = printFR(__ctx, uv);
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
