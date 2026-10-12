// Port of https://www.shadertoy.com/view/mddfDB; author: Lincac.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mddfDB.json.
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
  Texture3D channel0;
  Texture3D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

f32x3 box_min = f32x3(-2.3f, -1.3f, -1.9f);
f32x3 box_max = f32x3(2.1f, 2.6f, 2.2f);
const float kPI = 3.1415926f;
f32x3 getSkyColor(Context &__ctx, f32x3 dir) {
  float t = (dir.data[1] + 1.0f) * 0.5f;
  return (1.0f - t) * f32x3(1.0f, 1.0f, 1.0f) + t * f32x3(0.5f, 0.7f, 1.0f);
}
float getHeightPercent(Context &__ctx, f32x3 pos) {
  return (pos.data[1] - box_min.data[1]) / (box_max.data[1] - box_min.data[1]);
}
f32x2 getUV(Context &__ctx, f32x3 pos) {
  return (pos.swizzle<0, 2>() / box_max.swizzle<0, 2>()) * 0.5f + 0.5f;
}
float remap(Context &__ctx, float originalValue, float originalMin,
            float originalMax, float newMin, float newMax) {
  return newMin +
         (((originalValue - originalMin) / (originalMax - originalMin)) *
          (newMax - newMin));
}
float hgPhase(Context &__ctx, float g, float cosTheta) {
  float numer = 1.0f - g * g;
  float denom = 1.0f + g * g + 2.0f * g * cosTheta;
  return numer / (4.0f * kPI * denom * sqrt(denom));
}
float dualLobPhase(Context &__ctx, float g0, float g1, float w,
                   float cosTheta) {
  return mix(hgPhase(__ctx, g0, cosTheta), hgPhase(__ctx, g1, cosTheta), w);
}
bool hitbox(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 &start, f32x3 &end) {
  f32x3 t1 = (box_min - ro) / rd;
  f32x3 t2 = (box_max - ro) / rd;
  f32x3 tmin = min(t1, t2);
  f32x3 tmax = max(t1, t2);
  float entt = max(tmin.data[0], max(tmin.data[1], tmin.data[2]));
  float outt = min(tmax.data[0], min(tmax.data[1], tmax.data[2]));
  if (entt > outt || outt < 0.f)
    return false;
  start = ro + entt * rd;
  end = ro + outt * rd;
  return true;
}
float getDensityForCloud(Context &__ctx, float heightFraction,
                         float cloudType) {
  float stratusFactor = 1.0f - clamp(cloudType * 2.0f, 0.0f, 1.0f);
  float stratoCumulusFactor = 1.0f - abs(cloudType - 0.5f) * 2.0f;
  float cumulusFactor = clamp(cloudType - 0.5f, 0.0f, 1.0f) * 2.0f;
  f32x4 baseGradient = stratusFactor * f32x4(0.0f, 0.1f, 0.2f, 0.3f) +
                       stratoCumulusFactor * f32x4(0.02f, 0.2f, 0.48f, 0.625f) +
                       cumulusFactor * f32x4(0.00f, 0.1625f, 0.88f, 0.98f);
  return smoothstep(baseGradient.data[0], baseGradient.data[1],
                    heightFraction) -
         smoothstep(baseGradient.data[2], baseGradient.data[3], heightFraction);
}
float opSmoothUnion(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  return mix(d2, d1, h) - k * h * (1.0f - h);
}
float getDensity(Context &__ctx, f32x3 pos) {
  float height = getHeightPercent(__ctx, pos);
  if (height < 0.0f || height > 1.0f) {
    return 0.0f;
  }
  f32x2 uv = getUV(__ctx, pos);
  f32x2 mv_uv =
      getUV(__ctx, pos + height * normalize(f32x3(0.5f, 0.5f, 0)) * 2.f +
                       normalize(f32x3(0.5f, 0.5f, 0)) * iTime * 0.5f);
  f32x4 density = texture(__ctx.channel0, f32x3(mv_uv / 2.f, height));
  float fbm = dot(density.swizzle<1, 2, 3>(), f32x3(0.625f, 0.25f, 0.125f));
  float base_cloud =
      remap(__ctx, density.data[0], -(1.0f - fbm), 1.f, 0.0f, 1.0f);
  float density2 = getDensityForCloud(__ctx, height, 1.f);
  base_cloud *= (density2 / height);
  f32x3 erodeCloudNoise =
      texture(__ctx.channel1, f32x3(mv_uv, height)).swizzle<0, 1, 2>();
  float highFreqFBM = dot(erodeCloudNoise, f32x3(0.625f, 0.25f, 0.125f));
  float highFreqNoiseModifier = mix(highFreqFBM, 1.0f - highFreqFBM, height);
  base_cloud = base_cloud - highFreqNoiseModifier * (1.0f - base_cloud);
  return base_cloud - length(pos);
}
float LightRayMarch(Context &__ctx, f32x3 start, float ds) {
  f32x3 dir = normalize(f32x3(1, 0.8f, -1.f));
  f32x3 marchDir = ds * normalize(dir);
  float sigma_ds = -ds;
  float T = 1.0f;
  f32x3 pos = start;
  for (int i = 0; i < 6; i++) {
    float density = getDensity(__ctx, pos);
    if (density > .0f) {
      T *= exp(density * sigma_ds);
    }
  }
  return T;
}
f32x4 RayMarch(Context &__ctx, f32x3 start, f32x3 end, f32x3 bg,
               f32x2 fragcoord) {
  f32x3 path = end - start;
  float len = length(path);
  float ds = len / 32.f;
  f32x3 marchDir = ds * normalize(path);
  float sigma_ds = -ds;
  f32x3 scattering = f32x3(0, 0, 0);
  float VoL = dot(normalize(path), normalize(f32x3(1, 0.8f, -1.f)));
  float T = 1.0f;
  f32x3 pos = start;
  for (int i = 0; i < 32; i++) {
    float density = getDensity(__ctx, pos);
    if (density > 0.f) {
      float ds_Trans = exp(density * sigma_ds);
      float light_ds = LightRayMarch(__ctx, pos, ds * 0.1f);
      float sunPhase = dualLobPhase(__ctx, 0.5f, -0.5f, 0.2f, -VoL);
      f32x3 stepScattering =
          f32x3(1, 1.f, 0.9f) * 7.f * ds_Trans * sunPhase * light_ds;
      f32x3 sigmaS = f32x3(density);
      scattering += stepScattering * T * (sigmaS * ds);
      T *= ds_Trans;
    }
    if (T < 1e-1f)
      break;
    pos += marchDir;
  }
  f32x3 finalColor = T * bg + scattering;
  return f32x4(finalColor, T);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 ro = f32x3(0.0f, 0.0f, 5.0f);
  f32x3 rd = normalize(f32x3(uv, -1.0f));
  f32x2 q = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 v = -1.0f + 2.0f * q;
  v.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x2 mo = f32x2(iTime * .1f, cos(iTime * .25f) * 3.f);
  f32x3 org = 5.0f * normalize(f32x3(cos(2.75f - 3.0f * mo.data[0]),
                                     0.7f - 1.0f * (mo.data[1] - 1.0f),
                                     sin(2.75f - 3.0f * mo.data[0])));
  f32x3 ta = f32x3(0.0f, 1.0f, 0.0f);
  f32x3 ww = normalize(ta - org);
  f32x3 uu = normalize(cross(f32x3(0.0f, 1.0f, 0.0f), ww));
  f32x3 vv = normalize(cross(ww, uu));
  f32x3 dir = normalize(v.data[0] * uu + v.data[1] * vv + 1.5f * ww);
  f32x4 color = f32x4(.0f);
  f32x3 start;
  f32x3 end;
  f32x3 bg = getSkyColor(__ctx, rd);
  f32x4 col = f32x4(bg, 0);
  if (hitbox(__ctx, ro, rd, start, end)) {
    col = RayMarch(__ctx, start, end, bg, fragCoord);
    col.set_swizzle<0, 1, 2>(mix(col.swizzle<0, 1, 2>(), bg, col.data[3]));
  }
  fragColor = f32x4(col.swizzle<0, 1, 2>(), 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_3d<f32x4> channel0,
               __binding(1, 0) sampler sampling0,
               __binding(0, 1) texture_3d<f32x4> channel1,
               __binding(1, 1) sampler sampling1) {
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
