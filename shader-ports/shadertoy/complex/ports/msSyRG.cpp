// Port of https://www.shadertoy.com/view/msSyRG; author: oatmealine.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/msSyRG.json.
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
f32x2 smin(Context &__ctx, float a, float b, float k) {
  float h = max(k - abs(a - b), 0.0f) / k;
  float m = h * h * 0.5f;
  float s = m * k * (1.0f / 2.0f);
  return (a < b) ? f32x2(a - s, m) : f32x2(b - s, 1.0f - m);
}
f32x2 hash22(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(f32x3(p.swizzle<0, 1, 0>()) * f32x3(.1031f, .1030f, .0973f));
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.swizzle<0, 0>() + p3.swizzle<1, 2>()) * p3.swizzle<2, 1>());
}
f32x2 sineLayer(Context &__ctx, float i, float seed, float time) {
  f32x2 mag = hash22(__ctx, f32x2(i, seed)) * 0.5f;
  f32x2 speed = hash22(__ctx, f32x2(i, seed + 1.0f));
  f32x2 offset = hash22(__ctx, f32x2(i, seed + 2.0f));
  return f32x2(sin(iTime * speed.data[0] + offset.data[0] * 3.14159f),
               cos(iTime * speed.data[1] + offset.data[1] * 3.14159f)) *
         mag;
}
f32x2 goobabPos(Context &__ctx, f32x2 uv) {
  float m = 9999.9f;
  float angle = 0.0f;
  for (float i = 0.0f; i < 10.0f; i++) {
    float var = 0.1f;
    f32x2 pos = hash22(__ctx, f32x2(i, 0.0f)) * var + (0.5f - var * 0.5f);
    f32x2 p = pos + sineLayer(__ctx, i, 1.0f, iTime) +
              sineLayer(__ctx, i, 1.5f, iTime) * 0.75f +
              sineLayer(__ctx, i, 1.75f, iTime) * 0.5f +
              sineLayer(__ctx, i, 1.25f, iTime) * 0.5f;
    float dist = length(p - uv);
    f32x2 norm = normalize(p - uv);
    float dotp = dot(norm, normalize(f32x2(-1.0f, -1.0f)));
    float ang = acos(dotp) / 3.14159f;
    f32x2 minres = smin(__ctx, m, dist, 0.09f);
    m = minres.data[0];
    angle = mix(angle, ang, minres.data[1]);
  }
  return f32x2(m, angle);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  float pix = 80.0f;
  uv.data[0] -= 0.5f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  uv = floor(uv * pix) / pix;
  uv.data[0] += 0.5f;
  f32x2 uvP = uv;
  uv += f32x2(sin(iTime + uv.data[1] * 3.0f), sin(iTime + uv.data[0] * 2.0f)) *
        0.02f;
  f32x2 goo = goobabPos(__ctx, (uv - 0.5f) * 1.1f + 0.5f);
  float dist = goo.data[0];
  float angle = goo.data[1];
  float width = 1.f / pix * 0.5f;
  float outline = abs(dist - 0.11f) <= width ? 1.0f : 0.0f;
  float shine =
      max(1.0f - length(f32x2(0.0f, 0.5f) - f32x2(angle, dist / 0.11f)), 0.0f) >
              0.8f
          ? 1.0f
          : 0.0f;
  float shade = angle < 0.4f ? 1.0f : 0.0f;
  float ray = 0.0f;
  for (float i = 0.0f; i < 15.0f; i++) {
    f32x2 poses = hash22(__ctx, f32x2(i, 0.0f));
    f32x2 speedOffset = hash22(__ctx, f32x2(i, 1.0f));
    f32x2 scaleBri = hash22(__ctx, f32x2(i, 2.0f));
    float a = fract((iTime) / (10.0f + speedOffset.data[0] * 10.0f) +
                    speedOffset.data[1]);
    float mn = min(poses.data[0], poses.data[1]);
    float mx = max(poses.data[0], poses.data[1]);
    float x =
        mix(mn - 0.5f, mx + 0.5f, a) +
        sin(uv.data[1] * 10.0f + iTime * 0.2f + scaleBri.data[1] * 50.0f) *
            0.01f;
    float bri =
        (abs(uv.data[0] - (x - (1.0f - uv.data[1]) * 0.6f)) < 0.05f ? 1.0f
                                                                    : 0.0f) *
        (1.0f - (1.0f - uv.data[1]) * (1.2f + scaleBri.data[0] * 1.5f));
    ray += max(0.0f, bri) * (1.0f - abs(2.0f * a - 1.0f)) *
           (0.5f + scaleBri.data[1] * 0.5f);
  }
  f32x3 col = mix(f32x3(0.1f, 0.1f, 0.2f),
                  outline * f32x3(1.0f, 1.0f, 1.0f) +
                      (f32x3(0.2f, 0.6f, 0.2f) + shine * f32x3(1.0f) +
                       shade * f32x3(0.3f, 0.8f, 0.3f)) *
                          (dist < 0.11f ? 1.0f : 0.0f),
                  dist <= (0.11f + width) ? 1.0f : 0.0f) +
              ray * f32x3(0.4f, 0.6f, 1.0f);
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
