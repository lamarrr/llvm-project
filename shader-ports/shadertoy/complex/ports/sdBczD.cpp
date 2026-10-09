// Port of https://www.shadertoy.com/view/sdBczD; author: pancakespeople.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/sdBczD.json.
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

f32x2 random2(Context &__ctx, f32x2 st) {
  st = f32x2(dot(st, f32x2(127.1f, 311.7f)), dot(st, f32x2(269.5f, 183.3f)));
  return -1.0f + 2.0f * fract(sin(st) * 43758.5453123f * 0.7897f);
}
float noise(Context &__ctx, f32x2 st) {
  f32x2 i = floor(st);
  f32x2 f = fract(st);
  f32x2 u = f * f * (3.0f - 2.0f * f);
  return mix(
      mix(dot(random2(__ctx, i + f32x2(0.0f, 0.0f)), f - f32x2(0.0f, 0.0f)),
          dot(random2(__ctx, i + f32x2(1.0f, 0.0f)), f - f32x2(1.0f, 0.0f)),
          u.data[0]),
      mix(dot(random2(__ctx, i + f32x2(0.0f, 1.0f)), f - f32x2(0.0f, 1.0f)),
          dot(random2(__ctx, i + f32x2(1.0f, 1.0f)), f - f32x2(1.0f, 1.0f)),
          u.data[0]),
      u.data[1]);
}
float fbm(Context &__ctx, f32x2 x) {
  float v = 0.0f;
  float a = 0.5f;
  f32x2 shift = f32x2(100);
  f32x2x2 rot = f32x2x2(cos(0.5f), sin(0.5f), -sin(0.5f), cos(0.50f));
  for (int i = 0; i < 5; ++i) {
    v += a * noise(__ctx, x);
    x = rot * x * 2.0f + shift;
    a *= 0.5f;
  }
  return v;
}
float getSphereRadius(Context &__ctx, f32x3 point) {
  return 0.25f * fbm(__ctx, point.swizzle<0, 1>() * 5.0f) + 0.5f;
}
float getAtmosRadius(Context &__ctx, f32x3 point) {
  point.data[0] += iTime / 6.0f;
  return 0.25f * fbm(__ctx, point.swizzle<0, 1>() * 5.0f) + 0.5f;
}
float sphereAndPlane(Context &__ctx, f32x3 point) {
  f32x3 spherePos = f32x3(0.0f, 1.0f, -2.0f);
  float sphereRadius = getSphereRadius(__ctx, point);
  float sphereDist = length(point - spherePos) - sphereRadius;
  float planeDist = length(point.data[1] - 0.5f);
  return min(sphereDist, planeDist);
}
float atmosphere(Context &__ctx, f32x3 point) {
  f32x3 spherePos = f32x3(0.0f, 1.0f, -2.0f);
  float sphereRadius = getAtmosRadius(__ctx, point) + 0.1f;
  float sphereDist = length(point - spherePos) - sphereRadius;
  return sphereDist;
}
float rayMarch(Context &__ctx, f32x3 rayPoint, f32x3 rayDir) {
  float distMarched = 0.0f;
  for (int i = 0; i < 100; i++) {
    float dist = sphereAndPlane(__ctx, rayPoint);
    rayPoint += dist * rayDir;
    distMarched += dist;
    if (dist < 0.01f || dist > 100.0f) {
      break;
    }
  }
  return distMarched;
}
float rayMarchAtmosphere(Context &__ctx, f32x3 rayPoint, f32x3 rayDir) {
  float distMarched = 0.0f;
  for (int i = 0; i < 100; i++) {
    float dist = atmosphere(__ctx, rayPoint);
    rayPoint += dist * rayDir;
    distMarched += dist;
    if (dist < 0.01f || dist > 100.0f) {
      break;
    }
  }
  return distMarched;
}
f32x3 getNormal(Context &__ctx, f32x3 point) {
  float dist = sphereAndPlane(__ctx, point);
  f32x3 norm =
      dist - f32x3(sphereAndPlane(__ctx, point - f32x3(0.01f, 0.0f, 0.0f)),
                   sphereAndPlane(__ctx, point - f32x3(0.0f, 0.01f, 0.0f)),
                   sphereAndPlane(__ctx, point - f32x3(0.0f, 0.0f, 0.01f)));
  return normalize(norm);
}
f32x3 getAtmosNormal(Context &__ctx, f32x3 point) {
  float dist = atmosphere(__ctx, point);
  f32x3 norm =
      dist - f32x3(atmosphere(__ctx, point = f32x3(0.01f, 0.0f, 0.0f)),
                   atmosphere(__ctx, point - f32x3(0.0f, 0.01f, 0.0f)),
                   atmosphere(__ctx, point - f32x3(0.0f, 0.0f, 0.01f)));
  return normalize(norm);
}
f32x3 getColor(Context &__ctx, f32x3 point) {
  float radius = length(f32x3(0.0f, 1.0f, -2.0f) - point);
  float polarity = point.data[1] - 1.0f;
  if (radius > 0.5f) {
    if (radius > 0.515f) {
      return f32x3(1.0f);
    } else {
      return f32x3(0.0f, 1.0f, 0.0f) +
             smoothstep(0.15f, 0.2f, abs(polarity)) * f32x3(1.0f, 0.0f, 1.0f);
    }
  } else {
    return f32x3(0.0f, 0.0f, 1.0f) +
           smoothstep(0.1f, 0.5f, abs(polarity)) * f32x3(1.0f, 1.0f, 0.0f);
  }
}
float getSpecularity(Context &__ctx, f32x3 point) {
  float radius = length(f32x3(0.0f, 1.0f, -2.0f) - point);
  if (radius < 0.5f) {
    return 2.0f;
  } else {
    return 1.0f;
  }
}
float getSpecular(Context &__ctx, f32x3 point, f32x3 lightPos,
                  f32x3 cameraPos) {
  f32x3 viewDir = normalize(cameraPos - point);
  f32x3 lightDir = normalize(lightPos - point);
  f32x3 reflectDir = reflect(-lightDir, getNormal(__ctx, point));
  return pow(max(dot(viewDir, reflectDir), 0.0f), 32.0f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[0];
  f32x3 cameraPos = f32x3(0.0f, 1.0f, -4.0f);
  f32x3 cameraDir = f32x3(0.0f);
  f32x3 rayDir = f32x3(uv.data[0], uv.data[1], 1.0f);
  f32x3 lightPos =
      f32x3(cos(iTime / 4.0f) * 2.0f, 1.0f, sin(iTime / 4.0f) * 5.0f);
  float dist = rayMarch(__ctx, cameraPos, rayDir + cameraDir);
  f32x3 hit = rayDir * dist + cameraPos;
  f32x3 normal = getNormal(__ctx, hit);
  f32x3 lightDir = normalize(lightPos - hit);
  f32x3 col = f32x3(0.0f);
  float light = clamp(dot(lightDir, normal), 0.0f, 1.0f);
  float shadow = rayMarch(__ctx, hit + normal * 0.1f, lightDir);
  if (shadow < length(lightPos - hit)) {
    light *= 0.1f;
  }
  col = f32x3(light) *
        (getColor(__ctx, hit) + getSpecular(__ctx, hit, lightPos, cameraPos) *
                                    getSpecularity(__ctx, hit));
  float atmosDist = rayMarchAtmosphere(__ctx, cameraPos, rayDir + cameraDir);
  f32x3 atmosPoint = rayDir * atmosDist + cameraPos;
  float surfAtmosDist = rayMarch(
      __ctx, atmosPoint, normalize(f32x3(0.0f, 1.0f, -2.0f) - atmosPoint));
  if (surfAtmosDist < 1.0f) {
    f32x3 atmosNormal = getAtmosNormal(__ctx, atmosPoint);
    f32x3 atmosLightDir = normalize(lightPos - atmosPoint);
    float atmosLight = dot(atmosLightDir, atmosNormal);
    col += smoothstep(0.1f, 0.2f, surfAtmosDist) * atmosLight;
  }
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
