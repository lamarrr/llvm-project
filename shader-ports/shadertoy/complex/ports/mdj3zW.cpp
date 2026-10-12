// Port of https://www.shadertoy.com/view/mdj3zW; author: kishimisu.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mdj3zW.json.
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
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x2x2 rot(Context &__ctx, float a) {
  return f32x2x2(cos(a), -sin(a), sin(a), cos(a));
}
float noise(Context &__ctx, f32x3 p) {
  f32x3 ip = floor(p);
  f32x3 fp = fract(p);
  fp = fp * fp * (3.0f - 2.0f * fp);
  f32x2 tap = (ip.swizzle<0, 1>() + f32x2(37.0f, 17.0f) * ip.data[2]) +
              fp.swizzle<0, 1>();
  f32x2 rz =
      textureLod(__ctx.channel0, (tap + 0.5f) / 256.0f, 0.0f).swizzle<1, 0>();
  return mix(rz.data[0], rz.data[1], fp.data[2]);
}
float fbm(Context &__ctx, f32x3 x) {
  x += f32x3(iTime * .015f, iTime * .035f, 0.f);
  float rz = 0.f;
  float a = .35f;
  for (int i = 0; i < 4; i++) {
    rz += noise(__ctx, x) * a;
    a *= .35f;
    x *= 4.f;
  }
  return rz - .25f;
}
float vol(Context &__ctx, f32x3 p) {
  p.data[1] -= .2f;
  float r =
      fbm(__ctx, p) + p.data[1] * .4f + .08f + .05f * max(0.f, 1.f - length(p));
  float sphere = length(p) - 1.f;
  return max(sphere, r * .9f);
}
float sampleLight(Context &__ctx, f32x3 ro) {
  f32x3 rd = normalize(-ro + f32x3(0.f, .2f, 0.f));
  float t = 0.f, visibility = 1.f, stepSize = 0.005f;
  for (int i = 0; i < 20; i++) {
    f32x3 p = ro + t * rd;
    float d = vol(__ctx, p);
    t += max(stepSize, d) +
         fract(sin((p)*f32x3x3(127.1f, 311.7f, 74.7f, 269.5f, 183.3f, 246.1f,
                               113.5f, 271.9f, 124.6f)) *
               43758.5453123f)
                 .data[0] *
             .002f;
    if (d < 0.f)
      visibility *= exp(-stepSize * 17.f);
  }
  return visibility;
}
void initRayOriginAndDirection(Context &__ctx, f32x2 fragCoord, f32x3 &ro,
                               f32x3 &rd) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float d = 1.75f;
  ro = f32x3(d * cos(iTime * .1f), 1.5f, d * sin(iTime * .1f));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (rot(__ctx, -m.data[0] * 2.f)));
  float z = iMouse.data[2] == 0.f ? 1.4f : 1.2f + m.data[1];
  f32x3 f = normalize(-ro), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = ro + f * z,
        i = c + uv.data[0] * r + uv.data[1] * u;
  rd = normalize(i - ro);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 ro, rd;
  initRayOriginAndDirection(__ctx, fragCoord, ro, rd);
  float visibility = 1.f;
  f32x3 vColor = f32x3(0.f), color = f32x3(0.f, 0.f, .1f);
  float t = 0.f, eps = 0.0005f;
  float inside = vol(__ctx, ro) < 0.f ? 1.f : 0.f;
  for (float i = 0.f; i < 300.f; i++) {
    f32x3 p = ro + t * rd;
    float d = vol(__ctx, p);
    t += abs(d) + 0.001f * t;
    bool isInsideVolume = mod(inside, 2.f) > .5f;
    if (!isInsideVolume) {
      if (d < eps) {
        inside++;
        t += eps * 10.f;
      }
    } else {
      if (d > -eps) {
        inside++;
        t += eps * 10.f;
      } else {
        float prev = visibility;
        visibility *= exp(-abs(d) * 17.f);
        float absorption = prev - visibility;
        vColor += absorption * sampleLight(__ctx, p) *
                  (f32x3(1.3f, 1.f, .8f) + fbm(__ctx, p) * 2.f);
        if (visibility <= .01f || dot(vColor, vColor) >= 3.f)
          break;
      }
    }
    if (t > 8.f)
      break;
  }
  color = vColor + color * visibility;
  fragColor = f32x4(color, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
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
