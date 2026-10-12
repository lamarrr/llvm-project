// Port of https://www.shadertoy.com/view/7dScWK; author: pyBlob.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7dScWK.json.
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
f32x2 world(Context &__ctx, f32x2 O);
f32x2 wcam(Context &__ctx, f32x2 O);
float line(Context &__ctx, f32x2 p, f32x2 a, f32x2 b, float c0, float c1) {
  p -= a;
  b -= a;
  float u = clamp(dot(p, b) / dot(b, b), c0, c1);
  return length(p - b * u);
}
float line(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  return line(__ctx, p, a, b, 0.f, 1.f);
}
float arrow(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  a -= b;
  p -= b;
  f32x2 u = .2f * a;
  f32x2 v = .4f * f32x2(-u.data[1], u.data[0]);
  return min(line(__ctx, p, a, f32x2(0), 0.f, 1.f),
             min(line(__ctx, p, f32x2(0), u + v, 0.f, 1.f),
                 line(__ctx, p, f32x2(0), u - v, 0.f, 1.f)));
}
float f;
void blend(Context &__ctx, f32x4 &C, f32x3 v, float r) {
  float a = smoothstep(1.0f, 0.0f, r);
  C = mix(C, f32x4(v, 1), a);
}
f32x3 fin(Context &__ctx, f32x2 x) { return f32x3(x, 1); }
f32x3 inf(Context &__ctx, f32x2 x) { return f32x3(x, 0); }
f32x2 hom(Context &__ctx, f32x3 x) { return x.swizzle<0, 1>() / x.data[2]; }
void mainImage(Context &__ctx, f32x4 &C, f32x2 O0) {
  f = 5.f / iResolution.data[1];
  f32x2 O = world(__ctx, O0);
  f32x2 cam = f32x2(3, 3) + .1f * f;
  f32x2 a0 = cam + f32x2(-10, 1) / 10.f;
  f32x2 b0 = cam + f32x2(1, -10) / 10.f;
  if (iMouse.data[2] > 0.f)
    cam = world(__ctx, iMouse.swizzle<0, 1>() + .1f);
  float hf = .8f + .2f * cos(iTime / 3.f);
  cam.data[1] *= hf;
  a0.data[1] *= hf;
  b0.data[1] *= hf;
  float sf = .5f * cos(iTime / 2.f);
  cam.data[0] += cam.data[1] * sf;
  a0.data[0] += a0.data[1] * sf;
  b0.data[0] += b0.data[1] * sf;
  float cf = 1.f + cos(iTime);
  f32x2 a = a0 + (a0 - cam) * cf;
  f32x2 b = b0 + (b0 - cam) * cf;
  C = f32x4(0);
  blend(__ctx, C, f32x3(0, 0.2f, 1), abs(O.data[1]) / f);
  blend(__ctx, C, f32x3(.2f, .2f, .2f), length(O - cam) / f - 2.f);
  blend(__ctx, C, f32x3(.2f, .2f, .2f), line(__ctx, O, cam, a, 0.f, 1e2f) / f);
  blend(__ctx, C, f32x3(.2f, .2f, .2f), line(__ctx, O, cam, b, 0.f, 1e2f) / f);
  blend(__ctx, C, f32x3(.2f, .2f, .2f), length(O - a0) / f - 2.f);
  blend(__ctx, C, f32x3(.2f, .2f, .2f), length(O - b0) / f - 2.f);
  f32x3 ab = cross(fin(__ctx, a), fin(__ctx, b));
  {
    for (float i = -20.f; i <= 20.f; i++) {
      f32x2 s = f32x2(1, 0) * i;
      f32x2 t = hom(__ctx, cross(cross(fin(__ctx, s), fin(__ctx, cam)), ab));
      blend(__ctx, C, f32x3(0, 0.2f, 1), line(__ctx, O, t, s) / f);
    }
    f32x2 v = f32x2(-1, 0);
    f32x2 t = hom(__ctx, cross(cross(inf(__ctx, v), fin(__ctx, cam)), ab));
    blend(__ctx, C, f32x3(0, 0.2f, 1), arrow(__ctx, O, t, t + v) / f);
    blend(__ctx, C, f32x3(1, 0.2f, 0), line(__ctx, O, a, b) / f);
    blend(__ctx, C, f32x3(.5f, 0, 0), line(__ctx, O, a, b, -1e2f, 0.f) / f);
    blend(__ctx, C, f32x3(.5f, 0, 0), line(__ctx, O, a, b, 1.f, 1e2f) / f);
  }
  f = 1.f / iResolution.data[0];
  O = wcam(__ctx, O0);
  {
    blend(__ctx, C, f32x3(0), -(O.data[1] / f + iResolution.data[0] * .06f));
    f32x2 ca = floor(f32x2(0.1f, -0.01f) / f) * f;
    f32x2 cb = floor(f32x2(0.9f, -0.01f) / f) * f;
    blend(__ctx, C, f32x3(1, 0.2f, 0), line(__ctx, O, ca, cb) / f);
    blend(__ctx, C, f32x3(.5f, 0, 0), line(__ctx, O, ca, cb, -1e2f, 0.f) / f);
    blend(__ctx, C, f32x3(.5f, 0, 0), line(__ctx, O, ca, cb, 1.f, 1e2f) / f);
    blend(__ctx, C, f32x3(.2f, .2f, .2f),
          line(__ctx, O, ca, ca - f32x2(0, .02f)) / f);
    blend(__ctx, C, f32x3(.2f, .2f, .2f),
          line(__ctx, O, cb, cb - f32x2(0, .02f)) / f);
    for (float i = -20.f; i <= 20.f; i++) {
      f32x2 s = f32x2(1, 0) * i;
      f32x2 t = hom(__ctx, cross(cross(fin(__ctx, s), fin(__ctx, cam)), ab));
      t = ca + (cb - ca) * dot(t - a, b - a) / dot(b - a, b - a);
      blend(__ctx, C, f32x3(0, 0.2f, 1),
            line(__ctx, O, t, t - f32x2(0, .02f)) / f);
    }
    f32x2 v = f32x2(-1, 0);
    f32x2 t = hom(__ctx, cross(cross(inf(__ctx, v), fin(__ctx, cam)), ab));
    t = ca + (cb - ca) * dot(t - a, b - a) / dot(b - a, b - a);
    blend(__ctx, C, f32x3(0, 0.2f, 1),
          arrow(__ctx, O, t, t + f32x2(0, -.04f)) / f);
  }
  C = sqrt(C);
}
f32x2 world(Context &__ctx, f32x2 O) {
  O -= floor(iResolution.swizzle<0, 1>() / 2.f) + .5f;
  O *= f;
  O += f32x2(0, 2);
  return O;
}
f32x2 wcam(Context &__ctx, f32x2 O) {
  O -= floor(f32x2(0, 1) * iResolution.swizzle<0, 1>()) + .5f;
  O *= f;
  O += f32x2(0);
  return O;
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
