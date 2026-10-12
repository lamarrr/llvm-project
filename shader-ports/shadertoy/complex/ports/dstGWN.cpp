// Port of https://www.shadertoy.com/view/dstGWN; author: been_jamming.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dstGWN.json.
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
f32x2 points[3];
float min_res;
void initialize_points(Context &__ctx, float interp) {
  f32x2 center = f32x2(0.0f);
  for (int i = 0; i < 3; i++) {
    points[i].data[0] =
        (length(iResolution) / min_res + 2.0f * 0.05f) *
        cos(2.0f * (float(i) + 1.0f / 2.0f) * 3.1415927f / 3.0f +
            mod(iTime * 0.06f, 1.0f) * 2.0f * 3.1415927f);
    points[i].data[1] =
        (length(iResolution) / min_res + 2.0f * 0.05f) *
        sin(2.0f * (float(i) + 1.0f / 2.0f) * 3.1415927f / 3.0f +
            mod(iTime * 0.06f, 1.0f) * 2.0f * 3.1415927f);
  }
  for (int i = 0; i < 3; i++) {
    center = int(iTime * 0.5f) % 3 == i
                 ? (points[i] + mix(points[(i + 1) % 3], points[i], 0.5f) +
                    mix(points[(i + 2) % 3], points[i], 0.5f)) /
                       3.0f
                 : center;
  }
  points[0] = mix(points[0], 2.0f * points[0] - 2.0f * center, interp);
  points[1] = mix(points[1], 2.0f * points[1] - 2.0f * center, interp);
  points[2] = mix(points[2], 2.0f * points[2] - 2.0f * center, interp);
}
float triangle_dist(Context &__ctx, f32x2 p, f32x2 v0, f32x2 v1, f32x2 v2) {
  f32x2 n0 = normalize(f32x2(-1.0f, 1.0f) * (v0 - v1).swizzle<1, 0>());
  f32x2 n1 = normalize(f32x2(-1.0f, 1.0f) * (v1 - v2).swizzle<1, 0>());
  f32x2 n2 = normalize(f32x2(-1.0f, 1.0f) * (v2 - v0).swizzle<1, 0>());
  return max(max(dot(n0, p - v0), dot(n1, p - v1)), dot(n2, p - v2));
}
float fractal_dist(Context &__ctx, f32x2 uv) {
  float min_dist;
  int min_point = 0;
  float dist;
  float dist_factor = 1.0f;
  for (int i = 0; i < 20; i++) {
    min_dist = 100.0f;
    for (int j = 0; j < 3; j++) {
      dist = dot(uv - points[j], uv - points[j]);
      if (dist < min_dist) {
        min_dist = dist;
        min_point = j;
      }
    }
    uv = 2.0f * uv - points[min_point];
    dist = triangle_dist(__ctx, uv, points[0], points[1], points[2]);
    if (dist > 0.0f) {
      return dist * dist_factor;
    }
    dist_factor *= 0.5f;
  }
  return 0.0f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float interp = smoothstep(0.0f, 1.0f / 0.5f, mod(iTime, 1.0f / 0.5f));
  min_res = min(iResolution.data[0], iResolution.data[1]);
  f32x2 uv = (fragCoord - iResolution.swizzle<0, 1>() / 2.0f) / min_res * 2.0f;
  initialize_points(__ctx, interp);
  float dist = fractal_dist(__ctx, uv);
  f32x3 col =
      dist < 2.0f / min_res
          ? f32x3(1.0f, 1.0f, 1.0f)
          : f32x3(0.0f) + mix(f32x3(0.1f, 0.3f, 1.0f), f32x3(0.0f, 0.0f, 0.0f),
                              clamp(dist, 0.0f, 0.05f) / 0.05f);
  fragColor = f32x4(col, 1.0f);
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
