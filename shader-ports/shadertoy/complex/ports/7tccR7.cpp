// Port of https://www.shadertoy.com/view/7tccR7; author: nsbalbi.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7tccR7.json.
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
const float D_MAX = 30.0f;
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
float sceneSDFnoRep(Context &__ctx, f32x3 samplePoint) {
  return sdBox(__ctx, samplePoint, f32x3(0.1f));
}
float opRep(Context &__ctx, f32x3 p, f32x3 c) {
  f32x3 q = mod(p + 0.5f * c, c) - 0.5f * c;
  return sceneSDFnoRep(__ctx, q);
}
float sceneSDF(Context &__ctx, f32x3 samplePoint) {
  return opRep(__ctx, samplePoint, f32x3(1.0f));
}
f32x3 calcNormal(Context &__ctx, f32x3 pos) {
  f32x2 e = f32x2(1.0f, -1.0f) * 0.5773f;
  const float eps = 0.0005f;
  return normalize(
      e.swizzle<0, 1, 1>() * sceneSDF(__ctx, pos + e.swizzle<0, 1, 1>() * eps) +
      e.swizzle<1, 1, 0>() * sceneSDF(__ctx, pos + e.swizzle<1, 1, 0>() * eps) +
      e.swizzle<1, 0, 1>() * sceneSDF(__ctx, pos + e.swizzle<1, 0, 1>() * eps) +
      e.swizzle<0, 0, 0>() * sceneSDF(__ctx, pos + e.swizzle<0, 0, 0>() * eps));
}
f32x3 applyFog(Context &__ctx, f32x3 rgb, float dist, float distFactor) {
  float fogAmount = 1.0f - exp(-dist * distFactor);
  f32x3 fogColor = f32x3(0.0f, 0.0f, 0.0f);
  return mix(rgb, fogColor, fogAmount);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 eye = f32x3(1.0f - iTime, 0.4f, -1.0f + iTime);
  f32x3 center = f32x3(0.0f - iTime, 0.0f, 0.0f + iTime);
  f32x3 ww = normalize(center - eye);
  f32x3 uu = normalize(cross(ww, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 vv = normalize(cross(uu, ww));
  f32x3 tot = f32x3(0.0f);
  for (int m = 0; m < 2; m++)
    for (int n = 0; n < 2; n++) {
      f32x2 offset = f32x2(float(m), float(n)) / float(2) - 0.5f;
      f32x2 p = (-iResolution.swizzle<0, 1>() +
                 2.0f * (fragCoord.swizzle<0, 1>() + offset)) /
                iResolution.data[1];
      f32x3 ray = normalize(p.data[0] * uu + p.data[1] * vv + 1.5f * ww);
      float dTot = 0.0f;
      for (int i = 0; i < 256; i++) {
        f32x3 pos = eye + dTot * ray;
        float d = sceneSDF(__ctx, pos);
        if (d < 0.0001f || dTot > D_MAX)
          break;
        dTot += d;
      }
      f32x3 color = f32x3(0.0f);
      if (dTot < D_MAX) {
        f32x3 pos = eye + dTot * ray;
        f32x3 normal = calcNormal(__ctx, pos);
        float diffuse = clamp(dot(normal, f32x3(0.5f)), 0.0f, 1.0f);
        float ambient = 0.5f + 0.5f * dot(normal, f32x3(0.0f, 1.0f, 0.0f));
        color = f32x3(0.5882f, 0.302f, 0.302f) * ambient +
                f32x3(0.098f, 0.1843f, 0.5882f) * diffuse;
        color = applyFog(__ctx, color, dTot, 0.3f);
      }
      color = sqrt(color);
      tot += color;
    }
  tot /= float(2 * 2);
  fragColor = f32x4(tot, 1.0f);
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
