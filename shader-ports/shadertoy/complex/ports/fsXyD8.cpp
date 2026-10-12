// Port of https://www.shadertoy.com/view/fsXyD8; author: z0rg.
// Dataset license metadata: cc-by-nc-4.0; original retained in
// upstream/fsXyD8.json.
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
struct Context {
  Texture2D channel0;
};
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
float _cube(Context &__ctx, f32x3 p, f32x3 s) {
  f32x3 l = abs(p) - s;
  return max(l.data[0], max(l.data[1], l.data[2]));
}
f32x2 _min(Context &__ctx, f32x2 a, f32x2 b) {
  if (a.data[0] < b.data[0])
    return a;
  return b;
}
f32x2 map(Context &__ctx, f32x3 p) {
  f32x2 acc = f32x2(10000.f, -1.f);
  float th = 0.01f;
  f32x3 bSz = f32x3(1.f);
  float box = abs(_cube(__ctx, p, bSz)) - th;
  box = max(box, -_cube(__ctx, p - f32x3(0.f, -th * 3.f, 0.f), bSz));
  acc = _min(__ctx, acc, f32x2(box, 0.f));
  float flapH = bSz.data[2] / 2.f;
  f32x3 pFlap = p;
  pFlap.data[2] = -abs(pFlap.data[2]);
  pFlap -= f32x3(0.f, -bSz.data[1] + .1f, -bSz.data[2] - flapH);
  pFlap.set_swizzle<1, 2>(pFlap.swizzle<1, 2>() * (r2d(__ctx, -.25f)));
  float flap = _cube(__ctx, pFlap, f32x3(bSz.data[0], th, flapH));
  acc = _min(__ctx, acc, f32x2(flap, 0.f));
  for (int i = 0; i < 16; ++i) {
    float fi = float(i);
    f32x2 dir = f32x2(sin(fi), cos(fi));
    float fCube = mod(fi * .1f + iTime, 1.5f);
    float spread = pow(fCube, 0.25f);
    f32x3 pos = p - f32x3(mod(fi * .1f, .5f), fCube * fCube - 2.5f, 0.0f) +
                f32x3(dir.data[0], 0.0f, dir.data[1]) * spread * .25f;
    pos.set_swizzle<0, 1>(pos.swizzle<0, 1>() * (r2d(__ctx, fi)));
    pos.set_swizzle<0, 2>(pos.swizzle<0, 2>() * (r2d(__ctx, fi)));
    acc = _min(__ctx, acc, f32x2(_cube(__ctx, pos, f32x3(.1f)), 1.f));
  }
  return acc;
}
f32x3 getCam(Context &__ctx, f32x3 rd, f32x2 uv) {
  f32x3 r = normalize(cross(rd, f32x3(0.f, 1.f, 0.f)));
  f32x3 u = normalize(cross(rd, r));
  return normalize(rd + r * uv.data[0] + u * uv.data[1]);
}
f32x3 trace(Context &__ctx, f32x3 ro, f32x3 rd, int steps) {
  f32x3 p = ro;
  for (int i = 0; i < steps; ++i) {
    f32x2 res = map(__ctx, p);
    if (res.data[0] < 0.01f)
      return f32x3(res.data[0], distance(p, ro), res.data[1]);
    p += rd * res.data[0];
  }
  return f32x3(-1.f);
}
f32x3 getNorm(Context &__ctx, f32x3 p, float d) {
  f32x2 e = f32x2(0.01f, 0.f);
  return normalize(f32x3(d) -
                   f32x3(map(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                         map(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                         map(__ctx, p - e.swizzle<1, 1, 0>()).data[0]));
}
f32x3 rdr(Context &__ctx, f32x2 uv) {
  f32x3 col = f32x3(0.941f, 0.337f, 0.600f) *
              pow(1.f - clamp(length(uv) - .2f, 0.f, 1.f), .5f);
  float d = 10.f;
  float t = iTime * .5f;
  f32x3 ro = f32x3(sin(t) * d, -5.f, cos(t) * d);
  f32x3 ta = f32x3(0.f, -1.f, 0.f);
  f32x3 rd = normalize(ta - ro);
  rd = getCam(__ctx, rd, uv);
  f32x3 res = trace(__ctx, ro, rd, 128);
  if (res.data[1] > 0.f) {
    f32x3 p = ro + rd * res.data[1];
    f32x3 n = getNorm(__ctx, p, res.data[0]);
    col = n * .5f + .5f;
    f32x3 lpos = f32x3(8.f, -5.f, 5.f);
    f32x3 ldir = normalize(lpos - p);
    float ndotl = dot(n, ldir);
    if (res.data[2] == 0.f) {
      f32x2 coords = p.swizzle<0, 1>() *
                         clamp(abs(dot(n, f32x3(0.f, 0.f, 1.f))), 0.f, 1.f) +
                     p.swizzle<2, 1>() *
                         clamp(abs(dot(n, f32x3(1.f, 0.f, 0.f))), 0.f, 1.f);
      col = pow(texture(__ctx.channel0, coords * .2f).data[0], .025f) *
                f32x3(0.831f, 0.545f, 0.306f) +
            f32x3(.2f) * clamp(ndotl, 0.f, 1.f);
    }
    if (res.data[2] == 1.0f) {
      col = f32x3(.75f) + f32x3(.5f) * clamp(ndotl, 0.f, 1.f);
    }
  }
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord - .5f * iResolution.swizzle<0, 1>()) /
             iResolution.swizzle<0, 0>();
  f32x3 col = rdr(__ctx, uv);
  fragColor = f32x4(col, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
               __binding(1, 0) sampler sampling0) {
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
