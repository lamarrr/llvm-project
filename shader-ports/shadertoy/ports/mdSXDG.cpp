// Port of https://www.shadertoy.com/view/mdSXDG; author: derSchamane.
// Dataset license metadata: unknown-license-reference; original retained in
// upstream/mdSXDG.json.
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
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a);
  float c = cos(a);
  return f32x2x2(c, -s, s, c);
}
f32x3 Rot(Context &__ctx, f32x3 p, f32x3 r) {
  p.set_swizzle<0, 2>(cos(r.data[1]) * p.swizzle<0, 2>() +
                      sin(r.data[1]) * f32x2(p.swizzle<0, 2>().data[1],
                                             -p.swizzle<0, 2>().data[0]));
  p.set_swizzle<1, 0>(cos(r.data[2]) * p.swizzle<1, 0>() +
                      sin(r.data[2]) * f32x2(p.swizzle<1, 0>().data[1],
                                             -p.swizzle<1, 0>().data[0]));
  p.set_swizzle<2, 1>(cos(r.data[0]) * p.swizzle<2, 1>() +
                      sin(r.data[0]) * f32x2(p.swizzle<2, 1>().data[1],
                                             -p.swizzle<2, 1>().data[0]));
  return p;
}
float smin(Context &__ctx, float a, float b, float k) {
  float h = max(k - abs(a - b), 0.f);
  return min(a, b) - h * h * .25f / k;
}
float sin3(Context &__ctx, float x) { return sin(x) * sin(x) * sin(x); }
f32x3 sin3(Context &__ctx, f32x3 x) { return sin(x) * sin(x) * sin(x); }
const int iters = 7;
const f32x3 fTra = f32x3(0);
const f32x3 fRot = f32x3(0);
const f32x4 k = f32x4(2.f, 3.2f, 4.5f, 4.5f);
f32x2 Map(Context &__ctx, f32x3 p) {
  float d, col;
  f32x3 po = p;
  int i;
  float x1, y1, s = 10.f, lp = length(p);
  p /= 2.f;
  p.set_swizzle<0, 2>(mod(abs(p.swizzle<0, 2>() - s / 2.f), s) - s / 2.f);
  p.data[1] +=
      length(dot(cos(po.swizzle<2, 0>() / 50.f),
                 sin(po.swizzle<0, 2>() / 30.f + f32x2(iTime * .8f / 50.f)))) *
          10.f * smoothstep(10.f, 50.f, lp) -
      pow(length(po / 100.f), 3.f) * 3.f + smoothstep(20.f, 50.f, lp) * 3.f;
  float r =
      p.data[0] * p.data[0] + p.data[1] * p.data[1] + p.data[2] * p.data[2];
  for (i = 0; i < iters && r < 1e10f; i++) {
    if (i > 0) {
      p += fTra - 1.f - sin3(__ctx, iTime * .8f / f32x3(33, 42, 51));
      p = Rot(__ctx, p,
              fRot + .00001f +
                  3.14159265358979f * .5f *
                      sin3(__ctx, 3.14159265358979f +
                                      iTime * .8f / f32x3(61, 52, 43)));
    }
    p = abs(p);
    if (p.data[0] - p.data[1] < 0.f) {
      x1 = p.data[1];
      p.data[1] = p.data[0];
      p.data[0] = x1;
    }
    if (p.data[0] - p.data[2] < 0.f) {
      x1 = p.data[2];
      p.data[2] = p.data[0];
      p.data[0] = x1;
    }
    if (p.data[1] - p.data[2] < 0.f) {
      y1 = p.data[2];
      p.data[2] = p.data[1];
      p.data[1] = y1;
    }
    p.data[2] -= 0.5f * k.data[0] * (k.data[1] - 1.f) / k.data[1];
    p.data[2] = -abs(p.data[2]);
    p.data[2] += 0.5f * k.data[0] * (k.data[1] - 1.f) / k.data[1];
    p.data[0] = k.data[1] * p.data[0] - k.data[2] * (k.data[1] - 1.f);
    p.data[1] = k.data[1] * p.data[1] - k.data[3] * (k.data[1] - 1.f);
    p.data[2] = k.data[1] * p.data[2];
    r = p.data[0] * p.data[0] + p.data[1] * p.data[1] + p.data[2] * p.data[2];
  }
  d = length(p) * pow(k.data[1], float(-i));
  col = 0.1f;
  float wv = sin(p.data[0] / 1e3f + p.data[2] / 1e3f);
  float bt = pow(max(0.f, po.data[1] + .1f * wv), 1.2f);
  col = mix(col, 1.95f, step(bt, d));
  d = smin(__ctx, d, bt * .6f,
           1.f + .5f * sin(po.data[0] / 1e2f + iTime * .8f / 3.f));
  float sd = length(po - f32x3(0, 17.f + sin(iTime * .8f / 5.f), 0)) -
             .2f * smoothstep(.3f, 0.f, abs(fract(iTime * .8f / 3.f) - .5f));
  float sp = sd - 4.f * (1.f + pow(abs(cos(p.data[1] / 1e3f)), 2.f) * .1f);
  col = mix(col, 4.7f, step(sp, d));
  d = min(d, sp);
  s = sd - 4.05f;
  col = mix(col, -3.4f, step(s, d));
  d = min(d, s);
  return f32x2(d, col);
}
f32x3 Normal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(.001f, 0);
  return normalize(Map(__ctx, p).data[0] -
                   f32x3(Map(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                         Map(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                         Map(__ctx, p - e.swizzle<1, 1, 0>()).data[0]));
}
f32x3 RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float col = 0.f;
  float dO = .1f;
  int steps = 0;
  for (int i = 0; i < 200; i++) {
    steps = i;
    f32x3 p = ro + rd * dO;
    f32x2 dS = Map(__ctx, p);
    col = dS.data[1];
    dO +=
        dS.data[0] * mix(.9f, 1.f, dO / 300.f) *
        (1.f - smoothstep(20.f, -.4f - 2.7f * smoothstep(30.f, 10.f, p.data[1]),
                          length(p.swizzle<0, 2>())));
    if (dO > 300.f ||
        dS.data[0] < (.01f * (pow(dO / 300.f, 1.5f) * 40.f + 1.f)))
      break;
  }
  return f32x3(dO, steps, col);
}
float SoftShadow(Context &__ctx, f32x3 ro, f32x3 lp, float k) {
  const int maxIterationsShad = 24;
  f32x3 rd = lp - ro;
  float shade = 1.f;
  float dist = .002f;
  float end = max(length(rd), .001f);
  float stepDist = end / float(maxIterationsShad);
  rd /= end;
  for (int i = 0; i < maxIterationsShad; i++) {
    float h = Map(__ctx, ro + rd * dist).data[0];
    shade = min(shade, smoothstep(0.f, 1.f, k * h / dist));
    dist += clamp(h, .02f, .25f);
    if (h < .0f || dist > end)
      break;
  }
  return min(max(shade, 0.f) + .0f, 1.f);
}
float CalcAO(Context &__ctx, const f32x3 p, const f32x3 n) {
  float occ = 0.28f;
  float sca = 0.6f;
  for (int i = 0; i < 5; i++) {
    float h = .001f + .150f * float(i) / 4.0f;
    float d = Map(__ctx, p + h * n).data[0];
    occ += (h - d) * sca;
    sca *= 0.95f;
  }
  return smoothstep(.0f, 1.f, 1.f - 1.5f * occ);
}
f32x3 R(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = p + f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i - p);
  return d;
}
f32x3 hsv2rgb_smooth(Context &__ctx, f32x3 c) {
  f32x3 rgb =
      clamp(abs(mod(c.data[0] * 6.f + f32x3(0.f, 4.f, 2.f), 6.f) - 3.f) - 1.f,
            0.f, 1.f);
  rgb = rgb * rgb * (3.f - 2.f * rgb);
  return c.data[2] * mix(f32x3(1.f), rgb, c.data[1]);
}
const f32x3 ambCol = f32x3(.01f, .02f, .03f) * 3.f;
const f32x3 sunCol = f32x3(1.f, .8f, .7f) * 1.f;
const f32x3 skyCol = f32x3(.3f, .6f, 1.f) * 1.f;
const float specExp = 5.f;
f32x3 Shade(Context &__ctx, f32x3 col, float mat, f32x3 p, f32x3 n, f32x3 rd,
            f32x3 lP) {
  f32x3 lidi = normalize(lP - p);
  float mafa = max(mat, .0f), amoc = CalcAO(__ctx, p, n),
        shad = SoftShadow(__ctx, p + n * .015f, lP, 2.f),
        diff = max(dot(n, lidi), 0.f) * shad,
        spec = pow(diff, max(1.f, specExp * mafa)),
        refl = pow(max(0.f, dot(lidi, reflect(rd, n))),
                   max(1.f, specExp * 3.f * mafa)) *
               shad,
        indi = 1.f;
  return mix(indi * mix(ambCol, skyCol, .03f) * col * amoc +
                 mix(diff * col * sunCol, (spec * col + refl * mafa), mafa) *
                     sunCol,
             col * smoothstep(0.f, 1.f, amoc * amoc + .5f), max(-mat, 0.f));
}
f32x3 Palette(Context &__ctx, int index) {
  switch (index) {
  case 0:
    return f32x3(1.f, .85f, .7f);
  case 1:
    return hsv2rgb_smooth(__ctx, f32x3(.61f, .7f, .5f));
  case 2:
    return hsv2rgb_smooth(__ctx, f32x3(.18f, .3f, .9f));
  case 3:
    return hsv2rgb_smooth(
        __ctx,
        f32x3(.3f + .3f * (.5f + .5f * sin(iTime * .8f / 21.f)), .6f, .8f));
  case 4:
    return hsv2rgb_smooth(__ctx, f32x3(.95f, .8f, .6f));
  }
  return f32x3(0);
}
f32x4 PP(Context &__ctx, f32x3 col, f32x2 uv) {
  col = mix(col, smoothstep(f32x3(0.f), f32x3(1.f), col), 0.6f);
  col *= smoothstep(2.0f, -2.0f / 5.f, dot(uv, uv));
  col = pow(col, f32x3(.4545f));
  return f32x4(col, 1.f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  if (length(m) <= 0.f)
    m = f32x2(.5f);
  f32x3 ro = f32x3(0.f, 20.f, -50.f + sin3(__ctx, iTime * .8f / 42.f) * 20.f);
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() *
                       (Rot(__ctx, m.data[1] * 3.14159265358979f / 4.f +
                                       3.14159265358979f / 1.6f)));
  ro.set_swizzle<0, 2>(
      ro.swizzle<0, 2>() *
      (Rot(__ctx, -m.data[0] * 3.14159265358979f + iTime * .8f / 30.f)));
  f32x3 rd = R(__ctx, uv, ro, f32x3(0, 20, 0), 0.8f);
  f32x3 lPos = f32x3(1, 3, 3) * 100.f;
  f32x3 col = skyCol;
  f32x3 p = f32x3(0.f);
  f32x3 rmd = RayMarch(__ctx, ro, rd);
  p = ro + rd * rmd.data[0];
  f32x3 bg = mix(f32x3(0), skyCol, smoothstep(-100.f, 150.f, p.data[1]));
  if (rmd.data[0] < 300.f) {
    f32x3 n = Normal(__ctx, p);
    float shine = fract(rmd.data[2]) * abs(rmd.data[2]) / rmd.data[2];
    int ix = int(floor(abs(rmd.data[2])));
    col = Palette(__ctx, ix);
    if (ix == 1) {
      n = n * (dot(sin(p.swizzle<0, 2>() / 2.f *
                       Rot(__ctx, 1.f + .1f * sin(iTime * .8f / 8.f))),
                   cos(p.swizzle<0, 2>() / 3.f) *
                       Rot(__ctx, 1.f + .1f * sin(iTime * .8f / 5.f))) *
                   .03f +
               1.f);
      col = col * (1.f + (p.data[1] + .1f) * 5.f);
      col = col * (1.f + .05f * pow(clamp(smoothstep(250.f, 20.f, length(p)) *
                                              (p.data[1] + .1f) * 4.5f,
                                          0.f, 1.1f),
                                    30.f));
    } else if (ix == 0)
      col = mix(col, f32x3(.6f),
                .5f + .5f * dot(sin3(__ctx, p / 17.f + 5.f),
                                cos(Rot(__ctx, p / 13.f, f32x3(4)))));
    col = Shade(__ctx, col, shine, p, n, rd, lPos);
  }
  float disFac = smoothstep(0.f, 1.f, pow(rmd.data[0] / 300.f, 2.f));
  col = mix(col, bg, disFac);
  col +=
      pow(max(0.f, rmd.data[1] / float(200)), 1.5f) * normalize(ambCol) * 1.0f;
  float dir = dot(rd, normalize(lPos));
  col += pow(dir * disFac * .5f + .5f, 10.f) * sunCol * .4f;
  col += 2e-5f / (1e-5f + smoothstep(1.f, -1.f, dir * disFac)) * sunCol;
  col *= 1.f - 1.f / (1.f + iTime * .8f / 3.f);
  fragColor = PP(__ctx, col, uv);
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
