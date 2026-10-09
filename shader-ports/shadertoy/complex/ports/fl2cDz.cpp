// Port of https://www.shadertoy.com/view/fl2cDz; author: yasuo.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fl2cDz.json.
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
f32x2 hash22(Context &__ctx, f32x2 p) {
  p = p * f32x2x2(127.1f, 311.7f, 269.5f, 183.3f);
  p = -1.0f + 2.0f * fract(sin(p) * 43758.5453123f);
  return sin(p * 6.283f);
}
float perlin_noise(Context &__ctx, f32x2 p) {
  f32x2 pi = floor(p);
  f32x2 pf = p - pi;
  f32x2 w = pf * pf * (3.0f - 2.0f * pf);
  float f00 =
      dot(hash22(__ctx, pi + f32x2(0.0f, 0.0f)), pf - f32x2(0.0f, 0.0f));
  float f01 =
      dot(hash22(__ctx, pi + f32x2(0.0f, 1.0f)), pf - f32x2(0.0f, 1.0f));
  float f10 =
      dot(hash22(__ctx, pi + f32x2(1.0f, 0.0f)), pf - f32x2(1.0f, 0.0f));
  float f11 =
      dot(hash22(__ctx, pi + f32x2(1.0f, 1.0f)), pf - f32x2(1.0f, 1.0f));
  float xm1 = mix(f00, f10, w.data[0]);
  float xm2 = mix(f01, f11, w.data[0]);
  float ym = mix(xm1, xm2, w.data[1]);
  return ym;
}
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
f32x2 GetDist(Context &__ctx, f32x3 p) {
  p *= 0.8f;
  f32x3 prevP = p;
  p = prevP;
  p.data[0] += iTime * 2.1f;
  float n = perlin_noise(__ctx, p.swizzle<0, 2>() * 0.3f) * 3.0f;
  p = prevP;
  p.data[1] += n - 0.5f;
  p.data[0] += sin(n * 0.1f) * 0.6f;
  float d = length(p) - 0.5f;
  d = sdBox(__ctx, p, f32x3(1.2f, 0.01f, 0.9f));
  p = prevP;
  float f = (p.data[1] + n) + 0.5f;
  p = prevP;
  f32x2 model = f32x2(f * 0.6f, 0);
  f32x2 model1 = f32x2(d * 0.6f, 1);
  return combine(__ctx, model, model1);
}
f32x2 RayMarch(Context &__ctx, f32x3 ro, f32x3 rd, float side, int stepnum) {
  f32x2 dO = f32x2(0.0f);
  for (int i = 0; i < stepnum; i++) {
    f32x3 p = ro + rd * dO.data[0];
    f32x2 dS = GetDist(__ctx, p);
    dO.data[0] += dS.data[0] * side;
    dO.data[1] = dS.data[1];
    if (dO.data[0] > 100.f || abs(dS.data[0]) < .001f)
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
f32x3 diffuseMaterial(Context &__ctx, f32x3 n, f32x3 rd, f32x3 p, f32x3 col) {
  f32x3 diffCol = f32x3(0.0f);
  f32x3 lightDir = normalize(f32x3(1, 10, -10));
  float diff = clamp(dot(n, lightDir), 0.0f, 1.0f);
  float shadow =
      step(RayMarch(__ctx, p + n * 0.3f, lightDir, 1.0f, 15).data[0], 0.9f);
  float skyDiff = clamp(0.5f + 0.5f * dot(n, f32x3(0, 1, 0)), 0.0f, 1.0f);
  float bounceDiff = clamp(0.5f + 0.5f * dot(n, f32x3(0, -1, 0)), 0.0f, 1.0f);
  diffCol = col * f32x3(-0.5f) * diff * shadow;
  diffCol += col * f32x3(1.0f, 1.0f, 0.9f) * skyDiff;
  diffCol += col * f32x3(0.3f, 0.3f, 0.3f) * bounceDiff;
  diffCol += col * pow(max(dot(rd, reflect(lightDir, n)), 0.0f), 60.f);
  return diffCol;
}
float pattern(Context &__ctx, f32x2 p, float rot) {
  p *= f32x2x2(cos(radians(rot)), -sin(radians(rot)), sin(radians(rot)),
               cos(radians(rot)));
  float size = 0.1f;
  float d = max(abs(p).data[0] - f32x2(size).data[0],
                abs(p).data[1] - f32x2(size).data[1]);
  for (int i = 0; i < 3; i++) {
    f32x2 q = abs(p);
    q -= 0.15f;
    d = min(d, max(abs(q).data[0] - f32x2(size).data[0],
                   abs(q).data[1] - f32x2(size).data[1]));
  }
  return abs(d) - 0.02f;
}
float pattern2(Context &__ctx, f32x2 p, float rot) {
  p *= f32x2x2(cos(radians(rot)), -sin(radians(rot)), sin(radians(rot)),
               cos(radians(rot)));
  float size = 0.1f;
  p.data[0] *= 2.0f;
  float d = max(abs(p).data[0] - f32x2(size).data[0],
                abs(p).data[1] - f32x2(size).data[1]);
  for (int i = 0; i < 3; i++) {
    f32x2 q = abs(p);
    q -= 0.15f;
    q.data[0] *= 2.0f;
    d = min(d, max(abs(q).data[0] - f32x2(size).data[0],
                   abs(q).data[1] - f32x2(size).data[1]));
  }
  return abs(d) - 0.05f;
}
float pattern3(Context &__ctx, f32x2 p, float rot) {
  p *= f32x2x2(cos(radians(rot)), -sin(radians(rot)), sin(radians(rot)),
               cos(radians(rot)));
  float size = 0.1f;
  float d = length(p) - size;
  for (int i = 0; i < 3; i++) {
    f32x2 q = abs(p);
    q -= 0.15f;
    d = min(d, length(p) - size);
  }
  return abs(d) - 0.02f;
}
f32x3 magicCarpetTex(Context &__ctx, f32x3 p) {
  f32x2 uv = p.swizzle<0, 2>();
  uv.data[0] *= 0.85f;
  uv.data[1] *= 0.8f;
  f32x2 prevUV = uv;
  f32x3 col = f32x3(0.5f, 0.0f, 0.1f);
  uv = abs(uv) - 0.15f;
  uv = abs(uv) - 0.15f;
  float d = pattern(__ctx, uv, 35.0f);
  float d2 = pattern3(__ctx, uv, 40.0f);
  d = min(d, d2);
  uv.data[0] = abs(uv.data[0]) - 0.6f;
  d2 = pattern2(__ctx, uv, -30.0f);
  d = min(d, d2);
  uv = prevUV;
  col = mix(col, f32x3(0.2f, 0.5f, 0.8f),
            smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                       0.0f, d));
  d = max(abs(uv).data[0] - f32x2(1.2f, 0.8f).data[0],
          abs(uv).data[1] - f32x2(1.2f, 0.8f).data[1]);
  d = abs(d) - 0.02f;
  col = mix(col, f32x3(0.9f, 0.5f, 0.2f),
            smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                       0.0f, d));
  return col;
}
f32x3 materials(Context &__ctx, int mat, f32x3 n, f32x3 rd, f32x3 p,
                f32x3 col) {
  if (mat == 0) {
    col = diffuseMaterial(__ctx, n, rd, p, f32x3(0.9f, 0.9f, 0.3f));
  } else if (mat == 1) {
    col = diffuseMaterial(__ctx, n, rd, p, magicCarpetTex(__ctx, p));
  }
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 prevUV = uv;
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 ro = f32x3(0, 6, -3.f) * .6f;
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() *
                       (f32x2x2(cos(radians(-30.0f)), -sin(radians(-30.0f)),
                                sin(radians(-30.0f)), cos(radians(-30.0f)))));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() *
                       (f32x2x2(cos(radians(-5.0f + iTime * 20.0f)),
                                -sin(radians(-5.0f + iTime * 20.0f)),
                                sin(radians(-5.0f + iTime * 20.0f)),
                                cos(radians(-5.0f + iTime * 20.0f)))));
  f32x3 rd = R(__ctx, uv, ro, f32x3(0, 0.0f, 0), 1.0f);
  f32x2 d = RayMarch(__ctx, ro, rd, 1.f, 100);
  f32x3 col = f32x3(0.6f);
  if (d.data[0] < 100.f) {
    f32x3 p = ro + rd * d.data[0];
    f32x3 n = GetNormal(__ctx, p);
    int mat = int(d.data[1]);
    col = materials(__ctx, mat, n, rd, p, col);
    col *= exp(0.001f * d.data[0] * d.data[0]);
  } else {
  }
  col = pow(col, f32x3(0.9545f));
  uv.data[0] += iTime * 2.0f;
  col.data[0] +=
      hash22(__ctx, uv).data[0] * 3.0f * hash22(__ctx, uv).data[1] * 3.0f;
  fragColor = f32x4(col, 1.0f);
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
