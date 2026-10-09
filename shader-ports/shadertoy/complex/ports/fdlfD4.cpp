// Port of https://www.shadertoy.com/view/fdlfD4; author: nyri0.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fdlfD4.json.
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

const int FSAA = 4;
const float GRID_DIM = 1.0f;
const float R = 0.8f;
float jTime;
float sq(Context &__ctx, float x) { return x * x; }
f32x3 hash33(Context &__ctx, f32x3 p3) {
  p3 = fract(p3 * f32x3(.1031f, .11369f, .13787f));
  p3 += dot(p3, p3.swizzle<1, 0, 2>() + 19.19f);
  return -1.0f + 2.0f * fract(f32x3((p3.data[0] + p3.data[1]) * p3.data[2],
                                    (p3.data[0] + p3.data[2]) * p3.data[1],
                                    (p3.data[1] + p3.data[2]) * p3.data[0]));
}
float simplexNoise(Context &__ctx, f32x3 p) {
  const float K1 = 0.333333333f;
  const float K2 = 0.166666667f;
  f32x3 i = floor(p + (p.data[0] + p.data[1] + p.data[2]) * K1);
  f32x3 d0 = p - (i - (i.data[0] + i.data[1] + i.data[2]) * K2);
  f32x3 e = step(f32x3(0.0f), d0 - d0.swizzle<1, 2, 0>());
  f32x3 i1 = e * (1.0f - e.swizzle<2, 0, 1>());
  f32x3 i2 = 1.0f - e.swizzle<2, 0, 1>() * (1.0f - e);
  f32x3 d1 = d0 - (i1 - 1.0f * K2);
  f32x3 d2 = d0 - (i2 - 2.0f * K2);
  f32x3 d3 = d0 - (1.0f - 3.0f * K2);
  f32x4 h = max(
      0.6f - f32x4(dot(d0, d0), dot(d1, d1), dot(d2, d2), dot(d3, d3)), 0.0f);
  f32x4 n =
      h * h * h * h *
      f32x4(dot(d0, hash33(__ctx, i)), dot(d1, hash33(__ctx, i + i1)),
            dot(d2, hash33(__ctx, i + i2)), dot(d3, hash33(__ctx, i + 1.0f)));
  return dot(f32x4(31.316f), n);
}
float length_sq(Context &__ctx, f32x2 x) { return dot(x, x); }
float segment_df(Context &__ctx, f32x2 uv, f32x2 p0, f32x2 p1) {
  float l2 = length_sq(__ctx, p1 - p0);
  float t = clamp(dot(uv - p0, p1 - p0) / l2, 0.0f, 1.0f);
  f32x2 projection = p0 + t * (p1 - p0);
  return distance(uv, projection);
}
float semi_circle_sdf(Context &__ctx, f32x2 uv, f32x2 c, float r) {
  return max((length(uv - c) - r), -(uv.data[1] - c.data[1]));
}
float frame_sdf(Context &__ctx, f32x2 uv, f32x2 pf) {
  const float fm = 5.0f;
  const f32x2 A = f32x2(-R, 0.0f);
  const f32x2 B = f32x2(R, 0.0f);
  float sdf = min(segment_df(__ctx, uv, A, pf), segment_df(__ctx, uv, B, pf)) *
              mix(1.f, fm, smoothstep(0.f, pf.data[1], uv.data[1]));
  const int n_steps = 15;
  for (int i = 0; i < n_steps; i++) {
    float t = 1.f - exp(-0.3f * float(i));
    float fmi = mix(1.f, fm, t);
    sdf = min(sdf, abs(semi_circle_sdf(__ctx, uv, mix(f32x2(0, 0), pf, t),
                                       R * (1.f - t))) *
                       fmi);
  }
  return sdf;
}
f32x4 sampleColor(Context &__ctx, f32x2 sampleCoord) {
  float aspectRatio = iResolution.data[0] / iResolution.data[1];
  f32x2 uvf = 2.0f * (sampleCoord - iResolution.swizzle<0, 1>() / 2.0f) /
              iResolution.data[1];
  f32x2 pf = f32x2(0.55f * cos(0.7f * iTime), 0.4f);
  f32x2 pfl = pf + f32x2(0, -0.1f);
  f32x3 col;
  float my_noise = simplexNoise(__ctx, f32x3(500.0f * uvf, iTime));
  f32x3 bg_col = mix(f32x3(0, 0, 0), f32x3(0.9f, 0.3f, 1),
                     smoothstep(0.2f, 1.0f, my_noise));
  col = bg_col;
  if (uvf.data[1] < 0.0f)
    col = mix(f32x3(0, 0, 0), f32x3(0.9f, 0.3f, 1.f),
              0.1f + 0.9f * smoothstep(0.f, 1.f, my_noise));
  const float fm = 1.7f;
  const f32x2 po0 = f32x2(-0.6f * R, 0.f);
  const f32x2 po1 = f32x2(-0.1f * R, 0.f);
  f32x2 po0a = pfl + 0.5f * (po0 - pf);
  f32x2 po1a = pfl + 0.2f * (po1 - pf);
  f32x2 po0b = pfl + 3.f * (po0 - pf);
  f32x2 po1b = pfl + 3.f * (po1 - pf);
  f32x3 outer_col_orange =
      uvf.data[1] > 0.f ? f32x3(1, 0.35f, 0.f) : f32x3(1, .1f, 0);
  f32x3 inner_col_orange = f32x3(.95f, 1, .8f);
  float df_orange = min(segment_df(__ctx, uvf, po0b, po0a),
                        segment_df(__ctx, uvf, po1b, po1a)) *
                    mix(1.0f, fm, smoothstep(-R, pf.data[1], uvf.data[1]));
  col = mix(outer_col_orange, col,
            smoothstep(0.1f, 0.2f, df_orange + 0.1f * my_noise));
  col = mix(col, inner_col_orange,
            step(smoothstep(0.f, 0.35f, df_orange), my_noise));
  const f32x2 pp0 = f32x2(0.4f * R, 0.f);
  const f32x2 pp1 = f32x2(0.75f * R, 0.f);
  f32x2 pp0b = pfl + 3.f * (pp0 - pf);
  f32x2 pp1b = pfl + 3.f * (pp1 - pf);
  f32x3 outer_col_blue =
      uvf.data[1] > 0.f ? f32x3(0.5f, .8f, .9f) : f32x3(0.3f, 0.3f, 1);
  f32x3 inner_col_blue = f32x3(0.95f, 1, 1);
  float df_purple = min(segment_df(__ctx, uvf, pp0b, pfl),
                        segment_df(__ctx, uvf, pp1b, pfl)) *
                    mix(1.0f, fm, smoothstep(-R, pf.data[1], uvf.data[1]));
  col = mix(outer_col_blue, col,
            smoothstep(0.1f, 0.2f, df_purple + 0.1f * my_noise));
  col = mix(col, inner_col_blue,
            step(smoothstep(0.f, 0.35f, df_purple), my_noise));
  if (length(uvf) > R)
    col = bg_col;
  col = mix(f32x3(1, 1, 1), col,
            smoothstep(0.f, 0.007f, frame_sdf(__ctx, uvf, pf)));
  return f32x4(col, 1.0f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x4 colSum = f32x4(0);
  for (int i = 0; i < FSAA; i++) {
    for (int j = 0; j < FSAA; j++) {
      colSum += sampleColor(__ctx, fragCoord + f32x2(float(i) / float(FSAA),
                                                     float(j) / float(FSAA)));
    }
  }
  fragColor = colSum / colSum.data[3];
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
