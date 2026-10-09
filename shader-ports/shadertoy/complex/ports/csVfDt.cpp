// Port of https://www.shadertoy.com/view/csVfDt; author: ElSolem.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/csVfDt.json.
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

float gTime = 0.0f;
const float REPEAT = 5.0f;
f32x2x2 rot(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, s, -s, c);
}
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
float box(Context &__ctx, f32x3 pos, float scale) {
  pos *= scale;
  float base = sdBox(__ctx, pos, f32x3(0.4f, 0.4f, 0.1f)) / 1.5f;
  pos.set_swizzle<0, 1>(pos.swizzle<0, 1>() * (5.0f));
  pos.data[1] -= 3.5f;
  pos.set_swizzle<0, 1>(pos.swizzle<0, 1>() * (rot(__ctx, 0.75f)));
  float result = -base;
  return result;
}
float box_set(Context &__ctx, f32x3 pos, float iTime) {
  f32x3 pos_origin = pos;
  pos = pos_origin;
  pos.data[1] += sin(gTime * 0.4f) * 2.5f;
  pos.set_swizzle<0, 1>(pos.swizzle<0, 1>() * (rot(__ctx, 0.8f)));
  float box1 = box(__ctx, pos, 2.0f - abs(sin(gTime * 0.4f)) * 1.5f);
  pos = pos_origin;
  pos.data[1] -= sin(gTime * 0.4f) * 2.5f;
  pos.set_swizzle<0, 1>(pos.swizzle<0, 1>() * (rot(__ctx, 0.8f)));
  float box2 = box(__ctx, pos, 2.0f - abs(sin(gTime * 0.4f)) * 1.5f);
  pos = pos_origin;
  pos.data[0] += sin(gTime * 0.4f) * 2.5f;
  pos.set_swizzle<0, 1>(pos.swizzle<0, 1>() * (rot(__ctx, 0.8f)));
  float box3 = box(__ctx, pos, 2.0f - abs(sin(gTime * 0.4f)) * 1.5f);
  pos = pos_origin;
  pos.data[0] -= sin(gTime * 0.4f) * 2.5f;
  pos.set_swizzle<0, 1>(pos.swizzle<0, 1>() * (rot(__ctx, 0.8f)));
  float box4 = box(__ctx, pos, 2.0f - abs(sin(gTime * 0.4f)) * 1.5f);
  pos = pos_origin;
  pos.set_swizzle<0, 1>(pos.swizzle<0, 1>() * (rot(__ctx, 0.8f)));
  float box5 = box(__ctx, pos, 0.5f) * 6.0f;
  pos = pos_origin;
  float box6 = box(__ctx, pos, 0.5f) * 6.0f;
  float result = max(max(max(max(max(box1, box2), box3), box4), box5), box6);
  return result;
}
float map(Context &__ctx, f32x3 pos, float iTime) {
  f32x3 pos_origin = pos;
  float box_set1 = box_set(__ctx, pos, iTime);
  return box_set1;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p = (2.0f * fragCoord - iResolution.swizzle<0, 1>()) /
            min(iResolution.data[1], iResolution.data[0]);
  float xy = p.data[0] / p.data[1];
  float threshold = 2.0f;
  if (abs(xy) < abs(threshold)) {
    f32x2 p = (fragCoord.swizzle<0, 1>() * 2.0f - iResolution.swizzle<0, 1>()) /
              min(iResolution.data[0], iResolution.data[1]);
    f32x3 ro = f32x3(0.0f, -0.2f, iTime * 4.0f);
    f32x3 ray = normalize(f32x3(p, 1.5f));
    ray.set_swizzle<0, 1>(ray.swizzle<0, 1>() *
                          rot(__ctx, sin(iTime * 0.03f) * 5.0f));
    ray.set_swizzle<1, 2>(ray.swizzle<1, 2>() *
                          rot(__ctx, sin(iTime * 0.05f) * 0.2f));
    float t = 0.1f;
    f32x3 col = f32x3(0.0f);
    float ac = 0.0f;
    for (int i = 0; i < 99; i++) {
      f32x3 pos = ro + ray * t;
      pos = mod(pos - 2.0f, 4.0f) - 2.0f;
      gTime = iTime - float(i) * 0.01f;
      float d = map(__ctx, pos, iTime);
      d = max(abs(d), 0.01f);
      ac += exp(-d * 23.0f);
      t += d * 0.55f;
    }
    col = f32x3(ac * 0.02f);
    col += f32x3(0.0f, 0.2f * abs(sin(iTime)), 0.5f + sin(iTime) * 0.2f);
    fragColor = f32x4(col, 1.0f - t * (0.02f + 0.02f * sin(iTime)));
  } else {
    f32x2 p = (fragCoord.swizzle<0, 1>() * 2.0f - iResolution.swizzle<0, 1>()) /
              min(iResolution.data[0], iResolution.data[1]);
    f32x3 ro = f32x3(0.0f, -0.2f, iTime * 4.0f);
    f32x3 ray = normalize(f32x3(p, 1.5f));
    ray.set_swizzle<0, 1>(ray.swizzle<0, 1>() *
                          rot(__ctx, sin(iTime * 0.03f) * 5.0f));
    ray.set_swizzle<1, 2>(ray.swizzle<1, 2>() *
                          rot(__ctx, sin(iTime * 0.05f) * 0.2f));
    float t = 0.1f;
    f32x3 col = f32x3(0.0f);
    float ac = 0.0f;
    for (int i = 0; i < 99; i++) {
      f32x3 pos = ro + ray * t;
      pos = mod(pos - 2.0f, 4.0f) - 2.0f;
      gTime = iTime - float(i) * 0.01f;
      float d = map(__ctx, pos, iTime);
      d = max(abs(d), 0.01f);
      ac += exp(-d * 23.0f);
      t += d * 0.55f;
    }
    col = f32x3(ac * 0.02f);
    col += f32x3(0.0f, 0.2f * abs(sin(iTime)), 0.5f + sin(iTime) * 0.2f);
    fragColor = f32x4(col, 1.0f - t * (0.02f + 0.02f * sin(iTime)));
  }
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
