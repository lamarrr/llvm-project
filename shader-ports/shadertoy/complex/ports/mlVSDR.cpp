// Port of https://www.shadertoy.com/view/mlVSDR; author: dilshad.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mlVSDR.json.
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

const int MAX_MARCHING_STEPS = 2000;
const float MIN_DIST = 0.0f;
const float MAX_DIST = 2000.0f;
const float EPSILON = 0.0001f;
f32x3 modulo(Context &__ctx, f32x3 p, float n) {
  return f32x3(mod(p.data[0], n), mod(p.data[1], n), mod(p.data[2], n));
}
float sphereSDF(Context &__ctx, f32x3 samplePoint) {
  return length(modulo(__ctx, samplePoint, 0.9f) - f32x3(0.5f, 0.5f, 0.0f)) -
         0.25f;
}
float sceneSDF(Context &__ctx, f32x3 samplePoint) {
  return sphereSDF(__ctx, samplePoint);
}
float shortestDistanceToSurface(Context &__ctx, f32x3 eye,
                                f32x3 marchingDirection, float start,
                                float end) {
  float depth = start;
  for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
    float dist = sceneSDF(__ctx, eye + depth * marchingDirection);
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
f32x3 estimateNormal(Context &__ctx, f32x3 p) {
  return normalize(f32x3(
      sceneSDF(__ctx, f32x3(p.data[0] + EPSILON, p.data[1], p.data[2])) -
          sceneSDF(__ctx, f32x3(p.data[0] - EPSILON, p.data[1], p.data[2])),
      sceneSDF(__ctx, f32x3(p.data[0], p.data[1] + EPSILON, p.data[2])) -
          sceneSDF(__ctx, f32x3(p.data[0], p.data[1] - EPSILON, p.data[2])),
      sceneSDF(__ctx, f32x3(p.data[0], p.data[1], p.data[2] + EPSILON)) -
          sceneSDF(__ctx, f32x3(p.data[0], p.data[1], p.data[2] - EPSILON))));
}
f32x3 phongContribForLight(Context &__ctx, f32x3 k_d, f32x3 k_s, float alpha,
                           f32x3 p, f32x3 eye, f32x3 lightPos,
                           f32x3 lightIntensity) {
  f32x3 N = estimateNormal(__ctx, p);
  f32x3 L = normalize(lightPos - p);
  f32x3 V = normalize(eye - p);
  f32x3 R = normalize(reflect(-L, N));
  float dotLN = dot(L, N);
  float dotRV = dot(R, V);
  if (dotLN < 0.0f) {
    return f32x3(0.0f, 0.0f, 0.0f);
  }
  if (dotRV < 0.0f) {
    return lightIntensity * (k_d * dotLN);
  }
  return lightIntensity * (k_d * dotLN + k_s * pow(dotRV, alpha));
}
f32x3 phongIllumination(Context &__ctx, f32x3 k_a, f32x3 k_d, f32x3 k_s,
                        float alpha, f32x3 p, f32x3 eye) {
  const f32x3 ambientLight = 0.5f * f32x3(1.0f, 1.0f, 1.0f);
  f32x3 color = ambientLight * k_a;
  f32x3 light1Pos = f32x3(4.0f * sin(iTime), 2.0f, 4.0f * cos(iTime));
  f32x3 light1Intensity = f32x3(0.4f, 0.4f, 0.4f);
  color += phongContribForLight(__ctx, k_d, k_s, alpha, p, eye, light1Pos,
                                light1Intensity);
  f32x3 light2Pos =
      f32x3(2.0f * sin(0.37f * iTime), 2.0f * cos(0.37f * iTime), 2.0f);
  f32x3 light2Intensity = f32x3(0.4f, 0.4f, 0.4f);
  color += phongContribForLight(__ctx, k_d, k_s, alpha, p, eye, light2Pos,
                                light2Intensity);
  return color;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 dir =
      rayDirection(__ctx, 45.0f, iResolution.swizzle<0, 1>(), fragCoord);
  f32x3 eye =
      f32x3(-iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>(), 5.0f);
  float dist = shortestDistanceToSurface(__ctx, eye, dir, MIN_DIST, MAX_DIST);
  if (dist > MAX_DIST - EPSILON) {
    fragColor = f32x4(0.0f, 0.0f, 0.0f, 0.0f);
    return;
  }
  f32x3 p = eye + dist * dir;
  f32x3 K_a = f32x3(0.2f, 0.2f, 0.2f);
  f32x3 K_d = f32x3(0.7f, 0.2f, 0.2f);
  f32x3 K_s = f32x3(1.0f, 1.0f, 1.0f);
  float shininess = 10.0f;
  f32x3 color = phongIllumination(__ctx, K_a, K_d, K_s, shininess, p, eye);
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
