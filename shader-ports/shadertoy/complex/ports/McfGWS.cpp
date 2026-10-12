// Port of https://www.shadertoy.com/view/McfGWS; author: FordPerfect.
// Dataset license metadata: public-domain; original retained in
// upstream/McfGWS.json.
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
const float pi = 3.14159265358979323846f;
float K(Context &__ctx, float k) {
  k = abs(k);
  if (k < 0.9765625f) {
    float a = 1.0f, b = sqrt(1.0f - k * k);
    for (int i = 0; i < 4; ++i) {
      float an = 0.5f * (a + b);
      float bn = sqrt(a * b);
      a = an;
      b = bn;
    }
    return pi / (2.0f * a);
  } else {
    float x = 1.0f - k, y = log(x);
    return (1.03972077084f - 0.5f * y) +
           x * (0.26986038542f - 0.25f * y +
                x * (0.10616274088f - 0.15625f * y +
                     x * (0.05035558528f - 0.109375f * y)));
  }
}
float am(Context &__ctx, float u, float k) {
  float a0 = 1.0f, b0 = sqrt(1.0f - k * k);
  float a1 = 0.5f * (a0 + b0), b1 = sqrt(a0 * b0), c1 = 0.5f * (a0 - b0);
  float a2 = 0.5f * (a1 + b1), b2 = sqrt(a1 * b1), c2 = 0.5f * (a1 - b1);
  float a3 = 0.5f * (a2 + b2), b3 = sqrt(a2 * b2), c3 = 0.5f * (a2 - b2);
  float a4 = 0.5f * (a3 + b3), c4 = 0.5f * (a3 - b3);
  float phi4 = 16.0f * a4 * u;
  float phi3 = 0.5f * (phi4 + asin((c4 / a4) * sin(phi4)));
  float phi2 = 0.5f * (phi3 + asin((c3 / a3) * sin(phi3)));
  float phi1 = 0.5f * (phi2 + asin((c2 / a2) * sin(phi2)));
  float phi0 = 0.5f * (phi1 + asin((c1 / a1) * sin(phi1)));
  return phi0;
}
float cd(Context &__ctx, float u, float k) {
  u = mod(u, 4.0f * K(__ctx, k));
  float a = am(__ctx, u, k), sn = sin(a), cn = cos(a),
        dn = sqrt(1.0f - k * k * sn * sn);
  return cn / dn;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 xy =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  float A0 = pi / 2.0f;
  float w = 1.0f;
  f32x2 O = f32x2(0.0f, 1.0f);
  f32x3 col = f32x3(0.25f);
  if (true) {
    float k = sin(A0 / 2.0f);
    float theta = 2.0f * asin(k * cd(__ctx, w * iTime, k));
    f32x2 P = O + f32x2(sin(theta), -cos(theta));
    float a = smoothstep(0.05f, 0.04f, length(xy - P)) +
              smoothstep(0.01f, 0.00f,
                         abs(dot(P - O, f32x2(xy.data[1] - O.data[1],
                                              O.data[0] - xy.data[0])))) *
                  smoothstep(1.01f, 0.99f, length(xy - O));
    col += f32x3(0.25f, 0.625f, 0.25f) * clamp(a, 0.0f, 1.0f);
  }
  if (true) {
    float theta = A0 * cos(w * iTime);
    f32x2 P = O + f32x2(sin(theta), -cos(theta));
    float a = smoothstep(0.05f, 0.04f, length(xy - P)) +
              smoothstep(0.01f, 0.00f,
                         abs(dot(P - O, f32x2(xy.data[1] - O.data[1],
                                              O.data[0] - xy.data[0])))) *
                  smoothstep(1.01f, 0.99f, length(xy - O));
    col += f32x3(0.5f, 0.5f, 0.5f) * clamp(a, 0.0f, 1.0f);
  }
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
