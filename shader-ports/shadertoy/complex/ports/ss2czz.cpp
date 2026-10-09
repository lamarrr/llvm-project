// Port of https://www.shadertoy.com/view/ss2czz; author: bjornornorn.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ss2czz.json.
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
float Linear1(Context &__ctx, float c) {
  return (c <= 0.04045f) ? c / 12.92f : pow((c + 0.055f) / 1.055f, 2.4f);
}
f32x3 Linear3(Context &__ctx, f32x3 c) {
  return f32x3(Linear1(__ctx, c.data[0]), Linear1(__ctx, c.data[1]),
               Linear1(__ctx, c.data[2]));
}
float Srgb1(Context &__ctx, float c) {
  return (c < 0.0031308f ? c * 12.92f : 1.055f * pow(c, 0.41666f) - 0.055f);
}
f32x3 Srgb3(Context &__ctx, f32x3 c) {
  return f32x3(Srgb1(__ctx, c.data[0]), Srgb1(__ctx, c.data[1]),
               Srgb1(__ctx, c.data[2]));
}
f32x3 rgb_to_oklab(Context &__ctx, f32x3 c) {
  float l = 0.4121656120f * c.data[0] + 0.5362752080f * c.data[1] +
            0.0514575653f * c.data[2];
  float m = 0.2118591070f * c.data[0] + 0.6807189584f * c.data[1] +
            0.1074065790f * c.data[2];
  float s = 0.0883097947f * c.data[0] + 0.2818474174f * c.data[1] +
            0.6302613616f * c.data[2];
  float l_ = pow(l, 1.f / 3.f);
  float m_ = pow(m, 1.f / 3.f);
  float s_ = pow(s, 1.f / 3.f);
  f32x3 labResult;
  labResult.data[0] =
      0.2104542553f * l_ + 0.7936177850f * m_ - 0.0040720468f * s_;
  labResult.data[1] =
      1.9779984951f * l_ - 2.4285922050f * m_ + 0.4505937099f * s_;
  labResult.data[2] =
      0.0259040371f * l_ + 0.7827717662f * m_ - 0.8086757660f * s_;
  return labResult;
}
f32x3 oklab_to_rgb(Context &__ctx, f32x3 c) {
  float l_ = c.data[0] + 0.3963377774f * c.data[1] + 0.2158037573f * c.data[2];
  float m_ = c.data[0] - 0.1055613458f * c.data[1] - 0.0638541728f * c.data[2];
  float s_ = c.data[0] - 0.0894841775f * c.data[1] - 1.2914855480f * c.data[2];
  float l = l_ * l_ * l_;
  float m = m_ * m_ * m_;
  float s = s_ * s_ * s_;
  f32x3 rgbResult;
  rgbResult.data[0] =
      +4.0767245293f * l - 3.3072168827f * m + 0.2307590544f * s;
  rgbResult.data[1] =
      -1.2681437731f * l + 2.6093323231f * m - 0.3411344290f * s;
  rgbResult.data[2] =
      -0.0041119885f * l - 0.7034763098f * m + 1.7068625689f * s;
  return rgbResult;
}
f32x2 findCenterAndPurity(Context &__ctx, f32x3 x) {
  f32x3x3 M =
      f32x3x3(2.26775149f, -1.43293879f, 0.1651873f, -0.98535505f, 2.1260072f,
              -0.14065215f, -0.02501605f, -0.26349465f, 1.2885107f);
  x = x * M;
  float x_min = min(x.data[0], min(x.data[1], x.data[2]));
  float x_max = max(x.data[0], max(x.data[1], x.data[2]));
  float c = 0.5f * (x_max + x_min);
  float s = (x_max - x_min);
  f32x3 y = (x - c) / s;
  float c_smooth = c + dot(y * y * y, f32x3(1.0f / 3.0f)) * s;
  float s_smooth = sqrt(dot(x - c, x - c) / 2.0f);
  return f32x2(c_smooth, s_smooth);
}
f32x3 compute(Context &__ctx, float L, float hue, float sat) {
  f32x3 c;
  c.data[0] = L;
  c.data[1] = sat * cos(hue);
  c.data[2] = sat * sin(hue);
  float l_ = 0.0f + 0.3963377774f * c.data[1] + 0.2158037573f * c.data[2];
  float m_ = 0.0f - 0.1055613458f * c.data[1] - 0.0638541728f * c.data[2];
  float s_ = 0.0f - 0.0894841775f * c.data[1] - 1.2914855480f * c.data[2];
  f32x3 lms = f32x3(l_, m_, s_);
  if (sat > 0.001f) {
    f32x2 CP = findCenterAndPurity(__ctx, lms);
    lms -= (0.5f - 0.5f * cos(0.2f * iTime * 2.0f * 3.1415932f)) *
           (CP.data[0] + 1.0f * CP.data[1]);
    lms += c.data[0];
  } else {
    lms += c.data[0];
  }
  lms = lms * lms * lms;
  float l = lms.data[0];
  float m = lms.data[1];
  float s = lms.data[2];
  f32x3 rgbResult;
  rgbResult.data[0] =
      +4.0767245293f * l - 3.3072168827f * m + 0.2307590544f * s;
  rgbResult.data[1] =
      -1.2681437731f * l + 2.6093323231f * m - 0.3411344290f * s;
  rgbResult.data[2] =
      -0.0041119885f * l - 0.7034763098f * m + 1.7068625689f * s;
  return rgbResult;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float xx = (fragCoord.data[0] / iResolution.data[0]) * 24.0f;
  float yy = (fragCoord.data[1] / iResolution.data[0]) * 24.0f + 1.5f;
  float h = floor(xx) / 24.0f * 3.141592f * 2.f;
  float L = (iResolution.data[0] / iResolution.data[1]) * floor(yy) / 24.0f;
  float C = -0.5f * log(0.5f + 0.5f * sin(iTime));
  float C_guess = 1.0f;
  float C_max = 0.0f;
  for (int i = 0; i < 2000; i++) {
    f32x3 rgbTest = compute(__ctx, L, h, C_guess * float(i) / 2000.0f);
    float diff = length(rgbTest - clamp(rgbTest, 0.f, 1.f));
    if (diff < 0.0001f) {
      C_max = C_guess * float(i) / 2000.0f;
    }
  }
  f32x3 rgb;
  rgb = compute(__ctx, L, h, C_max);
  fragColor.set_swizzle<0, 1, 2>(Srgb3(__ctx, f32x3(L * L * L)));
  if (((xx)-floor(xx)) > 0.25f)
    if (((xx)-floor(xx)) < 0.75f)
      if (((yy)-floor(yy)) > 0.25f)
        if (((yy)-floor(yy)) < 0.75f)
          fragColor.set_swizzle<0, 1, 2>(Srgb3(__ctx, rgb));
  fragColor.data[3] = 1.f;
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
