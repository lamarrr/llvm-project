// Port of https://www.shadertoy.com/view/dtfGWj; author: z0rg.
// Dataset license metadata: cc-by-nc-4.0; original retained in
// upstream/dtfGWj.json.
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
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float _seed;
float hash11(Context &__ctx, float seed) {
  return fract(sin(seed * 123.456f) * 123.456f);
}
float rand(Context &__ctx) { return hash11(__ctx, _seed++); }
f32x2x2 r2d(Context &__ctx, float a) {
  float c = cos(a);
  float s = sin(a);
  return f32x2x2(c, -s, s, c);
}
float _sqr(Context &__ctx, f32x2 uv, f32x2 s) {
  f32x2 l = abs(uv) - s;
  return max(l.data[0], l.data[1]);
}
float _cube(Context &__ctx, f32x3 p, f32x3 s) {
  f32x3 l = abs(p) - s;
  return max(l.data[0], max(l.data[1], l.data[2]));
}
f32x3 getCam(Context &__ctx, f32x3 rd, f32x2 uv) {
  f32x3 r = normalize(cross(rd, f32x3(0.f, 1.f, 0.f)));
  f32x3 u = normalize(cross(rd, r));
  return normalize(rd + (r * uv.data[0] + u * uv.data[1]) * 3.5f);
}
f32x2 _min(Context &__ctx, f32x2 a, f32x2 b) {
  if (a.data[0] < b.data[0])
    return a;
  return b;
}
f32x2 map(Context &__ctx, f32x3 p) {
  f32x2 acc = f32x2(10000.f, -1.f);
  float height =
      clamp(-.3f +
                .7f * abs(asin(sin(p.data[0] + sin(p.data[2] + iTime * .3f)))),
            0.1f, 1.f) *
      1.5f;
  acc = _min(__ctx, acc, f32x2(-p.data[1] - height, 1.f));
  f32x3 p2 = p + f32x3(0.f, 20.f, -45.f);
  p2.set_swizzle<0, 1>(p2.swizzle<0, 1>() * (r2d(__ctx, iTime)));
  p2.set_swizzle<0, 2>(p2.swizzle<0, 2>() * (r2d(__ctx, iTime * .7f)));
  acc = _min(__ctx, acc, f32x2(_cube(__ctx, p2, f32x3(5.f)), 0.f));
  float tube = length(p.swizzle<0, 1>() - f32x2(0.f, -1.5f) -
                      f32x2(sin(p.data[2] + iTime + sin(p.data[2] * .5f)),
                            cos(p.data[2]))) -
               .02f;
  acc = _min(__ctx, acc, f32x2(tube, 2.f));
  return acc;
}
f32x3 getNorm(Context &__ctx, float d, f32x3 p) {
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
  for (int i = 0; i < steps; ++i) {
    f32x2 res = map(__ctx, p);
    if (res.data[0] < 0.01f)
      return f32x3(res.data[0], distance(p, ro), res.data[1]);
    p += rd * res.data[0] * .5f;
    if (res.data[1] == 2.f)
      accCol += f32x3(1.f, .2f, .2f) *
                (1.f - clamp(res.data[0] / .2f, 0.1f, 1.f)) * .25f;
  }
  return f32x3(-1.f);
}
f32x3 rdrSun(Context &__ctx, f32x3 rd) {
  f32x3 sun =
      (f32x3(255.f, 215.f, 90.f) / 255.f) *
      pow(clamp(5.f * (-.8f + dot(rd, normalize(f32x3(0.f, -.5f, 1.f)))), 0.1f,
                1.f),
          4.f) *
      float(rd.data[1] < 0.f);
  return sun;
}
f32x3 rdrSky(Context &__ctx, f32x3 rd) {
  f32x3 cols[5];
  cols[0] = f32x3(255.f, 71.f, 52.f) / 255.f;
  cols[1] = f32x3(255.f, 141.f, 97.f) / 255.f;
  cols[2] = f32x3(217.f, 142, 255.f) / 255.f;
  cols[3] = f32x3(103.f, 133.f, 255.f) / 255.f;
  cols[4] = .7f * f32x3(93.f, 83.f, 220.f) / 255.f;
  float cur = pow(abs(rd.data[1]) * 15.f, .5f);
  int icur = int(cur);
  f32x3 sky = mix(cols[min(icur, 4)], cols[min(icur + 1, 4)],
                  smoothstep(0.f, 1.f, fract(cur)));
  return sky * 1.3f;
}
f32x3 getMat(Context &__ctx, f32x3 p, f32x3 n, f32x3 rd, f32x3 res) {
  if (res.data[2] == 0.f)
    return (n * .5f + .5f) * .5f;
  if (res.data[2] == 1.f)
    return f32x3(.1f) * clamp(dot(rd, normalize(p - f32x3(-5.f))), 0.1f, 1.f);
  if (res.data[2] == 2.f)
    return f32x3(1.f, .3f, .4f);
  return f32x3(.1f);
}
f32x3 rdr(Context &__ctx, f32x2 uv) {
  f32x3 col = f32x3(0.f);
  uv *= r2d(__ctx, .2f * sin(iTime * .4f));
  float t = iTime * .33f;
  f32x3 ro = f32x3(0.f, -2.f, -5.f);
  f32x3 ta = f32x3(sin(iTime * .2f) * 3.f, -1.f + sin(iTime * .2f) * 2.f, 0.f);
  f32x3 rd = normalize(ta - ro);
  rd = getCam(__ctx, rd, uv);
  float depth = 100.f;
  f32x3 res = trace(__ctx, ro, rd, 256);
  if (res.data[1] > 0.f) {
    depth = res.data[1];
    f32x3 p = ro + rd * res.data[1];
    f32x3 n = getNorm(__ctx, res.data[0], p);
    col = getMat(__ctx, p, n, rd, res);
    f32x3 refl = normalize(
        reflect(rd, n) + .1f * (f32x3(rand(__ctx), rand(__ctx), rand(__ctx))));
    f32x3 resrefl = trace(__ctx, p + n * 0.01f, refl, 128);
    if (p.data[1] > -0.17f) {
      if (resrefl.data[1] > 0.f) {
        f32x3 prefl = p + refl * resrefl.data[1];
        f32x3 nrefl = getNorm(__ctx, resrefl.data[0], prefl);
        col += getMat(__ctx, prefl, nrefl, refl, resrefl);
      } else {
        col += rdrSky(__ctx, refl) + rdrSun(__ctx, refl);
      }
    }
  }
  col += accCol;
  col = mix(col, rdrSky(__ctx, rd), 1.f - exp(-depth * 0.01f));
  if (depth > 99.f)
    col += f32x3(1.f) *
           pow(texture(__ctx.channel0, rd.swizzle<0, 1>() * 1.5f).data[0], 5.f);
  if (depth > 99.f)
    col += rdrSun(__ctx, rd);
  float mask =
      1.f - clamp(_sqr(__ctx,
                       (uv - f32x2(0.f, .35f)) * r2d(__ctx, 3.14159265f * .25f),
                       f32x2(.35f)) *
                      400.f,
                  0.1f, 1.f);
  col = mix(col, col.swizzle<1, 2, 0>(), mask);
  col = pow(col, f32x3(.74545f));
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord.swizzle<0, 1>() - .5f * iResolution.swizzle<0, 1>()) /
             iResolution.swizzle<0, 0>();
  _seed = iTime + texture(__ctx.channel0, uv).data[0];
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
