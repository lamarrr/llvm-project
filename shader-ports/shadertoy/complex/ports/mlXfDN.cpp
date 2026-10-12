// Port of https://www.shadertoy.com/view/mlXfDN; author: AYAYA2.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mlXfDN.json.
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
float dot2(Context &__ctx, f32x2 v) { return dot(v, v); }
float dot2(Context &__ctx, f32x3 v) { return dot(v, v); }
float ndot(Context &__ctx, f32x2 a, f32x2 b) {
  return a.data[0] * b.data[0] - a.data[1] * b.data[1];
}
float Pow4(Context &__ctx, float x) {
  float xx = x * x;
  return xx * xx;
}
float Pow5(Context &__ctx, float x) { return Pow4(__ctx, x) * x; }
f32x3 lerp(Context &__ctx, f32x3 a, f32x3 b, float c) {
  float x = mix(a.data[0], b.data[0], c);
  float y = mix(a.data[1], b.data[1], c);
  float z = mix(a.data[2], b.data[2], c);
  return f32x3(x, y, z);
}
float sdPlane(Context &__ctx, f32x3 p) { return p.data[1]; }
float dSphere(Context &__ctx, f32x3 p, f32x3 o, float radius) {
  return length(p - o.swizzle<0, 1, 2>()) - radius;
}
f32x2 opUnion(Context &__ctx, f32x2 d1, f32x2 d2) {
  return d1.data[0] < d2.data[0] ? d1 : d2;
}
f32x2 GetDist(Context &__ctx, f32x3 p) {
  float planeDist = sdPlane(__ctx, p);
  float sd = dSphere(__ctx, p, f32x3(0, 1, 3), 1.25f);
  f32x2 hit = opUnion(__ctx, f32x2(planeDist, 0), f32x2(sd, 1));
  return hit;
}
f32x2 RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.f;
  float MaterialID = 0.0f;
  for (int i = 0; i < 1000; i++) {
    f32x3 p = ro + rd * dO;
    f32x2 dS = GetDist(__ctx, p);
    dO += dS.data[0];
    MaterialID = dS.data[1];
    if (dO > 1000.f || dS.data[0] < .01f)
      break;
  }
  return f32x2(dO, MaterialID);
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p).data[0];
  f32x2 e = f32x2(.01f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()).data[0]);
  return normalize(n);
}
float GetLight(Context &__ctx, f32x3 p, f32x3 lightPos) {
  f32x3 l = normalize(lightPos - p);
  f32x3 n = GetNormal(__ctx, p);
  float dif = clamp(dot(n, l), 0.f, 1.f);
  float d = RayMarch(__ctx, p + n * .01f * 2.f, l).data[0];
  if (d < length(lightPos - p))
    dif *= .1f;
  return dif;
}
float GetShadow(Context &__ctx, f32x3 P, f32x3 LightPos) {
  f32x3 L = normalize(LightPos - P);
  f32x3 N = GetNormal(__ctx, P);
  float Shadow = 1.0f;
  float Dist = RayMarch(__ctx, P + N * .01f * 2.f, L).data[0];
  if (Dist < length(LightPos - P))
    Shadow = 0.0f;
  return Shadow;
}
float D_GGX(Context &__ctx, float a2, float NoH) {
  float d = (NoH * a2 - NoH) * NoH + 1.0f;
  return a2 / (3.14159265359f * d * d);
}
float Vis_SmithJointApprox(Context &__ctx, float a2, float NoV, float NoL) {
  float a = sqrt(a2);
  float Vis_SmithV = NoL * (NoV * (1.0f - a) + a);
  float Vis_SmithL = NoV * (NoL * (1.0f - a) + a);
  return 0.5f * 1.0f / (Vis_SmithV + Vis_SmithL);
}
f32x3 F_Schlick(Context &__ctx, f32x3 SpecularColor, float VoH) {
  float Fc = Pow5(__ctx, 1.0f - VoH);
  return clamp(50.0f * SpecularColor.data[1], 0.0f, 1.0f) * Fc +
         (1.0f - Fc) * SpecularColor;
}
float DielectricSpecularToF0(Context &__ctx, float Specular) {
  return 0.08f * Specular;
}
f32x3 ComputeF0(Context &__ctx, float Specular, f32x3 BaseColor,
                float Metallic) {
  return lerp(__ctx, f32x3(DielectricSpecularToF0(__ctx, Specular)), BaseColor,
              Metallic);
}
f32x3 Tonemap_ACES(Context &__ctx, const f32x3 x) {
  const float a = 2.51f;
  const float b = 0.03f;
  const float c = 2.43f;
  const float d = 0.59f;
  const float e = 0.14f;
  return (x * (a * x + b)) / (x * (c * x + d) + e);
}
f32x3 OECF_sRGBFast(Context &__ctx, const f32x3 linear) {
  return pow(linear, f32x3(1.0f / 2.2f));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 BaseColor = f32x3(1.0f, 1.0f, 1.0f);
  float Specular = 0.9f;
  float Roughness = 0.9f;
  float Metalic = 0.9f;
  f32x3 Ray0 = f32x3(0, 3, 0);
  f32x3 LightPos = f32x3(0, 3, 1);
  LightPos.data[0] += sin(iTime) * 2.5f;
  f32x2 UV =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 RayDir = normalize(f32x3(UV.data[0], UV.data[1] - .3f, .35f));
  f32x2 Dist = RayMarch(__ctx, Ray0, RayDir);
  f32x3 Pos = Ray0 + RayDir * Dist.data[0];
  f32x3 N = GetNormal(__ctx, Pos);
  f32x3 V = normalize(Ray0 - Pos);
  f32x3 L = normalize(LightPos - Pos);
  f32x3 H = normalize(L + V);
  float NoV = abs(dot(N, V)) + 1e-5f;
  float NoL = clamp(dot(N, L), 0.0f, 1.0f);
  float NoH = clamp(dot(N, H), 0.0f, 1.0f);
  float LoH = clamp(dot(L, H), 0.0f, 1.0f);
  float VoH = clamp(dot(V, H), 0.0f, 1.0f);
  float a2 = Pow4(__ctx, Roughness);
  float D = D_GGX(__ctx, a2, NoH);
  float G = Vis_SmithJointApprox(__ctx, a2, NoV, NoL);
  f32x3 DiffuseColor = BaseColor - BaseColor * Metalic;
  f32x3 LightingDiffuse = DiffuseColor * (1.0f / 3.14159265359f);
  f32x3 SpecularColor = ComputeF0(__ctx, Specular, BaseColor, Metalic);
  f32x3 F = F_Schlick(__ctx, SpecularColor, VoH);
  f32x3 LightingSpecular = f32x3(G);
  f32x3 FinalColor = LightingSpecular;
  float MaterialID = Dist.data[1];
  if (MaterialID < 1.0f)
    FinalColor = f32x3(0.3f);
  float Shadow = GetShadow(__ctx, Pos, LightPos);
  FinalColor *= Shadow;
  FinalColor = Tonemap_ACES(__ctx, FinalColor);
  FinalColor = OECF_sRGBFast(__ctx, FinalColor);
  fragColor = f32x4(FinalColor, 1.0f);
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
