// Port of https://www.shadertoy.com/view/DlB3WG; author: mrange.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DlB3WG.json.
#include "../support.h"
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
const float planeDist = 1.0f - 0.80f;
const int furthest = 16;
const int fadeFrom = max(furthest - 4, 0);
const float fadeDist = planeDist * float(furthest - fadeFrom);
const float overSample = 4.0f;
const float ringDistance = 0.075f * overSample / 4.0f;
const float noOfRings = 20.0f * 4.0f / overSample;
const float glowFactor = 0.05f;
f32x4 alphaBlend(Context &__ctx, f32x4 back, f32x4 front) {
  float w = front.data[3] + back.data[3] * (1.0f - front.data[3]);
  f32x3 xyz =
      (front.swizzle<0, 1, 2>() * front.data[3] +
       back.swizzle<0, 1, 2>() * back.data[3] * (1.0f - front.data[3])) /
      w;
  return w > 0.0f ? f32x4(xyz, w) : f32x4(0.0f);
}
f32x3 alphaBlend(Context &__ctx, f32x3 back, f32x4 front) {
  return mix(back, front.swizzle<0, 1, 2>(), front.data[3]);
}
float tanh_approx(Context &__ctx, float x) {
  float x2 = x * x;
  return clamp(x * (27.0f + x2) / (27.0f + 9.0f * x2), -1.0f, 1.0f);
}
float hash(Context &__ctx, float co) {
  return fract(sin(co * 12.9898f) * 13758.5453f);
}
f32x3 offset(Context &__ctx, float z) {
  float a = z;
  f32x2 p = -0.15f * (f32x2(cos(a), sin(a * sqrt(2.0f))) +
                      f32x2(cos(a * sqrt(0.75f)), sin(a * sqrt(0.5f))));
  return f32x3(p, z);
}
f32x3 doffset(Context &__ctx, float z) {
  float eps = 0.05f;
  return 0.5f * (offset(__ctx, z + eps) - offset(__ctx, z - eps)) /
         (2.0f * eps);
}
f32x3 ddoffset(Context &__ctx, float z) {
  float eps = 0.05f;
  return 0.5f * (doffset(__ctx, z + eps) - doffset(__ctx, z - eps)) /
         (2.0f * eps);
}
f32x3 skyColor(Context &__ctx, f32x3 ro, f32x3 rd) { return f32x3(0.0f); }
float mod1(Context &__ctx, float &p, float size) {
  float halfsize = size * 0.5f;
  float c = floor((p + halfsize) / size);
  p = mod(p + halfsize, size) - halfsize;
  return c;
}
float atan_approx(Context &__ctx, float y, float x) {
  float cosatan2 = x / (abs(x) + abs(y));
  float t = (0.5f * 3.141592654f) - cosatan2 * (0.5f * 3.141592654f);
  return y < 0.0f ? -t : t;
}
f32x2 toPolar(Context &__ctx, f32x2 p) {
  return f32x2(length(p), atan_approx(__ctx, p.data[1], p.data[0]));
}
f32x3 glow(Context &__ctx, f32x2 pp, float h) {
  float hh = fract(h * 8677.0f);
  float b =
      (2.0f * 3.141592654f) * h + 0.5f * iTime * (hh > 0.5f ? 1.0f : -1.0f);
  float a = pp.data[1] + b;
  float d = max(abs(pp.data[0]) - 0.001f, 0.00125f);
  return (smoothstep(0.667f * ringDistance, 0.2f * ringDistance, d) *
          smoothstep(0.1f, 1.0f, cos(a)) * glowFactor * ringDistance / d) *
         (cos(a + b + f32x3(0, 1, 2)) + f32x3(1.0f));
}
f32x3 glowRings(Context &__ctx, f32x2 p, float hh) {
  f32x2 pp = toPolar(__ctx, p);
  f32x3 col = f32x3(0.0f);
  float h = 1.0f;
  const float nr = 1.0f / overSample;
  for (float i = 0.0f; i < overSample; ++i) {
    f32x2 ipp = pp;
    ipp.data[0] -= ringDistance * (nr * i);
    float rn = mod1(__ctx, ipp.data[0], ringDistance);
    h = hash(__ctx, rn + 123.0f * i);
    col += glow(__ctx, ipp, h) * step(rn, noOfRings);
  }
  col += (0.01f * f32x3(1.0f, 0.25f, 0.0f)) / length(p);
  return col;
}
f32x4 plane(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 pp, f32x3 off, float aa,
            float n) {
  float h = hash(__ctx, n + 123.4f);
  f32x3 hn;
  f32x2 p = (pp - off * f32x3(1.0f, 1.0f, 0.0f)).swizzle<0, 1>();
  float l = length(p);
  float fade = smoothstep(0.1f, 0.15f, l);
  if (fade < 0.1f)
    return f32x4(0.0f);
  f32x4 col = f32x4(0.0f);
  col.set_swizzle<0, 1, 2>(glowRings(__ctx, p * mix(0.5f, 4.0f, h), h));
  float i = max(max(col.data[0], col.data[1]), col.data[2]);
  col.data[3] = (tanh_approx(__ctx, 0.5f + max((i), 0.0f)) * fade);
  return col;
}
f32x3 color(Context &__ctx, f32x3 ww, f32x3 uu, f32x3 vv, f32x3 ro, f32x2 p) {
  float lp = length(p);
  f32x2 np = p + 1.0f / iResolution.swizzle<0, 1>();
  const float rdd_per = 10.0f;
  float rdd =
      (1.75f +
       0.75f * pow(lp, 1.5f) *
           tanh_approx(__ctx,
                       lp + 0.9f * 0.5f * (cos(rdd_per * p.data[0]) + 1.0f) *
                                0.5f * (cos(rdd_per * p.data[1]) + 1.0f)));
  f32x3 rd = normalize(p.data[0] * uu + p.data[1] * vv + rdd * ww);
  f32x3 nrd = normalize(np.data[0] * uu + np.data[1] * vv + rdd * ww);
  float nz = floor(ro.data[2] / planeDist);
  f32x3 skyCol = skyColor(__ctx, ro, rd);
  f32x4 acol = f32x4(0.0f);
  const float cutOff = 0.95f;
  bool cutOut = false;
  float maxpd = 0.0f;
  for (int i = 1; i <= furthest; ++i) {
    float pz = planeDist * nz + planeDist * float(i);
    float pd = (pz - ro.data[2]) / rd.data[2];
    if (pd > 0.0f && acol.data[3] < cutOff) {
      f32x3 pp = ro + rd * pd;
      maxpd = pd;
      f32x3 npp = ro + nrd * pd;
      float aa = 3.0f * length(pp - npp);
      f32x3 off = offset(__ctx, pp.data[2]);
      f32x4 pcol = plane(__ctx, ro, rd, pp, off, aa, nz + float(i));
      float nz = pp.data[2] - ro.data[2];
      float fadeIn = smoothstep(planeDist * float(furthest),
                                planeDist * float(fadeFrom), nz);
      float fadeOut = smoothstep(0.0f, planeDist * 0.1f, nz);
      pcol.data[3] *= fadeOut * fadeIn;
      pcol = clamp(pcol, 0.0f, 1.0f);
      acol = alphaBlend(__ctx, pcol, acol);
    } else {
      cutOut = true;
      acol.data[3] = acol.data[3] > cutOff ? 1.0f : acol.data[3];
      break;
    }
  }
  f32x3 col = alphaBlend(__ctx, skyCol, acol);
  return col;
}
f32x3 effect(Context &__ctx, f32x2 p, f32x2 q) {
  float tm = planeDist * iTime * (157.0f / 4.0f) / 60.0f;
  f32x3 ro = offset(__ctx, tm);
  f32x3 dro = doffset(__ctx, tm);
  f32x3 ddro = ddoffset(__ctx, tm);
  f32x3 ww = normalize(dro);
  f32x3 uu = normalize(cross(normalize(f32x3(0.0f, 1.0f, 0.0f) + ddro), ww));
  f32x3 vv = cross(ww, uu);
  f32x3 col = color(__ctx, ww, uu, vv, ro, p);
  col -= 0.075f * f32x3(2.0f, 3.0f, 1.0f);
  col *= sqrt(2.0f);
  col = clamp(col, 0.0f, 1.0f);
  col = sqrt(col);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 q = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 p = -1.f + 2.f * q;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 col = effect(__ctx, p, q);
  fragColor = f32x4(col, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output) {
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
