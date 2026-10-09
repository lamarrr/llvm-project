// Port of https://www.shadertoy.com/view/DdGGDR; author: oemfoe.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DdGGDR.json.
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
struct Context {
  Texture2D channel0;
  Texture2D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
const float E = 0.001f;
const float PI = 3.14159265f;
float diameter = 1.0f;
float rotOffset = 0.0f;
float scorchedFactor = 1.0f;
f32x3x3 rotY(Context &__ctx, float r) {
  return f32x3x3(cos(r), 0.f, sin(r), 0.f, 1.f, 0.f, -sin(r), 0.f, cos(r));
}
f32x3x3 rotX(Context &__ctx, float r) {
  return f32x3x3(1.f, 0.f, 0.f, 0.f, cos(r), sin(r), 0.f, -sin(r), cos(r));
}
f32x3x3 rotZ(Context &__ctx, float r) {
  return f32x3x3(cos(r), sin(r), 0.f, -sin(r), cos(r), 0.f, 0.f, 0.f, 1.f);
}
f32x2 sphereUV(Context &__ctx, f32x3 intersectionPoint, float offset) {
  f32x2 uv = f32x2(atan(intersectionPoint.data[2], intersectionPoint.data[0]) /
                           (2.0f * PI) +
                       0.5f + offset,
                   acos(intersectionPoint.data[1]) / PI);
  return uv;
}
float sphere(Context &__ctx, f32x3 p, float radius) {
  return length(p) - radius;
}
float box(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
f32x2 scene_with_id(Context &__ctx, f32x3 p) {
  f32x3 q = p;
  q.data[1] /= diameter;
  f32x2 uvs = sphereUV(__ctx, q, (iTime * 4.f) / 64.000f + rotOffset + 0.000f);
  f32x4 tectonics = texture(__ctx.channel1, mod(uvs, 1.f));
  float dd = 0.0f;
  float bolleke = sphere(__ctx, p, diameter);
  if (bolleke < 0.1f && scorchedFactor > 0.1f)
    bolleke = sphere(__ctx, p,
                     diameter - smoothstep(0.500f, 1.000f, tectonics.data[0]) *
                                    0.05f * scorchedFactor);
  float background = sphere(__ctx, p, -100.0f);
  float universe = min(bolleke, background);
  return f32x2(universe, (bolleke < background ? 1.0f : 2.0f));
}
float scene(Context &__ctx, f32x3 p) { return scene_with_id(__ctx, p).data[0]; }
f32x3 normal(Context &__ctx, f32x3 p) {
  return normalize(
      f32x3(scene(__ctx, f32x3(p.data[0] + E, p.data[1], p.data[2])) -
                scene(__ctx, f32x3(p.data[0] - E, p.data[1], p.data[2])),
            scene(__ctx, f32x3(p.data[0], p.data[1] + E, p.data[2])) -
                scene(__ctx, f32x3(p.data[0], p.data[1] - E, p.data[2])),
            scene(__ctx, f32x3(p.data[0], p.data[1], p.data[2] + E)) -
                scene(__ctx, f32x3(p.data[0], p.data[1], p.data[2] - E))));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 uv2 = fragCoord / iResolution.swizzle<0, 1>();
  float aspect = iResolution.data[0] / iResolution.data[1];
  f32x3 col = f32x3(0.000f, 0.369f, 0.722f);
  uv -= 0.5f;
  uv.data[1] /= aspect;
  f32x3 lightColor = f32x3(0.985f, 0.946f, 0.628f);
  f32x3 ro = f32x3(0.0f, 0.0f, -5.f);
  f32x3 rd = normalize(f32x3(uv, 1.0f));
  float dayNight = -(iTime * 4.f) / 8.0f;
  f32x3 ld = f32x3(0.5f, 0.5f, -1.f);
  float t = 0.0f;
  float d = 0.0f;
  float id = 0.0f;
  f32x4 c = f32x4(0.000f, 0.067f, 1.000f, 1.f);
  bool hit = false;
  for (int i = 0; i < 128; i++) {
    f32x3 p = ro + t * rd;
    p = rotX(__ctx, 0.5f) * p;
    f32x2 result = scene_with_id(__ctx, p);
    d = result.data[0];
    id = result.data[1];
    if (abs(d) < E) {
      hit = true;
      f32x3 n = normal(__ctx, p);
      float l = smoothstep(0.0f, 1.0f, dot(n, ld));
      p.data[1] /= diameter;
      f32x2 uvs = sphereUV(__ctx, p, (iTime * 4.f) / 64.f + rotOffset);
      f32x2 uvsClouds = sphereUV(__ctx, p, 0.f);
      if (id == 1.0f) {
        c = texture(__ctx.channel0, mod(uvs, 1.f)) * l;
        f32x4 tectonics = texture(__ctx.channel1, mod(uvs, 1.f));
        f32x3 scorched = c.swizzle<0, 1, 2>();
        f32x3 scorchedColor = f32x3(0.984f, 0.486f, 0.075f);
        scorched.set_swizzle<0, 1, 2>(
            mix(c.swizzle<0, 1, 2>(), scorchedColor,
                l * smoothstep(0.669f, 0.875f, tectonics.data[0])));
        c.set_swizzle<0, 1, 2>(
            mix(c.swizzle<0, 1, 2>(), scorched, scorchedFactor));
      }
      break;
    }
    t += d * 0.8f;
  }
  fragColor = c;
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0,
               __binding__(0, 1) texture_2d<f32x4> channel1,
               __binding__(1, 1) sampler sampling1) {
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
  Context __ctx{{channel0, sampling0}, {channel1, sampling1}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
