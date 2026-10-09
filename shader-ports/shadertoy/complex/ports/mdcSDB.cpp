// Port of https://www.shadertoy.com/view/mdcSDB; author: Peace.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mdcSDB.json.
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
struct Hit {
  float dist;
  float closest_dist;
  f32x3 p;
};
float specularBlinnPhong(Context &__ctx, f32x3 light_dir, f32x3 ray_dir,
                         f32x3 normal) {
  f32x3 halfway = normalize(light_dir + ray_dir);
  return max(0.0f, dot(normal, halfway));
}
f32x4 mod289(Context &__ctx, f32x4 x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}
f32x4 perm(Context &__ctx, f32x4 x) {
  return mod289(__ctx, ((x * 34.0f) + 1.0f) * x);
}
float noise(Context &__ctx, f32x3 p) {
  f32x3 a = floor(p);
  f32x3 d = p - a;
  d = d * d * (3.0f - 2.0f * d);
  f32x4 b = a.swizzle<0, 0, 1, 1>() + f32x4(0.0f, 1.0f, 0.0f, 1.0f);
  f32x4 k1 = perm(__ctx, b.swizzle<0, 1, 0, 1>());
  f32x4 k2 = perm(__ctx, k1.swizzle<0, 1, 0, 1>() + b.swizzle<2, 2, 3, 3>());
  f32x4 c = k2 + a.swizzle<2, 2, 2, 2>();
  f32x4 k3 = perm(__ctx, c);
  f32x4 k4 = perm(__ctx, c + 1.0f);
  f32x4 o1 = fract(k3 * (1.0f / 41.0f));
  f32x4 o2 = fract(k4 * (1.0f / 41.0f));
  f32x4 o3 = o2 * d.data[2] + o1 * (1.0f - d.data[2]);
  f32x2 o4 =
      o3.swizzle<1, 3>() * d.data[0] + o3.swizzle<0, 2>() * (1.0f - d.data[0]);
  return o4.data[1] * d.data[1] + o4.data[0] * (1.0f - d.data[1]);
}
float SDF(Context &__ctx, f32x3 point) {
  f32x3 p = f32x3(point.swizzle<0, 1>(), iTime * 0.3f + point.data[2]);
  float n = (noise(__ctx, p) + noise(__ctx, p * 2.0f) * 0.5f +
             noise(__ctx, p * 4.0f) * 0.25f) *
            0.57f;
  return length(point) - 0.35f - n * 0.3f;
}
f32x3 getNormal(Context &__ctx, f32x3 point) {
  f32x2 e = f32x2(0.002f, 0.0f);
  return normalize(SDF(__ctx, point) -
                   f32x3(SDF(__ctx, point - e.swizzle<0, 1, 1>()),
                         SDF(__ctx, point - e.swizzle<1, 0, 1>()),
                         SDF(__ctx, point - e.swizzle<1, 1, 0>())));
}
Hit raymarch(Context &__ctx, f32x3 p, f32x3 d) {
  Hit hit;
  hit.closest_dist = 4.0f;
  for (int i = 0; i < 32; ++i) {
    float sdf = SDF(__ctx, p);
    p += d * sdf;
    hit.closest_dist = min(hit.closest_dist, sdf);
    hit.dist += sdf;
    if (hit.dist >= 4.0f || abs(sdf) <= 0.002f)
      break;
  }
  hit.p = p;
  return hit;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord * 2.0f - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  fragColor = f32x4(0, 0, 0, 1);
  if (dot(uv, uv) > 1.0f)
    return;
  f32x3 pos = f32x3(0, 0, -1);
  f32x3 dir = normalize(f32x3(uv, 1));
  Hit hit = raymarch(__ctx, pos, dir);
  fragColor =
      f32x4(pow(max(0.0f, 1.0f - hit.closest_dist), 32.0f) *
                (max(0.0f, dot(uv, f32x2(0.707f))) * f32x3(0.3f, 0.65f, 1.0f) +
                 max(0.0f, dot(uv, f32x2(-0.707f))) * f32x3(0.6f, 0.35f, 1.0f) +
                 f32x3(0.4f, 0.5f, 1.0f)),
            max(0.0f, hit.closest_dist));
  if (hit.closest_dist >= 0.002f)
    return;
  f32x3 normal = getNormal(__ctx, hit.p);
  f32x3 ray_dir = normalize(pos - hit.p);
  float facing = max(0.0f, sqrt(dot(normal, f32x3(0.707f, 0.707f, 0))) * 1.5f -
                               dot(normal, -dir));
  fragColor = mix(f32x4(0), f32x4(0.3f, 0.65f, 1.0f, 1.0f),
                  0.75f * facing * facing * facing);
  facing = max(0.0f, sqrt(dot(normal, f32x3(-0.707f, -0.707f, 0))) * 1.5f -
                         dot(normal, -dir));
  fragColor = f32x4(fragColor.swizzle<0, 1, 2>(), 0) +
              mix(f32x4(0), f32x4(0.6f, 0.35f, 1.0f, 1.0f),
                  0.75f * facing * facing * facing);
  facing = max(0.0f, sqrt(dot(normal, f32x3(0.0f, 0.0f, -1.0f))) * 1.5f -
                         dot(normal, -dir));
  fragColor = f32x4(fragColor.swizzle<0, 1, 2>(), 0) +
              mix(f32x4(0), f32x4(0.4f, 0.5f, 1.0f, 1.0f),
                  0.5f * facing * facing * facing);
  fragColor = f32x4(fragColor.swizzle<0, 1, 2>(), 0) +
              mix(f32x4(0), f32x4(0.4f, 0.625f, 1.0f, 1.0f),
                  pow(specularBlinnPhong(
                          __ctx, normalize(f32x3(600, 800, -500) - hit.p),
                          ray_dir, normal),
                      12.0f) *
                      1.0f);
  fragColor = f32x4(fragColor.swizzle<0, 1, 2>(), 0) +
              mix(f32x4(0), f32x4(0.6f, 0.5625f, 1.0f, 1.0f),
                  pow(specularBlinnPhong(
                          __ctx, normalize(f32x3(-600, -800, -00) - hit.p),
                          ray_dir, normal),
                      16.0f) *
                      0.75f);
  fragColor.set_swizzle<0, 1, 2>(
      pow(fragColor.swizzle<0, 1, 2>(), f32x3(1.25f)));
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
