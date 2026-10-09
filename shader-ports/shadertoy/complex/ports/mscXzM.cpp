// Port of https://www.shadertoy.com/view/mscXzM; author: tianluo97.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mscXzM.json.
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

const f32x2 target_resolution = f32x2(940, 86);
f32x2 random(Context &__ctx, f32x2 uv) {
  uv = f32x2(dot(uv, f32x2(127.1f, 311.7f)), dot(uv, f32x2(269.5f, 183.3f)));
  return -1.0f + 2.0f * fract(sin(uv) * 43758.5453123f);
}
float noise(Context &__ctx, f32x2 uv) {
  f32x2 uv_index = floor(uv);
  f32x2 uv_fract = fract(uv);
  f32x2 blur = smoothstep(0.0f, 1.f, uv_fract);
  return mix(mix(dot(random(__ctx, uv_index + f32x2(0.0f, 0.0f)),
                     uv_fract - f32x2(0.0f, 0.0f)),
                 dot(random(__ctx, uv_index + f32x2(1.0f, 0.0f)),
                     uv_fract - f32x2(1.0f, 0.0f)),
                 blur.data[0]),
             mix(dot(random(__ctx, uv_index + f32x2(0.0f, 1.0f)),
                     uv_fract - f32x2(0.0f, 1.0f)),
                 dot(random(__ctx, uv_index + f32x2(1.0f, 1.0f)),
                     uv_fract - f32x2(1.0f, 1.0f)),
                 blur.data[0]),
             blur.data[1]) +
         0.5f;
}
float remap(Context &__ctx, float x, float t1, float t2, float s1, float s2) {
  return (x - t1) / (t2 - t1) * (s2 - s1) + s1;
}
float fbm(Context &__ctx, f32x2 st) {
  float value = 0.0f;
  float amplitude = .5f;
  float frequency = 0.f;
  for (int i = 0; i < 3; i++) {
    value += amplitude * noise(__ctx, st);
    st *= 1.5f;
    amplitude *= .5f;
  }
  return value;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 texCoord = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 uv = texCoord;
  uv -= .5f;
  float r = (iResolution.data[0] / iResolution.data[1]) /
            (target_resolution.data[0] / target_resolution.data[1]);
  if (r >= 1.f)
    uv.data[0] *= r;
  else
    uv.data[1] /= r;
  uv += .5f;
  uv.data[1] -= 0.5f / r - 0.5f;
  f32x2 p = uv;
  p.data[0] *= 7.f;
  p.data[1] *= 20.0f;
  p.data[0] += iTime * .8f;
  p.data[1] += iTime * 0.4f;
  float n = noise(__ctx, p);
  n = pow(n, 1.f);
  f32x3 col = f32x3(n, n, n);
  f32x3 colLight = col * f32x3(0.659f, 1.000f, 0.843f);
  col *= f32x3(0.102f, 1.000f, 0.580f);
  f32x2 p0 = uv;
  p0.data[0] *= 5.2f;
  p0.data[1] *= 7.0f;
  p0.data[0] -= iTime * 0.4f;
  p0.data[1] += iTime * 0.01f;
  p0.data[1] -= 2.5f;
  float n0 = fbm(__ctx, p0);
  n0 = pow(n0, 1.1f);
  f32x3 col0 = f32x3(n0, n0, n0);
  col0 *= f32x3(0.000f, 0.482f, 1.000f);
  f32x2 p1 = uv;
  p1.data[0] *= 1.2f;
  p1.data[1] *= 2.0f;
  p1.data[0] -= iTime * 0.4f;
  p1.data[1] += iTime * 0.01f;
  p1.data[1] -= 2.5f;
  float n1 = fbm(__ctx, p1);
  n1 = pow(n1, 1.1f);
  f32x3 col1 = f32x3(n1, n1, n1);
  col1 *= f32x3(0.000f, 0.678f, 0.569f);
  f32x3 color = col0 * .8f + col * 1.3f + colLight * 0.3f + col1 * 0.f;
  fragColor = f32x4(color, 1.0f);
  if (uv.data[0] < 0.f || uv.data[0] > 1.f || uv.data[1] < 0.f ||
      uv.data[1] > 1.f)
    fragColor = f32x4(0);
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
