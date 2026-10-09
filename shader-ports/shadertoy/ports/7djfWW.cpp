// Port of https://www.shadertoy.com/view/7djfWW; author: MiguelCepero.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7djfWW.json.
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
f32x3 random3(Context &__ctx, f32x3 c) {
  float j = 4096.0f * sin(dot(c, f32x3(17.0f, 59.4f, 15.0f)));
  f32x3 r;
  r.data[2] = fract(512.0f * j);
  j *= .125f;
  r.data[0] = fract(512.0f * j);
  j *= .125f;
  r.data[1] = fract(512.0f * j);
  return r - 0.5f;
}
const float F3 = 0.3333333f;
const float G3 = 0.1666667f;
float simplex3d(Context &__ctx, f32x3 p) {
  f32x3 s = floor(p + dot(p, f32x3(F3)));
  f32x3 x = p - s + dot(s, f32x3(G3));
  f32x3 e = step(f32x3(0.0f), x - x.swizzle<1, 2, 0>());
  f32x3 i1 = e * (1.0f - e.swizzle<2, 0, 1>());
  f32x3 i2 = 1.0f - e.swizzle<2, 0, 1>() * (1.0f - e);
  f32x3 x1 = x - i1 + G3;
  f32x3 x2 = x - i2 + 2.0f * G3;
  f32x3 x3 = x - 1.0f + 3.0f * G3;
  f32x4 w, d;
  w.data[0] = dot(x, x);
  w.data[1] = dot(x1, x1);
  w.data[2] = dot(x2, x2);
  w.data[3] = dot(x3, x3);
  w = max(0.6f - w, 0.0f);
  d.data[0] = dot(random3(__ctx, s), x);
  d.data[1] = dot(random3(__ctx, s + i1), x1);
  d.data[2] = dot(random3(__ctx, s + i2), x2);
  d.data[3] = dot(random3(__ctx, s + 1.0f), x3);
  w *= w;
  w *= w;
  d *= w;
  return dot(d, f32x4(52.0f));
}
const f32x3x3 rot1 =
    f32x3x3(-0.37f, 0.36f, 0.85f, -0.14f, -0.93f, 0.34f, 0.92f, 0.01f, 0.4f);
const f32x3x3 rot2 =
    f32x3x3(-0.55f, -0.39f, 0.74f, 0.33f, -0.91f, -0.24f, 0.77f, 0.12f, 0.63f);
const f32x3x3 rot3 = f32x3x3(-0.71f, 0.52f, -0.47f, -0.08f, -0.72f, -0.68f,
                             -0.7f, -0.45f, 0.56f);
