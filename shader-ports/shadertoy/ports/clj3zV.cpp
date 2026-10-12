// Port of https://www.shadertoy.com/view/clj3zV; author: balt.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/clj3zV.json.
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

const float EPSILON = 0.001f;
const int STEPS = 256;
const float NEAR_PLANE = 0.0f;
const float FAR_PLANE = 100.0f;
float lerp(Context &__ctx, float a, float b, float t) {
  return (1.0f - t) * a + t * b;
}
f32x2 lerp(Context &__ctx, f32x2 a, f32x2 b, float t) {
  return (1.0f - t) * a + t * b;
}
f32x3 lerp(Context &__ctx, f32x3 a, f32x3 b, float t) {
  return (1.0f - t) * a + t * b;
}
f32x4 lerp(Context &__ctx, f32x4 a, f32x4 b, float t) {
  return (1.0f - t) * a + t * b;
}
f32x2 angleToVec2(Context &__ctx, float theta) {
  return f32x2(cos(theta), sin(theta));
}
f32x3 hsv2rgb(Context &__ctx, f32x3 c) {
  f32x4 K = f32x4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
  f32x3 p = abs(fract(c.swizzle<0, 0, 0>() + K.swizzle<0, 1, 2>()) * 6.0f -
                K.swizzle<3, 3, 3>());
  return c.data[2] * mix(K.swizzle<0, 0, 0>(),
                         clamp(p - K.swizzle<0, 0, 0>(), 0.0f, 1.0f),
                         c.data[1]);
}
f32x3x3 rotX(Context &__ctx, float theta) {
  f32x2 v = angleToVec2(__ctx, theta);
  return f32x3x3(f32x3(1.0f, 0.0f, 0.0f), f32x3(0.0f, v.data[0], -v.data[1]),
                 f32x3(0.0f, v.data[1], v.data[0]));
}
f32x3x3 rotY(Context &__ctx, float theta) {
  f32x2 v = angleToVec2(__ctx, theta);
  return f32x3x3(f32x3(v.data[0], 0.0f, v.data[1]), f32x3(0.0f, 1.0f, 0.0f),
                 f32x3(-v.data[1], 0.0f, v.data[0]));
}
f32x3x3 rotZ(Context &__ctx, float theta) {
  f32x2 v = angleToVec2(__ctx, theta);
  return f32x3x3(f32x3(v.data[0], -v.data[1], 0.0f),
                 f32x3(v.data[1], v.data[0], 0.0f), f32x3(0.0f, 0.0f, 1.0f));
}
f32x3 screenToCameraRay(Context &__ctx, float fov, f32x2 pos) {
  f32x2 xy = pos - iResolution.swizzle<0, 1>() / 2.0f;
  float z = iResolution.data[1] / tan(radians(fov) / 2.0f);
  return normalize(f32x3(xy, -z));
}
f32x3x3 makeCameraWorldMatrix(Context &__ctx, f32x3 e, f32x3 c, f32x3 up) {
  f32x3 f = normalize(c - e);
  f32x3 s = normalize(cross(f, up));
  f32x3 u = cross(s, f);
  return f32x3x3(s, u, -f);
}
struct sdfOutput {
  float signedDistance;
  f32x3 diffuseColor;
  float roughness;
  float specularStrength;
};
sdfOutput sphereSDF(Context &__ctx, f32x3 pos, float radius, f32x3 color,
                    float roughness, float specularStrength) {
  return sdfOutput(length(pos) - radius, color, roughness, specularStrength);
}
sdfOutput metaballSDF(Context &__ctx, f32x3 pos, float threshold, f32x3 color,
                      float roughness, float specularStrength) {
  return sdfOutput((-threshold / length(pos)) + 1.0f, color, roughness,
                   specularStrength);
}
sdfOutput boxSDF(Context &__ctx, f32x3 pos, f32x3 size, f32x3 color,
                 float roughness, float specularStrength) {
  f32x3 q = abs(pos) - size;
  return sdfOutput(length(max(q, 0.0f)) +
                       min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f),
                   color, roughness, specularStrength);
}
sdfOutput planeSDF(Context &__ctx, f32x3 pos, f32x3 normal, f32x3 color,
                   float roughness, float specularStrength) {
  return sdfOutput(dot(pos, normal), color, roughness, specularStrength);
}
sdfOutput coneSDF(Context &__ctx, f32x3 p, f32x2 rh, f32x3 color,
                  float roughness, float specularStrength) {
  f32x2 q = rh.data[1] * f32x2(rh.data[0], -1.0f);
  f32x2 w = f32x2(length(p.swizzle<0, 2>()), p.data[1]);
  f32x2 a = w - q * clamp(dot(w, q) / dot(q, q), 0.0f, 1.0f);
  f32x2 b = w - q * f32x2(clamp(w.data[0] / q.data[0], 0.0f, 1.0f), 1.0f);
  float k = sign(q.data[1]);
  float d = min(dot(a, a), dot(b, b));
  float s = max(k * (w.data[0] * q.data[1] - w.data[1] * q.data[0]),
                k * (w.data[1] - q.data[1]));
  return sdfOutput(sqrt(d) * sign(s), color, roughness, specularStrength);
}
sdfOutput unionSDF(Context &__ctx, sdfOutput a, sdfOutput b) {
  if (a.signedDistance < b.signedDistance) {
    return a;
  } else {
    return b;
  }
}
sdfOutput intersectSDF(Context &__ctx, sdfOutput a, sdfOutput b) {
  if (a.signedDistance > b.signedDistance) {
    return a;
  } else {
    return b;
  }
}
sdfOutput differenceSDF(Context &__ctx, sdfOutput a, sdfOutput b) {
  return intersectSDF(__ctx, a,
                      sdfOutput(-b.signedDistance, b.diffuseColor, b.roughness,
                                b.specularStrength));
}
sdfOutput addSDF(Context &__ctx, sdfOutput a, sdfOutput b) {
  return sdfOutput(a.signedDistance + b.signedDistance,
                   lerp(__ctx, a.diffuseColor, b.diffuseColor, 0.5f),
                   lerp(__ctx, a.roughness, b.roughness, 0.5f),
                   lerp(__ctx, a.specularStrength, b.specularStrength, 0.5f));
}
sdfOutput lerpSDF(Context &__ctx, sdfOutput a, sdfOutput b, float t) {
  return sdfOutput(lerp(__ctx, a.signedDistance, b.signedDistance, t),
                   lerp(__ctx, a.diffuseColor, b.diffuseColor, t),
                   lerp(__ctx, a.roughness, b.roughness, t),
                   lerp(__ctx, a.specularStrength, b.specularStrength, t));
}
sdfOutput invertSDF(Context &__ctx, sdfOutput a) {
  return sdfOutput(-a.signedDistance, a.diffuseColor, a.roughness,
                   a.specularStrength);
}
sdfOutput sceneSDF(Context &__ctx, f32x3 samplePoint) {
  return unionSDF(
      __ctx,
      unionSDF(
          __ctx,
          unionSDF(
              __ctx,
              sphereSDF(__ctx, samplePoint + f32x3(2.0f, -1.0f, 0.0f), 1.0f,
                        f32x3(0.2f, 0.7f, 0.1f), 0.7f, 16.0f),
              boxSDF(__ctx,
                     rotY(__ctx, radians(20.0f)) * rotX(__ctx, radians(10.0f)) *
                         (samplePoint + f32x3(-1.2f, 1.0f, 2.0f)),
                     f32x3(1.0f), f32x3(0.1f, 0.6f, 1.0f), 0.7f, 2.0f)),
          planeSDF(
              __ctx, samplePoint + f32x3(0.0f, 3.0f, 0.0f),
              f32x3(0.0f, 1.0f, 0.0f),
              f32x3(ceil(
                  (sin((rotY(__ctx, radians(45.0f)) * samplePoint).data[0] *
                       4.0f) +
                   sin((rotY(__ctx, radians(45.0f)) * samplePoint).data[2] *
                       4.0f)) /
                  2.0f)),
              1.0f, 16.0f)),
      coneSDF(__ctx, samplePoint + f32x3(-3.0f, -2.0f, -2.0f),
              f32x2(0.5f, 2.0f), f32x3(1.0f, 0.1f, 0.1f), 0.7f, 32.0f));
}
f32x3 calcNormal(Context &__ctx, f32x3 pos) {
  return normalize(f32x3(
      sceneSDF(__ctx, f32x3(pos.data[0] + EPSILON, pos.data[1], pos.data[2]))
              .signedDistance -
          sceneSDF(__ctx,
                   f32x3(pos.data[0] - EPSILON, pos.data[1], pos.data[2]))
              .signedDistance,
      sceneSDF(__ctx, f32x3(pos.data[0], pos.data[1] + EPSILON, pos.data[2]))
              .signedDistance -
          sceneSDF(__ctx,
                   f32x3(pos.data[0], pos.data[1] - EPSILON, pos.data[2]))
              .signedDistance,
      sceneSDF(__ctx, f32x3(pos.data[0], pos.data[1], pos.data[2] + EPSILON))
              .signedDistance -
          sceneSDF(__ctx,
                   f32x3(pos.data[0], pos.data[1], pos.data[2] - EPSILON))
              .signedDistance));
}
const f32x3 GLOBAL_COLOR = f32x3(0.0f);
const f32x3 CLEAR_COLOR = f32x3(0.0f);
const f32x3 AMBIENT_COLOR = f32x3(0.2f);
struct light {
  f32x3 position;
  f32x3 color;
  float radius;
};
f32x3 lightSDF(Context &__ctx, f32x3 cameraRay, f32x3 position, f32x3 normal,
               f32x3 diffuse, float roughness, float specularity) {
  light[] LIGHTS = light[](light(f32x3(0.0f, 5.0f, 0.0f), f32x3(1.0f), 20.0f));
  f32x3 finalColor = GLOBAL_COLOR;
  for (int i = 0; i < LIGHTS.length(); i++) {
    f32x3 lightDir = normalize(LIGHTS[i].position - position);
    float diff = max(dot(normal, lightDir), 0.0f);
    f32x3 diffuseLight = diff * LIGHTS[i].color;
    float lightDistance = distance(position, LIGHTS[i].position);
    f32x3 reflectDir = reflect(lightDir, normal);
    float spec = pow(max(dot(cameraRay, reflectDir), 0.0f), specularity);
    f32x3 specularLight = roughness * spec * LIGHTS[i].color;
    float lightStrength = (1.0f - (lightDistance / LIGHTS[i].radius));
    finalColor += diffuse * (diffuseLight * lightStrength) +
                  (diffuse * AMBIENT_COLOR) + specularLight * lightStrength;
  }
  return finalColor;
}
struct sdOutput {
  float depth;
  sdfOutput sdf;
};
sdOutput surfaceDist(Context &__ctx, f32x3 camera, f32x3 ray) {
  float depth = NEAR_PLANE;
  for (int i = 0; i < STEPS; i++) {
    sdfOutput sdf = sceneSDF(__ctx, camera + depth * ray);
    float dist = sdf.signedDistance;
    if (dist < EPSILON) {
      return sdOutput(depth, sdf);
    }
    depth += dist;
    if (depth >= FAR_PLANE) {
      return sdOutput(FAR_PLANE, sdfOutput(0.0f, f32x3(0.0f), 0.0f, 0.0f));
    }
  }
  return sdOutput(FAR_PLANE, sdfOutput(0.0f, f32x3(0.0f), 0.0f, 0.0f));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  fragColor = f32x4(CLEAR_COLOR, 1.0f);
  f32x3 cameraRay = screenToCameraRay(__ctx, 65.0f, fragCoord);
  f32x3 cameraFocus = f32x3(0.0f, (sin(iTime / 2.0f) * 1.5f), 0.0f);
  f32x3 cameraPos = f32x3(sin(iTime) * 12.0f, 3.0f, cos(iTime) * 12.0f);
  f32x3x3 cameraWorldMatrix = makeCameraWorldMatrix(
      __ctx, cameraPos, -cameraFocus, f32x3(0.0f, 1.0f, 0.0f));
  f32x3 worldRay = cameraWorldMatrix * cameraRay;
  sdOutput sdOut = surfaceDist(__ctx, cameraPos, worldRay);
  float d = sdOut.depth;
  f32x3 hitPos = cameraPos + d * worldRay;
  if (d > FAR_PLANE - EPSILON) {
    return;
  }
  f32x3 normal = calcNormal(__ctx, hitPos);
  fragColor =
      f32x4(lightSDF(__ctx, worldRay, hitPos, normal, sdOut.sdf.diffuseColor,
                     sdOut.sdf.roughness, sdOut.sdf.specularStrength),
            1.0f);
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
