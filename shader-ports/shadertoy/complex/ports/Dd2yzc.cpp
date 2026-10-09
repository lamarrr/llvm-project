// Port of https://www.shadertoy.com/view/Dd2yzc; author: byt3_m3chanic.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/Dd2yzc.json.
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
float tspeed = 0.f, speed, tmod = 0.f, ga1 = 0.f, ga2 = 0.f, ga3 = 0.f,
      ga4 = 0.f;
f32x3 hp, hitpoint;
f32x2 gid, sid;
f32x2x2 grot, srot;
const float sz = 2.5f;
const float hf = sz / 2.f;
float hash21(Context &__ctx, f32x2 p) {
  return fract(sin(dot(p, f32x2(26.34f, 45.32f))) * 4324.23f);
}
f32x2x2 rot(Context &__ctx, float a) {
  return f32x2x2(cos(a), sin(a), -sin(a), cos(a));
}
float lsp(Context &__ctx, float b, float e, float t) {
  return clamp((t - b) / (e - b), 0.f, 1.f);
}
float eoc(Context &__ctx, float t) { return (t = t - 1.f) * t * t + 1.f; }
float box(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.f);
}
float box(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.f)) + min(max(d.data[0], d.data[1]), 0.f);
}
f32x2 map(Context &__ctx, f32x3 p) {
  f32x2 res = f32x2(1e5f, 0.f);
  p.data[2] -= iTime;
  f32x3 q = p;
  f32x2 id = floor((p.swizzle<0, 2>() + hf) / sz);
  p.set_swizzle<0, 2>(mod(p.swizzle<0, 2>() + hf, sz) - hf);
  float h = hash21(__ctx, f32x2(id.data[0], 0));
  float f = hash21(__ctx, id);
  float s = .5f + .5f * sin((h + id.data[1] * 2.5f) * .5f + iTime);
  s += .5f + .5f * cos((h + id.data[0] * 2.5f) * .5f + iTime);
  p.data[1] -= 1.75f + s;
  grot = rot(__ctx, f * 3.14159265359f - speed);
  p.set_swizzle<2, 1>(p.swizzle<2, 1>() * (grot));
  float d = box(__ctx, p, f32x3(.5f));
  if (d < res.data[0]) {
    res = f32x2(d, 2);
    hp = p + id.data[0] + id.data[1];
    sid = id;
  }
  float fl = q.data[1];
  if (fl < res.data[0]) {
    res = f32x2(fl, 1);
    hp = q;
  }
  return res;
}
f32x3 normal(Context &__ctx, f32x3 p, float t) {
  t *= 1e-4f;
  float d = map(__ctx, p).data[0];
  f32x2 e = f32x2(t, 0);
  f32x3 n = d - f32x3(map(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                      map(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                      map(__ctx, p - e.swizzle<1, 1, 0>()).data[0]);
  return normalize(n);
}
f32x2 marcher(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 &p) {
  float d = 0.f, m = 0.f;
  for (int i = 0; i < 128; i++) {
    f32x2 t = map(__ctx, p);
    d += i < 64 ? t.data[0] * .4f : t.data[0] * .8f;
    m = t.data[1];
    p = ro + rd * d;
    if (t.data[0] < d * 1e-4f || d > 55.f)
      break;
  }
  return f32x2(d, m);
}
f32x3 getFace(Context &__ctx, f32x3 h, f32x2 uv) {
  float px = 10.f / iResolution.data[0];
  f32x2 id = floor(uv), q = fract(uv) - .5f;
  float rnd = hash21(__ctx, id.swizzle<0, 1>());
  float sn = hash21(__ctx, id.swizzle<1, 0>()) * 11.f;
  sn = mod(sn, 11.f);
  float t1 = lsp(__ctx, sn, sn + .5f, tmod);
  float t2 = lsp(__ctx, sn + 2.f, sn + 2.5f, tmod);
  t1 = eoc(__ctx, t1);
  t1 = t1 * t1 * t1;
  t2 = eoc(__ctx, t2);
  t2 = t2 * t2 * t2;
  q.set_swizzle<0, 1>(q.swizzle<0, 1>() * (rot(__ctx, (t1 + t2) * 1.5707f)));
  if (rnd > .5f)
    q *= rot(__ctx, 1.5707f);
  rnd = fract(rnd * 32.232f);
  f32x2 q2 = length(q - .5f) < length(q + .5f) ? q - .5f : q + .5f;
  float tk = .1275f;
  float d1 = abs(length(q2) - .5f) - tk;
  float d3 = length(q) - .5f;
  if (rnd > .85f)
    d1 = min(length(q.data[0]) - tk, length(q.data[1]) - tk);
  d1 = max(d1, d3);
  float tt = .5f + .5f * sin(fract(iTime * .05f) * 3.14159265359f);
  tt = smoothstep(.7f, .8f, tt);
  f32x3 cr = mix(f32x3(0, .4f, .9f), f32x3(.03f), tt);
  h = mix(h, cr, smoothstep(px, -px, d3));
  h = mix(h, mix(f32x3(0), f32x3(1, 0, 0), t1 - t2), smoothstep(px, -px, d1));
  return h;
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 F) {
  tspeed = iTime * .3f;
  speed = iTime * .5f;
  tmod = mod(tspeed, 14.f);
  f32x2 uv = (2.f * F.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]);
  f32x3 ro = f32x3(0, 0, 11);
  f32x3 rd = normalize(f32x3(uv, -1.0f));
  float x = iMouse.swizzle<0, 1>() == f32x2(0) || iMouse.data[2] < 0.f
                ? 0.f
                : (iMouse.data[1] / iResolution.data[1] * .25f - .125f) *
                      3.14159265359f;
  float y = iMouse.swizzle<0, 1>() == f32x2(0) || iMouse.data[2] < 0.f
                ? 0.f
                : -(iMouse.data[0] / iResolution.data[0] * 2.f - 1.f) *
                      3.14159265359f;
  f32x2x2 rx = rot(__ctx, -.68f - x),
          ry = rot(__ctx, -1.18f - iTime * .06f - y);
  ro.set_swizzle<2, 1>(ro.swizzle<2, 1>() * (rx));
  rd.set_swizzle<2, 1>(rd.swizzle<2, 1>() * (rx));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (ry));
  rd.set_swizzle<0, 2>(rd.swizzle<0, 2>() * (ry));
  f32x3 C = f32x3(0);
  f32x3 p = ro;
  f32x2 ray = marcher(__ctx, ro, rd, p);
  float d = ray.data[0], m = ray.data[1];
  gid = sid;
  hitpoint = hp;
  srot = grot;
  if (d < 50.f) {
    f32x3 n = normal(__ctx, p, d);
    f32x3 tn = n;
    f32x3 lpos = f32x3(-15, 15, -15);
    f32x3 l = normalize(lpos - p);
    float diff = clamp(dot(n, l), 0.f, 1.f);
    float shdw = 1.f;
    for (float t = .01f; t < 16.f;) {
      float h = map(__ctx, p + l * t).data[0];
      if (h < 1e-4f) {
        shdw = 0.f;
        break;
      }
      shdw = min(shdw, 16.f * h / t);
      t += h * .75f;
      if (shdw < 1e-4f || t > 6.f)
        break;
    }
    diff = mix(diff, diff * shdw, .75f);
    float spec =
        .75f *
        pow(max(dot(normalize(p - ro), reflect(normalize(lpos), n)), 0.f),
            24.f);
    f32x3 h = f32x3(1.f);
    int face;
    if (m == 2.f) {
      f32x3 cuv;
      h = f32x3(.01f);
      tn.set_swizzle<2, 1>(tn.swizzle<2, 1>() * (srot));
      f32x3 aN = abs(tn);
      i32x3 idF = i32x3(tn.data[0] < -.25f ? 0 : 5, tn.data[1] < -.25f ? 1 : 4,
                        tn.data[2] < -.25f ? 2 : 3);
      face = aN.data[0] > .5f   ? idF.data[0]
             : aN.data[1] > .5f ? idF.data[1]
                                : idF.data[2];
      if (face == 0)
        cuv = hitpoint.swizzle<1, 2, 0>();
      if (face == 1)
        cuv = hitpoint.swizzle<0, 2, 1>();
      if (face == 2)
        cuv = hitpoint.swizzle<0, 1, 2>();
      if (face == 3)
        cuv = hitpoint.swizzle<0, 1, 2>();
      if (face == 4)
        cuv = hitpoint.swizzle<0, 2, 1>();
      if (face == 5)
        cuv = hitpoint.swizzle<1, 2, 0>();
      h = getFace(__ctx, h, (cuv.swizzle<0, 1>() + float(face)) * 2.f);
    }
    if (m == 1.f) {
      h = f32x3(.01f);
      f32x2 uv = hitpoint.swizzle<0, 2>() * .5f;
      h = getFace(__ctx, h, uv);
    }
    C = h * diff;
  }
  C = mix(C, f32x3(.005f), 1.f - exp(-.0006f * d * d * d));
  C = pow(C, f32x3(.4545f));
  O = f32x4(C, 1.f);
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
