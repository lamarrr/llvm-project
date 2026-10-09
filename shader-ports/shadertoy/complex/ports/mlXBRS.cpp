// Port of https://www.shadertoy.com/view/mlXBRS; author: felipunkerito.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mlXBRS.json.
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
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float hash13(Context &__ctx, f32x3 p3) {
  p3 = fract(p3 * .1030f);
  p3 += dot(p3, p3.swizzle<2, 1, 0>() + 31.33f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
f32x2x2 rot(Context &__ctx, const float a) {
  f32x2 sC = f32x2(sin(a), cos(a));
  return f32x2x2(sC.data[0], -sC.data[1], sC.data[1], sC.data[0]);
}
float map(Context &__ctx, f32x3 p) {
  f32x3 oP = p;
  const float t = 2.f;
  float fre = t;
  float am = 0.5f;
  for (int i = 0; i < 1; ++i) {
    f32x2 pXY = p.swizzle<0, 1>();
    am *= 1.f;
    fre *= 2.f;
  }
  float s = (p.data[2]) * 0.f + 0.f;
  float amp = 1.2f, freq = 0.2f, res = 0.f;
  for (int i = 0; i < 3; ++i) {
    f32x3 pT = p;
    res += amp * (dot(texture(__ctx.channel0, pT * freq + iTime * .005f),
                      f32x4(0.25f)) *
                      2.0f -
                  1.0f);
    amp *= .7f;
    freq *= 2.56f;
  }
  float r;
  float repO = 1.f;
  float rep = 1.f / repO;
  float hRep = rep * 0.5f;
  f32x3 id = floor((p + hRep) * repO);
  {
    oP = p;
    {
    }
    {
      r = oP.data[2] - 0.f - rep * 0.1f;
      r = r + res * rep * .3f;
    }
  }
  return r;
}
f32x2 march(Context &__ctx, const f32x3 ro, const f32x3 rd, const f32x3 lDir,
            f32x3 &res) {
  const unsigned marchSteps = unsigned(512), shadowSteps = unsigned(4);
  float d = 0.0f, t = 0.0f;
  float stepSize = 1.f / float(marchSteps);
  float stepL = 1.f / float(shadowSteps);
  f32x3 lD = lDir * stepL;
  f32x3 rD = rd * stepSize;
  res = f32x3(0.f);
  float curDensity = 0.f;
  float transmittance = 1.f;
  f32x3 lightEnergy = f32x3(0.f);
  f32x3 bac = mix(f32x3(0.996f, 0.867f, 0.584f), f32x3(0.408f, 0.596f, 0.973f),
                  rd.data[2] + 0.5f);
  for (unsigned i = 0u; i < marchSteps; ++i) {
    f32x3 p = ro + rd * t;
    d = map(__ctx, p);
    if (d < 0.001f) {
      float sha = 0.0f;
      f32x3 pL = p;
      float lT = 0.f;
      for (unsigned j = 0u; j < shadowSteps; ++j) {
        pL = pL + lDir * lT;
        float lD = map(__ctx, pL);
        sha += lD;
        lT += stepL;
      }
      curDensity = (d * -1.f);
      float shadowTerm = exp(-sha * stepL);
      float absorbedLight = clamp(shadowTerm * curDensity * 0.95f, 0.f, 1.f);
      lightEnergy +=
          f32x3(1.000f, 0.678f, 0.655f) * absorbedLight * transmittance;
      transmittance *= 1.f - curDensity;
    }
    if (transmittance < 1e-3f) {
      break;
    }
    t += stepSize;
  }
  res = lightEnergy;
  res += bac * transmittance;
  return f32x2(t, d);
}
f32x3 shade(Context &__ctx, const f32x3 ro, const f32x3 rd) {
  f32x3 r = f32x3(0);
  f32x2 mou = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float t = iTime * .1f;
  f32x3 lDir = normalize(f32x3(0.0f, 0.f, -.0f) +
                         f32x3(sin(t) * 2.2f, 100.f, cos(t) * 2.f));
  f32x2 m = march(__ctx, ro, rd, lDir, r);
  return r;
}
float sdRoundedBox(Context &__ctx, f32x2 p, f32x2 b, f32x4 r) {
  r.set_swizzle<0, 1>((p.data[0] > 0.0f) ? r.swizzle<0, 1>()
                                         : r.swizzle<2, 3>());
  r.data[0] = (p.data[1] > 0.0f) ? r.data[0] : r.data[1];
  f32x2 q = abs(p) - b + r.data[0];
  return min(max(q.data[0], q.data[1]), 0.0f) + length(max(q, 0.0f)) -
         r.data[0];
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (-iResolution.swizzle<0, 1>() + fragCoord * 2.0f) / iResolution.data[1];
  f32x2 mou = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 ro = f32x3(0.0f, 0.1f, -1.5f);
  f32x3 rd = normalize(f32x3(uv, 1.0f));
  f32x2x2 r = rot(__ctx, mou.data[1] * -acos(-1.f) * .05f);
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() * (r));
  rd.set_swizzle<1, 2>(rd.swizzle<1, 2>() * (r));
  r = rot(__ctx, mou.data[0] * acos(-1.f) * 2.0f);
  ro.set_swizzle<0, 1>(ro.swizzle<0, 1>() * (r));
  rd.set_swizzle<0, 1>(rd.swizzle<0, 1>() * (r));
  f32x2 recRes = 1.f / iResolution.swizzle<0, 1>();
  float mRecRes = max(recRes.data[0], recRes.data[1]);
  float rat = iResolution.data[0] / iResolution.data[1];
  float v =
      sdRoundedBox(__ctx, uv, f32x2(.87f, 0.8f) * f32x2(rat, 1.f), f32x4(0.f));
  f32x3 col0 = mix(f32x3(1), f32x3(0), smoothstep(0.f, mRecRes * 150.f, v));
  f32x3 col = shade(__ctx, ro, rd);
  fragColor = mix(f32x4(col, 1.f), f32x4(0.f), f32x4(1.f - col0, 1.f));
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_3d<f32x4> channel0,
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
