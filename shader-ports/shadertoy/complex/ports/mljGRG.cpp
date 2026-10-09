// Port of https://www.shadertoy.com/view/mljGRG; author: syndel.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mljGRG.json.
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
f32x2 wavedx(Context &__ctx, f32x2 position, f32x2 direction, float speed,
             float frequency, float timeshift) {
  float x = dot(direction, position) * frequency + timeshift * speed;
  float wave = exp(sin(x) - 1.0f);
  float dx = wave * cos(x);
  return f32x2(wave, -dx);
}
float getwaves(Context &__ctx, f32x2 position, int iterations) {
  float iter = 0.0f;
  float phase = 6.0f;
  float speed = .6f;
  float weight = 1.0f;
  float w = 0.0f;
  float ws = 0.0f;
  for (int i = 0; i < iterations; i++) {
    f32x2 p = f32x2(sin(iter), cos(iter));
    f32x2 res = wavedx(__ctx, position, p, speed, phase, (iTime));
    position += p * res.data[1] * weight * 0.048f;
    w += res.data[0] * weight;
    iter += 12.0f;
    ws += weight;
    weight = mix(weight, 0.0f, 0.2f);
    phase *= 1.18f;
    speed *= 1.07f;
  }
  return w / ws;
}
float raymarchwater(Context &__ctx, f32x3 camera, f32x3 start, f32x3 end,
                    float depth) {
  f32x3 pos = start;
  float h = 0.0f;
  float hupper = depth;
  float hlower = 0.0f;
  f32x2 zer = f32x2(0.0f);
  f32x3 dir = normalize(end - start);
  for (int i = 0; i < 318; i++) {
    h = getwaves(__ctx, pos.swizzle<0, 2>() * 0.1f, 13) * depth - depth;
    if (h + 0.01f > pos.data[1]) {
      return distance(pos, camera);
    }
    pos += dir * (pos.data[1] - h);
  }
  return -1.0f;
}
float H = 0.0f;
f32x3 normal(Context &__ctx, f32x2 pos, float e, float depth) {
  f32x2 ex = f32x2(e, 0);
  H = getwaves(__ctx, pos.swizzle<0, 1>() * 0.1f, 48) * depth;
  f32x3 a = f32x3(pos.data[0], H, pos.data[1]);
  return normalize(cross((a - f32x3(pos.data[0] - e,
                                    getwaves(__ctx,
                                             pos.swizzle<0, 1>() * 0.1f -
                                                 ex.swizzle<0, 1>() * 0.1f,
                                             48) *
                                        depth,
                                    pos.data[1])),
                         (a - f32x3(pos.data[0],
                                    getwaves(__ctx,
                                             pos.swizzle<0, 1>() * 0.1f +
                                                 ex.swizzle<1, 0>() * 0.1f,
                                             48) *
                                        depth,
                                    pos.data[1] + e))));
}
f32x3x3 rotmat(Context &__ctx, f32x3 axis, float angle) {
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0f - c;
  return f32x3x3(oc * axis.data[0] * axis.data[0] + c,
                 oc * axis.data[0] * axis.data[1] - axis.data[2] * s,
                 oc * axis.data[2] * axis.data[0] + axis.data[1] * s,
                 oc * axis.data[0] * axis.data[1] + axis.data[2] * s,
                 oc * axis.data[1] * axis.data[1] + c,
                 oc * axis.data[1] * axis.data[2] - axis.data[0] * s,
                 oc * axis.data[2] * axis.data[0] - axis.data[1] * s,
                 oc * axis.data[1] * axis.data[2] + axis.data[0] * s,
                 oc * axis.data[2] * axis.data[2] + c);
}
f32x3 getRay(Context &__ctx, f32x2 uv) {
  uv = (uv * 2.0f - 1.0f) * f32x2((iResolution.swizzle<0, 1>()).data[0] /
                                      (iResolution.swizzle<0, 1>()).data[1],
                                  1.0f);
  f32x3 proj = normalize(f32x3(uv.data[0], uv.data[1], 1.0f) +
                         f32x3(uv.data[0], uv.data[1], -1.0f) *
                             pow(length(uv), 2.0f) * 0.05f);
  if ((iResolution.swizzle<0, 1>()).data[0] < 400.0f)
    return proj;
  f32x3 ray =
      rotmat(
          __ctx, f32x3(0.0f, -1.0f, 100.0f),
          3.0f *
              (((iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>()).data[0] +
                0.5f) *
                   2.0f -
               1.0f)) *
      rotmat(
          __ctx, f32x3(1.0f, 0.0f, 100.0f),
          1.5f *
              (((iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>()).data[1] +
                0.5f) *
                   2.0f -
               1.0f)) *
      proj;
  return ray;
}
float intersectPlane(Context &__ctx, f32x3 origin, f32x3 direction, f32x3 point,
                     f32x3 normal) {
  return clamp(dot(point - origin, normal) / dot(direction, normal), -1.0f,
               9991999.0f);
}
f32x3 extra_cheap_atmosphere(Context &__ctx, f32x3 raydir, f32x3 sundir) {
  sundir.data[1] = max(sundir.data[1], -0.07f);
  float special_trick = 1.0f / (raydir.data[1] * 1.0f + 0.1f);
  float special_trick2 = 1.0f / (sundir.data[1] * 11.0f + 1.0f);
  float raysundt = pow(abs(dot(sundir, raydir)), 2.0f);
  float sundt = pow(max(0.0f, dot(sundir, raydir)), 8.0f);
  float mymie = sundt * special_trick * 0.2f;
  f32x3 suncolor =
      mix(f32x3(1.0f),
          max(f32x3(0.0f), f32x3(1.0f) - f32x3(5.5f, 13.0f, 22.4f) / 22.4f),
          special_trick2);
  f32x3 bluesky = f32x3(5.5f, 13.0f, 22.4f) / 22.4f * suncolor;
  f32x3 bluesky2 =
      max(f32x3(0.0f), bluesky - f32x3(5.5f, 13.0f, 22.4f) * 0.002f *
                                     (special_trick +
                                      -6.0f * sundir.data[1] * sundir.data[1]));
  bluesky2 *= special_trick * (0.24f + raysundt * 0.24f);
  return bluesky2 * (1.0f + 1.0f * pow(1.0f - raydir.data[1], 3.0f)) +
         mymie * suncolor;
}
f32x3 getatm(Context &__ctx, f32x3 ray) {
  return extra_cheap_atmosphere(__ctx, ray, normalize(f32x3(1.0f))) * 0.5f;
}
float sun(Context &__ctx, f32x3 ray) {
  f32x3 sd = normalize(f32x3(1.0f));
  return pow(max(0.0f, dot(ray, sd)), 528.0f) * 110.0f;
}
f32x3 aces_tonemap(Context &__ctx, f32x3 color) {
  f32x3x3 m1 = f32x3x3(0.59719f, 0.07600f, 0.02840f, 0.35458f, 0.90834f,
                       0.13383f, 0.04823f, 0.01566f, 0.83777f);
  f32x3x3 m2 = f32x3x3(1.60475f, -0.10208f, -0.00327f, -0.53108f, 1.10813f,
                       -0.07276f, -0.07367f, -0.00605f, 1.07602f);
  f32x3 v = m1 * color;
  f32x3 a = v * (v + 0.0245786f) - 0.000090537f;
  f32x3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
  return pow(clamp(m2 * (a / b), 0.0f, 1.0f), f32x3(1.0f / 2.2f));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float waterdepth = 2.1f;
  f32x3 wfloor = f32x3(0.0f, -waterdepth, 0.0f);
  f32x3 wceil = f32x3(0.0f, 0.0f, 0.0f);
  f32x3 orig = f32x3(0.0f, 2.0f, 0.0f);
  f32x3 ray = getRay(__ctx, uv);
  float hihit =
      intersectPlane(__ctx, orig, ray, wceil, f32x3(0.0f, 1.0f, 0.0f));
  if (ray.data[1] >= -0.01f) {
    f32x3 C = getatm(__ctx, ray) * 2.0f + sun(__ctx, ray);
    C = aces_tonemap(__ctx, C);
    fragColor = f32x4(C, 1.0f);
    return;
  }
  float lohit =
      intersectPlane(__ctx, orig, ray, wfloor, f32x3(0.0f, 1.0f, 0.0f));
  f32x3 hipos = orig + ray * hihit;
  f32x3 lopos = orig + ray * lohit;
  float dist = raymarchwater(__ctx, orig, hipos, lopos, waterdepth);
  f32x3 pos = orig + ray * dist;
  f32x3 N = normal(__ctx, pos.swizzle<0, 2>(), 0.001f, waterdepth);
  f32x2 velocity = N.swizzle<0, 2>() * (1.0f - N.data[1]);
  N = mix(f32x3(0.0f, 1.0f, 0.0f), N, 1.0f / (dist * dist * 0.01f + 1.0f));
  f32x3 R = reflect(ray, N);
  float fresnel =
      (0.04f + (1.0f - 0.04f) * (pow(1.0f - max(0.0f, dot(-N, ray)), 5.0f)));
  f32x3 C = fresnel * getatm(__ctx, R) * 2.0f + fresnel * sun(__ctx, R) +
            f32x3(0.0293f, 0.0698f, 0.1717f);
  C = aces_tonemap(__ctx, C);
  fragColor = f32x4(C, 1.0f);
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