float simplex3d_fractal(Context &__ctx, f32x3 m) {
  return 0.5333333f * simplex3d(__ctx, m * rot1) +
         0.2666667f * simplex3d(__ctx, 2.0f * m * rot2) +
         0.1333333f * simplex3d(__ctx, 4.0f * m * rot3) +
         0.0666667f * simplex3d(__ctx, 8.0f * m);
}
f32x3 random3f(Context &__ctx, f32x3 p) {
  return fract(sin(f32x3(dot(p, f32x3(1.0f, 57.0f, 113.0f)),
                         dot(p, f32x3(57.0f, 113.0f, 1.0f)),
                         dot(p, f32x3(113.0f, 1.0f, 57.0f)))) *
               43758.5453f);
}
f32x3 voronoi(Context &__ctx, f32x3 x, f32x3 &center) {
  f32x3 p = floor(x);
  f32x3 f = fract(x);
  float id = 0.0f;
  f32x2 res = f32x2(100.0f);
  for (int k = -1; k <= 1; k++)
    for (int j = -1; j <= 1; j++)
      for (int i = -1; i <= 1; i++) {
        f32x3 b = f32x3(float(i), float(j), float(k));
        f32x3 r = f32x3(b) - f + random3f(__ctx, p + b);
        float d = dot(r, r);
        if (d < res.data[0]) {
          id = dot(p + b, f32x3(1.0f, 57.0f, 113.0f));
          center = random3f(__ctx, p + b);
          res = f32x2(d, res.data[0]);
        } else if (d < res.data[1]) {
          res.data[1] = d;
        }
      }
  return f32x3(sqrt(res), abs(id));
}
f32x3 voronoisq(Context &__ctx, f32x3 x, f32x3 &center) {
  f32x3 p = floor(x);
  f32x3 f = fract(x);
  float id = 0.0f;
  f32x2 res = f32x2(100.0f);
  for (int k = -1; k <= 1; k++)
    for (int j = -1; j <= 1; j++)
      for (int i = -1; i <= 1; i++) {
        f32x3 b = f32x3(float(i), float(j), float(k));
        f32x3 r = f32x3(b) - f + random3f(__ctx, p + b);
        float d = max(abs(r.data[0]), max(abs(r.data[1]), abs(r.data[2])));
        if (d < res.data[0]) {
          id = dot(p + b, f32x3(1.0f, 57.0f, 113.0f));
          center = random3f(__ctx, p + b);
          res = f32x2(d, res.data[0]);
        } else if (d < res.data[1]) {
          res.data[1] = d;
        }
      }
  return f32x3(sqrt(res), abs(id));
}
float levels(Context &__ctx, float x, float xmin, float xmax) {
  return clamp((x - xmin) / (xmax - xmin), -1.0f, 1.0f);
}
float sphere(Context &__ctx, f32x3 pos, f32x3 center, float radius) {
  return pow(length(pos - center), 2.0f) - radius * radius + 1.0f * pos.data[2];
}
float rocks(Context &__ctx, f32x3 pos) {
  f32x3 org;
  float mask = levels(
      __ctx,
      simplex3d(__ctx, 5.0f * pos) + 0.5f * simplex3d(__ctx, 20.0f * pos) +
          1.3f * (1.0f - voronoi(__ctx, 10.0f * pos, org).data[0]),
      -0.01f, 0.01f);
  mask = clamp(mask, 0.0f, 1.0f);
  f32x3 noise3 = voronoi(__ctx, 5.0f * pos, org);
  float field = levels(__ctx,
                       simplex3d(__ctx, 5.0f * pos) +
                           0.5f * simplex3d(__ctx, 20.0f * pos) +
                           1.3f * voronoi(__ctx, 30.0f * pos, org).data[0],
                       -0.01f, 1.2f);
  return (field + 170.0f * pos.data[2]);
}
float dirt(Context &__ctx, f32x3 pos) {
  f32x3 org;
  f32x3 noise3 = voronoi(
      __ctx,
      f32x3(4.5f, 3.3f + 0.5f * simplex3d(__ctx, 0.01f * pos), 4.0f) * pos +
          0.02f * f32x3(simplex3d(__ctx, 10.0f * pos)),
      org);
  float noise1 = simplex3d(__ctx, 100.0f * org);
  f32x3 scale = f32x3(4.0f + 0.5f * noise1);
  float field =
      simplex3d(__ctx, scale * pos * f32x3(1.0f, 0.01f, 1.0f) + f32x3(noise1));
  field = min(field,
              simplex3d(__ctx, 0.5f * scale * pos * f32x3(0.01f, 1.0f, 1.0f) -
                                   f32x3(noise1)));
  return 0.01f * (field) + pos.data[2];
}
float rockfield(Context &__ctx, f32x3 pos) { return dirt(__ctx, pos); }
float sand(Context &__ctx, f32x3 pos, f32x3 warp) {
  float noise = simplex3d(__ctx, 0.5f * pos);
  pos += 0.2f * noise * f32x3(1.0f, 1.0f, 0.0f) + warp;
  float amp = clamp(1.0f + simplex3d(__ctx, 2.15f * pos), 0.0f, 1.0f);
  pos.data[0] *= 0.1f;
  pos.data[1] *= 5.0f;
  float field = simplex3d(__ctx, pos);
  field = 0.1f * pow(1.5f + field, 5.0f);
  return field + amp * 200.0f * pos.data[2];
}
f32x3 gradientRocks(Context &__ctx, f32x3 pos) {
  float e = 0.001f;
  float m;
  float dx = rockfield(__ctx, pos + f32x3(e, 0.0f, 0.0f)) -
             rockfield(__ctx, pos - f32x3(e, 0.0f, 0.0f));
  float dy = rockfield(__ctx, pos + f32x3(0.0f, e, 0.0f)) -
             rockfield(__ctx, pos - f32x3(0.0f, e, 0.0f));
  float dz = rockfield(__ctx, pos + f32x3(0.0f, 0.0f, e)) -
             rockfield(__ctx, pos - f32x3(0.0f, 0.0f, e));
  return f32x3(dx, dy, dz) / e;
}
float field3d(Context &__ctx, f32x3 pos) {
  float rockfield = rockfield(__ctx, pos);
  return rockfield;
}
f32x3 gradient(Context &__ctx, f32x3 pos) {
  float e = 0.001f;
  float m;
  float dx = field3d(__ctx, pos + f32x3(e, 0.0f, 0.0f)) -
             field3d(__ctx, pos - f32x3(e, 0.0f, 0.0f));
  float dy = field3d(__ctx, pos + f32x3(0.0f, e, 0.0f)) -
             field3d(__ctx, pos - f32x3(0.0f, e, 0.0f));
  float dz = field3d(__ctx, pos + f32x3(0.0f, 0.0f, e)) -
             field3d(__ctx, pos - f32x3(0.0f, 0.0f, e));
  return f32x3(dx, dy, dz) / e;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / f32x2(iResolution.data[0]);
  f32x3 pos = f32x3(uv.data[0], uv.data[1], 0.00f * iTime);
  pos += f32x3(0.04f * iTime, 0.04f * iTime, 0.0f);
  float field = field3d(__ctx, pos);
  f32x3 normal = gradient(__ctx, pos);
  if (normal.data[2] < 0.0f)
    normal.data[2] = 0.0f;
  normal = normalize(normal);
  f32x3 light = normalize(f32x3(sin(-0.01f * iMouse.data[0]),
                                -1.0f * cos(0.01f * iMouse.data[1]), -1.0f));
  float l = dot(light, -normal);
  f32x3 color = f32x3(1.0f, 1.0f, 1.0f);
  fragColor = f32x4(color * l, 1.0f);
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
