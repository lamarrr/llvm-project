// Port of https://www.shadertoy.com/view/mdVGRc; author: me_123.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mdVGRc.json.
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
const int it = 10;
const float eps = 0.001f;
f32x2 boxx(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 boxSize,
           f32x3 &outNormal) {
  f32x3 m = 1.0f / rd;
  f32x3 n = m * ro;
  f32x3 k = abs(m) * boxSize;
  f32x3 t1 = -n - k;
  f32x3 t2 = -n + k;
  float tN = max(max(t1.data[0], t1.data[1]), t1.data[2]);
  float tF = min(min(t2.data[0], t2.data[1]), t2.data[2]);
  if (tN > tF || tF < 0.0f)
    return f32x2(1000.f);
  outNormal = -sign(rd) * step(t1.swizzle<1, 2, 0>(), t1.swizzle<0, 1, 2>()) *
              step(t1.swizzle<2, 0, 1>(), t1.swizzle<0, 1, 2>());
  return f32x2(tN, tF);
}
f32x3 getRay(Context &__ctx, f32x3 cameraDir, f32x2 uv) {
  f32x3 cameraPlaneU =
      f32x3(normalize(f32x2(cameraDir.data[1], -cameraDir.data[0])), 0);
  f32x3 cameraPlaneV = cross(cameraPlaneU, cameraDir);
  return normalize(cameraDir + uv.data[0] * cameraPlaneU +
                   uv.data[1] * cameraPlaneV);
}
f32x3 hash33(Context &__ctx, u32x3 q) {
  q *= u32x3(1597334673U, 3812015801U, 2798796415U);
  q = (q.data[0] ^ q.data[1] ^ q.data[2]) *
      u32x3(1597334673U, 3812015801U, 2798796415U);
  return f32x3(q) * (1.0f / float(0xffffffffU));
}
f32x3 hash31(Context &__ctx, unsigned q) {
  u32x3 n = q * u32x3(1597334673U, 3812015801U, 2798796415U);
  n = (n.data[0] ^ n.data[1] ^ n.data[2]) *
      u32x3(1597334673U, 3812015801U, 2798796415U);
  return f32x3(n) * (1.0f / float(0xffffffffU));
}
struct hit {
  f32x3 p;
  f32x3 n;
  float d;
  int i;
};
f32x2 opt(Context &__ctx, f32x2 x) { return 1.65f * x / (abs(x) + 1.f); }
f32x3 fog(Context &__ctx, f32x3 o, f32x3 d, f32x2 x, f32x3 color) {
  float dd = dot(d, d);
  float d0 = dot(o, d);
  float q =
      1.f / (sqrt(0.00001f + dd * (o.data[2] * o.data[2] - d0 * d0 / dd +
                                   dot(o.swizzle<0, 1>(), o.swizzle<0, 1>()))));
  f32x2 k = opt(__ctx, (dd * x + d0) * q);
  return 1.f - exp(-0.05f * color * (k.data[1] - k.data[0]) * q);
}
f32x3 getColor(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x3 t1;
  float s = ro.data[2] / -rd.data[2];
  if (s < 0.0f) {
    s = 1000.f;
  };
  f32x2 b = boxx(__ctx, ro, rd, f32x3(1.5f), t1);
  f32x3 c = f32x3(0);
  if (b.data[0] < 1000.f) {
    float dd = dot(rd, rd);
    float ddi = 1.f / dd;
    for (unsigned i = 0u; i < 100u; i += 1u) {
      f32x3 p = (hash31(__ctx, i) - 0.5f) * 3.0f;
      f32x3 o = ro - p;
      f32x3 color = 0.1f / hash31(__ctx, i + 63400u);
      float d0 = dot(o, rd);
      float q =
          1.f /
          (sqrt(0.000003f + dd * (o.data[2] * o.data[2] - d0 * d0 * ddi +
                                  dot(o.swizzle<0, 1>(), o.swizzle<0, 1>()))));
      f32x2 k = opt(__ctx, (dd * b + d0) * q);
      c += 5.0f * 0.001f * color * (k.data[1] - k.data[0]) * q;
    }
  }
  return 1.f - exp(-c);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>() * 0.5f) /
             iResolution.data[1];
  f32x2 m = f32x2(iTime * 0.8f, sin(iTime * 0.25f) * 0.6f + 3.1415f * 0.5f);
  if (iMouse.data[2] > 0.0f)
    m = ((iMouse.swizzle<0, 1>()) / iResolution.swizzle<0, 1>()) *
        f32x2(6.28f, 3.14159263f);
  f32x3 ro = f32x3(sin(m.data[1]) * cos(-m.data[0]),
                   sin(m.data[1]) * sin(-m.data[0]), cos(m.data[1])) *
             5.0f;
  f32x3 rd = getRay(__ctx, -normalize(ro), uv);
  f32x3 color = getColor(__ctx, ro, rd) +
                hash33(__ctx, u32x3(fragCoord.swizzle<0, 1>(), iTime)) / 128.f;
  fragColor = f32x4(color, 1);
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
