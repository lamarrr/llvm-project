// Port of https://www.shadertoy.com/view/Ddt3R7; author: rrrola.
// Dataset license metadata: public-domain; original retained in
// upstream/Ddt3R7.json.
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
float nm_min = 300.f, nm_max = 750.f;
float sd_nm = 30.f;
float scale = 0.46f;
float add_gray = 0.22f;
f32x3 xyz_from_wavelength(Context &__ctx, float nm) {
  float x1 = (nm - 442.0f) * (nm < 442.0f ? .0624f : .0374f);
  float x2 = (nm - 599.8f) * (nm < 599.8f ? .0264f : .0323f);
  float x3 = (nm - 501.1f) * (nm < 501.1f ? .0490f : .0382f);
  float y1 = (nm - 568.8f) * (nm < 568.8f ? .0213f : .0247f);
  float y2 = (nm - 530.9f) * (nm < 530.9f ? .0613f : .0322f);
  float z1 = (nm - 437.0f) * (nm < 437.0f ? .0845f : .0278f);
  float z2 = (nm - 459.0f) * (nm < 459.0f ? .0385f : .0725f);
  return f32x3(0.362f * exp(-.5f * x1 * x1) + 1.056f * exp(-.5f * x2 * x2) -
                   0.065f * exp(-.5f * x3 * x3),
               0.821f * exp(-.5f * y1 * y1) + 0.286f * exp(-.5f * y2 * y2),
               1.217f * exp(-.5f * z1 * z1) + 0.681f * exp(-.5f * z2 * z2));
}
f32x3 rgb_from_xyz(Context &__ctx, f32x3 xyz) {
  return f32x3x3(f32x3(3.2404542f, -0.9692660f, 0.0556434f),
                 f32x3(-1.5371385f, 1.8760108f, -0.2040259f),
                 f32x3(-0.4985314f, 0.0415560f, 1.0572252f)) *
         xyz;
}
float normal_pdf(Context &__ctx, float mean, float sd, float x) {
  float z = (x - mean) / sd;
  return exp(-0.5f * z * z) / (sd * sqrt(2.0f * 3.14159265358979323846f));
}
f32x3 convolve_spectrum_with_normal_pdf(Context &__ctx, float mean, float sd) {
  float sum_w = 0.f;
  f32x3 sum = f32x3(0);
  for (float i = -5.f; i <= 5.f; i += 0.0625f) {
    float nm = mean + i * sd;
    float w = normal_pdf(__ctx, mean, sd, nm);
    sum_w += w;
    sum += w * rgb_from_xyz(__ctx, xyz_from_wavelength(__ctx, nm));
  }
  return sum / sum_w;
}
f32x3 color(Context &__ctx, float mean, float sd) {
  return add_gray + scale * convolve_spectrum_with_normal_pdf(__ctx, mean, sd);
}
float unmix(Context &__ctx, float a, float b, float x) {
  return (x - a) / (b - a);
}
float aa(Context &__ctx, float r, float x) { return clamp(r - x, 0.f, 1.f); }
float seg_dist(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  p -= a;
  b -= a;
  return length(p - b * clamp(dot(p, b) / dot(b, b), 0.f, 1.f));
}
float line_dist(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  p -= a;
  b -= a;
  return length(p - b * (dot(p, b) / dot(b, b)));
}
float aa_seg(Context &__ctx, float width, f32x2 p, f32x2 a, f32x2 b) {
  return aa(__ctx, width, seg_dist(__ctx, p, a, b));
}
float aa_line(Context &__ctx, float width, f32x2 p, f32x2 a, f32x2 b) {
  return aa(__ctx, width, line_dist(__ctx, p, a, b));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float nm = mix(nm_min, nm_max, fragCoord.data[0] / iResolution.data[0]);
  float x0 = nm;
  f32x3 y0 = color(__ctx, x0, sd_nm);
  float x1 = nm + .001f;
  f32x3 y1 = color(__ctx, x1, sd_nm);
  f32x3 col = y0;
  col = mix(col, f32x3(0),
            aa_line(__ctx, 1.f, fragCoord,
                    (iResolution.swizzle<0, 1>() *
                     f32x2(unmix(__ctx, nm_min, nm_max, 0.f), add_gray)),
                    (iResolution.swizzle<0, 1>() *
                     f32x2(unmix(__ctx, nm_min, nm_max, 1.f), add_gray))));
  float tick = round(nm / 10.f) * 10.f;
  float tick_mod_100 = mod(tick, 100.f);
  float tick_size = tick_mod_100 == 0.f || tick_mod_100 == 100.f ? 0.03f
                    : tick_mod_100 == 50.f                       ? 0.02f
                                                                 : 0.01f;
  col = mix(
      col, f32x3(0),
      aa_seg(
          __ctx, 1.f, fragCoord,
          (iResolution.swizzle<0, 1>() *
           f32x2(unmix(__ctx, nm_min, nm_max, tick), add_gray - tick_size)),
          (iResolution.swizzle<0, 1>() *
           f32x2(unmix(__ctx, nm_min, nm_max, tick), add_gray + tick_size))));
  col = mix(col, f32x3(0, 0, 0),
            aa_line(__ctx, 2.5f, fragCoord,
                    (iResolution.swizzle<0, 1>() *
                     f32x2(unmix(__ctx, nm_min, nm_max, x0), y0.data[0])),
                    (iResolution.swizzle<0, 1>() *
                     f32x2(unmix(__ctx, nm_min, nm_max, x1), y1.data[0]))));
  col = mix(col, f32x3(.87f, .14f, .09f),
            aa_line(__ctx, 1.5f, fragCoord,
                    (iResolution.swizzle<0, 1>() *
                     f32x2(unmix(__ctx, nm_min, nm_max, x0), y0.data[0])),
                    (iResolution.swizzle<0, 1>() *
                     f32x2(unmix(__ctx, nm_min, nm_max, x1), y1.data[0]))));
  col = mix(col, f32x3(0, 0, 0),
            aa_line(__ctx, 2.5f, fragCoord,
                    (iResolution.swizzle<0, 1>() *
                     f32x2(unmix(__ctx, nm_min, nm_max, x0), y0.data[1])),
                    (iResolution.swizzle<0, 1>() *
                     f32x2(unmix(__ctx, nm_min, nm_max, x1), y1.data[1]))));
  col = mix(col, f32x3(.07f, .47f, .06f),
            aa_line(__ctx, 1.5f, fragCoord,
                    (iResolution.swizzle<0, 1>() *
                     f32x2(unmix(__ctx, nm_min, nm_max, x0), y0.data[1])),
                    (iResolution.swizzle<0, 1>() *
                     f32x2(unmix(__ctx, nm_min, nm_max, x1), y1.data[1]))));
  col = mix(col, f32x3(0, 0, 0),
            aa_line(__ctx, 2.5f, fragCoord,
                    (iResolution.swizzle<0, 1>() *
                     f32x2(unmix(__ctx, nm_min, nm_max, x0), y0.data[2])),
                    (iResolution.swizzle<0, 1>() *
                     f32x2(unmix(__ctx, nm_min, nm_max, x1), y1.data[2]))));
  col = mix(col, f32x3(.13f, .31f, 1),
            aa_line(__ctx, 1.5f, fragCoord,
                    (iResolution.swizzle<0, 1>() *
                     f32x2(unmix(__ctx, nm_min, nm_max, x0), y0.data[2])),
                    (iResolution.swizzle<0, 1>() *
                     f32x2(unmix(__ctx, nm_min, nm_max, x1), y1.data[2]))));
  fragColor = f32x4(pow(col, f32x3(1.0f / 2.2f)), 1);
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
