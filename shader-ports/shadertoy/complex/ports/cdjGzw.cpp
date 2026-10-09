// Port of https://www.shadertoy.com/view/cdjGzw; author: kishimisu.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cdjGzw.json.
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
  p.data[2] += iTime * .8f;
  float r = fbm(__ctx, p) + max(0.f, p.data[1]) - .1f -
            .08f * smoothstep(1.f, 0.f, length(p.swizzle<0, 1>()));
  return p.data[1] - r * ((cos(iTime * .146f + 1.f) * .5f + .5f) * .1f + .9f) -
         (sin(iTime * .2f) * .5f + .5f) * abs(p.data[0]) * .1f;
}
float sampleLight(Context &__ctx, f32x3 ro, f32x3 lightPos, float att) {
  float t = 0.f, visibility = 1.f, stepSize = 0.005f * 2.f;
  f32x3 rd = normalize(ro - lightPos);
  float dstLight = length(ro - lightPos);
  float attenuation =
      att < 0.f ? 1.f : smoothstep(0.f, 1.f, att / pow(dstLight, 2.0f));
  for (int i = 0; i < 9; i++) {
    f32x3 p = ro + t * rd;
    float d = vol(__ctx, p);
    t += max(stepSize, d) +
         fract(sin((p)*f32x3x3(127.1f, 311.7f, 74.7f, 269.5f, 183.3f, 246.1f,
                               113.5f, 271.9f, 124.6f)) *
               43758.5453123f)
                 .data[0] *
             .002f;
    if (d < 0.f)
      visibility *= exp(-stepSize * 10.f);
    if (t > dstLight)
      break;
  }
  return visibility * attenuation;
}
void initCamera(Context &__ctx, f32x2 fragCoord, f32x3 &ro, f32x3 &rd) {
  f32x2 uv =
      (fragCoord - iResolution.swizzle<0, 1>() * .5f) / iResolution.data[1];
  f32x2 m = iMouse.data[2] == 0.f
                ? f32x2(.5f)
                : iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  ro = f32x3(0.f, 0.25f, 0.f);
  float mx = (sin(-(iTime - 3.f) * .2f) * .5f) * .8f -
             (m.data[0] * 2.f - 1.f) * .5f + 1.54f;
  float my = (m.data[1] - .5f) * .5f;
  f32x3 f = normalize(f32x3(cos(mx), my, sin(mx))),
        r = normalize(cross(f32x3(0, 1, 0), f)), u = cross(f, r), c = ro + f,
        i = c + uv.data[0] * r + uv.data[1] * u;
  rd = normalize(i - ro);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 ro, rd;
  initCamera(__ctx, fragCoord, ro, rd);
  float visibility = 1.f;
  f32x3 vColor = f32x3(0.f), color = f32x3(.14f, 0.08f, .12f) * .85f;
  float t = 0.f, eps = 0.0005f;
  float inside = vol(__ctx, ro) < 0.f ? 1.f : 0.f;
  for (float i = 0.f; i < 300.f; i++) {
    f32x3 p = ro + t * rd;
    float dvol = vol(__ctx, p);
    float d = abs(dvol) + 0.001f * t;
    t += d;
    bool isInsideVolume = mod(inside, 2.f) > .5f;
    if (!isInsideVolume) {
      if (dvol < eps) {
        inside++;
        t += eps * 10.f;
      }
    } else {
      if (dvol > -eps) {
        inside++;
        t += eps * 10.f;
      } else {
        float prev = visibility;
        visibility *= exp(-d * 10.f);
        f32x3 absorption = f32x3(prev - visibility);
        absorption *= mix(color, f32x3(1.f), exp(-t * .05f));
        vColor += absorption *
                  sampleLight(__ctx, p, -f32x3(0.f, 4.f, 0.f), 10.f) *
                  f32x3(1.2f, .6f, .3f) * .75f;
        vColor += absorption *
                  sampleLight(__ctx, p, -f32x3(0.f, 1e6f, 0.f), -1.f) *
                  (f32x3(1.3f, 1.f, .8f) * .5f + 0.5f * fbm(__ctx, p));
        if (visibility <= .01f || dot(vColor, vColor) >= 3.f)
          break;
      }
    }
    if (t > 25.f) {
      i = 0.f;
      break;
    }
  }
  color = vColor + color * visibility;
  fragColor = f32x4(color, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
