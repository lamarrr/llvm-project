// Port of https://www.shadertoy.com/view/csVyz1; author: Renslay.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/csVyz1.json.
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
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
const int MAX_MARCHING_STEPS = 128;
const float MIN_DIST = 0.0f;
const float MAX_DIST = 128.0f;
const float PRECISION = 0.001f;
const float EPSILON = 0.005f;
const float PI = 3.14159265359f;
const f32x3 BACKGROUND_COLOR = f32x3(0.6f, 0.6f, 0.835f);
const f32x3 TORUS_COLOR = f32x3(1.0f, 1.0f, 1.0f);
const f32x3 AMBIENT = f32x3(0.5f, 0.5f, 0.5f);
const f32x3 CAM_LOOK_AT = f32x3(0, 0, 0);
const f32x3 LIGHT_POS = f32x3(2.f, 5, -1.0f);
f32x3 textColor(Context &__ctx, f32x3 p, f32x3 normal) {
  float b = dot(normal, f32x3(0.0f, 0.0f, 1.0f));
  f32x2 q = f32x2(0.5f - 0.5f * atan(normal.data[2], normal.data[0]) / PI,
                  -acos(p.data[1]) / PI);
  return texture(__ctx.channel0, f32x2(iTime / 5.0f, 0.0f) + q)
      .swizzle<0, 1, 2>();
}
struct Surface {
  float id;
  float sd;
  f32x3 col;
};
Surface sdSphere(Context &__ctx, f32x3 p, float r, f32x3 offset) {
  p = p - offset;
  f32x3 n = normalize(p);
  f32x3 color = textColor(__ctx, p, n);
  p = p - 0.05f * (0.21f * color.data[0] + 0.71f * color.data[1] +
                   0.07f * color.data[2]);
  float d = length(p) - r;
  return Surface(1.0f, d, color);
}
Surface sdTorus(Context &__ctx, f32x3 p, f32x2 t, f32x3 offset, f32x3 col) {
  p = p - offset;
  f32x2 q = f32x2(length(p.swizzle<0, 2>()) - t.data[0], p.data[1]);
  float d = length(q) - t.data[1];
  return Surface(2.0f, d, col);
}
Surface sdFloor(Context &__ctx, f32x3 p, f32x3 col) {
  float d = p.data[1] + 1.f;
  return Surface(0.0f, d, col);
}
Surface unionSDF(Context &__ctx, Surface obj1, Surface obj2) {
  if (obj1.sd < obj2.sd)
    return obj1;
  return obj2;
}
Surface sdScene(Context &__ctx, f32x3 p) {
  f32x3 floorColor = f32x3(
      0.5f, 1.f + 0.7f * mod(floor(p.data[0]) + floor(p.data[2]), 2.0f), 0.5f);
  Surface flooor = sdFloor(__ctx, p, floorColor);
  Surface sphere =
      sdSphere(__ctx, p, 1.f, f32x3(-1.5f * cos(iTime), 0, 1.5f * sin(iTime)));
  Surface torus =
      sdTorus(__ctx, p, f32x2(1.0f, 0.25f),
              f32x3(1.5f * cos(iTime), 0, -1.5f * sin(iTime)), TORUS_COLOR);
  Surface co = sphere;
  co = unionSDF(__ctx, co, torus);
  co = unionSDF(__ctx, co, flooor);
  return co;
}
Surface rayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float depth = MIN_DIST;
  Surface co;
  for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
    f32x3 p = ro + depth * rd;
    co = sdScene(__ctx, p);
    depth += co.sd;
    if (co.sd < PRECISION || depth > MAX_DIST)
      break;
  }
  co.sd = depth;
  return co;
}
float softshadow(Context &__ctx, f32x3 ro, f32x3 rd, float mint, float maxt,
                 float w) {
  float res = 1.0f;
  float ph = 1e20f;
  float t = mint;
  for (int i = 0; i < 256 && t < maxt; i++) {
    float h = sdScene(__ctx, ro + rd * t).sd;
    if (h < 0.001f)
      return 0.0f;
    float y = h * h / (2.0f * ph);
    float d = sqrt(h * h - y * y);
    res = min(res, d / (w * max(0.0f, t - y)));
    ph = h;
    t += h;
  }
  return res;
}
f32x3 calcNormal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(1, -1) * EPSILON;
  return normalize(
      e.swizzle<0, 1, 1>() * sdScene(__ctx, p + e.swizzle<0, 1, 1>()).sd +
      e.swizzle<1, 1, 0>() * sdScene(__ctx, p + e.swizzle<1, 1, 0>()).sd +
      e.swizzle<1, 0, 1>() * sdScene(__ctx, p + e.swizzle<1, 0, 1>()).sd +
      e.swizzle<0, 0, 0>() * sdScene(__ctx, p + e.swizzle<0, 0, 0>()).sd);
}
f32x3x3 camera(Context &__ctx, f32x3 cameraPos, f32x3 lookAtPoint) {
  f32x3 forward = normalize(cameraPos - lookAtPoint);
  f32x3 right = normalize(cross(f32x3(0, 1, 0), forward));
  f32x3 up = normalize(cross(forward, right));
  return f32x3x3(right, up, forward);
}
float angle(Context &__ctx, f32x3 v1, f32x3 v2) {
  return clamp(dot(v1, v2), 0.0f, 1.0f);
}
float lightIntensity(Context &__ctx, f32x3 p, f32x3 norm, f32x3 camPos,
                     f32x3 lightPos) {
  float kd = 0.9f;
  float ks = 0.7f;
  float id = 0.7f;
  float is = 0.5f;
  float a = 4.0f;
  f32x3 lightDir = normalize(lightPos - p);
  f32x3 camDir = normalize(camPos - p);
  f32x3 reflectDir = 2.0f * angle(__ctx, lightDir, norm) * norm - lightDir;
  float diffuseLight = kd * angle(__ctx, lightDir, norm) * id;
  float specularLight = ks * pow(angle(__ctx, reflectDir, camDir), a) * is;
  float color = (diffuseLight + specularLight);
  return color;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 camPos =
      f32x3(4.0f * sin(iTime / 5.0f), 1.5f, 4.0f * cos(iTime / 5.0f));
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col;
  f32x3x3 camMatrix = camera(__ctx, camPos, CAM_LOOK_AT);
  f32x3 rd = camMatrix * normalize(f32x3(uv, -1.f));
  Surface co = rayMarch(__ctx, camPos, rd);
  float dist = co.sd;
  if (co.sd > MAX_DIST) {
    col = BACKGROUND_COLOR;
  } else {
    f32x3 p = camPos + rd * co.sd;
    f32x3 normal = calcNormal(__ctx, p);
    col = co.col * (0.5f * lightIntensity(__ctx, p, normal, camPos, LIGHT_POS) +
                    AMBIENT);
    f32x3 lightDir = normalize(LIGHT_POS - p);
    f32x3 newRayOrigin = p + normal * PRECISION * 2.f;
    if (co.id == 2.0f) {
      rd = reflect(rd, normal);
      co = rayMarch(__ctx, newRayOrigin, rd);
      if (co.sd < MAX_DIST) {
        col = 0.5f * col + 0.5f * co.col;
      } else
        col = 0.5f * col + 0.5f * BACKGROUND_COLOR;
    }
    float shadow = softshadow(__ctx, newRayOrigin, lightDir, 0.01f, 3.0f, 0.2f);
    shadow = clamp(0.0f, 1.0f, shadow + 0.3f);
    col = shadow * col;
  }
  float fogAmount = 1.0f - exp(-dist * 0.05f);
  f32x3 fogColor = f32x3(0.5f, 0.6f, 0.7f);
  col = mix(col, fogColor, fogAmount);
  fragColor = f32x4(col, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
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
