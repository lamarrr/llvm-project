// Port of https://www.shadertoy.com/view/Nlcyzf; author: IWBTShyGuy.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Nlcyzf.json.
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

const float PI = 3.141592653f;
struct Camera {
  f32x3 position;
  f32x3 direction;
  f32x3 up_direction;
  float fov;
  float aspect;
};
struct Ray {
  f32x3 origin;
  f32x3 direction;
};
Ray cameraRay(Context &__ctx, Camera camera, f32x2 uv) {
  uv = uv * 2.0f - 1.0f;
  float h = tan(camera.fov * 0.5f);
  float w = h * camera.aspect;
  f32x3 right = normalize(cross(camera.direction, camera.up_direction));
  f32x3 up = normalize(cross(right, camera.direction));
  f32x3 direction = normalize(right * w * uv.data[0] + up * h * uv.data[1] +
                              camera.direction);
  Ray ray;
  ray.origin = camera.position;
  ray.direction = direction;
  return ray;
}
f32x2 hash21(Context &__ctx, float p) {
  f32x3 p3 = fract(f32x3(p) * f32x3(.1031f, .1030f, .0973f));
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.swizzle<0, 0>() + p3.swizzle<1, 2>()) * p3.swizzle<2, 1>());
}
f32x3 randomUnit(Context &__ctx, float gen) {
  f32x2 u = hash21(__ctx, gen);
  float t = 2.0f * PI * u.data[0];
  float z = 2.0f * u.data[1] - 1.0f;
  float r = sqrt(1.0f - z * z);
  return f32x3(r * cos(t), r * sin(t), z);
}
float opSmoothUnion(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  return mix(d2, d1, h) - k * h * (1.0f - h);
}
float sDist(Context &__ctx, f32x3 p) {
  float dist = length(p) - 1.5f;
  for (int i = 0; i < 20; i++) {
    f32x3 c = 5.0f *
              (sin((float(i) * 0.5f + iTime * 1.2f) / 5.0f * PI) + 1.0f) *
              randomUnit(__ctx, float(i));
    dist = opSmoothUnion(__ctx, dist, length(p - c) - 0.5f, 0.5f);
  }
  return dist;
}
f32x3 calcNormal(Context &__ctx, f32x3 p) {
  const f32x2 h = f32x2(0, 1.0e-5f);
  return normalize(f32x3(sDist(__ctx, p + h.swizzle<1, 0, 0>()) -
                             sDist(__ctx, p - h.swizzle<1, 0, 0>()),
                         sDist(__ctx, p + h.swizzle<0, 1, 0>()) -
                             sDist(__ctx, p - h.swizzle<0, 1, 0>()),
                         sDist(__ctx, p + h.swizzle<0, 0, 1>()) -
                             sDist(__ctx, p - h.swizzle<0, 0, 1>())));
}
void mainImage0(Context &__ctx, f32x4 &O, f32x2 U) {
  f32x2 r = iResolution.swizzle<0, 1>(),
        pos = f32x2(cos(iTime * 0.12f), sin(iTime * 0.12f));
  U /= r;
  Camera cam = Camera(f32x3(pos.data[1], 0, pos.data[0]) * 20.0f,
                      f32x3(-pos.data[1], 0, -pos.data[0]), f32x3(0, 1, 0),
                      PI / 4.0f, r.data[0] / r.data[1]);
  Ray ray = cameraRay(__ctx, cam, U);
  float dist = 0.0f;
  int i;
  for (i = 0; i < 256; i++) {
    f32x3 p = ray.origin + dist * ray.direction;
    float dist0 = sDist(__ctx, p);
    if (dist0 < 1.0e-3f || dist > 40.0f)
      break;
    dist += dist0;
  }
  float k = clamp((0.5f + U.data[1] - U.data[0]) / 4.0f, 0.0f, 1.0f);
  O = f32x4(0, k, 0.25f - k, 1);
  if (i < 256 && dist < 40.0f) {
    f32x3 p = ray.origin + dist * ray.direction;
    f32x3 normal = calcNormal(__ctx, p);
    float c = -dot(normal, normalize(f32x3(-1)));
    O = f32x4(c * 0.1f, 0, (1.0f - c) * 0.1f, 1);
  }
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 U) {
  mainImage0(__ctx, O, U);
  if (fwidth(length(O.swizzle<0, 1, 2>())) > 0.1f) {
    f32x4 o;
    for (int k = 0; k < 4; k++) {
      mainImage0(__ctx, o, U + (f32x2(k % 2, k / 2) - 0.5f) / 1.5f);
      O += o;
    }
    O /= 5.0f;
  }
  O = pow(O, f32x4(.4545f));
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
