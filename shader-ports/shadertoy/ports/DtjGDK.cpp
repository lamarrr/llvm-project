// Port of https://www.shadertoy.com/view/DtjGDK; author: mla.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtjGDK.json.
#include "../support.h"
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
bool alert = false;
void assert(Context &__ctx, bool b) {
  if (!b)
    alert = true;
}
f32x2x2 rotate(Context &__ctx, float t) {
  return f32x2x2(cos(t), -sin(t), sin(t), cos(t));
}
f32x3 h2rgb(Context &__ctx, float h) {
  f32x3 rgb =
      clamp(abs(mod(h * 6.0f + f32x3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f,
            0.0f, 1.0f);
  return rgb * rgb * (3.0f - 2.0f * rgb);
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 I) {
  int AA = 2, NBITS = 80;
  int NCOLS = 8, LOOP = NCOLS / 2;
  f32x2x2 A = 0.5f * f32x2x2(-1, -1, 1, -1);
  f32x3 basecol = f32x3(1, 1, 0.5f);
  I -= 0.5f * iResolution.swizzle<0, 1>();
  O = f32x4(0);
  for (int i = 0; i < AA; i++) {
    for (int j = 0; j < AA; j++) {
      f32x2 p = I + f32x2(i, j) / float(AA);
      p *= rotate(__ctx, 0.1f * iTime);
      p *= exp2(mod(-0.75f * iTime, float(LOOP)));
      int i = 0;
      for (i = 0; i < NBITS; i++) {
        assert(__ctx, i < NBITS - 1);
        p = floor(A * p);
        if (p == f32x2(0))
          break;
      }
      O.set_swizzle<0, 1, 2>(O.swizzle<0, 1, 2>() +
                             (float(i % NCOLS) / float(NCOLS - 1) * basecol));
    }
  }
  O /= float(AA * AA);
  O = pow(O, f32x4(0.4545f));
  if (alert)
    O.data[0] = 1.0f;
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
