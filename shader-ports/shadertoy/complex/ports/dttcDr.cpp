// Port of https://www.shadertoy.com/view/dttcDr; author: scry.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dttcDr.json.
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
f32x3 cs = f32x3(1.f, 2.f, 3.f);
f32x2x2 r2d(Context &__ctx, float a) {
  return f32x2x2(cos(a), sin(a), -sin(a), cos(a));
}
f32x3 fcos(Context &__ctx, f32x3 x) {
  f32x3 w = fwidth(x);
  return cos(x) * sin(0.5f * w) / (0.5f * w);
}
f32x3 t1(Context &__ctx, f32x2 uv, float t, float v) {
  f32x3 col = f32x3(0.f);
  float c = length(uv);
  uv = f32x2(log(length(uv)), atan(uv.data[0], uv.data[1]));
  uv.data[0] += v * -2.f;
  uv.data[0] *= 0.3f;
  uv.data[0] += uv.data[1] * 0.0795f;
  float sx = floor(uv.data[0] * 2.f) / acos(-1.f) + 2.f +
             fract((uv.data[1] / acos(-1.f) / 2.f + 0.5f)) * -0.3183f;
  uv.data[0] = (fract(uv.data[0] * 2.f) - 0.5f) / 2.f;
  float s = (min(abs(uv.data[0] - sin(uv.data[1] * 8.f) * 0.1f),
                 abs(uv.data[0] + sin(uv.data[1] * 8.f) * 0.1f)));
  col -= (smoothstep(0.01f, 0.f, abs(uv.data[0] - 0.25f)) +
          smoothstep(0.01f, 0.f, abs(uv.data[0] + 0.25f))) *
         0.02f;
  col -= fcos(__ctx, uv.swizzle<0, 0, 0>() * 190.f + t * 25.f) * 0.1f;
  col += fcos(__ctx, sx * 28.f + t + cs * 0.2f + t * 8.f) * 0.12f;
  col += fcos(__ctx, sx * 0.65f * v * v + t * 0.1f + cs * 0.8f +
                         iTime * acos(-1.f) / 10.f * 2.f) *
             0.4f +
         0.6f;
  col = clamp(col, -0.f, 1.f);
  return col;
}
f32x3 t2(Context &__ctx, f32x2 uv, float t) {
  f32x3 col = f32x3(0.f);
  float c = length(uv);
  uv = uv / dot(uv, uv);
  uv *= 0.002f;
  uv *= 30.f;
  uv = (fract(uv * 12.f - 0.5f) - 0.5f) / 12.f;
  uv = abs(uv) - 0.024f;
  uv *= r2d(__ctx, iTime * acos(-1.f) / 10.f * -12.f);
  col = t1(__ctx, uv, t, log(c) * 0.8f) * 0.7f;
  col = clamp(col, 0.f, 1.f);
  col = mix(col, f32x3(0.5f), log(1.f - c) * 0.f);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 tv = uv;
  uv -= 0.5f;
  uv.data[0] *=
      iResolution.swizzle<0, 1>().data[0] / iResolution.swizzle<0, 1>().data[1];
  uv *= 0.6f;
  f32x3 col = f32x3(0.f);
  float c = length(uv);
  for (int j = 0; j < 4; j++) {
    float jj = float(j + 1);
    f32x2 ov = uv;
    uv *= 1.f + sin(iTime * acos(-1.f) / 10.f + jj + c +
                    sin(uv.data[0] * 8.f - iTime * acos(-1.f) / 10.f) +
                    sin(uv.data[1] * 8.f + iTime * acos(-1.f) / 10.f)) *
                    0.06f;
    uv *= r2d(__ctx, sin(iTime * acos(-1.f) / 10.f + jj + c * 2.f) *
                         acos(-1.f) / 180.f * 4.f);
    for (int i = 0; i < 12; i++) {
      float ii = float(i);
      float aa =
          fract(ii / 48.f + jj / 3.f + iTime * acos(-1.f) / 10.f + c * 1.5f);
      float sa = abs(aa - 0.5f) * 2.f;
      uv *= 1.f + sin(iTime * acos(-1.f) / 10.f + jj + c +
                      cos(uv.data[0] * 8.f - iTime * acos(-1.f) / 10.f) +
                      cos(uv.data[1] * 8.f + iTime * acos(-1.f) / 10.f)) *
                      0.0005f;
      f32x3 nl =
          t2(__ctx, uv, iTime * acos(-1.f) / 10.f - ii * acos(-1.f)) * sa *
          0.03f *
          (sin(jj * acos(-1.f) + iTime * acos(-1.f) / 10.f) * 0.5f + 0.5f);
      col +=
          nl + sin(col * 8.f + nl * 12.f + sin(atan(uv.data[0], uv.data[1])) +
                   iTime * acos(-1.f) / 10.f + c * 2.f) *
                   0.002f;
      uv += f32x2(sin(iTime * acos(-1.f) / 10.f + c * 5.f * sa + sa +
                      cos(uv.data[0] * 18.f + iTime * acos(-1.f) / 10.f) + jj),
                  cos(iTime * acos(-1.f) / 10.f + c * 8.f * sa + sa +
                      sin(uv.data[1] * 18.f +
                          iTime * acos(-1.f) / 10.f * 0.4f))) *
            0.0013f * sa *
            sin(c * 1.2f + iTime * acos(-1.f) / 10.f + jj * 2.f) * jj;
    }
    ov *= 0.95f;
    uv = ov;
  }
  col *= 3.f;
  col += 0.2f;
  col = pow(col, f32x3(2.f / 1.2f));
  fragColor = f32x4(col, 1.0f);
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
