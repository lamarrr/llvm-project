// Port of https://www.shadertoy.com/view/DtBXzz; author: VPaltoDance.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtBXzz.json.
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
struct LightBuffer {
  f32x3 worldPos;
  f32x3 normal;
  f32x3 tangent;
  f32x3 bitangent;
  f32x3 viewDir;
  f32x3 lightDir;
  f32x3 lightCol;
  f32x3 halfVec;
  float lambert;
};
struct MaterialBuffer {
  f32x3 diffuse;
  f32x3 specular;
  float roughness;
  float metalness;
  float shiness;
};
float sphereSDF(Context &__ctx, f32x3 pos, float r) { return length(pos) - r; }
void fillCameraData(Context &__ctx, f32x3 &camPos, f32x3 &camLookAt) {
  camPos = f32x3(0.0f, 0.0f, 5.0f);
  camLookAt = f32x3(0.0f, 0.0f, 0.0f);
}
f32x3 backgroundColor(Context &__ctx, f32x2 uv) {
  return f32x3(0.0f, 0.0125f, 0.025f) * (1.0f - 0.2f * length(uv));
}
float rayMarch(Context &__ctx, f32x3 rayOrigin, f32x3 rayDir) {
  const float maxd = 20.0f;
  float t = 0.0f;
  for (int i = 0; i < 64; i++) {
    float d = sphereSDF(__ctx, rayOrigin + rayDir * t, 1.0f);
    if (d < 0.01f || t > 20.0f)
      break;
    t += d;
  }
  return t < 20.0f ? t : -1.0f;
}
MaterialBuffer fillMaterialData(Context &__ctx) {
  MaterialBuffer mb;
  mb.diffuse = f32x3(0.5f, 0.0f, 0.0f);
  mb.specular = f32x3(0.5f, 0.33f, 0.157f);
  mb.roughness = 0.25f;
  mb.metalness = 0.8f;
  mb.shiness = 16.0f;
  return mb;
}
LightBuffer fillLightData(Context &__ctx, f32x3 worldPos, f32x3 rayDir) {
  LightBuffer lb;
  lb.worldPos = worldPos;
  lb.normal = normalize(worldPos);
  if (lb.normal == f32x3(0.0f, 1.0f, 0.0f))
    lb.tangent = f32x3(1.0f, 0.0f, 0.0f);
  else
    lb.tangent = normalize(cross(f32x3(0.0f, 1.0f, 0.0f), lb.normal));
  lb.bitangent = normalize(cross(lb.normal, lb.tangent));
  lb.viewDir = -rayDir;
  lb.lightDir = normalize(f32x3(sin(iTime), 1.0f, cos(iTime)));
  lb.halfVec = normalize(lb.viewDir + lb.lightDir);
  lb.lightCol = f32x3(0.5f, 0.5f, 0.5f);
  lb.lambert = max(0.0f, dot(lb.normal, lb.lightDir));
  return lb;
}
f32x3 fresnelSchlick3(Context &__ctx, float cosTheta, f32x3 F0) {
  return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}
float fresnelSchlick1(Context &__ctx, float cosTheta, float F0) {
  return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}
