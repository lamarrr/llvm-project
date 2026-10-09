// Port of https://www.shadertoy.com/view/DsSGRy; author: kishimisu.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DsSGRy.json.
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
float sdBox(Context &__ctx, f32x3 p, f32x3 s) {
  f32x3 q = abs(p) - s;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f) - .0f;
}
f32x3 map(Context &__ctx, f32x3 p) {
  float box = sdBox(__ctx, p, f32x3(.9f, 1.5f, .9f)) - .05f;
  p.set_swizzle<0, 1>(
      p.swizzle<0, 1>() *
      (f32x2x2(
          cos(pow(smoothstep(0.f, .5f,
                             sin(3.141592f + (iTime - 4.5f) * .3f) * .5f + .5f),
                  10.f) *
              3.14f),
          -sin(
              pow(smoothstep(0.f, .5f,
                             sin(3.141592f + (iTime - 4.5f) * .3f) * .5f + .5f),
                  10.f) *
              3.14f),
          sin(pow(smoothstep(0.f, .5f,
                             sin(3.141592f + (iTime - 4.5f) * .3f) * .5f + .5f),
                  10.f) *
              3.14f),
          cos(pow(smoothstep(0.f, .5f,
                             sin(3.141592f + (iTime - 4.5f) * .3f) * .5f + .5f),
                  10.f) *
              3.14f))));
  p.set_swizzle<0, 2>(
      p.swizzle<0, 2>() *
      (f32x2x2(cos(pow(smoothstep(0.f, .5f,
                                  sin(0.f + (iTime - 4.5f) * .2f) * .5f + .5f),
                       10.f) *
                   1.57f),
               -sin(pow(smoothstep(0.f, .5f,
                                   sin(0.f + (iTime - 4.5f) * .2f) * .5f + .5f),
                        10.f) *
                    1.57f),
               sin(pow(smoothstep(0.f, .5f,
                                  sin(0.f + (iTime - 4.5f) * .2f) * .5f + .5f),
                       10.f) *
                   1.57f),
               cos(pow(smoothstep(0.f, .5f,
                                  sin(0.f + (iTime - 4.5f) * .2f) * .5f + .5f),
                       10.f) *
                   1.57f))));
  p = abs(abs(p) -
          pow(smoothstep(0.f, .5f,
                         sin(3.141592f + (iTime - 4.5f) * 1.f) * .5f + .5f),
              10.f) *
              .4f);
  p = mix(p, abs(p * 2.f - .3f),
          pow(smoothstep(0.f, .5f, sin(0.f + (iTime - 4.5f) * .5f) * .5f + .5f),
              10.f));
  float shape =
      mix(length(p) - .25f, sdBox(__ctx, p, f32x3(.25f)),
          pow(smoothstep(0.f, .5f, sin(0.f + (iTime - 4.5f) * 2.f) * .5f + .5f),
              10.f)) *
      .5f;
  shape = max(box, shape);
  return f32x3(box, shape, min(abs(box), shape));
}
f32x3 normal(Context &__ctx, f32x3 p) {
  const float h = 0.0001f;
  const f32x2 k = f32x2(1, -1);
  return normalize(
      k.swizzle<0, 1, 1>() * map(__ctx, p + k.swizzle<0, 1, 1>() * h).data[0] +
      k.swizzle<1, 1, 0>() * map(__ctx, p + k.swizzle<1, 1, 0>() * h).data[0] +
      k.swizzle<1, 0, 1>() * map(__ctx, p + k.swizzle<1, 0, 1>() * h).data[0] +
      k.swizzle<0, 0, 0>() * map(__ctx, p + k.swizzle<0, 0, 0>() * h).data[0]);
}
f32x3 rrefract(Context &__ctx, f32x3 rd, f32x3 n, float ior) {
  f32x3 r = refract(rd, n, ior);
  return r == f32x3(0.f) ? reflect(rd, n) : r;
}
void initRayOriginAndDirection(Context &__ctx, f32x2 fragCoord, f32x3 &ro,
                               f32x3 &rd) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>() * 2.f - 1.f;
  float z = iMouse.data[2] > 0.f ? 1.f : smoothstep(0.f, 6.f, iTime);
  ro = f32x3(0.f, 0.f, 4.2f);
  ro.set_swizzle<1, 2>(
      ro.swizzle<1, 2>() *
      (f32x2x2(cos(iMouse.data[2] > 0.f ? m.data[1] * 1.5f
                                        : -1.5f + mix(0.5f, 1.85f, z)),
               -sin(iMouse.data[2] > 0.f ? m.data[1] * 1.5f
                                         : -1.5f + mix(0.5f, 1.85f, z)),
               sin(iMouse.data[2] > 0.f ? m.data[1] * 1.5f
                                        : -1.5f + mix(0.5f, 1.85f, z)),
               cos(iMouse.data[2] > 0.f ? m.data[1] * 1.5f
                                        : -1.5f + mix(0.5f, 1.85f, z)))));
  ro.set_swizzle<2, 0>(ro.swizzle<2, 0>() *
                       (f32x2x2(cos(m.data[0] * 2.f + iTime * .2f),
                                -sin(m.data[0] * 2.f + iTime * .2f),
                                sin(m.data[0] * 2.f + iTime * .2f),
                                cos(m.data[0] * 2.f + iTime * .2f))));
  f32x3 f = normalize(f32x3(0.f, .2f, 0.f) - ro),
        r = normalize(cross(f32x3(0, 1, 0), f));
  rd = normalize(f + uv.data[0] * r + uv.data[1] * cross(f, r));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 tot;
  for (int j = 0; j < 2; j++)
    for (int k = 0; k < 2; k++) {
      f32x2 o = f32x2(float(j), float(k)) / float(2) - 0.5f;
      f32x3 ro, rd;
      initRayOriginAndDirection(__ctx, fragCoord + o / 2.f, ro, rd);
      f32x3 bgColor =
          mix(f32x3(.17f, .0f, .16f), f32x3(0.f, 0.f, .12f), rd.data[1] - .3f) *
          .8f;
      f32x3 tintColor = f32x3(1.f, .5f, .5f) +
                        .07f * sin(f32x3(1.4f, .54f, 1.23f) * iTime * .3f);
      f32x3 p = ro, color = bgColor;
      float i, t, inside, entry, fresnel;
      const float ior = 1.7f;
      for (i = 0.f; i < 150.f; i++) {
        f32x3 d = map(__ctx, p);
        if (d.data[1] < .001f) {
          color = tintColor;
          break;
        }
        p += d.data[2] * rd;
        t += d.data[2];
        if (inside < .5f) {
          if (d.data[0] < .001f) {
            f32x3 n = normal(__ctx, p);
            fresnel = pow(1.f - clamp(-dot(rd, n), 0.f, 1.f), 4.f);
            rd = rrefract(__ctx, rd, n, 1.f / ior);
            p += rd * .04f;
            entry = t;
            inside++;
          }
        } else {
          if (d.data[0] > -.001f) {
            f32x3 n = normal(__ctx, p);
            rd = reflect(rd, n);
            p += rd * .1f;
            inside++;
          }
        }
        if (inside > 6.f || t > 15.f)
          break;
      }
      if (inside > 0.f) {
        float att = exp(-(t - entry) * .3f);
        color = mix(bgColor * .7f, color - f32x3(.5f, .5f, 2.f) * fresnel,
                    exp(-i * .005f) * att);
      }
      tot += color;
    }
  tot /= float(2 * 2);
  fragColor.set_swizzle<0, 1, 2>(tot);
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
