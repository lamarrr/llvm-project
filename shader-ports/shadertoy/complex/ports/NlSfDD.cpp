// Port of https://www.shadertoy.com/view/NlSfDD; author: yasuo.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NlSfDD.json.
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
f32x2 combine(Context &__ctx, f32x2 val1, f32x2 val2) {
  return (val1.data[0] < val2.data[0]) ? val1 : val2;
}
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
float sdCutHollowSphere(Context &__ctx, f32x3 p, float r, float h, float t) {
  float w = sqrt(r * r - h * h);
  f32x2 q = f32x2(length(p.swizzle<0, 2>()), p.data[1]);
  return ((h * q.data[0] < w * q.data[1]) ? length(q - f32x2(w, h))
                                          : abs(length(q) - r)) -
         t;
}
float sdTorus(Context &__ctx, f32x3 p, f32x2 t) {
  f32x2 q = f32x2(length(p.swizzle<0, 2>()) - t.data[0], p.data[1]);
  return length(q) - t.data[1];
}
float hash(Context &__ctx, f32x2 co) {
  return fract(sin(dot(co.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453f);
}
float randomTeaCups(Context &__ctx, f32x3 p, float m) {
  f32x3 prevP = p;
  f32x2 id = floor(p.swizzle<0, 2>() / m);
  p.set_swizzle<0, 2>(mod(p.swizzle<0, 2>(), m) - m * 0.5f);
  float time = iTime * 0.0f;
  float h = 1.0f + clamp(hash(__ctx, id), .1f, .7f) * 3.1415f * 2.3f;
  p.data[1] -= h - 1.5f;
  float size = 3.0f;
  float d = sdBox(__ctx, p, f32x3(h * 0.03f, h, h * 0.03f));
  float dir = 1.0f;
  float n = hash(__ctx, id);
  if (n < 0.5f)
    dir = -1.0f;
  p.set_swizzle<0, 2>(
      p.swizzle<0, 2>() *
      (f32x2x2(cos(radians(45.0f + n * 20.0f + iTime * 100.0f * dir)),
               -sin(radians(45.0f + n * 20.0f + iTime * 100.0f * dir)),
               sin(radians(45.0f + n * 20.0f + iTime * 100.0f * dir)),
               cos(radians(45.0f + n * 20.0f + iTime * 100.0f * dir)))));
  float d2 = sdCutHollowSphere(
      __ctx, p - f32x3(h * 0.25f + 3.0f, -h + size * 1.7f, 0.0f), size * 1.7f,
      0.1f, 0.01f);
  p -= f32x3(h * 0.25f + 3.0f, -h + size * 0.8f, 0.0f);
  float d3 = sdCutHollowSphere(__ctx, p, size * 0.8f, 0.5f, 0.05f);
  float maskD3 = sdCutHollowSphere(__ctx, p, size * 0.6f, 0.5f, 0.6f);
  p.data[1] *= 0.8f;
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() *
                      (f32x2x2(cos(radians(90.0f)), -sin(radians(90.0f)),
                               sin(radians(90.0f)), cos(radians(90.0f)))));
  float d4 =
      sdTorus(__ctx, p - f32x3(0.0f, 0.6f, size - 0.3f), f32x2(0.5f, 0.15f));
  d4 = max(-maskD3, d4);
  p = prevP;
  d2 = max(p.data[1] + 0.5f, d2);
  d = min(d, d2);
  d = min(d, d3);
  d = min(d, d4);
  return d * 0.4f;
}
f32x2 GetDist(Context &__ctx, f32x3 p) {
  f32x3 prevP = p;
  p = prevP;
  p.data[2] += iTime * 5.0f;
  p.data[1] += 5.0f;
  float d = randomTeaCups(__ctx, p, 16.0f);
  p = prevP;
  f32x2 model = f32x2(d * 0.6f, 0);
  f32x2 model2 = f32x2(p.data[1] + 6.5f, 1);
  return combine(__ctx, model, model2);
}
f32x2 RayMarch(Context &__ctx, f32x3 ro, f32x3 rd, float side, int stepnum) {
  f32x2 dO = f32x2(0.0f);
  for (int i = 0; i < stepnum; i++) {
    f32x3 p = ro + rd * dO.data[0];
    f32x2 dS = GetDist(__ctx, p);
    dO.data[0] += dS.data[0] * side;
    dO.data[1] = dS.data[1];
    if (dO.data[0] > 128.f || abs(dS.data[0]) < .001f)
      break;
  }
  return dO;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p).data[0];
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()).data[0]);
  return normalize(n);
}
f32x3 R(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = p + f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i - p);
  return d;
}
float calcOcclusion(Context &__ctx, f32x3 pos, f32x3 nor) {
  float occ = 0.0f;
  float sca = 1.0f;
  for (int i = (min(iFrame, 0)); i < 4; i++) {
    float h = 0.01f + 0.15f * float(i) / 4.0f;
    f32x3 opos = pos + h * nor;
    float d = GetDist(__ctx, opos).data[0];
    occ += (h - d) * sca;
    sca *= 0.95f;
  }
  return clamp(1.0f - 2.0f * occ, 0.0f, 1.0f);
}
f32x3 diffuseMaterial(Context &__ctx, f32x3 n, f32x3 rd, f32x3 p, f32x3 col) {
  float occ = calcOcclusion(__ctx, p, n);
  f32x3 diffCol = f32x3(0.0f);
  f32x3 lightDir = normalize(f32x3(10, 10, -10));
  float diff = clamp(dot(n, lightDir), 0.0f, 1.0f);
  float shadow =
      step(RayMarch(__ctx, p + n * 0.3f, lightDir, 1.0f, 15).data[0], 0.9f);
  float skyDiff = clamp(0.5f + 0.5f * dot(n, f32x3(0, 1, 0)), 0.0f, 1.0f);
  float bounceDiff = clamp(0.5f + 0.5f * dot(n, f32x3(0, -1, 0)), 0.0f, 1.0f);
  diffCol = col * f32x3(-0.5f) * diff * shadow * occ;
  diffCol += col * f32x3(0.9f) * skyDiff * occ;
  diffCol += col * f32x3(0.3f, 0.3f, 0.3f) * bounceDiff * occ;
  diffCol += col * pow(max(dot(rd, reflect(lightDir, n)), 0.0f), 60.f) * occ;
  return diffCol;
}
f32x3 materials(Context &__ctx, int mat, f32x3 n, f32x3 rd, f32x3 p,
                f32x3 col) {
  f32x3 prevP = p;
  if (mat == 0) {
    f32x3 c = f32x3(0.9f);
    p.data[0] += iTime * 2.0f;
    c += hash(__ctx, p.swizzle<0, 2>() * 50.0f) * 0.3f;
    p = prevP;
    col = diffuseMaterial(__ctx, n, rd, p, c);
  } else if (mat == 1) {
    p.data[2] += iTime * 5.0f;
    p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (0.1f));
    f32x2 c = f32x2(mod(floor(p.data[0]) + floor(p.data[2]), 2.0f));
    f32x3 c2 = f32x3(c.data[0], c.data[0], c.data[1]);
    p.data[0] += iTime * 2.0f;
    c2 += hash(__ctx, p.swizzle<0, 2>() * 50.0f) * 0.3f;
    p = prevP;
    col = diffuseMaterial(__ctx, n, rd, p, c2);
  }
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 prevUV = uv;
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 ro = f32x3(0, 5.0f, -5.f);
  if (iMouse.data[2] > 0.f) {
    ro.set_swizzle<1, 2>(
        ro.swizzle<1, 2>() *
        (f32x2x2(cos(m.data[1] * 3.14f + 1.f), -sin(m.data[1] * 3.14f + 1.f),
                 sin(m.data[1] * 3.14f + 1.f), cos(m.data[1] * 3.14f + 1.f))));
    ro.data[1] = max(-0.9f, ro.data[1]);
    ro.set_swizzle<0, 2>(
        ro.swizzle<0, 2>() *
        (f32x2x2(cos(-m.data[0] * 6.2831f), -sin(-m.data[0] * 6.2831f),
                 sin(-m.data[0] * 6.2831f), cos(-m.data[0] * 6.2831f))));
  } else {
    ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() *
                         (f32x2x2(cos(radians(20.0f)), -sin(radians(20.0f)),
                                  sin(radians(20.0f)), cos(radians(20.0f)))));
  }
  f32x3 rd = R(__ctx, uv, ro, f32x3(0, 3, 0), 0.8f);
  f32x2 d = RayMarch(__ctx, ro, rd, 1.f, 256);
  f32x3 col = f32x3(0.3f);
  if (d.data[0] < 128.f) {
    f32x3 p = ro + rd * d.data[0];
    f32x3 n = GetNormal(__ctx, p);
    int mat = int(d.data[1]);
    col = materials(__ctx, mat, n, rd, p, col);
    col *= exp(-0.00001f * d.data[0] * d.data[0]);
  } else {
    uv.data[0] += iTime * 2.0f;
    col += hash(__ctx, uv * 100.0f) * 0.1f;
  }
  col = pow(col, f32x3(0.9545f));
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
