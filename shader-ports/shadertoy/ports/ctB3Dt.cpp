// Port of https://www.shadertoy.com/view/ctB3Dt; author: byt3_m3chanic.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/ctB3Dt.json.
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
float hash21(Context &__ctx, f32x2 p) {
  return fract(sin(dot(p, f32x2(23.43f, 84.21f))) * 4832.3234f);
}
f32x2x2 rot(Context &__ctx, float a) {
  return f32x2x2(cos(a), sin(a), -sin(a), cos(a));
}
float torus(Context &__ctx, f32x3 p, f32x2 a) {
  return length(f32x2(length(p.swizzle<0, 1>()) - a.data[0], p.data[2])) -
         a.data[1];
}
f32x3 hp, hitpoint;
f32x2 gid, sid;
const float sz = 2.35f;
const float hf = sz / 2.f;
const float zz = 4.25f;
float truchet(Context &__ctx, f32x3 p, float id) {
  float res = 1e5f;
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (rot(__ctx, iTime * .025f)));
  p.data[1] += iTime * 1.5f;
  f32x3 q = p;
  f32x2 gid = floor((p.swizzle<0, 1>() + hf) / sz);
  p.set_swizzle<0, 1>(mod(p.swizzle<0, 1>() + hf, sz) - hf);
  float rnd = hash21(__ctx, gid + id);
  if (rnd > .5f)
    p.data[1] *= -1.f;
  float thk = .35f;
  thk += .1f * sin(q.data[1] * 1.3f);
  thk += .1f * sin(q.data[0] * 1.3f);
  f32x2 d2 =
      f32x2(length(p.swizzle<0, 1>() - hf), length(p.swizzle<0, 1>() + hf));
  f32x2 gx = d2.data[0] < d2.data[1] ? f32x2(p.swizzle<0, 1>() - hf)
                                     : f32x2(p.swizzle<0, 1>() + hf);
  f32x3 pv = f32x3(gx.swizzle<0, 1>(), p.data[2]);
  hp = pv;
  float d = torus(__ctx, pv, f32x2(hf, thk));
  if (rnd > .9f) {
    d = min(length(p.swizzle<0, 2>()) - thk, length(p.swizzle<1, 2>()) - thk);
  }
  if (d < res) {
    res = d;
    hp = p;
  }
  return res;
}
f32x2 map(Context &__ctx, f32x3 p) {
  f32x2 res = f32x2(1e5f, 0.f);
  float d = 1e5f;
  float m = 0.f;
  for (float i = 0.f; i < 3.f; i++) {
    float e = truchet(__ctx, p, i);
    p.data[2] += zz;
    d = min(e, d);
    m += .5f;
    if (d < res.data[0])
      res = f32x2(d, m);
  }
  return res;
}
f32x3 normal(Context &__ctx, f32x3 p, float t) {
  t *= .0001f;
  float d = map(__ctx, p).data[0];
  f32x2 e = f32x2(t, 0);
  f32x3 n = d - f32x3(map(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                      map(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                      map(__ctx, p - e.swizzle<1, 1, 0>()).data[0]);
  return normalize(n);
}
f32x2 marcher(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 &p, int steps) {
  float d = 0.f, m = 0.f;
  for (int i = 0; i < steps; i++) {
    f32x2 t = map(__ctx, p);
    d += i < 32 ? t.data[0] * .5f : t.data[0];
    m = t.data[1];
    p = ro + rd * d;
    if (abs(t.data[0]) < d * .0001f || d > 75.f)
      break;
  }
  return f32x2(d, m);
}
f32x3 hue(Context &__ctx, float t) {
  t += iTime * .06f;
  return .75f + .75f * cos(6.28318530718f * t *
                           (f32x3(1.f, .99f, .95f) + f32x3(.1f, .34f, .27f)));
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 F) {
  f32x2 uv = (2.f * F.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]);
  f32x3 ro = f32x3(0, 0, 10);
  f32x3 rd = normalize(f32x3(uv, -1.f));
  f32x3 C = f32x3(0);
  f32x3 p = ro;
  f32x2 ray = marcher(__ctx, ro, rd, p, 100);
  float d = ray.data[0];
  float m = 23.f + (ray.data[1] * .7f);
  gid = sid;
  hitpoint = hp;
  if (d < 50.f) {
    f32x3 n = normal(__ctx, p, d);
    f32x3 lpos = f32x3(5.5f, -3, 13.f);
    f32x3 l = normalize(lpos - p);
    float diff = clamp(dot(n, l), 0.f, 1.f);
    float shdw = 1.f;
    for (float t = .01f; t < 24.f;) {
      float h = map(__ctx, p + l * t).data[0];
      if (h < .0001f) {
        shdw = 0.f;
        break;
      }
      shdw = min(shdw, 24.f * h / t);
      t += h * .95f;
      if (shdw < .0001f || t > 24.f)
        break;
    }
    diff = mix(diff, diff * shdw, .35f);
    float spec =
        .75f *
        pow(max(dot(normalize(p - ro), reflect(normalize(lpos), n)), 0.f),
            24.f);
    f32x3 h = hue(__ctx, m + hitpoint.data[0] * .2f);
    f32x3 h2 = hue(__ctx, 2.52f + (m * 2.f));
    float px = 8.f / iResolution.data[0];
    float f = length(hitpoint.swizzle<0, 1>()) - (hf * .75f);
    float ff = abs(abs(abs(f) - .2f) - .1f) - .05f;
    float fs = smoothstep(.1f + px, -px, ff);
    f = smoothstep(px, -px, ff);
    h = mix(h, h * .4f, fs);
    h = mix(h, h2, f);
    C = h * diff + spec;
  }
  C = mix(C, f32x3(.04f), 1.f - exp(-.00045f * d * d * d));
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
