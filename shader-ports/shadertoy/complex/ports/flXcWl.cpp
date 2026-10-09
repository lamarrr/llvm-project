// Port of https://www.shadertoy.com/view/flXcWl; author: lambmeow.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/flXcWl.json.
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
float cir(Context &__ctx, f32x3 p, float d) {
  p = mod(p + f32x3(6.f, 6.f, 10.f), f32x3(12, 12.f, 20.f)) -
      f32x3(6.f, 6.f, 10.f);
  p.data[1] += 2.5f;
  p.data[0] = abs(p.data[0]) - 3.f;
  f32x3 q = abs(p - f32x3(0, 5.f, 0)) - f32x3(1, 2.f, 1);
  float ang = radians(180.f);
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() *
                      (f32x2x2(cos(ang), -sin(ang), sin(ang), cos(ang))));
  p.data[1] *= .78f;
  f32x3 qa = abs(p);
  float tr = max(qa.data[2] - 1.f,
                 max(qa.data[0] * 0.866025f + p.data[1] * 0.5f, -p.data[1]) -
                     2.f * 0.5f);
  return min(tr, length(max(q, 0.f)) +
                     min(max(q.data[0], max(q.data[1], q.data[2])), 0.f));
}
float box(Context &__ctx, f32x3 p, f32x3 b, float r) {
  f32x3 q = abs(p) - b;
  return length(max(p, 0.f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.f);
}
float four(Context &__ctx, f32x3 p) { return length(p) - 1.f; }
f32x2 ss(Context &__ctx, f32x3 p) {
  f32x2 res = f32x2(100, -1);
  float a = cir(__ctx, p, 1.f);
  res.data[0] = min(a, res.data[0]);
  if (a < 0.01f)
    res.data[1] = 1.f;
  float b = four(__ctx, p - f32x3(0, 0, (mod(iTime, 50.f)) * 10.f));
  res.data[0] = min(res.data[0], b);
  if (abs(res.data[0] - b) < 0.01f)
    res.data[1] = 0.f;
  return res;
}
f32x3 nn(Context &__ctx, f32x3 p) {
  f32x3 n = f32x3(0);
  for (int i = min(iFrame, 0); i < 4; i++) {
    f32x3 e =
        0.5773f *
        (2.0f * f32x3((((i + 3) >> 1) & 1), ((i >> 1) & 1), (i & 1)) - 1.0f);
    n += e * ss(__ctx, p + e * 0.0001f).data[0];
  }
  return normalize(n);
}
f32x2 rt(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 &p) {
  f32x2 dd = f32x2(0);
  float ll = 0.f;
  for (int i = 0; i < 200; i++) {
    p = ro + rd * ll;
    dd = ss(__ctx, p);
    ll += dd.data[0];
    if (dd.data[0] < 0.001f || ll > 100.f)
      break;
  }
  if (dd.data[0] > 0.001f)
    p = f32x3(-1);
  return dd;
}
f32x4 rfl(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 &p) {
  f32x2 dd;
  for (int i = 0; i < 2; i++) {
    dd = rt(__ctx, p, rd, p);
    if (dd.data[0] > 0.01f)
      return f32x4(rd, -1);
    f32x3 n = nn(__ctx, p);
    rd = (refract(n, rd, 0.55f));
    p += rd * 0.001f;
  }
  return f32x4(rd, dd.data[1]);
}
f32x4 bb(Context &__ctx, f32x3 p, f32x3 rd, float id, f32x3 ro) {
  float y = max(.5f - rd.data[1], 0.f);
  f32x4 bg = f32x4((1.f - y) * f32x3(0.5f, 0.5f, .8f), 1.f) +
             f32x4(y * f32x3(0.2f, 1.f, 0.8f), 1.f);
  if (id == 1.f) {
    f32x3 n = nn(__ctx, p);
    f32x3 ll = normalize(f32x3(1, 1, 1));
    float ndl = max(dot(ll, n), 0.f);
    float noise =
        texture(__ctx.channel0, floor(p.swizzle<0, 1>()) / 10000.f +
                                    sin((mod(iTime, 50.f)) + 3.f) * .001f)
            .data[0];
    return mix(f32x4(noise * f32x3(.2f, .5f, .5f), 1.f), bg,
               f32x4(ndl * length(ro - p) / 100.f));
  }
  return bg;
}
f32x4 gc(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x3 p = ro;
  f32x3 rp = p;
  f32x3 op = p;
  f32x2 dd = rt(__ctx, ro, rd, op);
  if (dd.data[0] < 0.001f) {
    if (dd.data[1] == 0.f) {
      f32x4 refl = rfl(__ctx, p, rd, p);
      f32x4 crel = bb(__ctx, p, refl.swizzle<0, 1, 2>(), refl.data[3], ro);
      f32x3 n = nn(__ctx, p);
      f32x3 ll = normalize(f32x3(1, 1, 1));
      float spec = dot(reflect(n, ll), -rd);
      return crel * 0.25f + f32x4(.3f, 0, .9f, 1) *
                                max(dot(normalize(f32x3(1, 1, 1)), n), 0.f);
    }
  }
  return bb(__ctx, op, rd, dd.data[1], ro);
}
void mainImage(Context &__ctx, f32x4 &c, f32x2 f) {
  f32x2 r = iResolution.swizzle<0, 1>(), u = (2.f * f - r) / r.data[1];
  f32x3 ro = f32x3(0, 0, (mod(iTime, 50.f)) * 10.f - 10.f),
        rd = normalize(f32x3(u, 2.f));
  c = gc(__ctx, ro, rd);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
