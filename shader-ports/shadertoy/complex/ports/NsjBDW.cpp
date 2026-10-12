// Port of https://www.shadertoy.com/view/NsjBDW; author: yasuo.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NsjBDW.json.
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
float B3D(Context &__ctx, f32x3 p, f32x3 s) {
  p = abs(p) - s;
  return max(max(p.data[0], p.data[1]), p.data[2]);
}
f32x3 path(Context &__ctx, float z) {
  f32x3 p = f32x3(sin(z) * .6f, cos(z * .3f), z);
  p.data[0] += sin(z * 0.12f) * 2.0f;
  return p;
}
f32x2 GetDist(Context &__ctx, f32x3 p) {
  f32x3 prevP = p;
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() -
                      (path(__ctx, p.data[2]).swizzle<0, 1>()));
  p.data[2] += iTime * 2.0f;
  float d = -length(p.swizzle<0, 1>()) + 3.0f;
  d = abs(d) - 0.1f;
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() *
                      (f32x2x2(cos(iTime * 0.0001f), -sin(iTime * 0.0001f),
                               sin(iTime * 0.0001f), cos(iTime * 0.0001f))));
  p *= 3.0f;
  float d2 = abs(0.5f * dot(sin(p), cos(p.swizzle<1, 2, 0>())) / 3.0f) - 0.15f;
  p = prevP;
  p.data[2] += iTime * 2.0f;
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() *
                      (f32x2x2(cos(iTime * 0.0001f), -sin(iTime * 0.0001f),
                               sin(iTime * 0.0001f), cos(iTime * 0.0001f))));
  p *= 3.5f;
  float d3 = abs(0.5f * dot(sin(p), cos(p.swizzle<1, 2, 0>())) / 3.5f) - 0.1f;
  p = prevP;
  return combine(__ctx, f32x2(0.7f * max(-d2, d), 0),
                 f32x2(0.7f * max(-d3, d), 1));
}
f32x2 RayMarch(Context &__ctx, f32x3 ro, f32x3 rd, float side, int stepnum) {
  f32x2 dO = f32x2(0.0f);
  for (int i = 0; i < stepnum; i++) {
    f32x3 p = ro + rd * dO.data[0];
    f32x2 dS = GetDist(__ctx, p);
    dO.data[0] += dS.data[0] * side;
    dO.data[1] = dS.data[1];
    if (dO.data[0] > 32.f || abs(dS.data[0]) < .001f)
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
  f32x3 lightDir = normalize(f32x3(p.data[0], p.data[1], -2));
  float diff = clamp(dot(n, lightDir), 0.0f, 1.0f);
  float skyDiff = clamp(0.5f + 0.5f * dot(n, f32x3(0, 1, 0)), 0.0f, 1.0f);
  float bounceDiff = clamp(0.5f + 0.5f * dot(n, f32x3(0, -1, 0)), 0.0f, 1.0f);
  diffCol = col * f32x3(-0.5f) * diff * occ;
  diffCol += col * f32x3(0.9f) * skyDiff * occ;
  diffCol += col * f32x3(0.7f) * bounceDiff * occ;
  diffCol += col * pow(max(dot(rd, reflect(lightDir, n)), 0.0f), 20.f) * occ;
  return diffCol;
}
f32x3 materials(Context &__ctx, int mat, f32x3 n, f32x3 rd, f32x3 p,
                f32x3 col) {
  if (mat == 0) {
    col = diffuseMaterial(__ctx, n, rd, p, f32x3(0.8f, 0.3f, 0.3f));
  } else if (mat == 1) {
    col = diffuseMaterial(__ctx, n, rd, p, f32x3(0.3f, 0.3f, 0.8f));
  }
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  float t = iTime * 2.0f;
  f32x3 ro = path(__ctx, t + 1.5f);
  f32x3 rd = R(__ctx, uv, ro, f32x3(0, 0.0f, 0), 1.0f);
  f32x2 d = RayMarch(__ctx, ro, rd, 1.f, 80);
  f32x3 col = f32x3(1.0f);
  if (d.data[0] < 32.f) {
    f32x3 p = ro + rd * d.data[0];
    f32x3 n = GetNormal(__ctx, p);
    int mat = int(d.data[1]);
    col = materials(__ctx, mat, n, rd, p, col);
    col *= exp(-0.0001f * d.data[0] * d.data[0] * d.data[0] * d.data[0]);
  } else {
    col = f32x3(0.0f);
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
