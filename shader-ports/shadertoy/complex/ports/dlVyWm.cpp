// Port of https://www.shadertoy.com/view/dlVyWm; author: Falken.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dlVyWm.json.
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
const int kSubsampleFactor = 1;
const int kNumberOfLayers = 48;
const float kShellDensity = 100.0f;
const float kShellLayerSpacing = 0.15f;
const int kNoiseOctaves = 5;
float hash(Context &__ctx, float p) {
  p = fract(p * 0.011f);
  p *= p + 7.5f;
  p *= p + p;
  return fract(p);
}
float hash(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(f32x3(p.swizzle<0, 1, 0>()) * 0.13f);
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 3.333f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
float noise(Context &__ctx, f32x3 x) {
  const f32x3 step = f32x3(110, 241, 171);
  f32x3 i = floor(x);
  f32x3 f = fract(x);
  float n = dot(i, step);
  f32x3 u = f * f * (3.0f - 2.0f * f);
  return mix(mix(mix(hash(__ctx, n + dot(step, f32x3(0, 0, 0))),
                     hash(__ctx, n + dot(step, f32x3(1, 0, 0))), u.data[0]),
                 mix(hash(__ctx, n + dot(step, f32x3(0, 1, 0))),
                     hash(__ctx, n + dot(step, f32x3(1, 1, 0))), u.data[0]),
                 u.data[1]),
             mix(mix(hash(__ctx, n + dot(step, f32x3(0, 0, 1))),
                     hash(__ctx, n + dot(step, f32x3(1, 0, 1))), u.data[0]),
                 mix(hash(__ctx, n + dot(step, f32x3(0, 1, 1))),
                     hash(__ctx, n + dot(step, f32x3(1, 1, 1))), u.data[0]),
                 u.data[1]),
             u.data[2]);
}
float fbm(Context &__ctx, f32x3 x) {
  float v = 0.0f;
  float a = 0.5f;
  f32x3 shift = f32x3(100);
  for (int i = 0; i < kNoiseOctaves; ++i) {
    v += a * noise(__ctx, x);
    x = x * 2.0f + shift;
    a *= 0.5f;
  }
  return v;
}
float sdBox(Context &__ctx, f32x3 position, f32x3 halfSize) {
  position = abs(position) - halfSize;
  return length(max(position, 0.0f)) +
         min(max(max(position.data[0], position.data[1]), position.data[2]),
             0.0f);
}
float sdSphere(Context &__ctx, f32x3 position, float size) {
  return length(position) - size;
}
float sdf(Context &__ctx, f32x3 position, float size) {
  return sdSphere(__ctx, position, size);
}
f32x3 normal(Context &__ctx, f32x3 position, float scale) {
  float epsilon = 0.001f;
  f32x3 gradient =
      f32x3(sdf(__ctx, position + f32x3(epsilon, 0, 0), scale) -
                sdf(__ctx, position + f32x3(-epsilon, 0, 0), scale),
            sdf(__ctx, position + f32x3(0, epsilon, 0), scale) -
                sdf(__ctx, position + f32x3(0, -epsilon, 0), scale),
            sdf(__ctx, position + f32x3(0, 0, epsilon), scale) -
                sdf(__ctx, position + f32x3(0, 0, -epsilon), scale));
  return normalize(gradient);
}
float raycast(Context &__ctx, f32x3 rayOrigin, f32x3 rayDirection,
              float scale) {
  int stepCount = 128 * 2;
  float maximumDistance = 5.0f;
  float t = 0.0f;
  for (int i = 0; i < stepCount; i++) {
    if (t > maximumDistance) {
      break;
    }
    f32x3 position = rayOrigin + rayDirection * t;
    float d = sdf(__ctx, position, scale);
    if (d < 0.0001f) {
      return t;
    }
    t += d;
  }
  return 0.0f;
}
f32x3x3 lookAtMatrix(Context &__ctx, f32x3 from, f32x3 to) {
  f32x3 forward = normalize(to - from);
  f32x3 right = normalize(cross(forward, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 up = cross(right, forward);
  return f32x3x3(right, up, forward);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float angle = 0.5f * (iTime - 8.0f);
  f32x3 cameraOrigin = f32x3(1.0f * cos(angle), 0.4f, 1.0f * sin(angle));
  f32x3 cameraTarget = f32x3(0.0f);
  f32x3x3 cameraTransform = lookAtMatrix(__ctx, cameraOrigin, cameraTarget);
  f32x3 result = f32x3(0.0f);
  const float layerYInc = 1.0f / float(kNumberOfLayers);
  i32x2 sampleCount = i32x2(kSubsampleFactor, kSubsampleFactor);
  for (int y = 0; y < sampleCount.data[1]; y++) {
    for (int x = 0; x < sampleCount.data[0]; x++) {
      f32x2 uv =
          fragCoord + (f32x2(float(x), float(y)) / f32x2(sampleCount) - 0.5f);
      uv = uv / iResolution.swizzle<0, 1>();
      uv = (uv * 2.0f) - 1.0f;
      uv.data[0] *= iResolution.data[0] / iResolution.data[1];
      f32x3 rayDirection = normalize(f32x3(uv, 1.5f));
      rayDirection = cameraTransform * rayDirection;
      f32x3 color = f32x3(0.0f);
      float layerY = 0.0f;
      for (int layerIndex = 0; layerIndex < kNumberOfLayers; ++layerIndex) {
        float scale = (layerY * kShellLayerSpacing) + 0.4f;
        float t = raycast(__ctx, cameraOrigin, rayDirection, scale);
        if (t > 0.0f) {
          f32x3 position = cameraOrigin + rayDirection * t;
          f32x3 lightDirection = f32x3(0.57735f);
          f32x3 n = normal(__ctx, position, scale);
          float diffuseAngle = max(dot(n, lightDirection), 0.0f);
          float v = fbm(__ctx, normalize(position) * kShellDensity);
          if (v > max(0.1f, layerY)) {
            float r = 0.82f * layerY;
            float g = 0.71f * layerY;
            float b = 0.43f * layerY;
            color = f32x3(r, g, b) * (diffuseAngle + 0.05f);
          }
        }
        layerY += layerYInc;
      }
      color = sqrt(color);
      result += color;
    }
  }
  result /= float(sampleCount.data[0] * sampleCount.data[1]);
  fragColor = f32x4(result, 1.0f);
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
