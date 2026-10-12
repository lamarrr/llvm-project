// Port of https://www.shadertoy.com/view/mlcyDn; author: mds2.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mlcyDn.json.
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
float box_length(Context &__ctx, f32x3 pt) {
  f32x3 p = abs(pt);
  return max(p.data[0], max(p.data[1], p.data[2]));
}
float single_item_sdf(Context &__ctx, f32x3 pt) { return length(pt) - 1.0f; }
float gridded_sdf(Context &__ctx, f32x3 pt) {
  f32x3 new_pt = mod(20.0f * pt + f32x3(4.0f), f32x3(8.0f)) - f32x3(4.0f);
  if (pt.data[2] > 0.6f || pt.data[2] < -0.8f) {
    new_pt.data[2] = 20.0f * pt.data[2];
  }
  return 0.05f * single_item_sdf(__ctx, new_pt);
}
float sdf(Context &__ctx, f32x3 pt) {
  float height = pt.data[1] + 0.5f;
  float base = length(pt.swizzle<0, 2>() + f32x2(0.0f, 1.0f)) - 1.25f;
  float d = length(f32x2(height, base)) - 0.375f;
  float theta = atan(pt.data[2] + 1.0f, pt.data[0]);
  float phi = atan(height, base);
  const float scale = 2.0f;
  f32x3 xformed =
      scale * f32x3(0.25f * 3.2f * phi / 3.141592654f + 0.25f * iTime,
                    0.5f * iTime + 3.2f * theta / 3.141592654f, d);
  return gridded_sdf(__ctx, xformed) / scale;
}
f32x3 normed_grad(Context &__ctx, f32x3 pt) {
  float v = sdf(__ctx, pt);
  return normalize(f32x3(sdf(__ctx, pt + f32x3(0.01f, 0.0f, 0.0f)) - v,
                         sdf(__ctx, pt + f32x3(0.0f, 0.01f, 0.0f)) - v,
                         sdf(__ctx, pt + f32x3(0.0f, 0.0f, 0.01f)) - v));
}
float raycast(Context &__ctx, f32x3 dir, f32x3 orig, float &accum) {
  accum = 0.0f;
  float along = 0.0f;
  float last_dist = sdf(__ctx, orig);
  for (int i = 0; i < 100; ++i) {
    float stepsize = 0.4f * last_dist;
    float next_dist = sdf(__ctx, orig + (along + stepsize) * dir);
    accum += stepsize * 0.5f *
             (1.0f / max(last_dist * last_dist, 0.001f) +
              1.0f / max(next_dist * next_dist, 00.1f));
    last_dist = next_dist;
    along += stepsize;
    if (abs(last_dist) < 1.0e-3f) {
      return along;
    }
    if (last_dist > 5.0f) {
      return 5.0f + 1.0f;
    }
  }
  return along;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (2.0f * fragCoord - iResolution.swizzle<0, 1>()) /
             min(iResolution.data[0], iResolution.data[1]);
  f32x3 ray = normalize(f32x3(uv, -1.0f));
  f32x3 orig = f32x3(0.0f, 0.0f, 1.0f);
  float pitch = -1.0f * iMouse.data[1] / iResolution.data[1];
  float cpitch = cos(pitch);
  float spitch = sin(pitch);
  f32x3x3 orient =
      f32x3x3(1.0f, 0.0f, 0.0f, 0.0f, cpitch, spitch, 0.0f, -spitch, cpitch);
  orig = orient * (orig + f32x3(0.0f, 0.5f, 1.0f)) - f32x3(0.0f, 0.5f, 1.0f);
  ray = orient * ray;
  float accum;
  float d = raycast(__ctx, ray, orig, accum);
  f32x3 col = 0.00625f * accum * f32x3(0.5f, 0.1f, 1.0f);
  if (d < 5.0f) {
    f32x3 surfNorm = normed_grad(__ctx, orig + d * ray);
    f32x3 bounce = normalize(reflect(ray, surfNorm));
    float raw_shine =
        max(0.0f, dot(bounce, normalize(f32x3(0.5f, 1.0f, 1.0f))));
    raw_shine = raw_shine * raw_shine;
    raw_shine = raw_shine * raw_shine;
    col.data[1] += raw_shine * raw_shine * raw_shine;
    col += 0.4f * max(f32x3(0.5f),
                      f32x3(1.5f, 1.0f, 0.5f) *
                          texture(__ctx.channel0, bounce).swizzle<0, 1, 2>());
  }
  fragColor = f32x4(col, 1.0f);
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
