// Port of https://www.shadertoy.com/view/fdGyz1; author: totetmatt.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fdGyz1.json.
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
f32x3 erot(Context &__ctx, f32x3 p, f32x3 ax, float t) {
  return mix(dot(ax, p) * ax, p, cos(t)) + cross(ax, p) * sin(t);
}
float box2(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 q = abs(p) - b;
  return length(max(f32x2(0.f), q)) + min(0.f, max(q.data[0], q.data[1]));
}
float box3(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(f32x3(0.f), q)) +
         min(0.f, max(q.data[0], max(q.data[1], q.data[2])));
}
float sdOrientedBox(Context &__ctx, f32x2 p, f32x2 a, f32x2 b, float th) {
  float l = length(b - a);
  f32x2 d = (b - a) / l;
  f32x2 q = (p - (a + b) * 0.5f);
  q = f32x2x2(d.data[0], -d.data[1], d.data[1], d.data[0]) * q;
  q = abs(q) - f32x2(l, th) * 0.5f;
  return length(max(q, 0.0f)) + min(max(q.data[0], q.data[1]), 0.0f);
}
float diam(Context &__ctx, f32x3 p, float s) {
  p = abs(p);
  return (p.data[0] + p.data[1] + p.data[2] - s) * inverse_sqrt(3.f);
}
f32x2 sdf(Context &__ctx, f32x3 p) {
  f32x3 tp = p, hp = p;
  hp.data[1] -= abs(asin(sin(iTime)));
  float gy = dot(sin(p), cos(p.swizzle<2, 0, 1>() * 7.f));
  hp = erot(__ctx, hp, normalize(f32x3(1.f, 2.f, 3)), iTime);
  f32x2 h;
  h.data[0] = length(hp) - 1.f;
  h.data[0] = .7f *
              mix(diam(__ctx, hp, 1.f), box3(__ctx, hp, f32x3(1.f)),
                  sin(iTime + gy * .2f) * .5f - 0.2f) *
              .9f;
  h.data[1] = 1.f;
  f32x2 t;
  tp.data[1] += 1.f;
  tp.data[0] += iTime;
  tp.set_swizzle<0, 2>(asin(sin(tp.swizzle<0, 2>())));
  float sc = 1.f;
  for (float i = 0.f; i < 4.f; i++) {
    tp.data[0] = .5f - abs(tp.data[0]);
    tp *= 2.5f;
    sc *= 2.5f;
    tp = erot(__ctx, tp, f32x3(0, 1, 0), .785f);
  }
  float dd, c = 10.f / 3.141592f;
  tp.set_swizzle<0, 2>(f32x2(log(dd = length(tp.swizzle<0, 2>())),
                             atan(tp.data[0], tp.data[2])) *
                       c);
  tp.data[1] /= dd /= c;
  tp.data[0] += iTime;
  tp.set_swizzle<0, 2>(asin(sin(tp.swizzle<0, 2>())));
  tp.data[1] = abs(tp.data[1]) - 0.5f - abs(gy) * .1f;
  gy = dot(sin(tp), cos(tp.swizzle<2, 0, 1>() * 7.f));
  t.data[0] = dd * .8f *
              min(box2(__ctx, tp.swizzle<1, 2>(), f32x2(.01f)),
                  box2(__ctx, tp.swizzle<0, 1>(), f32x2(.01f))) /
              sc;
  t.data[1] = 2.f;
  h = t.data[0] < h.data[0] ? t : h;
  return h;
}
f32x3 pal(Context &__ctx, float t) {
  return .5f + .5f * cos(6.28f * (1.f * t + f32x3(.0f, .3f, .7f)));
}
f32x3 norm(Context &__ctx, f32x3 p, float ee) {
  f32x2 e = f32x2(-ee, ee);
  return normalize(
      e.swizzle<0, 1, 1>() * sdf(__ctx, p + e.swizzle<0, 1, 1>()).data[0] +
      e.swizzle<1, 0, 1>() * sdf(__ctx, p + e.swizzle<1, 0, 1>()).data[0] +
      e.swizzle<1, 1, 0>() * sdf(__ctx, p + e.swizzle<1, 1, 0>()).data[0] +
      e.swizzle<0, 0, 0>() * sdf(__ctx, p + e.swizzle<0, 0, 0>()).data[0]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col = f32x3(.1f, .2f, .7f) * max(0.f, 1.f + uv.data[1] * 7.f);
  f32x3 ro = f32x3(0.f, 1.f + sin(iTime), -5.f);
  if (mod(iTime, 20.f) > 10.f)
    ro = erot(__ctx, ro, normalize(f32x3(0.f, 1.f, 0.f)), iTime * .5f);
  f32x3 rt = f32x3(0.f);
  f32x3 z = normalize(rt - ro);
  f32x3 x = normalize(cross(z, f32x3(0.f, -1.f, 0.f)));
  f32x3 y = normalize(cross(z, x));
  f32x3 rd = f32x3x3(x, y, z) *
             normalize(f32x3(uv, 1.f - sin(floor(iTime * .5f) * 55.f) * .8f *
                                           sqrt(length(uv))));
  f32x3 rp = ro;
  f32x3 light = f32x3(1.f, 2.f, -3);
  float dd = 0.f;
  f32x3 acc = f32x3(0.f);
  for (float i = 0.f; i < 128.f; i++) {
    f32x2 d = sdf(__ctx, rp);
    if (d.data[1] == 2.f) {
      acc += f32x3(1.f, 1.f, 0.f) * .25f * exp(i * -abs(d.data[0])) /
             (60.f + sin(iTime * 10.f + rp.data[1] * 10.f) * 50.f);
      d.data[0] = max(.001f, abs(d.data[0]));
    }
    rp += rd * d.data[0];
    dd += d.data[0];
    if (dd > 50.f)
      break;
    if (d.data[0] < .0001f) {
      f32x3 n = norm(__ctx, rp, .001f);
      f32x3 n2 = norm(__ctx, rp, .02f + cos(rp.data[2] * 10.f) * .01f);
      float dif = max(0.5f, dot(normalize(light - rp), n));
      float spc = pow(max(0.f, dot(rd, reflect(normalize(light), n))), 32.f);
      float fres = pow(1.f - dot(-rd, n), 5.f);
      float ol = step(.1f, .3f * length(n - n2));
      if (ol == 0.f) {
        rd = reflect(rd, n);
        rp += rd * .1f;
        continue;
      }
      col = pal(__ctx, fres * 2.f - iTime * .5f + dif) * ol + spc + fres;
      break;
    }
  }
  fragColor = f32x4(col + acc, 1.0f);
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
