// Port of https://www.shadertoy.com/view/fdSyzR; author: z0rg.
// Dataset license metadata: cc-by-nc-4.0; original retained in
// upstream/fdSyzR.json.
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
f32x2x2 r2d(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, -s, s, c);
}
f32x3 getCam(Context &__ctx, f32x3 rd, f32x2 uv) {
  float fov = (sin(iTime * .25f) * .3f + .5f) * 2.f;
  f32x3 r = normalize(cross(rd, f32x3(0.f, 1.f, 0.f)));
  f32x3 u = normalize(cross(rd, r));
  return normalize(rd + (r * uv.data[0] + u * uv.data[1]) * fov);
}
f32x2 _min(Context &__ctx, f32x2 a, f32x2 b) {
  if (a.data[0] < b.data[0])
    return a;
  return b;
}
float _cube(Context &__ctx, f32x3 p, f32x3 s) {
  f32x3 l = abs(p) - s;
  return max(l.data[0], max(l.data[1], l.data[2]));
}
f32x2 map(Context &__ctx, f32x3 p) {
  f32x2 acc = f32x2(1000.f, -1.f);
  p.set_swizzle<0, 1>(
      p.swizzle<0, 1>() *
      (r2d(__ctx, p.data[2] * .05f * length(p.swizzle<0, 1>()) * .2f)));
  f32x3 pshape = p;
  float repay = 10.f;
  float repa = 5.f;
  float repz = 20.f;
  float idz = floor((pshape.data[2] + repz * .5f) / repz);
  pshape.data[2] =
      mod(pshape.data[2] + iTime * 20.f + repz * .5f, repz) - repz * .5f;
  pshape.data[1] = mod(pshape.data[1] + repay * .5f, repay) - repay * .5f;
  pshape.data[0] += (sin(iTime + pshape.data[1] * 2.f) + iTime);
  pshape.data[0] = mod(pshape.data[0] + repa * .5f, repa) - repa * .5f;
  float shape =
      mix(_cube(__ctx, pshape, f32x3(.3f * (sin(iTime) * .2f + .5f), 1.f, .1f)),
          length(pshape) - .5f, sin(idz + iTime * .2f));
  acc = _min(__ctx, acc, f32x2(shape, 0.f));
  f32x3 pshape2 = p + f32x3(0.f, 0.f, iTime * 13.f);
  f32x3 rep2 = f32x3(5.f);
  pshape2 = mod(pshape2 + rep2 * .5f, rep2) - rep2 * .5f;
  float shape2 = _cube(__ctx, pshape2, f32x3(.1f));
  acc = _min(__ctx, acc, f32x2(shape2, 0.f));
  return acc;
}
f32x3 getNorm(Context &__ctx, f32x3 p, float d) {
  f32x2 e = f32x2(0.01f, 0.f);
  return normalize(f32x3(d) -
                   f32x3(map(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                         map(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                         map(__ctx, p - e.swizzle<1, 1, 0>()).data[0]));
}
f32x3 accCol;
f32x3 trace(Context &__ctx, f32x3 ro, f32x3 rd, int steps) {
  accCol = f32x3(0.f);
  f32x3 p = ro;
  for (int i = 0; i < 64; ++i) {
    f32x2 res = map(__ctx, p);
    if (res.data[0] < 0.01f)
      return f32x3(res.data[0], distance(p, ro), res.data[1]);
    accCol += f32x3(1.f, .2f, .5f * (sin(p.data[2] * 10.f) * .5f + .5f))
                  .swizzle<2, 1, 0>() *
              (1.f - clamp(res.data[0] / 1.5f, 0.f, 1.f)) * .02f;
    p += rd * res.data[0];
  }
  return f32x3(-1.f);
}
f32x3 rdr(Context &__ctx, f32x2 uv) {
  f32x3 col = f32x3(0.f);
  f32x3 ro = f32x3(1.f, sin(iTime), -15.f);
  f32x3 ta = f32x3(0.f, sin(iTime * .0125f) * 14.f, 0.f);
  f32x3 rd = normalize(ta - ro);
  rd = getCam(__ctx, rd, uv);
  f32x3 res = trace(__ctx, ro, rd, 128);
  if (res.data[1] > 0.f) {
    f32x3 p = ro + rd * res.data[1];
    f32x3 n = getNorm(__ctx, p, res.data[0]);
    col = n * .5f + .5f;
    col = f32x3(0.8f, 0.4f, 0.3f) * (1.f - clamp(res.data[1] / 30.f, 0.f, 1.f));
  }
  col += accCol;
  return col;
}
float _sqr(Context &__ctx, f32x2 uv, f32x2 s) {
  f32x2 l = abs(uv) - s;
  return max(l.data[0], l.data[1]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord.swizzle<0, 1>() - .5f * iResolution.swizzle<0, 1>()) /
             iResolution.swizzle<0, 0>();
  f32x2 ouv = uv;
  uv -= f32x2(.0f);
  float a = atan(uv.data[1], uv.data[0]);
  float stp = 3.14159265f * 2.f / (3.f + mod(floor(iTime * .5f), 5.f));
  float b = mod(a + stp * .5f, stp) - stp * .5f;
  uv = f32x2(sin(b), cos(b)) * length(uv);
  f32x3 col = rdr(__ctx, uv);
  f32x2 coords = mix(uv, ouv, sin(iTime)) + f32x2(iTime * .1f, 0.f) * .1f;
  float rep = .3f;
  coords.data[0] = mod(coords.data[0] + rep * .5f, rep) - rep * .5f;
  float shape = _sqr(__ctx, coords, f32x2(.05f, .2f));
  col = mix(col, col.swizzle<2, 0, 1>(), 1.f - clamp(shape * 400.f, 0.f, 1.f));
  col = pow(col, f32x3(1.f));
  col.set_swizzle<0, 1>(col.swizzle<0, 1>() * (r2d(__ctx, iTime * 1.5f)));
  col.set_swizzle<0, 2>(col.swizzle<0, 2>() * (r2d(__ctx, -iTime * 1.25f)));
  fragColor = f32x4(col * 5.f, 1.0f);
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
