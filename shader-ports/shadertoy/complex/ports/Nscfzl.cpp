// Port of https://www.shadertoy.com/view/Nscfzl; author: byt3_m3chanic.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/Nscfzl.json.
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
  Texture2D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float noise(Context &__ctx, f32x2 uv) {
  f32x2 i = floor(uv), f = fract(uv);
  float a = fract(sin(dot(i, f32x2(21.23f, 41.32f))) * 43758.5453f),
        b = fract(sin(dot(i + f32x2(1.0f, 0.0f), f32x2(21.23f, 41.32f))) *
                  43758.5453f),
        c = fract(sin(dot(i + f32x2(0.0f, 1.0f), f32x2(21.23f, 41.32f))) *
                  43758.5453f),
        d = fract(sin(dot(i + f32x2(1.0f, 1.0f), f32x2(21.23f, 41.32f))) *
                  43758.5453f);
  f32x2 u = f * f * (3.f - 2.0f * f);
  return mix(a, b, u.data[0]) + (c - a) * u.data[1] * (1.0f - u.data[0]) +
         (d - b) * u.data[0] * u.data[1];
}
float fbm(Context &__ctx, f32x2 uv, float oct) {
  float v = .0f, a = .5f;
  f32x2 shift = f32x2(100.0f);
  for (float i = 0.f; i < oct; ++i) {
    v += a * noise(__ctx, uv);
    uv = f32x2x2(cos(.15f + f32x4(3, 14, 36, 3))) * uv * 2.0f + shift;
    a *= 0.5f;
  }
  return v;
}
const float sz = 3.f, hf = sz / 2.f, hz = hf * .25f;
const f32x2 vc = f32x2(0, .5f);
f32x3 hp, hitp;
f32x2 sid, idi;
float time = 0.f, slow = .0f, glow = .0f, shs, ghs, ffs = 0.f;
f32x2 path(Context &__ctx, float z) {
  f32x2 p1 = f32x2(2.3f * sin(z * .065f), 2.4f * sin(z * .045f));
  f32x2 p2 = f32x2(1.2f * cos(z * .050f), 2.1f * cos(z * .125f));
  return p1 - p2;
}
f32x2 map(Context &__ctx, f32x3 p) {
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() - (f32x2(1.5f, (ffs - .5f))));
  f32x2 r = f32x2(1e5f, 0);
  f32x2 track = p.swizzle<0, 1>() - path(__ctx, p.data[2]);
  f32x3 q = f32x3(track, p.data[2]);
  f32x3 qq = q;
  f32x2 id = floor((q.swizzle<0, 2>() + hf) / sz);
  q.set_swizzle<0, 2>(mod(q.swizzle<0, 2>() + hf, sz) - hf);
  float f = length(q.data[1] - 1.f) - .1f;
  float hs = fract(sin(dot(id, f32x2(21.23f, 41.32f))) * 43758.5453f);
  shs = hs;
  idi = id;
  if (hs > .65f) {
    f32x3 pp = q;
    pp.set_swizzle<0, 2>(abs(pp.swizzle<0, 2>()) - (hf * .5f));
    pp.data[1] -= .75f;
    f = min(length(max(abs(pp) - f32x3(.01f, .5f, .01f), 0.f)), f);
    float lm = length(pp + f32x3(0, .5f, 0)) - .095f;
    f = min(lm, f);
    slow += .00003f / (.00005f + lm * lm);
  }
  p.set_swizzle<0, 2>(abs(q.swizzle<0, 2>()) - hf);
  q.data[1] -= .5f;
  float g = length(max(abs(q) - f32x3(hz, hf + hs, hz), 0.f)) - .015f;
  if (f < r.data[0]) {
    r = f32x2(f, 2);
    hp = q;
  }
  if (g < r.data[0]) {
    r = f32x2(g, 4);
    hp = qq;
  }
  return r;
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 F) {
  time = iTime + texture(__ctx.channel1, F / 8.f).data[0] * iTimeDelta;
  ffs = clamp((3.5f - iTime * .08f), 0.f, 3.75f);
  float mt = mod(floor(iTime * .5f), 14.f);
  float sp = clamp((3.f + iTime) * .09f, 0.f, 11.f);
  float travelSpeed = sp * time;
  float alpha = 1.f;
  f32x2 uv = (2.f * F.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]),
        id = f32x2(0);
  f32x3 C = f32x3(0), p = f32x3(0), lp = f32x3(0, 0, -travelSpeed),
        ro = f32x3(0, 0, .25f);
  float crop = clamp((-.02f) + (iTime * .04f), 0.f, .6f);
  if (uv.data[1] < crop && uv.data[1] > -crop) {
    float x = iMouse.swizzle<0, 1>() == f32x2(0) || iMouse.data[2] < 0.f
                  ? 0.f
                  : (iMouse.data[1] / iResolution.data[1] * 0.5f - .25f) *
                        3.14159265359f;
    float y = iMouse.swizzle<0, 1>() == f32x2(0) || iMouse.data[2] < 0.f
                  ? mt > 8.f ? 3.14159265359f : 0.f
                  : (iMouse.data[0] / iResolution.data[0] * 2.0f - 1.0f) *
                        3.14159265359f;
    ro.set_swizzle<2, 1>(ro.swizzle<2, 1>() *
                         (f32x2x2(cos(x + f32x4(3, 14, 36, 3)))));
    ro += lp;
    lp.set_swizzle<0, 1>(lp.swizzle<0, 1>() + (path(__ctx, lp.data[2])));
    ro.set_swizzle<0, 1>(ro.swizzle<0, 1>() + (path(__ctx, ro.data[2])));
    f32x3 f = normalize(lp - ro), r = normalize(cross(f32x3(0, 1, 0), f)),
          u = normalize(cross(f, r)), c = ro + f * .65f,
          i = c + uv.data[0] * r + uv.data[1] * u, rd = i - ro;
    rd.set_swizzle<0, 1>(
        f32x2x2(cos(-path(__ctx, lp.data[2] + 2.f).data[0] / 24.f +
                    f32x4(3, 14, 36, 3))) *
        rd.swizzle<0, 1>());
    rd.set_swizzle<0, 2>(f32x2x2(cos(y + f32x4(3, 14, 36, 3))) *
                         rd.swizzle<0, 2>());
    float clouds = max(rd.data[1], 0.0f) * 1.5f;
    f32x2 sv = .75f * rd.swizzle<0, 2>() / rd.data[1];
    clouds +=
        1.45f *
        (-3.0f + 4.f * smoothstep(-0.2f, 0.85f,
                                  fbm(__ctx, sv + f32x2(0, iTime * .2f), 6.f)));
    clouds = mix(clouds, 1.f, clamp(abs(sv.data[1]) * .075f, 0.f, 1.f));
    f32x3 fde = f32x3(0.212f, 0.212f, 0.212f);
    f32x3 sky = mix(f32x3(.3f), fde, clamp(clouds, 0.f, 1.f));
    float d = 0.f, m = 0.f;
    for (int i = 0; i++ < 164 && d < 85.f;) {
      p = ro + rd * d;
      f32x2 t = map(__ctx, p);
      d += i < 32 ? t.data[0] * .65f : t.data[0];
      m = t.data[1];
    }
    sid = idi;
    hitp = hp;
    glow = slow;
    ghs = shs;
    float t = map(__ctx, p).data[0], sd = 1.f, z = .01f,
          hs = fract(sin(dot(sid, f32x2(21.23f, 41.32f))) * 43758.5453f);
    f32x2 e = f32x2(d * .001f, 0);
    f32x3 l = normalize(f32x3(-5, -15, 5)),
          n = t - f32x3(map(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                        map(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                        map(__ctx, p - e.swizzle<1, 1, 0>()).data[0]);
    n = normalize(n);
    for (float z = .01f; z < 16.f;) {
      float h = map(__ctx, p + l * z).data[0];
      if (h < .001f) {
        sd = 0.f;
        break;
      }
      sd = min(sd, 18.f * h / z);
      z += h;
      if (sd < .001f)
        break;
    }
    float ch = mod(sid.data[0] + sid.data[1], 2.f) * 2.f - 1.f,
          diff = clamp(dot(n, l), .1f, .9f);
    diff = mix(diff, diff * sd, .75f);
    f32x3 h = ch > .5f ? f32x3(.2f, 0, 0) : f32x3(.7f);
    if (m == 2.f) {
      h = f32x3(.15f);
      float sc = 1.f / sz, px = 4.f / iResolution.data[0];
      p.set_swizzle<0, 2>(p.swizzle<0, 2>() - (hf));
      f32x2 f = fract(hitp.swizzle<0, 2>() * sc) - .5f;
      float e = min(length(f.data[0]) - .2f, length(f.data[1]) - .2f),
            d = smoothstep(px, -px, abs(abs(e) - .01f) - .005f);
      e = smoothstep(px, -px, e);
      h = mix(h, f32x3(.01f), e);
      h = mix(h, f32x3(.7f), d);
      float fy = mod(f.data[1] + .05f, .1f) - .05f;
      e = max(length(abs(f.data[0]) - .01f) - .001f, length(fy) - .03f);
      e = smoothstep(px, -px, e);
      h = mix(h, f32x3(.4f, .2f, .0f), e);
    }
    if (m == 3.f)
      h = f32x3(.5f);
    if (m == 4.f) {
      f32x3 f = floor(hitp * 16.f);
      float hs = min(fract(sin(dot(f.swizzle<0, 1>(), f32x2(21.23f, 41.32f))) *
                           43758.5453f),
                     fract(sin(dot(f.swizzle<2, 1>(), f32x2(21.23f, 41.32f))) *
                           43758.5453f));
      float lvl = .925f + ghs;
      h = hitp.data[1] > -lvl ? f32x3(.05f) : f32x3(.2f);
      if (hs > .5f && hitp.data[1] > -lvl)
        h = hs > .75f ? f32x3(.9f, .9f, .2f) : f32x3(.4f);
      if (hs < .15f && hitp.data[1] > -lvl)
        h = f32x3(.025f);
    }
    C = diff * h;
    glow = clamp(glow, 0.f, .95f);
    C = mix(C, f32x3(.9f, .9f, .6f), glow);
    C = mix(C, fde, 1.f - exp(-.00004f * d * d * d));
    C = d < 85.f ? C : sky;
  }
  O = f32x4(pow(C, f32x3(.4545f)), 1);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 1) texture_2d<f32x4> channel1,
               __binding__(1, 1) sampler sampling1) {
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
  Context __ctx{{channel1, sampling1}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
