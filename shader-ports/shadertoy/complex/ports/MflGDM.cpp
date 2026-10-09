// Port of https://www.shadertoy.com/view/MflGDM; author: Kris_Katur.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/MflGDM.json.
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
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float Smin(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  return mix(d2, d1, h) - k * h * (1.0f - h);
}
float Smax(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f - 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  return mix(d2, d1, h) + k * h * (1.0f - h);
}
float Sphere(Context &__ctx, f32x3 p, float s) { return length(p) - s; }
float Torus(Context &__ctx, f32x3 p, f32x2 t) {
  f32x2 q = f32x2(length(p.swizzle<0, 2>()) - t.data[0], p.data[1]);
  return length(q) - t.data[1];
}
float Box(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
f32x3 CastRay(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u;
  return normalize(i);
}
float SDF(Context &__ctx, f32x3 p) {
  float t = iTime * .5f;
  t += sin(iTime * .7f) * .3f + .5f;
  float iter = 16.f;
  float tor = 1e9f;
  for (float i = min(t, 0.f); i < iter; i++) {
    f32x3 pt = p;
    float j = i - iter * .5f;
    float s = sin(t + i) * .2f + .7f;
    pt.data[0] += sin(t * .15f + i * 1.3f) * 3.f;
    pt.data[1] += sin(t * .52f + i * 1.2f);
    pt.data[2] += sin(t * .32f + i * 1.1f) * 2.f;
    pt.set_swizzle<0, 2>(pt.swizzle<0, 2>() * (Rot(__ctx, t * .59f - i)));
    pt.set_swizzle<0, 1>(pt.swizzle<0, 1>() * (Rot(__ctx, t * .62f + i)));
    pt.set_swizzle<1, 2>(pt.swizzle<1, 2>() * (Rot(__ctx, t * .47f - i)));
    tor = Smin(__ctx, tor, Torus(__ctx, pt, f32x2(s, s * .2f)), .8f);
  }
  float sph = 1e9f;
  for (float i = min(t, 0.f); i < iter * .5f; i++) {
    f32x3 ps = p;
    ps.data[0] += sin(t * .33f + i * 1.1f) * 3.f;
    ps.data[1] += sin(t * .21f + i * 1.4f);
    ps.data[2] -= sin(t * .27f + i * 1.5f);
    sph = Smin(__ctx, sph, Sphere(__ctx, ps, 1.f), 1.f);
    sph += (cos(t * .13f + i - ps.data[0] * 2.f) +
            sin(t * .13f - i + ps.data[1] * 2.f) +
            sin(t * .15f + i + ps.data[2] * 2.f)) *
           .1f;
  }
  f32x3 pg = p;
  float sg = 1.3f;
  float gyr =
      abs(dot(sin(pg * sg), cos(pg.swizzle<2, 0, 1>() * sg))) / sg - .6f;
  float d = Smax(__ctx, sph, -tor, 1.f);
  d = Smin(__ctx, d, tor + .02f, .1f);
  gyr = Smax(__ctx, gyr, d - .1f, .2f);
  d = min(d, gyr);
  return d;
}
float March(Context &__ctx, f32x3 ro, f32x3 rd) {
  float h = 0.f;
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + rd * h;
    float dS = SDF(__ctx, p);
    h += dS;
    if (h > 20.f || abs(dS) < .001f)
      break;
  }
  return h;
}
f32x3 CalcNormal(Context &__ctx, f32x3 p) {
  f32x3 n = f32x3(0.0f);
  for (int i = min(iFrame, 0); i < 4; i++) {
    f32x3 e =
        0.5773f *
        (2.0f * f32x3((((i + 3) >> 1) & 1), ((i >> 1) & 1), (i & 1)) - 1.0f);
    n += e * SDF(__ctx, p + .001f * e);
  }
  return normalize(n);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float t = iTime * .5f;
  f32x2 uv =
      (2.f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 ro = f32x3(0, 0, 7), rd = CastRay(__ctx, uv, ro, f32x3(0), 2.f),
        col = f32x3(0);
  float d = March(__ctx, ro, rd);
  if (d < 20.f) {
    f32x3 p = ro + rd * d, n = CalcNormal(__ctx, p);
    n.set_swizzle<0, 2>(n.swizzle<0, 2>() * (Rot(__ctx, sin(t) * .5f)));
    float fresnel = pow(1.f + dot(rd, n), 2.f);
    col = f32x3(fresnel) * (n.data[1] * .5f + .5f);
    col += n * .5f + .5f;
  }
  col = mix(col, f32x3(1), 1.f - exp(-pow(.12f * d, 6.f)));
  col = pow(col, f32x3(.4545f));
  fragColor = f32x4(col, 1.0f);
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
