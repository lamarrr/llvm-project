// Port of https://www.shadertoy.com/view/mddXzB; author: Veggiebob.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mddXzB.json.
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
const f32x3 light = f32x3(0.0f, 0.0f, 2.0f);
const float EPSILON = 0.0001f;
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
f32x3 rotate(Context &__ctx, f32x3 v, float angle, f32x3 axis) {
  float cosAngle = cos(angle);
  float sinAngle = sin(angle);
  float oneMinusCosAngle = 1.0f - cosAngle;
  f32x3x3 rotationMat = f32x3x3(
      cosAngle + axis.data[0] * axis.data[0] * oneMinusCosAngle,
      axis.data[0] * axis.data[1] * oneMinusCosAngle - axis.data[2] * sinAngle,
      axis.data[0] * axis.data[2] * oneMinusCosAngle + axis.data[1] * sinAngle,
      axis.data[1] * axis.data[0] * oneMinusCosAngle + axis.data[2] * sinAngle,
      cosAngle + axis.data[1] * axis.data[1] * oneMinusCosAngle,
      axis.data[1] * axis.data[2] * oneMinusCosAngle - axis.data[0] * sinAngle,
      axis.data[2] * axis.data[0] * oneMinusCosAngle - axis.data[1] * sinAngle,
      axis.data[2] * axis.data[1] * oneMinusCosAngle + axis.data[0] * sinAngle,
      cosAngle + axis.data[2] * axis.data[2] * oneMinusCosAngle);
  return rotationMat * v;
}
float ripple(Context &__ctx, f32x3 v) {
  v += sin(v.data[2] + iTime) * 0.3f;
  f32x3 l = rotate(__ctx, f32x3(1.f, 0.f, 0.f) + v * 0.1f, iTime * 0.3f,
                   f32x3(0.f, 1.f, 0.f));
  l *= 0.5f;
  return dot(f32x3(1.f), sin(v + (cos(dot(v, l) * 2.f) + iTime * 3.f) * 0.5f));
}
float sdf(Context &__ctx, f32x3 p) {
  return min(sdBox(__ctx, p, f32x3(0.3f)) - ripple(__ctx, p * 25.f) * 0.003f,
             p.data[1] + 1.5f);
}
f32x3 computeNormal(Context &__ctx, f32x3 p) {
  f32x3 normal = f32x3(0.0f);
  float eps = EPSILON;
  normal.data[0] = sdf(__ctx, f32x3(p.data[0] + eps, p.data[1], p.data[2])) -
                   sdf(__ctx, f32x3(p.data[0] - eps, p.data[1], p.data[2]));
  normal.data[1] = sdf(__ctx, f32x3(p.data[0], p.data[1] + eps, p.data[2])) -
                   sdf(__ctx, f32x3(p.data[0], p.data[1] - eps, p.data[2]));
  normal.data[2] = sdf(__ctx, f32x3(p.data[0], p.data[1], p.data[2] + eps)) -
                   sdf(__ctx, f32x3(p.data[0], p.data[1], p.data[2] - eps));
  return normalize(normal);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>()) * 2.0f - 1.0f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 camPos = f32x3(0.0f, 0.0f, 3.0f);
  f32x3 camUp = f32x3(0.0f, 1.0f, 0.0f);
  float camFov = 60.0f;
  f32x2 mouse =
      iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - f32x2(0.5f);
  camPos = rotate(__ctx, camPos, mouse.data[1] * 3.f, cross(camPos, camUp));
  camPos = rotate(__ctx, camPos, mouse.data[0] * 4.f, f32x3(0.f, 1.f, 0.f));
  camPos = normalize(camPos) * 1.2f;
  f32x3 camDir = normalize(-camPos);
  f32x3 camRight = normalize(cross(camDir, camUp));
  f32x3 camU = normalize(-cross(camDir, camRight));
  f32x3 rayDir = normalize(camDir + uv.data[0] * camRight + uv.data[1] * camU);
  f32x3 rayOrigin = camPos;
  float t = 0.0f;
  float maxDist = 100.0f;
  float minDist = 0.1f;
  int iterations = 800;
  float n2 = 1.1f;
  float n1 = 1.0f;
  f32x3 p = rayOrigin;
  bool inWater = false;
  for (int i = 0; i < iterations; i++) {
    float dist = sdf(__ctx, p);
    if (inWater)
      dist *= -1.f;
    if (dist < minDist) {
      if (length(p) < 1.f) {
        f32x3 normal = computeNormal(__ctx, p);
        if (inWater)
          normal *= -1.f;
        if (dot(-rayDir, normal) < 0.5f) {
          rayDir = reflect(rayDir, normal);
        } else {
          f32x3 prev = rayDir;
          if (!inWater) {
            rayDir = normalize(refract(rayDir, normal, n2 / n1));
          } else {
            rayDir = normalize(refract(rayDir, normal, n1 / n2));
          }
          if (dot(cross(-prev, normal), cross(rayDir, -normal)) > 0.f) {
            inWater = !inWater;
            p += rayDir * abs(dist * 5.0f);
          }
        }
      } else {
        break;
      }
    }
    if (t > maxDist)
      break;
    float d = 0.1f * dist;
    t += d;
    p += d * rayDir;
  }
  f32x3 color = f32x3(1.0f);
  if (p.data[1] < -1.f) {
    color *= mod(p.swizzle<0, 2, 0>(), f32x3(3.f));
  }
  f32x3 normal = computeNormal(__ctx, p);
  if (t < maxDist) {
    f32x3 toLight = normalize(light - p);
    float diffuse = max(dot(normalize(normal), toLight), 0.f);
    float specular = pow(max(dot(reflect(-rayDir, normal), toLight), 0.f), 8.f);
    color *= (0.f + diffuse + 0.f * 0.3f);
  }
  if (inWater) {
    color = f32x3(1.f);
  }
  color = max(color, f32x3(0.f, 1.f, 1.f) * 0.01f * p.data[1]);
  fragColor = f32x4(color, 1.0f);
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
