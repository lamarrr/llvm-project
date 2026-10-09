// Port of https://www.shadertoy.com/view/ctSGzG; author: shaofun.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctSGzG.json.
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
struct Context {
  TextureCube channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

const float ZERO = 0.0f;
const float PI = 3.141592653589f;
const float TAU = 2.0f * PI;
struct ray {
  f32x3 origin;
  f32x3 direction;
  f32x3 color;
};
struct camera {
  f32x3 lookfrom;
  f32x3 lookat;
  f32x3 vup;
  float vfov;
  float aspect;
  float aperture;
  float focus;
};
float seed;
float rand13(Context &__ctx, f32x3 x) {
  u32x3 p = floatBitsToUint(x);
  p = 1103515245U * ((p.swizzle<0, 1, 2>() >> 1U) ^ (p.swizzle<1, 2, 0>()));
  unsigned h32 = 1103515245U * ((p.data[0] ^ p.data[2]) ^ (p.data[1] >> 3U));
  unsigned n = h32 ^ (h32 >> 16U);
  return float(n) * (1.0f / float(0xffffffffU));
}
float rand11(Context &__ctx) {
  u32x2 n = floatBitsToUint(seed++) * u32x2(1597334673U, 3812015801U);
  unsigned q = (n.data[0] ^ n.data[1]) * 1597334673U;
  return float(q) * (1.0f / float(0xffffffffU));
}
f32x2 rand21(Context &__ctx) {
  u32x2 n = floatBitsToUint(seed++) * u32x2(1597334673U, 3812015801U);
  n = (n.data[0] ^ n.data[1]) * u32x2(1597334673U, 3812015801U);
  return f32x2(n) * (1.0f / float(0xffffffffU));
}
f32x3 at(Context &__ctx, ray r, float t) { return r.origin + t * r.direction; }
f32x2 random_in_unit_disk(Context &__ctx) {
  f32x2 r = rand21(__ctx) * f32x2(1.0f, TAU);
  return sqrt(r.data[0]) * f32x2(sin(r.data[1]), cos(r.data[1]));
}
ray get_ray(Context &__ctx, camera c, f32x2 uv, f32x3 color) {
  float theta = radians(c.vfov);
  float half_height = tan(theta * 0.5f);
  float half_width = c.aspect * half_height;
  f32x3 z = normalize(c.lookfrom - c.lookat);
  f32x3 x = normalize(cross(c.vup, z));
  f32x3 y = cross(z, x);
  f32x3 hwfx = half_width * c.focus * x;
  f32x3 hhfy = half_height * c.focus * y;
  f32x3 lower_left_corner = c.lookfrom - hwfx - hhfy - c.focus * z;
  f32x3 horizontal = 2.0f * hwfx;
  f32x3 vertical = 2.0f * hhfy;
  float lens_radius = c.aperture * 0.5f;
  f32x2 rud = lens_radius * random_in_unit_disk(__ctx);
  f32x3 offset = x * rud.data[0] + y * rud.data[1];
  f32x3 ro = c.lookfrom + offset;
  f32x3 po =
      lower_left_corner + uv.data[0] * horizontal + uv.data[1] * vertical;
  f32x3 rd = normalize(po - ro);
  return ray(ro, rd, color);
}
f32x3x3 CameraRotation(Context &__ctx, f32x2 m) {
  m *= f32x2x2(f32x2(0, -1), f32x2(1, 0));
  f32x2 s = sin(m), c = cos(m);
  f32x3x3 rotX = f32x3x3(1.0f, 0.0f, 0.0f, 0.0f, c.data[0], s.data[0], 0.0f,
                         -s.data[0], c.data[0]);
  f32x3x3 rotY = f32x3x3(c.data[1], 0.0f, -s.data[1], 0.0f, 1.0f, 0.0f,
                         s.data[1], 0.0f, c.data[1]);
  return rotY * rotX;
}
f32x2 spherical_map(Context &__ctx, f32x3 p) {
  f32x2 uv = f32x2(atan(p.data[2], p.data[0]), asin(p.data[1]));
  uv *= f32x2(1.0f / TAU, 1.0f / PI);
  uv += 0.5f;
  return uv;
}
f32x3 spherical_map(Context &__ctx, f32x2 uv) {
  uv -= 0.5f;
  uv *= f32x2(TAU, PI);
  f32x2 s = sin(uv), c = cos(uv);
  return f32x3(c.data[0] * c.data[1], s.data[1], s.data[0] * c.data[1]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 mouse = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  if (all(equal(mouse, f32x2(0))))
    mouse = f32x2(0.5f);
  f32x3 p = spherical_map(__ctx, uv);
  p *= CameraRotation(__ctx, (mouse - 0.5f) * f32x2(PI));
  f32x3 col = texture(__ctx.channel0, p).swizzle<0, 1, 2>();
  fragColor = f32x4(col, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_cube<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
  Context __ctx{{channel0, sampling0}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
