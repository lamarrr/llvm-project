// Port of https://www.shadertoy.com/view/dtS3Wz; author: zommuter.
// Dataset license metadata: cc-by-nc-4.0; original retained in
// upstream/dtS3Wz.json.
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
float N21(Context &__ctx, f32x2 p, float offset) {
  f32x3 a = fract(f32x3(p.swizzle<0, 1, 0>()) *
                  (f32x3(213.897f, 653.453f, 253.098f) + offset));
  a += dot(a, a.swizzle<1, 2, 0>() + 79.76f);
  return fract((a.data[0] + a.data[1]) * a.data[2]);
}
f32x2 N22(Context &__ctx, f32x2 p) {
  return f32x2(N21(__ctx, p, 32.f), N21(__ctx, p, -314.159f));
}
f32x2 N12(Context &__ctx, float seed) { return N22(__ctx, f32x2(seed)); }
float N11(Context &__ctx, float seed) {
  return N21(__ctx, f32x2(seed, 1.f / seed), seed);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float N_CHARGES =
      5.f + N21(__ctx, f32x2(iMouse.data[0] + 1.f, iMouse.data[1] + 1.f),
                -3.14159f -
                    (2.3f + iMouse.data[0] * iMouse.data[1]) /
                        iResolution.data[0] / iResolution.data[1] -
                    1251.23f) *
                137.f;
  f32x2 uv_ar = fragCoord / iResolution.data[1];
  float aspect = iResolution.data[0] / iResolution.data[1];
  float potential = 0.f;
  for (float n_charge = 0.f; n_charge < N_CHARGES; n_charge++) {
    float x_omega_charge =
        N11(__ctx, n_charge + -3.14159f -
                       (2.3f + iMouse.data[0] * iMouse.data[1]) /
                           iResolution.data[0] / iResolution.data[1] +
                       N_CHARGES + 1.f / 137.f);
    float y_omega_charge =
        N11(__ctx, n_charge - -3.14159f -
                       (2.3f + iMouse.data[0] * iMouse.data[1]) /
                           iResolution.data[0] / iResolution.data[1] +
                       N_CHARGES - 1.f / 137.f);
    float t = 0.75f * iTime +
              N11(__ctx, -3.14159f -
                             (2.3f + iMouse.data[0] * iMouse.data[1]) /
                                 iResolution.data[0] / iResolution.data[1] -
                             1.f);
    f32x2 uv_charge =
        0.5f * f32x2(sin(x_omega_charge * t), cos(y_omega_charge * t)) + .5f;
    float charge =
        (N11(__ctx, n_charge + -3.14159f -
                        (2.3f + iMouse.data[0] * iMouse.data[1]) /
                            iResolution.data[0] / iResolution.data[1] -
                        N_CHARGES) -
         .5f) *
        1e2f;
    float r = max(.0005f * abs(charge),
                  length(uv_ar - uv_charge * f32x2(aspect, 1.f)));
    potential += charge / r;
  }
  float l2 = log(1.f + abs(potential) / 100.f) / 4.f;
  float l1 = -sign(potential) * l2;
  float l3 = sign(potential) * l2;
  float l4 = length(uv_ar - .5f) / 2.f;
  float r1 = N11(__ctx, -3.14159f -
                            (2.3f + iMouse.data[0] * iMouse.data[1]) /
                                iResolution.data[0] / iResolution.data[1] -
                            272154.f);
  float r2 = N11(__ctx, -3.14159f -
                            (2.3f + iMouse.data[0] * iMouse.data[1]) /
                                iResolution.data[0] / iResolution.data[1] -
                            1231.f);
  float r3 = N11(__ctx, -3.14159f -
                            (2.3f + iMouse.data[0] * iMouse.data[1]) /
                                iResolution.data[0] / iResolution.data[1] +
                            12454.f);
  float r4 = N11(__ctx, -3.14159f -
                            (2.3f + iMouse.data[0] * iMouse.data[1]) /
                                iResolution.data[0] / iResolution.data[1] +
                            r3 - r1 * r2);
  float r = (r1 * l1 + r2 * l2 + r3 * l3 + r4 * l4) / (r1 + r2 + r3 + r4);
  float g1 = N11(__ctx, -3.14159f -
                            (2.3f + iMouse.data[0] * iMouse.data[1]) /
                                iResolution.data[0] / iResolution.data[1] -
                            r1);
  float g2 = N11(__ctx, -3.14159f -
                            (2.3f + iMouse.data[0] * iMouse.data[1]) /
                                iResolution.data[0] / iResolution.data[1] -
                            r2);
  float g3 = N11(__ctx, -3.14159f -
                            (2.3f + iMouse.data[0] * iMouse.data[1]) /
                                iResolution.data[0] / iResolution.data[1] +
                            r3);
  float g4 = N11(__ctx, -3.14159f -
                            (2.3f + iMouse.data[0] * iMouse.data[1]) /
                                iResolution.data[0] / iResolution.data[1] +
                            g3 - g1 * g2);
  float g = (g1 * l1 + g2 * l2 + g3 * l3 + g4 * l4) / (g1 + g2 + g3 + g4);
  float b1 = N11(__ctx, -3.14159f -
                            (2.3f + iMouse.data[0] * iMouse.data[1]) /
                                iResolution.data[0] / iResolution.data[1] -
                            g1);
  float b2 = N11(__ctx, -3.14159f -
                            (2.3f + iMouse.data[0] * iMouse.data[1]) /
                                iResolution.data[0] / iResolution.data[1] -
                            g3);
  float b3 = N11(__ctx, -3.14159f -
                            (2.3f + iMouse.data[0] * iMouse.data[1]) /
                                iResolution.data[0] / iResolution.data[1] +
                            g2);
  float b4 = N11(__ctx, -3.14159f -
                            (2.3f + iMouse.data[0] * iMouse.data[1]) /
                                iResolution.data[0] / iResolution.data[1] +
                            b3 - b1 * b2);
  float b = (b1 * l1 + b2 * l2 + b3 * l3 + b4 * l4) / (b1 + b2 + b3 + b4);
  f32x3 col = f32x3(r, g, b) * 2.f;
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
