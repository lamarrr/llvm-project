// Port of https://www.shadertoy.com/view/sdcyz4; author: yasuo.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/sdcyz4.json.
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
f32x3 N33(Context &__ctx, f32x3 p) {
  f32x3 a = fract(p * f32x3(123.34f, 234.34f, 345.65f));
  a += dot(a, a + 34.45f);
  return fract(f32x3(a.data[0] * a.data[1], a.data[1] * a.data[2],
                     a.data[2] * a.data[0]));
}
float smin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.f, 1.f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
float hash(Context &__ctx, f32x2 co) {
  return fract(sin(dot(co.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453f);
}
float metaball(Context &__ctx, f32x3 p, float i, float t) {
  f32x3 n = N33(__ctx, f32x3(i));
  f32x3 p2 = sin(n * t) * 0.2f;
  f32x3 spp = p - p2;
  float sp = length(spp) - 0.01f;
  return sp;
}
f32x2 GetDist(Context &__ctx, f32x3 p) {
  float k = 0.7f;
  float d = 10.0f;
  float t = iTime * 3.0f;
  d = smin(__ctx, d, metaball(__ctx, p, 0.3f, t), k);
  d = smin(__ctx, d, metaball(__ctx, p, 0.6f, t), k);
  d = smin(__ctx, d, metaball(__ctx, p, 0.9f, t), k);
  f32x2 model = f32x2(d, 0);
  return model;
}
f32x2 RayMarch(Context &__ctx, f32x3 ro, f32x3 rd, float side, int stepnum) {
  f32x2 dO = f32x2(0.0f);
  for (int i = 0; i < stepnum; i++) {
    f32x3 p = ro + rd * dO.data[0];
    f32x2 dS = GetDist(__ctx, p);
    dO.data[0] += dS.data[0] * side;
    dO.data[1] = dS.data[1];
    if (dO.data[0] > 256.f || abs(dS.data[0]) < .0005f)
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
float sdBox(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.0f)) + min(max(d.data[0], d.data[1]), 0.0f);
}
float charS(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p.data[1] *= 1.5f;
  float d = abs(length(p - f32x2(-0.02f, 0.06f)) - 0.06f) - 0.02f;
  float d2 =
      max(abs(p - f32x2(0.03f, 0.02f)).data[0] - f32x2(0.045f, 0.04f).data[0],
          abs(p - f32x2(0.03f, 0.02f)).data[1] - f32x2(0.045f, 0.04f).data[1]);
  d = max(-d2, d);
  d2 = abs(length(p - f32x2(-0.02f, -0.06f)) - 0.06f) - 0.02f;
  float d3 = max(
      abs(p - f32x2(-0.06f, -0.02f)).data[0] - f32x2(0.045f, 0.04f).data[0],
      abs(p - f32x2(-0.06f, -0.02f)).data[1] - f32x2(0.045f, 0.04f).data[1]);
  d2 = max(-d3, d2);
  d = min(d, d2);
  return d;
}
float charH(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p.data[1] *= 1.5f;
  p.data[0] = abs(p.data[0]) - 0.06f;
  float d = max(abs(p).data[0] - f32x2(0.02f, 0.14f).data[0],
                abs(p).data[1] - f32x2(0.02f, 0.14f).data[1]);
  p = prevP;
  float d2 = max(abs(p).data[0] - f32x2(0.08f, 0.02f).data[0],
                 abs(p).data[1] - f32x2(0.08f, 0.02f).data[1]);
  d = min(d, d2);
  return d;
}
float charA(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p.data[1] *= 1.5f;
  p.data[0] = abs(p.data[0]) - 0.04f;
  p *= f32x2x2(cos(radians(-15.0f)), -sin(radians(-15.0f)),
               sin(radians(-15.0f)), cos(radians(-15.0f)));
  float d = max(abs(p).data[0] - f32x2(0.02f, 0.16f).data[0],
                abs(p).data[1] - f32x2(0.02f, 0.16f).data[1]);
  p = prevP;
  float d2 =
      max(abs(p - f32x2(0.0f, -0.03f)).data[0] - f32x2(0.05f, 0.02f).data[0],
          abs(p - f32x2(0.0f, -0.03f)).data[1] - f32x2(0.05f, 0.02f).data[1]);
  d = min(d, d2);
  d = max((abs(p.data[1]) - 0.09f), d);
  p = prevP;
  p *= f32x2x2(cos(radians(22.0f)), -sin(radians(22.0f)), sin(radians(22.0f)),
               cos(radians(22.0f)));
  d2 = max(
      abs(p - f32x2(-0.037f, -0.12f)).data[0] - f32x2(0.019f, 0.12f).data[0],
      abs(p - f32x2(-0.037f, -0.12f)).data[1] - f32x2(0.019f, 0.12f).data[1]);
  return d;
}
float charD(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p.data[1] *= 1.5f;
  float d = abs(sdBox(__ctx, p, f32x2(0.02f, 0.075f)) - 0.04f) - 0.02f;
  d = max(-p.data[0] - 0.03f, d);
  float d2 =
      max(abs(p - f32x2(-0.05f, 0.0f)).data[0] - f32x2(0.02f, 0.135f).data[0],
          abs(p - f32x2(-0.05f, 0.0f)).data[1] - f32x2(0.02f, 0.135f).data[1]);
  d = min(d, d2);
  return d;
}
float charE(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p.data[1] *= 1.5f;
  float d = max(abs(p).data[0] - f32x2(0.065f, 0.13f).data[0],
                abs(p).data[1] - f32x2(0.065f, 0.13f).data[1]);
  p.data[1] = abs(p.data[1]) - 0.055f;
  float d2 =
      max(abs(p - f32x2(0.03f, 0.0f)).data[0] - f32x2(0.065f, 0.03f).data[0],
          abs(p - f32x2(0.03f, 0.0f)).data[1] - f32x2(0.065f, 0.03f).data[1]);
  d = max(-d2, d);
  return d;
}
float charR(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p.data[1] *= 1.5f;
  float d = abs(sdBox(__ctx, p - f32x2(-0.01f, 0.05f), f32x2(0.03f, 0.022f)) -
                0.04f) -
            0.023f;
  d = max(-p.data[0] - 0.03f, d);
  float d2 =
      max(abs(p - f32x2(-0.05f, 0.0f)).data[0] - f32x2(0.02f, 0.135f).data[0],
          abs(p - f32x2(-0.05f, 0.0f)).data[1] - f32x2(0.02f, 0.135f).data[1]);
  d = min(d, d2);
  p *= f32x2x2(cos(radians(-20.0f)), -sin(radians(-20.0f)),
               sin(radians(-20.0f)), cos(radians(-20.0f)));
  d2 = max(abs(p - f32x2(0.02f, -0.14f)).data[0] - f32x2(0.02f, 0.13f).data[0],
           abs(p - f32x2(0.02f, -0.14f)).data[1] - f32x2(0.02f, 0.13f).data[1]);
  p *= f32x2x2(cos(radians(20.0f)), -sin(radians(20.0f)), sin(radians(20.0f)),
               cos(radians(20.0f)));
  d2 = max(-p.data[1] - 0.132f, d2);
  d = min(d, d2);
  return d;
}
float shaderText(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p.data[1] *= 0.9f;
  float d = charS(__ctx, p - f32x2(-0.5f, 0.0f));
  float d2 = charH(__ctx, p - f32x2(-0.34f, 0.0f));
  d = min(d, d2);
  d2 = charA(__ctx, p - f32x2(-0.14f, 0.0f));
  d = min(d, d2);
  d2 = charD(__ctx, p - f32x2(0.05f, 0.0f));
  d = min(d, d2);
  d2 = charE(__ctx, p - f32x2(0.22f, 0.0f));
  d = min(d, d2);
  d2 = charR(__ctx, p - f32x2(0.38f, 0.0f));
  d = min(d, d2);
  return d;
}
f32x3 drawBg(Context &__ctx, f32x2 p, f32x3 col) {
  float d = shaderText(__ctx, p * 1.7f);
  col = mix(col, f32x3(0.0f),
            smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                       0.0f, d));
  return col;
}
f32x3 reflectMaterial(Context &__ctx, f32x3 p, f32x3 rd, f32x3 n) {
  f32x3 r = reflect(rd, n);
  f32x3 refTex =
      drawBg(__ctx, p.swizzle<0, 1>(), f32x3(max(0.95f, r.data[1]))) +
      (r * sin(iTime) * 0.5f);
  return refTex;
}
f32x3 materials(Context &__ctx, int mat, f32x3 n, f32x3 rd, f32x3 p,
                f32x3 col) {
  if (mat == 0) {
    col = reflectMaterial(__ctx, p, rd, n);
  }
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 prevUV = uv;
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 ro = f32x3(0, 0, -1.0f);
  f32x3 rd = R(__ctx, uv, ro, f32x3(0, 0.0f, 0.0f), 1.0f);
  f32x2 d = RayMarch(__ctx, ro, rd, 1.f, 256);
  f32x3 col = f32x3(1.0f);
  if (d.data[0] < 256.f) {
    f32x3 p = ro + rd * d.data[0];
    f32x3 n = GetNormal(__ctx, p);
    int mat = int(d.data[1]);
    col = materials(__ctx, mat, n, rd, p, col);
  } else {
    col = drawBg(__ctx, uv, col);
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
