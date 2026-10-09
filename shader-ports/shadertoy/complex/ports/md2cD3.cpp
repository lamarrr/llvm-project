// Port of https://www.shadertoy.com/view/md2cD3; author: cnfrm.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/md2cD3.json.
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
const float dispValue = 1.f;
const float dispOffset = .5f;
const float sphereBaseRadius = .6f;
const f32x3 ld = f32x3(.4f, 0.3f, .0f);
float hash(Context &__ctx, f32x3 p) {
  float h = dot(p, f32x3(127.1f, 311.7f, 79.1f));
  return fract(sin(h) * 43758.5453123f);
}
f32x3 rotateY(Context &__ctx, f32x3 v, float t) {
  float cost = cos(t);
  float sint = sin(t);
  return f32x3(v.data[0] * cost + v.data[2] * sint, v.data[1],
               -v.data[0] * sint + v.data[2] * cost);
}
f32x3 rotateX(Context &__ctx, f32x3 v, float t) {
  float cost = cos(t);
  float sint = sin(t);
  return f32x3(v.data[0], v.data[1] * cost - v.data[2] * sint,
               v.data[1] * sint + v.data[2] * cost);
}
float noise(Context &__ctx, f32x3 p) {
  f32x3 i = floor(p);
  f32x3 f = fract(p);
  f32x3 u = f * f * f * (f * (f * 6.0f - 15.0f) + 10.0f);
  float v = mix(mix(mix(hash(__ctx, i + f32x3(0.f, 0.f, 0.f)),
                        hash(__ctx, i + f32x3(1.f, 0.f, 0.f)), u.data[0]),
                    mix(hash(__ctx, i + f32x3(0.f, 1.f, 0.f)),
                        hash(__ctx, i + f32x3(1.f, 1.f, 0.f)), u.data[0]),
                    u.data[1]),
                mix(mix(hash(__ctx, i + f32x3(0.f, 0.f, 1.f)),
                        hash(__ctx, i + f32x3(1.f, 0.f, 1.f)), u.data[0]),
                    mix(hash(__ctx, i + f32x3(0.f, 1.f, 1.f)),
                        hash(__ctx, i + f32x3(1.f, 1.f, 1.f)), u.data[0]),
                    u.data[1]),
                u.data[2]);
  v = v * 2.f - .5f;
  float cv = abs(cos(v));
  float sv = abs(sin(v));
  v = mix(sv, cv, sv);
  return v;
}
float noiseLayer(Context &__ctx, f32x3 p) {
  float freq = 1.2f;
  const int iter = 11;
  float lacunarity = 1.f;
  float v = 0.f;
  float sum = 0.f;
  for (int i = 0; i < iter; i++) {
    float layerUp = 1.f / pow(freq, lacunarity);
    v += noise(__ctx, p * freq) * layerUp;
    sum += layerUp;
    freq *= 1.8f;
  }
  v /= sum;
  return v;
}
float map(Context &__ctx, f32x3 p) {
  p = rotateX(__ctx, p, iMouse.data[1] / 100.f + iTime / 7.f);
  p = rotateY(__ctx, p, iMouse.data[0] / 100.f + iTime / 7.f);
  float timeOffset = iTime / 70.f;
  return noiseLayer(__ctx, p + timeOffset) - dispOffset;
}
f32x3 calcNormal(Context &__ctx, f32x3 pos, float t) {
  f32x2 eps = f32x2(0.005f * t, 0.0f);
  return normalize(f32x3(map(__ctx, pos + eps.swizzle<0, 1, 1>()) -
                             map(__ctx, pos - eps.swizzle<0, 1, 1>()),
                         map(__ctx, pos + eps.swizzle<1, 0, 1>()) -
                             map(__ctx, pos - eps.swizzle<1, 0, 1>()),
                         map(__ctx, pos + eps.swizzle<1, 1, 0>()) -
                             map(__ctx, pos - eps.swizzle<1, 1, 0>())));
}
float shadowRay(Context &__ctx, f32x3 startP, f32x3 lightDir) {
  float t = 0.0f;
  float shadowHardness = 10.f;
  float minD = 1.f;
  for (int r = 0; r < 8; ++r) {
    f32x3 p = startP +
              f32x3(t * lightDir.data[0], t * lightDir.data[1],
                    t * lightDir.data[2]) +
              normalize(startP) / shadowHardness;
    f32x3 pp = normalize(p) * sphereBaseRadius;
    float sp = sphereBaseRadius + map(__ctx, pp) * dispValue;
    float d = length(p) - sp;
    minD = min(minD, d);
    if (d < 0.0f) {
      break;
    }
    t += 0.08f;
  }
  return smoothstep(0.f, 1.0f, minD * shadowHardness);
}
f32x3 skyColor(Context &__ctx, f32x2 uv, float minD) {
  float stars =
      ((1.f - noiseLayer(__ctx, f32x3(uv * 100.f + iTime / 40.f, 0.f))) * 10.f -
       8.f) *
      0.0f;
  float stars2 =
      (((1.f - noiseLayer(__ctx, f32x3(uv * 30.f + iTime / 120.f, 3.f))) *
            10.f -
        8.8f) *
       0.0f);
  f32x3 sky = max(0.f, 1.0f) * f32x3(0.f, 0.f, 0.f);
  sky += max(0.f, 0.02f * 10.f) * f32x3(0.39f, 0.31f, 0.35f);
  float halo = (1.f - length(uv * 0.7f)) * 2.f;
  sky *= halo;
  minD = 1.f - minD;
  minD = pow(minD, 3.f) * .3f;
  sky += minD * f32x3(0.78f, 0.79f, 0.8f);
  return sky;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      fragCoord.swizzle<0, 1>() * 2.0f / iResolution.swizzle<0, 1>() - 1.0f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 color = f32x3(skyColor(__ctx, uv, 0.f));
  float minD = 1.f;
  float t = 0.0f;
  for (int r = 0; r < 16; ++r) {
    f32x3 p = f32x3(uv.data[0], uv.data[1], -2.f + t);
    f32x3 pp = normalize(p) * sphereBaseRadius;
    float sp = sphereBaseRadius + map(__ctx, pp) * dispValue;
    float d = length(p) - sp;
    minD = min(minD, d);
    if (d < 0.01f) {
      float rayCount = float(r);
      rayCount /= 16.f;
      f32x3 n =
          mix(calcNormal(__ctx, pp, 0.1f), calcNormal(__ctx, pp, 10.f), .5f);
      float light1 = max(dot(n, -ld), .0f) * 1.2f;
      if (light1 > 0.1f) {
        light1 *= shadowRay(__ctx, p, ld);
      }
      light1 += pow(light1, 15.f) / 500.f;
      color = f32x3(0.78f, 0.79f, 0.8f) * light1;
      color += max(dot(n, f32x3(0.39f, 0.31f, 0.35f)), .0f) * .15f *
               f32x3(0.21f, 0.13f, 0.17f);
      color += f32x3(0.21f, 0.13f, 0.17f) *
               pow(max(.0f, -map(__ctx, pp) + .2f), 3.f) * 8.f *
               max(.0f, 1.f - n.data[2]);
      color += pow(rayCount, 3.f) * f32x3(0.81f, 0.49f, 0.6f);
      break;
    } else if (t > 4.f) {
      color = skyColor(__ctx, uv, minD);
      break;
    }
    t += d;
  }
  fragColor = f32x4(color, 1.f);
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
