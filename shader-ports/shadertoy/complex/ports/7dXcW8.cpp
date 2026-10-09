// Port of https://www.shadertoy.com/view/7dXcW8; author: vipiao.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7dXcW8.json.
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
float sdSphere(Context &__ctx, f32x3 pos, f32x3 center, float radius) {
  return length(pos - center) - radius;
}
float sdSphereWorld(Context &__ctx, f32x3 pos) {
  pos = mod(pos, 1.0f);
  return sdSphere(__ctx, pos, f32x3(0.5f), 0.5f);
}
f32x3 normalSphereWorld(Context &__ctx, f32x3 pos) {
  float sd = sdSphereWorld(__ctx, pos);
  f32x2 e = f32x2(0.001f, 0.f);
  f32x3 normal =
      normalize(sd - f32x3(sdSphereWorld(__ctx, pos - e.swizzle<0, 1, 1>()),
                           sdSphereWorld(__ctx, pos - e.swizzle<1, 0, 1>()),
                           sdSphereWorld(__ctx, pos - e.swizzle<1, 1, 0>())));
  return normal;
}
float sdSpiral(Context &__ctx, f32x3 pos) {
  float radius = 1.f;
  f32x3 pos1 =
      f32x3(cos(pos.data[2]) * radius, sin(pos.data[2]) * radius, pos.data[2]);
  float angle = atan(pos.data[1], pos.data[0]);
  float offset =
      pos.data[2] / 3.1415926538f / 2.f - angle / 3.1415926538f / 2.f;
  f32x3 pos2 =
      f32x3(pos.swizzle<0, 1>() / length(pos.swizzle<0, 1>()) * radius,
            pos.data[2] + abs(offset - round(offset)) * 3.1415926538f * 2.f);
  float rr = length(pos - (pos1 + pos2) / 2.0f);
  float sd = rr - 0.4f;
  return sd * 0.5f;
}
f32x3 normalsdSpiral(Context &__ctx, f32x3 pos) {
  float sd = sdSpiral(__ctx, pos);
  f32x2 e = f32x2(0.001f, 0.f);
  f32x3 normal =
      normalize(sd - f32x3(sdSpiral(__ctx, pos - e.swizzle<0, 1, 1>()),
                           sdSpiral(__ctx, pos - e.swizzle<1, 0, 1>()),
                           sdSpiral(__ctx, pos - e.swizzle<1, 1, 0>())));
  return normal;
}
float sdTotalWorld(Context &__ctx, f32x3 pos, float resolution) {
  float size = 100.f;
  pos.set_swizzle<0, 1>(mod(pos.swizzle<0, 1>() + size * 0.5f, size) -
                        size * 0.5f);
  float speed = 0.2f;
  float sine = sin(iTime * speed);
  float cosine = cos(iTime * speed);
  pos = f32x3(cosine * pos.data[0] + sine * pos.data[1],
              sine * pos.data[0] - cosine * pos.data[1], pos.data[2]);
  float ss = sdSphereWorld(__ctx, pos);
  float ff = 1.0f;
  for (int i = 0; i < int(resolution); i++) {
    ff *= 4.f;
    ss = min(ss, sdSphereWorld(__ctx, pos * ff) / ff);
  }
  ff *= 4.f;
  ss = mix(ss, min(ss, sdSphereWorld(__ctx, pos * ff) / ff),
           mod(resolution, 1.f));
  return max(min(sdSpiral(__ctx, pos),
                 sdSpiral(__ctx, f32x3(pos.swizzle<0, 1>(),
                                       pos.data[2] - 3.1415926538f))),
             -ss);
}
f32x3 normalSdTotalWorld(Context &__ctx, f32x3 pos, float resolution,
                         float delta) {
  float sd = sdTotalWorld(__ctx, pos, resolution);
  f32x2 e = f32x2(delta, 0.f);
  f32x3 normal = normalize(
      sd - f32x3(sdTotalWorld(__ctx, pos - e.swizzle<0, 1, 1>(), resolution),
                 sdTotalWorld(__ctx, pos - e.swizzle<1, 0, 1>(), resolution),
                 sdTotalWorld(__ctx, pos - e.swizzle<1, 1, 0>(), resolution)));
  return normal;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float rr = 3.f;
  float ss = 0.1f;
  f32x3 cameraPos =
      f32x3(sin(iTime * ss) * rr, cos(iTime * ss) * rr, -iTime * 1.2f);
  f32x3 forward = normalize(f32x3(normalize(-cameraPos.swizzle<0, 1>()),
                                  -1.8f + 1.4f * sin(iTime * 0.3f)));
  f32x3 right = normalize(cross(forward, f32x3(0, 0, 1)));
  f32x3 up = cross(right, forward);
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[0];
  float aspectRatio = iResolution.data[0] / iResolution.data[1];
  f32x3 camPos = f32x3(cameraPos.data[0], cameraPos.data[1], cameraPos.data[2]);
  float zoom = 0.9f;
  f32x3 viewDir =
      normalize(forward * 0.8f * zoom + right * uv.data[0] + up * uv.data[1]);
  float debug = 0.0f;
  f32x3 color = f32x3(0.1f, 0, 0);
  f32x3 reflectionConstant = f32x3(1.0f);
  f32x3 pos = camPos;
  f32x3 dir = viewDir;
  float dist = 0.0f;
  float maxDist = 200.0f;
  int maxNumSteps = 200;
  int numSteps = 0;
  float minDist = 0.0001f;
  f32x3 prevPos = pos;
  float prevSD = 0.0f;
  float sd = 0.0f;
  bool shaddowMode = false;
  float distToSurface = maxDist;
  int numStepsToSurface = maxNumSteps;
  for (; numSteps < maxNumSteps; numSteps++) {
    prevSD = sd;
    float dd = shaddowMode ? distToSurface : dist;
    float resolution = clamp(-log2(dd * 0.003f) / log2(4.f), 0.f, 8.f);
    sd = sdTotalWorld(__ctx, pos, resolution);
    if (sd < minDist) {
      float backFactor = (minDist - sd) / (prevSD - sd);
      pos = mix(pos, prevPos, backFactor);
      sd *= (1.0f - backFactor);
      f32x3 normal = normalSdTotalWorld(__ctx, pos, resolution, 0.001f);
      f32x3 surfaceColor = f32x3(1, 1, 0.9f);
      float ambient = 0.1f;
      if (shaddowMode) {
        color = surfaceColor * ambient;
        break;
      }
      shaddowMode = true;
      pos += normal * minDist;
      const f32x3 lightDir = normalize(f32x3(1, 1, 2));
      color = surfaceColor * (f32x3(dot(lightDir, normal)) + ambient);
      distToSurface = dist;
      numStepsToSurface = numSteps;
      dir = lightDir;
    }
    const float brightnessFactor = 0.1f;
    if (dist + sd > maxDist) {
      dist = maxDist;
      break;
    }
    prevPos = pos;
    pos += normalize(dir) * sd;
    dist += sd;
  }
  float fogFactor = max(float(numStepsToSurface) / float(maxNumSteps),
                        distToSurface / maxDist);
  f32x3 fogColor =
      mix(f32x3(0.3f, 0.3f, .6f), f32x3(0.1f, 0.1f, .2f), viewDir.data[2]);
  color = mix(color, fogColor, fogFactor);
  color.data[0] += debug;
  fragColor = f32x4(color, 1.0f);
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
