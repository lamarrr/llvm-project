// Port of https://www.shadertoy.com/view/sdGyzR; author: scry.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/sdGyzR.json.
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
  return f32x2x2(cos(a), sin(a), -sin(a), cos(a));
}
float sdBox(Context &__ctx, f32x3 p, f32x3 s) {
  return max(abs(p.data[2]) - s.data[2],
             max(abs(p.data[0]) - s.data[0], abs(p.data[1]) - s.data[1]));
}
f32x3 lpos(Context &__ctx, float a) {
  f32x3 l = f32x3(0.f, 1.f, 3.f);
  l -= 0.3f;
  l += a * 0.1f;
  l.set_swizzle<0, 2>(
      l.swizzle<0, 2>() *
      (r2d(__ctx, iTime * 1.f * 0.5f + a * 0.1f * iTime * 1.f)));
  return l;
}
f32x4 map(Context &__ctx, f32x3 p) {
  f32x3 o = p;
  p.data[2] -= 3.f;
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() *
                      (r2d(__ctx, sin(iTime * 1.f / 3.f) * 0.1f)));
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (r2d(__ctx, iTime * 1.f / 6.f)));
  float d = sdBox(__ctx, p, f32x3(0.05f));
  float od = d;
  for (int i = 0; i < 4; i++) {
    p = abs(p) - sin(iTime * 1.f / 4.f + p.data[1] * 0.2f) * 0.1f - 0.1f;
    d = min(d,
            sdBox(__ctx, p,
                  f32x3(sin(iTime * 1.f * 0.3f + d * 0.5f + p.data[1] * 0.2f) *
                            0.5f +
                        0.5f) *
                      0.1f));
  }
  d = min(d, o.data[1] + 0.5f + 0.5f - od * 0.2f);
  return f32x4(p, d);
}
f32x2 RM(Context &__ctx, f32x3 ro, f32x3 rd, float s) {
  float dO = 0.f;
  float ii = 0.f;
  for (int i = 0; i < 99; i++) {
    f32x3 p = ro + rd * dO;
    float dS = map(__ctx, p).data[3] * s;
    dO += dS;
    ii += 0.01f;
    if (dO > 100.f || dS < 0.001f) {
      break;
    }
  }
  return f32x2(dO, ii);
}
f32x3 calcNorm(Context &__ctx, f32x3 p) {
  f32x2 h = f32x2(0.001f, 0.f);
  return normalize(f32x3(map(__ctx, p - h.swizzle<0, 1, 1>()).data[3] -
                             map(__ctx, p + h.swizzle<0, 1, 1>()).data[3],
                         map(__ctx, p - h.swizzle<1, 0, 1>()).data[3] -
                             map(__ctx, p + h.swizzle<1, 0, 1>()).data[3],
                         map(__ctx, p - h.swizzle<1, 1, 0>()).data[3] -
                             map(__ctx, p + h.swizzle<1, 1, 0>()).data[3]));
}
f32x3 sh(Context &__ctx, f32x3 p, f32x3 n, float a) {
  f32x3 l = lpos(__ctx, a);
  f32x3 rd = normalize(l - p);
  f32x2 d = RM(__ctx, p - n * 0.003f, rd, 1.f);
  float dif = clamp(dot(rd, -n), 0.f, 1.f);
  float s = 0.f;
  if (length(l - p) > d.data[0]) {
    s = 1.f;
  }
  return f32x3(s, dif, d.data[0]);
}
float point(Context &__ctx, f32x3 p, f32x3 rd, float a) {
  f32x3 l = lpos(__ctx, a);
  float l1 = clamp(length(l - p) * 0.5f, 0.f, 1.f);
  float l2 = pow(
      1.f - length(l.swizzle<0, 1>() - rd.swizzle<0, 1>() * l.data[2]), 3.f);
  return clamp(pow(l2 + 0.1f, 5.f), 0.f, 1.f);
}
f32x3 colo(Context &__ctx, f32x3 p, f32x3 n, f32x2 d, f32x3 rd) {
  f32x3 col = 1.f - d.swizzle<0, 0, 0>() / 7.f;
  f32x3 mp = map(__ctx, p).swizzle<0, 1, 2>();
  col += 1.f - n;
  col *= 0.05f;
  col *= 0.f;
  col.set_swizzle<0, 1>(col.swizzle<0, 1>() + (sin(mp.data[2] * 10.f) * 0.1f));
  f32x3 rs = sh(__ctx, p, n, 0.f);
  float p1 = point(__ctx, p, rd, 0.f);
  col.data[0] += p1;
  col.data[0] -= rs.data[0] * 0.5f;
  col.data[0] += rs.data[1] * 0.4f;
  f32x3 bs = sh(__ctx, p, n, 1.f);
  p1 = point(__ctx, p, rd, 1.f);
  col.data[2] += p1;
  col.data[2] -= bs.data[0] * 0.5f;
  col.data[2] += bs.data[1] * 0.4f;
  f32x3 gs = sh(__ctx, p, n, 2.f);
  p1 = point(__ctx, p, rd, 2.f);
  col.data[1] += p1;
  col.data[1] -= gs.data[0] * 0.5f;
  col.data[1] += gs.data[1] * 0.4f;
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 R = iResolution.swizzle<0, 1>();
  float ar = R.data[0] / R.data[1];
  uv -= 0.5f;
  uv.data[0] *= ar;
  f32x3 col = f32x3(0.f);
  f32x3 ro = f32x3(0.f);
  f32x3 rd = normalize(f32x3(uv, 1.f));
  f32x2 d = RM(__ctx, ro, rd, 1.f);
  f32x3 p = ro + rd * d.data[0];
  f32x3 n = calcNorm(__ctx, p);
  col = colo(__ctx, p, n, d, rd);
  f32x3 ocol = col;
  f32x3 oro = ro;
  f32x3 ord = rd;
  f32x2 od = d;
  f32x3 op = p;
  f32x3 on = n;
  float rl = 1.5f;
  for (int i = 0; i < 3; i++) {
    ro = p - n * 0.003f;
    rd = reflect(rd, n);
    d = RM(__ctx, ro, rd, 1.f);
    p = ro + rd * d.data[0];
    n = calcNorm(__ctx, p);
    col += colo(__ctx, p, n, d, rd) * rl;
    rl *= 0.9f;
  }
  col = clamp(col, 0.f, 1.f);
  col += ocol;
  col *= 1.f;
  fragColor = f32x4(col, 1.0f);
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
