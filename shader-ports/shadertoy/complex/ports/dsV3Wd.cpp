// Port of https://www.shadertoy.com/view/dsV3Wd; author: domrally.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dsV3Wd.json.
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
float sphIntersect(Context &__ctx, f32x3 ro, f32x3 rd, f32x4 sph) {
  f32x3 oc = ro - sph.swizzle<0, 1, 2>();
  float b = dot(oc, rd);
  float c = dot(oc, oc) - sph.data[3] * sph.data[3];
  float h = b * b - c;
  if (h < 0.0f)
    return -1.0f;
  return -b - sqrt(h);
}
f32x3 normalAtPoint(Context &__ctx, f32x3 point) {
  f32x3 xTangent = dFdx(point);
  f32x3 yTangent = dFdy(point);
  f32x3 normal = normalize(cross(yTangent, xTangent));
  return normal;
}
f32x3 computeSpectralIOR(Context &__ctx, float eta, float abbeNumber) {
  f32x3 etaOut;
  etaOut.data[1] = eta;
  etaOut.data[2] = (etaOut.data[1] - 1.f) / (2.f * abbeNumber) + etaOut.data[1];
  etaOut.data[0] = 2.f * etaOut.data[1] - etaOut.data[2];
  return etaOut;
}
float f(Context &__ctx, float x) {
  if (x >= 0.0031308f)
    return (1.055f) * pow(x, (1.0f / 2.4f) - 0.055f);
  else
    return 12.92f * x;
}
float f_inv(Context &__ctx, float x) {
  if (x >= 0.04045f)
    return pow(((x + 0.055f) / (1.f + 0.055f)), 2.4f);
  else
    return x / 12.92f;
}
f32x3 computeRefractionDirection(Context &__ctx, f32x3 N, f32x3 wo,
                                 float neta) {
  float NdotWo = dot(N, wo);
  f32x3 Nf;
  if (NdotWo > 0.0f) {
    Nf = N;
  } else {
    Nf = -N;
  }
  NdotWo = abs(NdotWo);
  float NdotWi = 1.0f - neta * neta * (1.0f - (NdotWo * NdotWo));
  if (NdotWi < 0.0f) {
    return f32x3(0.f);
  }
  NdotWi = sqrt(NdotWi);
  float negMu = neta * NdotWo - NdotWi;
  f32x3 wi = negMu * Nf - neta * wo;
  wi = normalize(wi);
  return wi;
}
f32x3 computeReflectionDirection(Context &__ctx, f32x3 N, f32x3 wo) {
  float NDotWo = dot(N, wo);
  f32x3 wi = -1.0f * wo + 2.0f * NDotWo * N;
  wi = normalize(wi);
  return wi;
}
f32x4 sampleSpectralIOR(Context &__ctx, float r, float etaR, float etaG,
                        float etaB) {
  float sampleEta;
  f32x3 dispersionColor;
  if (r < .35f) {
    r /= .35f;
    dispersionColor = f32x3(1.f, 0.f, 0.f);
    sampleEta = etaR;
  } else if (r < .7f) {
    r = (r - .35f) / .35f;
    dispersionColor = f32x3(0.f, 1.f, 0.f);
    sampleEta = etaG;
  } else {
    r = (r - .7f) / .3f;
    dispersionColor = f32x3(0.f, 0.f, 1.f);
    sampleEta = etaB;
  }
  return f32x4(dispersionColor, sampleEta);
}
f32x3 rotationRodrigues(Context &__ctx, f32x3 v, f32x3 axis, float angle) {
  float c = cos(angle);
  return v * c + cross(axis, v) * sin(angle) + axis * dot(axis, v) * (1.0f - c);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p = .6f *
            (2.f * fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
            iResolution.data[1];
  f32x3 up = f32x3(0.f, 1.f, 0.f);
  f32x3 wo = f32x3(p, -2.f);
  float t = iTime * .1f;
  wo = rotationRodrigues(__ctx, wo, up, t);
  f32x3 cam = f32x3(0.f, 0.f, -4.f);
  cam = rotationRodrigues(__ctx, cam, up, t);
  f32x3 ray = wo - cam;
  ray = normalize(ray);
  float d = sphIntersect(__ctx, wo, ray, f32x4(0.f, 0.f, 0.f, 1.f));
  f32x3 pt = ray * d + cam;
  f32x3 normy = normalAtPoint(__ctx, pt);
  f32x3 ri = computeSpectralIOR(__ctx, .5f, 10.f);
  f32x4 eta =
      sampleSpectralIOR(__ctx, 1.0f, ri.data[0], ri.data[1], ri.data[2]);
  f32x3 refracted = computeRefractionDirection(__ctx, normy, ray, eta.data[3]);
  f32x3 rfrct[3];
  rfrct[0] = computeRefractionDirection(__ctx, normy, ray, ri.data[0]);
  rfrct[1] = computeRefractionDirection(__ctx, normy, ray, ri.data[1]);
  rfrct[2] = computeRefractionDirection(__ctx, normy, ray, ri.data[2]);
  f32x3 reflected = computeReflectionDirection(__ctx, normy, ray);
  f32x3 background = texture(__ctx.channel0, ray).swizzle<0, 1, 2>();
  rfrct[0].set_swizzle<0, 1>(rfrct[0].swizzle<0, 1>() * (-1.f));
  rfrct[1].set_swizzle<0, 1>(rfrct[1].swizzle<0, 1>() * (-1.f));
  rfrct[2].set_swizzle<0, 1>(rfrct[2].swizzle<0, 1>() * (-1.f));
  refracted.set_swizzle<0, 1>(refracted.swizzle<0, 1>() * (-1.f));
  f32x3 refraction = texture(__ctx.channel0, refracted).swizzle<0, 1, 2>();
  f32x3 reflection = .2f * pow(normy, f32x3(7.f)) *
                     float(texture(__ctx.channel0, reflected).data[0] > .5f);
  refraction =
      f32x3(f_inv(__ctx, refraction.data[0]), f_inv(__ctx, refraction.data[1]),
            f_inv(__ctx, refraction.data[2]));
  float counter = 0.f;
  for (float r = 255.f; r > 0.f; r -= 64.f) {
    for (float g = 255.f; g > 0.f; g -= 64.f) {
      for (float b = 255.f; b > 0.f; b -= 64.f) {
        f32x3 w = f32x3(r, g, b) / (r + g + b);
        f32x3 re =
            rfrct[0] * w.data[0] + rfrct[1] * w.data[1] + rfrct[2] * w.data[2];
        f32x3 s = texture(__ctx.channel0, re).swizzle<0, 1, 2>();
        s = f32x3(f_inv(__ctx, s.data[0]), f_inv(__ctx, s.data[1]),
                  f_inv(__ctx, s.data[2]));
        refraction += s * w;
        counter++;
      }
    }
  }
  refraction /= counter;
  refraction *= 3.f * f32x3(.99f, 1.f, .8f);
  refraction = f32x3(f(__ctx, refraction.data[0]), f(__ctx, refraction.data[1]),
                     f(__ctx, refraction.data[2]));
  fragColor.set_swizzle<0, 1, 2>(float(d <= 0.f) * background +
                                 float(d > 0.f) * (refraction + reflection));
  fragColor.set_swizzle<0, 1, 2>(
      pow(fragColor.swizzle<0, 1, 2>(), f32x3(2.2f)));
  fragColor.data[3] = 1.f;
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_cube<f32x4> channel0,
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
