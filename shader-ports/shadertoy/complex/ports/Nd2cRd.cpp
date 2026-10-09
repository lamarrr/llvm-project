// Port of https://www.shadertoy.com/view/Nd2cRd; author: ErinZ.
// Dataset license metadata: mit; original retained in upstream/Nd2cRd.json.
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
float psrdnoise(Context &__ctx, f32x2 x, f32x2 period, float alpha,
                f32x2 &gradient) {
  f32x2 uv = f32x2(x.data[0] + x.data[1] * 0.5f, x.data[1]);
  f32x2 i0 = floor(uv), f0 = fract(uv);
  float cmp = step(f0.data[1], f0.data[0]);
  f32x2 o1 = f32x2(cmp, 1.0f - cmp);
  f32x2 i1 = i0 + o1, i2 = i0 + 1.0f;
  f32x2 v0 = f32x2(i0.data[0] - i0.data[1] * 0.5f, i0.data[1]);
  f32x2 v1 = f32x2(v0.data[0] + o1.data[0] - o1.data[1] * 0.5f,
                   v0.data[1] + o1.data[1]);
  f32x2 v2 = f32x2(v0.data[0] + 0.5f, v0.data[1] + 1.0f);
  f32x2 x0 = x - v0, x1 = x - v1, x2 = x - v2;
  f32x3 iu, iv, xw, yw;
  if (any(greaterThan(period, f32x2(0.0f)))) {
    xw = f32x3(v0.data[0], v1.data[0], v2.data[0]);
    yw = f32x3(v0.data[1], v1.data[1], v2.data[1]);
    if (period.data[0] > 0.0f)
      xw = mod(f32x3(v0.data[0], v1.data[0], v2.data[0]), period.data[0]);
    if (period.data[1] > 0.0f)
      yw = mod(f32x3(v0.data[1], v1.data[1], v2.data[1]), period.data[1]);
    iu = floor(xw + 0.5f * yw + 0.5f);
    iv = floor(yw + 0.5f);
  } else {
    iu = f32x3(i0.data[0], i1.data[0], i2.data[0]);
    iv = f32x3(i0.data[1], i1.data[1], i2.data[1]);
  }
  f32x3 hash = mod(iu, 289.0f);
  hash = mod((hash * 51.0f + 2.0f) * hash + iv, 289.0f);
  hash = mod((hash * 34.0f + 10.0f) * hash, 289.0f);
  f32x3 psi = hash * 0.07482f + alpha;
  f32x3 gx = cos(psi);
  f32x3 gy = sin(psi);
  f32x2 g0 = f32x2(gx.data[0], gy.data[0]);
  f32x2 g1 = f32x2(gx.data[1], gy.data[1]);
  f32x2 g2 = f32x2(gx.data[2], gy.data[2]);
  f32x3 w = 0.8f - f32x3(dot(x0, x0), dot(x1, x1), dot(x2, x2));
  w = max(w, 0.0f);
  f32x3 w2 = w * w;
  f32x3 w4 = w2 * w2;
  f32x3 gdotx = f32x3(dot(g0, x0), dot(g1, x1), dot(g2, x2));
  float n = dot(w4, gdotx);
  f32x3 w3 = w2 * w;
  f32x3 dw = -8.0f * w3 * gdotx;
  f32x2 dn0 = w4.data[0] * g0 + dw.data[0] * x0;
  f32x2 dn1 = w4.data[1] * g1 + dw.data[1] * x1;
  f32x2 dn2 = w4.data[2] * g2 + dw.data[2] * x2;
  gradient = 10.9f * (dn0 + dn1 + dn2);
  return 10.9f * n;
}
float fbm(Context &__ctx, f32x2 x, float n, f32x2 &g) {
  f32x2 p = f32x2(0.f);
  float alpha = iTime * .05f;
  float scale = .5f;
  float noise = 0.f;
  for (float i = 0.f; i < n; i++) {
    noise += psrdnoise(__ctx, x, p, alpha, g) * scale;
    x *= 2.f;
    scale /= 2.f;
    alpha *= 1.3f;
  }
  return noise;
}
f32x3 pattern2(Context &__ctx, f32x2 uv) {
  float alpha = iTime * 0.1f;
  f32x2 g;
  f32x2 p = f32x2(6);
  f32x3 col =
      mix(f32x3(0.f, 0.35f, 1.f), f32x3(1.f, .15f, .1f),
          psrdnoise(__ctx, uv * .5f + f32x2(.23f, .67f), p, alpha, g) * 0.5f +
              0.5f);
  col += f32x3(0.f, .6f, 0.2f) *
         (psrdnoise(__ctx, uv * .4f + f32x2(.092f, .137f), p, alpha + 2.f, g) *
              0.5f +
          .2f);
  col *= clamp(length(g) * .3f, 0.f, .6f) + .4f;
  float n = fbm(__ctx, uv * .5f, 2.f, g);
  n = clamp(n * .5f + .5f, .2f, 1.f);
  n = fract(n * 10.f);
  float sf = iResolution.data[1] / 300.f / max(length(g), .001f);
  n = min(n, n * (1.f - n) * 48.f * sf);
  col *= n;
  col.data[0] = pow(col.data[0], 0.7f);
  return col;
}
float logo(Context &__ctx, f32x2 uv);
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (2.f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 uv1 = uv * 1.5f;
  const f32x2 p = f32x2(8.0f, 8.0f);
  float alpha = iTime * .1f;
  f32x2 g;
  float n = psrdnoise(__ctx, uv1, p, alpha, g);
  f32x3 col;
  col = sqrt(f32x3(n * 0.5f + 0.5f));
  col = pattern2(__ctx, uv) * 0.9f;
  col = mix(col,
            f32x3(0, .9f, .6f) *
                pattern2(__ctx, uv + f32x2(0.876f, 0.1873f)).data[1] * n,
            n - .2f);
  col += logo(__ctx, uv);
  fragColor = f32x4(col, 1.0f);
}
float logo(Context &__ctx, f32x2 uv) {
  float n = 0.f;
  n += smoothstep(abs(length(uv - f32x2(-1.23f, 0)) - .12f),
                  abs(length(uv - f32x2(-1.23f, 0)) - .12f) +
                      3.f / iResolution.data[1],
                  .05f);
  n *= 1.f -
       smoothstep(uv.data[0], uv.data[0] + 3.f / iResolution.data[1], -1.25f);
  n += smoothstep(abs(uv.data[1]), abs(uv.data[1]) + 3.f / iResolution.data[1],
                  .17f) *
       smoothstep(abs(uv.data[0] + 1.29f),
                  abs(uv.data[0] + 1.29f) + 3.f / iResolution.data[1], .05f);
  n += smoothstep(abs(length(uv - f32x2(-1.5f, 0)) - .12f),
                  abs(length(uv - f32x2(-1.5f, 0)) - .12f) +
                      3.f / iResolution.data[1],
                  .05f);
  n += smoothstep(abs(length(uv - f32x2(-0.9f, 0)) - .12f),
                  abs(length(uv - f32x2(-0.9f, 0)) - .12f) +
                      3.f / iResolution.data[1],
                  .05f);
  n += (1.f +
        smoothstep(uv.data[0], uv.data[0] + 3.f / iResolution.data[1], -.6f) -
        smoothstep(uv.data[1], uv.data[1] + 3.f / iResolution.data[1], .05f)) *
       smoothstep(abs(length(uv - f32x2(-0.6f, .03f)) - .09f),
                  abs(length(uv - f32x2(-0.6f, .03f)) - .09f) +
                      3.f / iResolution.data[1],
                  .05f);
  n += (1.f -
        smoothstep(uv.data[0], uv.data[0] + 3.f / iResolution.data[1], -.6f) +
        smoothstep(uv.data[1], uv.data[1] + 3.f / iResolution.data[1], -.05f)) *
       smoothstep(abs(length(uv - f32x2(-0.6f, -.04f)) - .09f),
                  abs(length(uv - f32x2(-0.6f, -.04f)) - .09f) +
                      3.f / iResolution.data[1],
                  .05f);
  return n;
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
