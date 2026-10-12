// Port of https://www.shadertoy.com/view/ddtGR8; author: krupitskas.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ddtGR8.json.
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
const float EPSILON = 0.001f;
float sphere(Context &__ctx, f32x3 p, float s) { return length(p) - s; }
float scene(Context &__ctx, f32x3 p) {
  float a1 = sphere(
      __ctx,
      p + f32x3(2.0f * sin(iTime * 0.7f), 1.0f, 2.0f * cos(iTime * 0.7f)),
      1.0f);
  float a2 = sphere(__ctx, p, 1.0f);
  float a3 = sphere(__ctx, p - f32x3(0.0f, -33.5f, 0.0f), 32.0f);
  float a4 =
      sphere(__ctx, p - f32x3(0.0f, -2.0f, 0.0f), 0.5f + abs(sin(iTime)));
  float a5 = sphere(__ctx, p + f32x3(-4.0f, -0.5f + sin(iTime), 0.0f), 1.0f);
  return min(min(min(min(a1, a2), a3), a4), a5);
}
f32x3 estimateNormal(Context &__ctx, f32x3 p) {
  return normalize(f32x3(
      scene(__ctx, f32x3(p.data[0] + EPSILON, p.data[1], p.data[2])) -
          scene(__ctx, f32x3(p.data[0] - EPSILON, p.data[1], p.data[2])),
      scene(__ctx, f32x3(p.data[0], p.data[1] + EPSILON, p.data[2])) -
          scene(__ctx, f32x3(p.data[0], p.data[1] - EPSILON, p.data[2])),
      scene(__ctx, f32x3(p.data[0], p.data[1], p.data[2] + EPSILON)) -
          scene(__ctx, f32x3(p.data[0], p.data[1], p.data[2] - EPSILON))));
}
float shortestDistanceToSurface(Context &__ctx, f32x3 eye,
                                f32x3 marchingDirection, float start,
                                float end) {
  const int MAX_MARCHING_STEPS = 255;
  float depth = start;
  for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
    float dist = scene(__ctx, eye + depth * marchingDirection);
    if (dist < EPSILON) {
      return depth;
    }
    depth += dist;
    if (depth >= end) {
      return end;
    }
  }
  return end;
}
f32x3 rayDirection(Context &__ctx, float fieldOfView, f32x2 size,
                   f32x2 fragCoord) {
  f32x2 xy = fragCoord - size / 2.0f;
  float z = size.data[1] / tan(radians(fieldOfView) / 2.0f);
  return normalize(f32x3(xy, -z));
}
u32x2 quasi2 = u32x2(0xC13FA9A9u, 0x91E10DA5u);
u32x3 quasi3 = u32x3(3518319155, 2882110345, 2360945575);
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 dir =
      rayDirection(__ctx, 45.0f, iResolution.swizzle<0, 1>(), fragCoord);
  f32x3 eye = f32x3(0.0f, 0.0f, 15.0f);
  const float MIN_DIST = 0.0f;
  const float MAX_DIST = 100.0f;
  float dist = shortestDistanceToSurface(__ctx, eye, dir, MIN_DIST, MAX_DIST);
  f32x3 p = eye + dist * dir;
  f32x3 N = estimateNormal(__ctx, p);
  unsigned seed = unsigned(fragCoord.data[0]) * quasi2.data[0] +
                  unsigned(fragCoord.data[1]) * quasi2.data[1];
  const float CONES_NUM = 5.0f;
  const float MAX_DIST_CONE = 5.0f;
  const float MAX_STEPS_CONE = 16.0f;
  const float CONE_CAP_RAD = 3.1415926535897932384626433832795f / CONES_NUM;
  const float phi = 1.6180339887498948f;
  const float tau = 6.2831853071795865f;
  float ao = 0.0f;
  for (int j = 0; j < int(CONES_NUM); j++) {
    float r = sqrt((float(j) + 0.5f) / CONES_NUM);
    float a = phi * tau * float(j);
    a += tau * (float(seed) / exp2(32.f));
    f32x3 dir = f32x3(sin(a) * r, sqrt(1.0f - r * r), cos(a) * r);
    f32x3 tangent = normalize(cross(N, f32x3(1, 0, 0)));
    f32x3 bitangent = cross(tangent, N);
    dir = dir.data[0] * tangent + dir.data[1] * N + dir.data[2] * bitangent;
    f32x3 endTrace = p + MAX_DIST_CONE * dir;
    float len = length(p - endTrace);
    float stepSize = len / MAX_STEPS_CONE;
    float vis = 1.0f;
    for (int i = 1; i <= int(MAX_STEPS_CONE); i++) {
      f32x3 stepPos = p + dir * stepSize * float(i);
      float distToStep = length(p - stepPos);
      float radiusAtStep = tan(CONE_CAP_RAD / 2.0f) * distToStep;
      float sdf_d = scene(__ctx, stepPos);
      vis = min(vis, smoothstep(-radiusAtStep, radiusAtStep, sdf_d));
    }
    ao += vis;
  }
  ao /= CONES_NUM;
  fragColor = f32x4(ao, ao, ao, 1.0f);
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
