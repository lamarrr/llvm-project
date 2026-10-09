// Port of https://www.shadertoy.com/view/dt23zz; author: jonasfrey.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dt23zz.json.
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

float f_n_mixed(Context &__ctx, float n1, float n2, float n_intensity) {
  float n_mixed = n1 * (1.f - n_intensity) + n2 * n_intensity;
  return n_mixed;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 o_fc_nor = fragCoord / iResolution.swizzle<0, 1>();
  f32x4[] a_o = f32x4[](
      f32x4(f_n_mixed(__ctx, 1.f, 0.f, o_fc_nor.data[0]), 0.f, 0.f, 0.f),
      f32x4(0.f, f_n_mixed(__ctx, 1.f, 0.f, o_fc_nor.data[0]), 0.f, 0.f),
      f32x4(0.f, 0.f, f_n_mixed(__ctx, 1.f, 0.f, o_fc_nor.data[0]), 0.f),
      f32x4(f_n_mixed(__ctx, 1.f, 0.f, o_fc_nor.data[0]), 0.f,
            f_n_mixed(__ctx, 0.f, 1.f, o_fc_nor.data[0]), 0.f),
      f32x4(f_n_mixed(__ctx, 0.f, 1.f, 1.f - o_fc_nor.data[0]), 0.f,
            f_n_mixed(__ctx, 0.f, 1.f, o_fc_nor.data[0]), 0.f),
      f32x4(f_n_mixed(__ctx, 0.f, 1.f, o_fc_nor.data[0]), 0.f,
            f_n_mixed(__ctx, 1.f, 0.f, o_fc_nor.data[0]), 0.f),
      f32x4(f_n_mixed(__ctx, 1.f, 0.f, fract(o_fc_nor.data[0] * 2.f)), 0.f, 0.f,
            0.f),
      f32x4(f_n_mixed(__ctx, 1.f, 0.f, fract(o_fc_nor.data[0] * 2.f)), 0.f, 0.f,
            0.f),
      f32x4(f_n_mixed(__ctx, 1.f, 0.f,
                      1.f - (abs(o_fc_nor.data[0] - 0.5f) * 2.f)),
            0.f, 0.f, 0.f));
  float n_index = (o_fc_nor.data[1] / (1.f / float(a_o.length())));
  float n_index_flr = floor(n_index);
  f32x4 o = a_o[int(n_index_flr)];
  float n_index_nor = (n_index_flr / float(a_o.length()));
  float n_d_line = fract(n_index);
  n_d_line = smoothstep(0.05f, 0.075f, n_d_line);
  float n_d_line2 = float(n_d_line < 0.01f);
  fragColor = o * n_d_line + n_d_line2;
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