f32x3 blinnPhong(Context &__ctx, LightBuffer lightBuff,
                 MaterialBuffer matBuff) {
  float diff = lightBuff.lambert;
  float spec =
      pow(max(dot(lightBuff.normal, lightBuff.halfVec), 0.0f), matBuff.shiness);
  spec = diff > 0.0f ? spec : 0.0f;
  return matBuff.diffuse * diff + matBuff.specular * spec;
}
f32x3 phong(Context &__ctx, LightBuffer lightBuff, MaterialBuffer matBuff) {
  f32x3 r = reflect(-lightBuff.viewDir, lightBuff.normal);
  float diff = lightBuff.lambert;
  float spec = pow(max(dot(lightBuff.lightDir, r), 0.0f), matBuff.shiness);
  return matBuff.diffuse * diff + matBuff.specular * spec;
}
f32x3 cookTorrance(Context &__ctx, LightBuffer lightBuff,
                   MaterialBuffer matBuff) {
  float a = matBuff.roughness * matBuff.roughness;
  float nDotH = max(dot(lightBuff.normal, lightBuff.halfVec), 0.0f);
  float vDotH = max(dot(lightBuff.viewDir, lightBuff.halfVec), 0.0f);
  float nDotV = max(dot(lightBuff.normal, lightBuff.viewDir), 0.0f);
  float nDotH2 = nDotH * nDotH;
  f32x3 F0 = f32x3(0.04f);
  F0 = mix(F0, matBuff.specular, matBuff.metalness);
  f32x3 F = fresnelSchlick3(__ctx, nDotV, F0);
  float power = -(1.0f - nDotH2) / (nDotH2 * a);
  float BDF = pow(2.7182818284f, power) / (a * nDotH2 * nDotH2);
  float c = 2.0f * nDotH / vDotH;
  float G = min(1.0f, min(c * nDotV, c * lightBuff.lambert));
  f32x3 specBRDF = F * BDF * G / (3.1415926f * nDotV * lightBuff.lambert);
  float diff = lightBuff.lambert;
  f32x3 spec = max(f32x3(0.0f), specBRDF);
  return matBuff.diffuse * diff + matBuff.specular * spec;
}
float computeA(Context &__ctx, float roughness) {
  float rsqr = roughness * roughness;
  return 1.0f - 0.5f * rsqr / (rsqr + 0.33f);
}
float computeB(Context &__ctx, float roughness) {
  float rsqr = roughness * roughness;
  return 0.45f * rsqr / (rsqr + 0.09f);
}
f32x3 orenNayar(Context &__ctx, LightBuffer lightBuff, MaterialBuffer matBuff) {
  float A = computeA(__ctx, 0.5f);
  float B = computeB(__ctx, 0.5f);
  float nDotL = dot(lightBuff.normal, lightBuff.lightDir);
  float nDotV = dot(lightBuff.normal, lightBuff.viewDir);
  f32x3 lightDSP = normalize(lightBuff.lightDir - lightBuff.normal * nDotL);
  f32x3 viewDSP = normalize(lightBuff.viewDir - lightBuff.normal * nDotV);
  float projCos = max(dot(lightDSP, viewDSP), 0.0f);
  float cosAlpha = max(nDotL, nDotV);
  float cosBeta = min(nDotL, nDotV);
  float sinAlpha = 1.0f - cosAlpha * cosAlpha;
  float sinBeta = 1.0f - cosBeta * cosBeta;
  float sinTan = sqrt(sinAlpha * sinBeta) / cosBeta;
  float diff = lightBuff.lambert * (A + B * projCos * sinTan);
  return matBuff.diffuse * diff;
}
f32x3 minnaert(Context &__ctx, LightBuffer lightBuff, MaterialBuffer matBuff) {
  float nDotV = max(dot(lightBuff.normal, lightBuff.viewDir), 0.0f);
  float minnaert =
      pow(lightBuff.lambert, 1.0f + 1.0f) * pow(1.0f - nDotV, 1.0f - 1.0f);
  return matBuff.diffuse * minnaert;
}
float computeC(Context &__ctx) {
  return sqrt((15.5f + 1.0f) * (15.5f + 1.0f)) / (8.0f * 3.1415926f);
}
f32x3 ashikhminShirley(Context &__ctx, LightBuffer lightBuff,
                       MaterialBuffer matBuff) {
  float ndotV = max(0.0f, dot(lightBuff.normal, lightBuff.viewDir));
  float ndotH = max(0.0f, dot(lightBuff.normal, lightBuff.halfVec));
  float hDotL = max(0.0f, dot(lightBuff.halfVec, lightBuff.lightDir));
  float hDotV = max(0.0f, dot(lightBuff.halfVec, lightBuff.viewDir));
  float bDotH = dot(lightBuff.bitangent, lightBuff.halfVec);
  float tDotH = dot(lightBuff.tangent, lightBuff.halfVec);
  float C = computeC(__ctx);
  f32x3 F0 = f32x3(0.04f);
  F0 = mix(F0, matBuff.specular, matBuff.metalness);
  f32x3 F = fresnelSchlick3(__ctx, hDotL, F0);
  f32x3 diff = 2.0f * (f32x3(1.0f) - F0) *
               (1.0f - pow(1.0f - 0.5f * ndotV, 5.0f)) *
               (1.0f - pow(1.0f - 0.5f * lightBuff.lambert, 5.0f));
  float B = pow(ndotH, (15.5f * bDotH * bDotH + 15.5f * tDotH * tDotH) /
                           (1.0f - ndotH * ndotH));
  float denom = hDotV * max(ndotV, lightBuff.lambert);
  f32x3 spec = C * B * F / denom;
  return lightBuff.lambert * (matBuff.diffuse * diff + matBuff.specular * spec);
}
f32x3 lommelSeeliger(Context &__ctx, LightBuffer lightBuff,
                     MaterialBuffer matBuff) {
  float denom =
      lightBuff.lambert + max(0.0f, dot(lightBuff.normal, lightBuff.viewDir));
  return matBuff.diffuse * lightBuff.lambert / denom;
}
float strFresnel(Context &__ctx, float x, float kf) {
  float dx = x - kf;
  float d1 = 1.0f - kf;
  float kf2 = kf * kf;
  return (1.0f / (dx * dx) - 1.0f / kf2) / (1.0f / (d1 * d1) - 1.0f / kf2);
}
float strShadow(Context &__ctx, float x, float ks) {
  float dx = x - ks;
  float d1 = 1.0f - ks;
  float ks2 = ks * ks;
  return (1.0f / (dx * dx) - 1.0f / ks2) / (1.0f / (d1 * d1) - 1.0f / ks2);
}
f32x3 strauss(Context &__ctx, LightBuffer lightBuff, MaterialBuffer matBuff) {
  float smoothness = 1.0f - matBuff.roughness;
  const float transparency = 0.0f;
  f32x3 lrn = reflect(lightBuff.lightDir, lightBuff.normal);
  float nDotV = dot(lightBuff.normal, lightBuff.viewDir);
  float hDotV = dot(lrn, lightBuff.viewDir);
  float strF = strFresnel(__ctx, lightBuff.lambert, 1.12f);
  float smoothness3 = smoothness * smoothness * smoothness;
  float omst = (1.0f - smoothness3) * (1.0f - transparency);
  f32x3 diff = lightBuff.lambert * (1.0f - matBuff.metalness * smoothness) *
               omst * matBuff.diffuse;
  float r = (1.0f - transparency) - omst;
  float j = strF * strShadow(__ctx, lightBuff.lambert, 1.01f) *
            strShadow(__ctx, nDotV, 1.01f);
  float refl = min(1.0f, r + j * (r + 0.1f));
  f32x3 Cs = f32x3(1.0f) + matBuff.metalness * (1.0f - strF) *
                               (matBuff.diffuse - f32x3(1.0f));
  f32x3 spec = Cs * pow(-hDotV, 3.0f / (1.0f - smoothness)) * refl;
  diff = max(f32x3(0.0f), diff);
  spec = max(f32x3(0.0f), spec);
  return diff + spec * matBuff.specular;
}
f32x3 ward(Context &__ctx, LightBuffer lightBuff, MaterialBuffer matBuff) {
  float nDotH = dot(lightBuff.halfVec, lightBuff.normal);
  float nDotH2 = nDotH * nDotH;
  float spec = exp(-10.0f * (1.0f - nDotH2) / nDotH2);
  spec = lightBuff.lambert > 0.0f ? spec : 0.0f;
  return matBuff.diffuse * lightBuff.lambert + matBuff.specular * spec;
}
f32x3 doLighting(Context &__ctx, LightBuffer lightBuff, MaterialBuffer matBuff,
                 f32x2 screenIdx) {
  int lightIdx = int(screenIdx.data[1] * 3.0f + screenIdx.data[0]);
  f32x3 linLight = blinnPhong(__ctx, lightBuff, matBuff);
  switch (lightIdx) {
  case 1:
    linLight = phong(__ctx, lightBuff, matBuff);
    break;
  case 2:
    linLight = cookTorrance(__ctx, lightBuff, matBuff);
    break;
  case 3:
    linLight = orenNayar(__ctx, lightBuff, matBuff);
    break;
  case 4:
    linLight = minnaert(__ctx, lightBuff, matBuff);
    break;
  case 5:
    linLight = ashikhminShirley(__ctx, lightBuff, matBuff);
    break;
  case 6:
    linLight = lommelSeeliger(__ctx, lightBuff, matBuff);
    break;
  case 7:
    linLight = strauss(__ctx, lightBuff, matBuff);
    break;
  case 8:
    linLight = ward(__ctx, lightBuff, matBuff);
    break;
  }
  linLight *= lightBuff.lightCol;
  linLight += (f32x3(0.0f, 0.0125f, 0.025f) + matBuff.diffuse) * 0.033f;
  return linLight;
}
f32x3 render(Context &__ctx, f32x2 uv, f32x2 screenIdx) {
  f32x3 camPos, camLookAt;
  fillCameraData(__ctx, camPos, camLookAt);
  f32x3 camForward = normalize(camLookAt - camPos);
  f32x3 camRight = normalize(cross(camForward, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 camUp = normalize(cross(camRight, camForward));
  f32x3x3 camMatrix = f32x3x3(camRight, camUp, camForward);
  const float focalLength = 2.0f;
  f32x3 onScreenPoint = f32x3(uv, focalLength);
  f32x3 rayDir = normalize(camMatrix * onScreenPoint);
  f32x3 col = backgroundColor(__ctx, uv);
  float t = rayMarch(__ctx, camPos, rayDir);
  if (t > -0.5f) {
    f32x3 worldPos = camPos + rayDir * t;
    LightBuffer lightBuff = fillLightData(__ctx, worldPos, rayDir);
    MaterialBuffer matBuff = fillMaterialData(__ctx);
    col = doLighting(__ctx, lightBuff, matBuff, screenIdx);
  }
  f32x2 postProcessUV = uv * 0.5f + 0.5f;
  col *= pow(16.f * postProcessUV.data[0] * postProcessUV.data[1] *
                 (1.f - postProcessUV.data[0]) * (1.f - postProcessUV.data[1]),
             .2f);
  col = pow(clamp(col, 0.0f, 1.0f), f32x3(0.4545f));
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 fragCoordScaled = f32x2(3.0f, 3.0f) * fragCoord;
  f32x2 uv = (mod(fragCoordScaled, iResolution.swizzle<0, 1>()) -
              0.5f * iResolution.swizzle<0, 1>()) /
             min(iResolution.data[0], iResolution.data[1]);
  f32x2 screenIdx = floor(fragCoordScaled / iResolution.swizzle<0, 1>());
  fragColor = f32x4(render(__ctx, uv, screenIdx), 1.0f);
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
