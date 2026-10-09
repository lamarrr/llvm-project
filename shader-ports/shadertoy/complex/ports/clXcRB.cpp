// Port of https://www.shadertoy.com/view/clXcRB; author: Danielhu.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/clXcRB.json.
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

float box(Context &__ctx, f32x3 position, f32x3 halfSize, float cornerRadius) {
  position = abs(position) - halfSize + cornerRadius;
  return length(max(position, 0.0f)) +
         min(max(max(position.data[0], position.data[1]), position.data[2]),
             0.0f) -
         cornerRadius;
}
f32x3x3 lookAtMatrix(Context &__ctx, f32x3 from, f32x3 to) {
  f32x3 forward = normalize(to - from);
  f32x3 right = normalize(cross(forward, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 up = cross(right, forward);
  return f32x3x3(right, up, forward);
}
f32x3x3 rotationMatrixAxisAngle(Context &__ctx, f32x3 axis, float angle) {
  float s = sin(angle);
  float c = cos(angle);
  f32x3 column1 = axis * axis.data[0] * (1.0f - c) +
                  f32x3(c, axis.data[2] * s, -axis.data[1] * s);
  f32x3 column2 = axis * axis.data[1] * (1.0f - c) +
                  f32x3(-axis.data[2] * s, c, axis.data[0] * s);
  f32x3 column3 = axis * axis.data[2] * (1.0f - c) +
                  f32x3(axis.data[1] * s, -axis.data[0] * s, c);
  return f32x3x3(column1, column2, column3);
}
float sdf(Context &__ctx, f32x3 position) {
  float halfEdge = 0.25f;
  float cornerRadius = 0.06f;
  float separation = halfEdge - cornerRadius;
  float distance = 1e0f;
  int mask = 1 | 2 | 4 | 16;
  int cubeIndex = 0;
  for (int x = -1; x <= 1; x += 2) {
    for (int y = -1; y <= 1; y += 2) {
      for (int z = -1; z <= 1; z += 2) {
        if ((mask & (1 << cubeIndex)) != 0) {
          f32x3 center = f32x3(x, y, z) * separation;
          f32x3 p = position - center;
          distance =
              min(distance, box(__ctx, p, f32x3(halfEdge, halfEdge, halfEdge),
                                cornerRadius));
        }
        cubeIndex++;
      }
    }
  }
  return distance;
}
f32x3 normal(Context &__ctx, f32x3 position) {
  float epsilon = 0.001f;
  f32x3 gradient = f32x3(sdf(__ctx, position + f32x3(epsilon, 0, 0)) -
                             sdf(__ctx, position + f32x3(-epsilon, 0, 0)),
                         sdf(__ctx, position + f32x3(0, epsilon, 0)) -
                             sdf(__ctx, position + f32x3(0, -epsilon, 0)),
                         sdf(__ctx, position + f32x3(0, 0, epsilon)) -
                             sdf(__ctx, position + f32x3(0, 0, -epsilon)));
  return normalize(gradient);
}
float raycast(Context &__ctx, f32x3 rayOrigin, f32x3 rayDirection) {
  int stepCount = 128 * 2;
  float maximumDistance = 5.0f;
  float t = 0.0f;
  for (int i = 0; i < stepCount; i++) {
    if (t > maximumDistance) {
      break;
    }
    f32x3 currentPosition = rayOrigin + rayDirection * t;
    float d = sdf(__ctx, currentPosition);
    if (d < 0.0001f) {
      return t;
    }
    t += d;
  }
  return 0.0f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float angle = 0.5f * (iTime - 10.0f);
  f32x3 rayOrigin = f32x3(1.0f * cos(angle), 0.4f, 1.0f * sin(angle));
  f32x3 targetPosition = f32x3(0.0f);
  f32x3x3 cameraTransform = lookAtMatrix(__ctx, rayOrigin, targetPosition);
  f32x3 result = f32x3(0.0f);
  i32x2 sampleCount = i32x2(3.0f, 3.0f);
  for (int y = 0; y < sampleCount.data[1]; y++) {
    for (int x = 0; x < sampleCount.data[0]; x++) {
      f32x2 uv =
          fragCoord + (f32x2(float(x), float(y)) / f32x2(sampleCount) - 0.5f);
      uv = uv / iResolution.swizzle<0, 1>();
      uv = (uv * 2.0f) - 1.0f;
      uv.data[0] *= iResolution.data[0] / iResolution.data[1];
      f32x3 rayDirection = normalize(f32x3(uv, 1.5f));
      rayDirection = cameraTransform * rayDirection;
      float t = raycast(__ctx, rayOrigin, rayDirection);
      f32x3 color = f32x3(0.0f);
      if (t > 0.0f) {
        f32x3 position = rayOrigin + rayDirection * t;
        f32x3 lightDirection = f32x3(0.57735f);
        f32x3 n = normal(__ctx, position);
        float diffuseAngle = max(dot(n, lightDirection), 0.0f);
        color = f32x3(0.8f, 0.7f, 0.5f) * diffuseAngle;
        color += f32x3(0.2f, 0.3f, 0.4f) * ((n.data[1] + 1.0f) * 0.5f);
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
