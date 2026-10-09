// Port of https://www.shadertoy.com/view/slfcDj; author: lambmeow.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/slfcDj.json.
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
float bb(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 q = abs(p) - b;
  return length(max(q, 0.f)) + min(max(q.data[0], q.data[1]), 0.f);
}
float tws(Context &__ctx, f32x2 p) {
  float res = bb(__ctx, p, f32x2(0.5f));
  res = max(-bb(__ctx, p + f32x2(0.13f, -0.2f), f32x2(0.45f, .06f)), res);
  res = max(-bb(__ctx, p + f32x2(-0.13f, 0.2f), f32x2(0.45f, .06f)), res);
  return res;
}
float ss(Context &__ctx, f32x3 p) {
  p = p / atan(iTime);
  p.data[0] += iTime;
  p.data[0] = mod(p.data[0] + .75f, 1.5f) - .75f;
  float tw = tws(__ctx, p.swizzle<0, 1>());
  f32x2 w = f32x2(tw, abs(p.data[2]) - (.5f));
  float res = min(max(w.data[0], w.data[1]), 0.f) + length(max(w, 0.f));
  return res - 0.02f;
}
f32x3 nn(Context &__ctx, f32x3 value) {
  f32x3 n = f32x3(0.0f);
  for (int i = 0; i < 4; i++) {
    f32x3 e =
        0.5773f *
        (2.0f * f32x3((((i + 3) >> 1) & 1), ((i >> 1) & 1), (i & 1)) - 1.0f);
    n += e * ss(__ctx, value + 0.0005f * e);
  }
  return normalize(n);
}
f32x3x3 la(Context &__ctx, f32x3 ro, f32x3 ll, f32x3 up) {
  f32x3 ww = normalize(ll - ro);
  f32x3 uu = normalize(cross(ww, normalize(up)));
  f32x3 vv = normalize(cross(uu, ww));
  return f32x3x3(uu, vv, ww);
}
float ggx(Context &__ctx, float ndv, float r) {
  float rr = r + 1.f;
  float s = (rr * rr) / 8.f;
  return ndv / (ndv * (1.f - s) + s);
}
float gSmth(Context &__ctx, float ndv, float ndl, float r) {
  return ggx(__ctx, ndv, r) * ggx(__ctx, ndl, r);
}
f32x3 fsnl(Context &__ctx, float dl, f32x3 F0) {
  return F0 + (1.f - F0) * pow(1.f - dl, 5.f);
}
float rt(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 &p) {
  float ll = 0.f, dd = 0.f;
  for (int i = 0; i < 255; i++) {
    p = ro + rd * ll;
    dd = ss(__ctx, p) * atan(iTime);
    ll += dd;
    if (dd < 0.001f || ll > 100.f)
      break;
  }
  return dd;
}
float nd(Context &__ctx, f32x3 d, f32x3 l) { return max(dot(d, l), .0f); }
f32x4 gc(Context &__ctx, f32x3 p, f32x3 n, float dd, f32x3 rd, f32x2 u) {
  f32x3 al = f32x3(1, 0, 0);
  float met = 1.f;
  float r = .1f;
  dd = max(1.f - dd, 0.f);
  f32x3 l = normalize(f32x3(sin(iTime) * 3.f, -cos(iTime), 1));
  f32x3 v = -rd;
  f32x3 h = normalize(v + l);
  float ndh = nd(__ctx, n, h);
  float ndl = nd(__ctx, n, l);
  float ndv = nd(__ctx, n, v);
  f32x3 bg = f32x3(0, 0, 1) * (1.5f - u.data[1]);
  if (dd > 0.01f) {
    f32x3 f = mix(bg, pow(al, f32x3(2.2f)), met);
    float nf = ggx(__ctx, ndh, r);
    float g = gSmth(__ctx, ndv, ndl, r);
    f32x3 fs = fsnl(__ctx, nd(__ctx, h, v), f);
    f32x3 kd = f32x3(1.f) - fs;
    kd *= 1.f - met;
    f32x3 nu = nf * g * f;
    float de = 4.f * ndv * ndl;
    f32x3 sp = nu / max(de, 0.001f);
    return f32x4(bg * 0.1f, 1.f) +
           f32x4(1.f * (kd * pow(al, f32x3(2.2f)) / 3.14159265359f + sp) *
                     (ndl),
                 1);
  } else {
    return f32x4(bg, 1.f);
  }
}
void mainImage(Context &__ctx, f32x4 &c, f32x2 f) {
  f32x2 r = iResolution.swizzle<0, 1>(), u = (2.f * f - r) / r.data[1];
  f32x3 ro = f32x3(-.7f * cos(iTime), sin(iTime) * 0.4f, 4),
        rd = normalize(f32x3(u, 2.f)), p;
  float dd = 0.f;
  rd *= la(__ctx, ro, f32x3(0), f32x3(0, 1, sin(iTime) * 0.01f));
  f32x3 rf = rd, rfo = ro;
  float d = rt(__ctx, ro, rd, p);
  f32x3 op = p;
  for (int i = 0; i < 2; i++) {
    dd = rt(__ctx, rfo, rf, p);
    if (dd > 0.01f)
      break;
    rf = reflect(rd, nn(__ctx, p));
    rfo = p + 0.01f;
  }
  c = mix(gc(__ctx, p, nn(__ctx, p), d, rd, u),
          gc(__ctx, rfo, nn(__ctx, rfo), dd, rf, u), 0.15f);
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
