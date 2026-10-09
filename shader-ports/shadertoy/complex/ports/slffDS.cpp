// Port of https://www.shadertoy.com/view/slffDS; author: El_Sargo.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/slffDS.json.
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
const float pi = 3.14159265359f;
f32x2x2 rot(Context &__ctx, float a) {
  float c = cos(a);
  float s = sin(a);
  return f32x2x2(c, -s, s, c);
}
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
f32x4 map(Context &__ctx, f32x3 p) {
  f32x2x2 rotx =
      rot(__ctx, sin(2.f * iTime * 0.1f) * pi + sin(pi * iTime * 0.1f) * pi);
  f32x2x2 rotz = rot(__ctx, cos(iTime / 4.f) * pi);
  f32x2x2 roty = rot(__ctx, sin(iTime / 4.f + 12313.123f) * pi);
  int maxI = 8;
  float scale = 1.f;
  const float factor = 2.f;
  const float baseSize = .7f;
  float d = 10e10f;
  for (int i = min(iFrame, 0); i < maxI; i++) {
    p = abs(p) - .4f * factor;
    p *= factor;
    scale /= factor;
    p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (rotx));
    p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (rotz));
    p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (roty));
  }
  d = min(d, sdBox(__ctx, p - f32x3(0, .44f * factor, 0), (f32x3(baseSize))) *
                     scale -
                 (0.4f * scale));
  return f32x4(d, p);
}
float intersection(Context &__ctx, f32x3 ro, f32x3 rd) {
  float T = 0.f;
  for (int i = 0; i < 30; i++) {
    float dist = map(__ctx, ro + rd * T).data[0];
    T += dist;
    if (dist < 0.0026f * T || dist > 4.f)
      break;
  }
  return T;
}
float softshadow(Context &__ctx, f32x3 ro, f32x3 rd, float mint, float maxt,
                 float k, float d) {
  float res = 1.0f;
  for (float t = mint; t < maxt;) {
    float h = map(__ctx, ro + rd * t).data[0];
    if (h < 0.0005f * d)
      return 0.0f;
    res = min(res, k * h / t);
    t += h;
  }
  return res;
}
f32x3 calcNormal(Context &__ctx, f32x3 p, float t) {
  float h = 0.002f * t;
  f32x3 n = f32x3(0.0f);
  for (int i = min(iFrame, 0); i < 4; i++) {
    f32x3 e =
        0.5773f *
        (2.0f * f32x3((((i + 3) >> 1) & 1), ((i >> 1) & 1), (i & 1)) - 1.0f);
    n += e * map(__ctx, p + e * h).data[0];
  }
  return normalize(n);
}
float calcAO(Context &__ctx, f32x3 pos, f32x3 nor) {
  float occ = 0.0f;
  float sca = .4f;
  for (int i = min(iFrame, 0); i < 5; i++) {
    float h = 0.01f + 0.25f * float(i) / 4.0f;
    float d = map(__ctx, pos + h * nor).data[0];
    occ += (h - d) * sca;
    sca *= 0.95f;
  }
  return clamp(1.0f - 3.0f * occ, 0.0f, 1.0f);
}
f32x4 render(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dist = clamp(0.f, 10.f, intersection(__ctx, ro, rd));
  f32x3 pos = dist * rd + ro;
  f32x4 d_t = map(__ctx, pos);
  f32x3 trp = d_t.swizzle<1, 2, 3>();
  float dis = d_t.data[0];
  float trd = distance(pos, trp);
  f32x3 sun = normalize(f32x3(1.f, 1.f, 1.1f));
  float sha = softshadow(__ctx, pos, sun, 0.01f, 5.f, 4.f, dist);
  f32x3 nor = calcNormal(__ctx, pos, dist);
  float occ = calcAO(__ctx, pos, nor);
  float lig = (dot(sun, nor) * 0.5f + 0.5f);
  f32x3 col = f32x3(0);
  f32x3 bcl = 0.8f * f32x3(log(trd), 0.5f, cos(trd)) * f32x3(1, 2, 2);
  f32x3 scl = f32x3(0.933f, 0.918f, 0.827f);
  f32x3 amb = f32x3(0.949f, 0.702f, 1.000f);
  f32x3 ref = reflect(rd, nor);
  f32x3 hvc = normalize(sun - rd);
  float fogAmount;
  {
    float a = rd.data[1] * 0.0125f;
    float h = ro.data[1] - 2.5f;
    float fogDist = length(rd.swizzle<0, 1>()) * dist;
    fogAmount = (exp(fogDist * a * 1.1f) - 1.f) * exp(-fogDist * a - h) / a;
  }
  float sunFog;
  {
    float a = sun.data[1] * 0.0125f;
    float h = pos.data[1] - 2.5f;
    float fogDist = length(sun.swizzle<0, 1>()) * 1000.f;
    sunFog = (exp(fogDist * a) - 1.f) * exp(-fogDist * a - h) / a;
  }
  float sunOcc = exp(-sunFog * 0.0003f);
  if (dis < 0.1f) {
    col += 4.f * lig * sha * bcl * scl * sunOcc;
    col += .2f * amb * occ * bcl * (abs(dot(nor, sun)) * .5f + .5f) * sunOcc;
    col += .2f * occ * (dot(ref, rd) + 1.f);
    col += amb * pow(dot(nor, hvc) * .5f + .5f, 5.f) * sha;
    col += occ * 2.f * f32x3(0.757f, 0.486f, 0.024f) * bcl * scl *
           (.75f + 0.25f * dot(nor, -sun)) /
           smoothstep(-2.0f, 2.f, length(pos.swizzle<0, 2>())) * sunOcc;
  } else {
    fogAmount = 1.f;
  }
  col = mix(col, amb, clamp(fogAmount, 0.f, 1.f));
  col += f32x3(0.878f, 0.412f, 0.102f) * smoothstep(-.2f, .5f, dot(rd, sun));
  return f32x4(col, 1);
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
  f32x3 ro = f32x3(0, 0, -5.5f);
  f32x2x2 yrot = rot(__ctx, iTime * 0.2f);
  f32x2x2 xrot = rot(__ctx, .9f);
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() * (xrot));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (yrot));
  f32x4 tot = f32x4(0);
  for (int m = 0; m < 1; m++) {
    for (int n = 0; n < 1; n++) {
      f32x2 o = f32x2(float(m), float(n)) / float(1) - 0.5f;
      f32x2 u = ((fragCoord + o) / iResolution.swizzle<0, 1>() - 0.5f) /
                f32x2(iResolution.data[1] / iResolution.data[0], 1);
      f32x3 rd = normalize(f32x3(u, 1));
      rd.set_swizzle<1, 2>(rd.swizzle<1, 2>() * (xrot));
      rd.set_swizzle<0, 2>(rd.swizzle<0, 2>() * (yrot));
      tot += render(__ctx, ro, rd) / float(1 * 1);
    }
  }
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>() - 0.5f;
  tot *= smoothstep(1.34f, 0.f, length(uv));
  tot = smoothstep(.0f, 1.6f, tot);
  tot = pow(tot, f32x4(1.f, 1.4f, 1.1f, 1.f));
  fragColor = f32x4(
      aces_tonemap(__ctx, pow(tot.swizzle<0, 1, 2>(), f32x3(1.15f))), 1.f);
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
