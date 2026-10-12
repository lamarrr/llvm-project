// Port of https://www.shadertoy.com/view/4fsGzH; author: FabulousCodingFox.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/4fsGzH.json.
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
struct Sphere {
  f32x3 pos;
  float radius;
};
float dSphere(Context &__ctx, f32x3 pos, Sphere obj) {
  return length(pos - obj.pos) - obj.radius;
}
struct Cube {
  f32x3 pos;
  float side;
};
float dCube(Context &__ctx, f32x3 pos, Cube obj) {
  f32x3 q = abs(pos - obj.pos) - f32x3(obj.side);
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f) - .1f;
}
struct Pyramid {
  f32x3 pos;
  float height;
  float side;
};
float dPyramid(Context &__ctx, f32x3 pos, Pyramid obj) {
  pos = pos - obj.pos - f32x3(0, -obj.height * .5f, 0);
  float m2 = obj.height * obj.height + 0.25f;
  pos.set_swizzle<0, 2>(abs(pos.swizzle<0, 2>()));
  pos.set_swizzle<0, 2>((pos.data[2] > pos.data[0]) ? pos.swizzle<2, 0>()
                                                    : pos.swizzle<0, 2>());
  pos.set_swizzle<0, 2>(pos.swizzle<0, 2>() - (0.5f));
  f32x3 q = f32x3(pos.data[2], obj.height * pos.data[1] - 0.5f * pos.data[0],
                  obj.height * pos.data[0] + 0.5f * pos.data[1]);
  float s = max(-q.data[0], 0.0f);
  float t = clamp((q.data[1] - 0.5f * pos.data[2]) / (m2 + 0.25f), 0.0f, 1.0f);
  float a = m2 * (q.data[0] + s) * (q.data[0] + s) + q.data[1] * q.data[1];
  float b = m2 * (q.data[0] + 0.5f * t) * (q.data[0] + 0.5f * t) +
            (q.data[1] - m2 * t) * (q.data[1] - m2 * t);
  float d2 = min(q.data[1], -q.data[0] * m2 - q.data[1] * 0.5f) > 0.0f
                 ? 0.0f
                 : min(a, b);
  return sqrt((d2 + q.data[2] * q.data[2]) / m2) *
         sign(max(q.data[2], -pos.data[1]));
}
float getDistance(Context &__ctx, f32x3 position) {
  float dSPHERE = dSphere(__ctx, position, Sphere(f32x3(0, 1, 10), 1.f));
  float dCUBE = dCube(__ctx, position, Cube(f32x3(3, 1, 10), 1.f));
  float dPYRAMID =
      dPyramid(__ctx, position, Pyramid(f32x3(-3, 1, 10), 2.f, 1.f));
  return min(min(min(position.data[1], dSPHERE), dCUBE), dPYRAMID);
}
struct Ray {
  f32x3 origin, direction;
};
f32x4 rayMarch(Context &__ctx, Ray ray) {
  float distanceFromOrigin = 0.f;
  while (true) {
    f32x3 position = ray.origin + ray.direction * distanceFromOrigin;
    float dist = getDistance(__ctx, position);
    distanceFromOrigin += dist;
    if (distanceFromOrigin > 100.f || dist < .01f)
      break;
  }
  return f32x4(ray.origin + ray.direction * distanceFromOrigin,
               distanceFromOrigin);
}
f32x3 getNormal(Context &__ctx, f32x3 position) {
  float dist = getDistance(__ctx, position);
  f32x2 e = f32x2(.01f, 0);
  f32x3 normal =
      dist - f32x3(getDistance(__ctx, position - e.swizzle<0, 1, 1>()),
                   getDistance(__ctx, position - e.swizzle<1, 0, 1>()),
                   getDistance(__ctx, position - e.swizzle<1, 1, 0>()));
  return normalize(normal);
}
float getLight(Context &__ctx, f32x3 position) {
  f32x3 lightPos = f32x3(0, 5, 6);
  lightPos.set_swizzle<0, 2>(lightPos.swizzle<0, 2>() +
                             (f32x2(sin(iTime), cos(iTime)) * 2.f));
  f32x3 light = normalize(lightPos - position);
  f32x3 normal = getNormal(__ctx, position);
  float dif = clamp(dot(normal, light), 0.f, 1.f);
  f32x4 dist = rayMarch(__ctx, Ray(position + normal * .01f * 2.f, light));
  if (dist.data[3] < length(lightPos - position))
    dif *= .1f;
  return dif;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  Ray ray = Ray(f32x3(0, 1, 0), normalize(f32x3(uv.data[0], uv.data[1], 1)));
  f32x4 result = rayMarch(__ctx, ray);
  float dif = getLight(__ctx, result.swizzle<0, 1, 2>());
  f32x3 col = f32x3(dif);
  col = pow(col, f32x3(.4545f));
  fragColor = f32x4(col, 1.0f);
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
