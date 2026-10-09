// Port of https://www.shadertoy.com/view/dllyzH; author: timmaffett.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dllyzH.json.
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
f32x3 hsb2rgb(Context &__ctx, f32x3 c) {
  f32x3 rgb = clamp(
      abs(mod(c.data[0] * 6.0f + f32x3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f,
      0.0f, 1.0f);
  rgb = rgb * rgb * (3.0f - 2.0f * rgb);
  return (c.data[2] * mix(f32x3(1.0f), rgb, c.data[1]));
}
float smin(Context &__ctx, float a, float b, float k) {
  float res = exp2(-k * a) + exp2(-k * b);
  return -log2(res) / k;
}
float sdCircle(Context &__ctx, f32x2 uv, f32x2 pos, float radius) {
  return length(uv - pos) - radius;
}
float sdLine(Context &__ctx, f32x2 uv, f32x2 start, f32x2 end) { return 0.0f; }
float randomSingle(Context &__ctx, f32x2 p) {
  p = fract(p * f32x2(233.34f, 851.73f));
  p += dot(p, p + 23.45f);
  return fract(p.data[0] * p.data[1]);
}
f32x4 randomPoint(Context &__ctx, f32x2 p) {
  float x = randomSingle(__ctx, p);
  float y = randomSingle(__ctx, f32x2(x, p.data[0]));
  return f32x4(x, y, randomSingle(__ctx, f32x2(y, x)),
               randomSingle(__ctx, f32x2(x, y)));
}
float Star(Context &__ctx, f32x2 uv, float dist, f32x2 id) {
  f32x4 rand = randomPoint(__ctx, id);
  float progress = fract(iTime * 0.15f + rand.data[2]);
  f32x2 dir = 2.0f * (normalize(rand.swizzle<0, 1>()) - 0.5f);
  rand.data[3] = clamp((rand.data[3] - 0.5f) * 999.0f, -1.0f, 1.0f);
  dir *= rand.data[3];
  return smin(__ctx, dist,
              sdCircle(__ctx, uv, dir * progress * 20.0f, 0.001f) /
                  (progress + 0.7f),
              200.0f);
}
float Graph(Context &__ctx, f32x2 uv, float r) {
  float dist = sdCircle(__ctx, uv, f32x2(0.0f, 0.0f), r);
  dist = Star(__ctx, uv, dist, f32x2(1.0f, 1.0f));
  for (int s = 1; s < 90; ++s)
    dist = Star(__ctx, uv, dist, f32x2(-1.0f, s));
  dist *= 60.0f;
  dist = max(dist, 0.0f);
  dist = 1.0f / (dist + 0.001f);
  dist *= clamp(0.8f, 0.98f, length(uv));
  return dist;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord * 2.0f - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  float angle = iTime * 0.4f;
  f32x2 center = iResolution.swizzle<0, 1>() / 2.0f;
  f32x2 toRing = normalize(fragCoord.swizzle<0, 1>() - center);
  float ringValue = atan(toRing.data[0], -toRing.data[1]);
  ringValue /= 3.14159265359f;
  ringValue = 1.f - ringValue;
  float ringAngle = ringValue / 2.0f;
  f32x3 hsv = hsb2rgb(__ctx, f32x3(ringAngle + angle, 0.85f, 0.96f));
  float scale = 1.0f / iResolution.data[1];
  float outerRadius = 0.99f;
  float innerRadius = 0.3f;
  f32x3 col = f32x3(0);
  f32x3 colB = f32x3(0.2f);
  float dis = length(uv);
  float bg = smoothstep(scale, -scale,
                        sdCircle(__ctx, uv, f32x2(0.0f, 0.0f), outerRadius));
  col = mix(f32x3(0.0f), colB, bg);
  float ring = smoothstep(outerRadius - 0.2f, outerRadius, dis);
  col *= ring;
  if (dis > outerRadius) {
    fragColor = f32x4(hsv, 1.f) * 0.1f;
    return;
  }
  if (dis < innerRadius / 2.9f) {
    fragColor =
        f32x4(hsb2rgb(__ctx, f32x3(ringAngle + angle, 0.85f, 0.96f)), 1.0f);
    return;
  }
  float sinA = sin(angle);
  float cosA = cos(angle);
  f32x2x2 rot = f32x2x2(cosA, -sinA, sinA, cosA);
  uv *= rot;
  uv *= 3.0f;
  float m = Graph(__ctx, uv, innerRadius);
  f32x3 tint = hsb2rgb(__ctx, f32x3(ringAngle + angle, 0.85f, 0.96f));
  col += m * mix(tint, tint * 0.8f, m);
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
