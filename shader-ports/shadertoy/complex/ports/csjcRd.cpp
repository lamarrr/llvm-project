// Port of https://www.shadertoy.com/view/csjcRd; author: Dragonpeak.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/csjcRd.json.
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
  TextureCube channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

const float EPS = .001f;
const float FAR = 40.f;
const int ITRS = 16;
const float FOCAL_LEN = .9f;
struct Ray {
  f32x3 origin;
  f32x3 dir;
};
Ray constructViewRay(Context &__ctx, f32x2 screen_pos, f32x3 pos,
                     f32x3 look_dir, f32x3 up, float len) {
  Ray r = Ray(pos, f32x3(0));
  f32x3 side = cross(up, look_dir);
  f32x3 cam_up = cross(look_dir, side);
  r.dir = normalize(side * screen_pos.data[0] + cam_up * screen_pos.data[1] +
                    look_dir * len);
  return r;
}
float smin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
const f32x3 degr2 = f32x3(0.7071f, -0.7071f, 0);
float map(Context &__ctx, f32x3 p, float &depth) {
  float im = iMouse.data[0] / iResolution.data[0] * 6.28f;
  float dist = FAR;
  float sc = 1.f;
  int id = 0;
  for (int i = 0; i < ITRS; i++) {
    float di = float(i) * .2f + sin(iTime - 3.f) * .1f + .05f;
    f32x3 plane = normalize(f32x3(sin(di), cos(di), cos(di)));
    p -= plane * min(dot(plane, p), 0.f) * 2.f;
    p -= degr2.swizzle<1, 1, 2>() * min(dot(degr2.swizzle<1, 1, 2>(), p), 0.f) *
         2.f;
    p -= degr2.swizzle<0, 2, 0>() * min(dot(degr2.swizzle<0, 2, 0>(), p), 0.f) *
         2.f;
    p -= f32x3(0, 0, 1) * min(dot(f32x3(0, 0, 1), p), 0.f) * 2.f;
    float it = sin(iTime * .1f) * .15f + .075f;
    p = f32x3x3(f32x3(cos(it), 0, sin(it)), f32x3(0, 1, 0),
                f32x3(-sin(it), 0, cos(it))) *
        p;
    float box = sdBox(__ctx, p, f32x3(1, 1, 4)) * sc;
    depth = (box < dist) ? depth : float(i);
    dist = smin(__ctx, dist, box, .2f);
    p.data[2] -= 2.2f;
    p.data[0] -= .3f;
    p.data[1] += .3f;
    p *= 1.1f;
    sc /= 1.1f;
  }
  return dist;
}
float march(Context &__ctx, Ray r, float &itrDepth) {
  float depth = .01f;
  itrDepth = 0.f;
  while (depth < FAR) {
    float dist = map(__ctx, r.origin + r.dir * depth, itrDepth);
    if (dist < EPS) {
      break;
    }
    depth += dist;
  }
  return depth;
}
f32x3 mapNormal(Context &__ctx, f32x3 pos) {
  float ignore = 0.f;
  f32x3 eps = f32x3(.001f, 0, 0);
  return normalize(
      f32x3(map(__ctx, pos + eps, ignore) - map(__ctx, pos - eps, ignore),
            map(__ctx, pos + eps.swizzle<1, 0, 1>(), ignore) -
                map(__ctx, pos - eps.swizzle<1, 0, 1>(), ignore),
            map(__ctx, pos + eps.swizzle<1, 1, 0>(), ignore) -
                map(__ctx, pos - eps.swizzle<1, 1, 0>(), ignore)));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>());
  f32x2 screenPos =
      (fragCoord.swizzle<0, 1>() / iResolution.data[0] * 2.f - 1.f) +
      f32x2(0, iResolution.data[1] / iResolution.data[0]) * .75f;
  f32x3 eye = f32x3(sin(iTime - 5.f), cos(iTime - 5.f), 0.f) * 25.f;
  Ray viewRay = constructViewRay(__ctx, screenPos, eye, normalize(-eye),
                                 f32x3(0, 0, 1), FOCAL_LEN);
  float itrDepth = 0.f;
  float dist = march(__ctx, viewRay, itrDepth);
  f32x3 col = texture(__ctx.channel0, viewRay.dir.swizzle<0, 2, 1>())
                  .swizzle<0, 1, 2>();
  if (dist < FAR) {
    f32x3 nrm = mapNormal(__ctx, viewRay.origin + viewRay.dir * dist);
    col = mix(f32x3(0.855f, 0.463f, 0.106f), f32x3(0.008f, 0.043f, 0.094f),
              float(itrDepth) / float(ITRS));
    col *= max(dot(nrm, normalize(f32x3(.4f, .4f, .6f))), 0.f) * f32x3(4.f) +
           texture(__ctx.channel0, nrm.swizzle<0, 2, 1>()).swizzle<0, 1, 2>();
  }
  fragColor =
      f32x4(pow(col * f32x3(0.867f, 0.765f, 0.675f), f32x3(1.f / 2.2f)), 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_cube<f32x4> channel0,
               __binding(1, 0) sampler sampling0) {
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
  Context __ctx{{channel0, sampling0}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
