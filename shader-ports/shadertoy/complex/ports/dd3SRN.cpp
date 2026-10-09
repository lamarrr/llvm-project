// Port of https://www.shadertoy.com/view/dd3SRN; author: ford442.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dd3SRN.json.
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

f32x3 rotX(Context &__ctx, f32x3 p, float a) {
  f32x2 sc = sin(f32x2(a, a + 1.57079632f));
  return f32x3(p.data[0], p.data[1] * sc.data[1] + p.data[2] * sc.data[0],
               p.data[1] * -sc.data[0] + p.data[2] * sc.data[1]);
}
f32x3 rotY(Context &__ctx, f32x3 p, float a) {
  f32x2 sc = sin(f32x2(a, a + 1.57079632f));
  return f32x3(p.data[0] * sc.data[1] + p.data[2] * sc.data[0], p.data[1],
               p.data[0] * sc.data[0] - p.data[2] * sc.data[1]);
}
f32x3 rotZ(Context &__ctx, f32x3 p, float a) {
  f32x2 sc = sin(f32x2(a, a + 1.57079632f));
  return f32x3(p.data[0] * sc.data[1] + p.data[1] * sc.data[0],
               p.data[0] * -sc.data[0] + p.data[1] * sc.data[1], p.data[2]);
}
f32x2 hash22(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(f32x3(p.swizzle<0, 1, 0>()) * f32x3(.1031f, .1030f, .0973f));
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 19.19f);
  return fract((p3.swizzle<0, 0>() + p3.swizzle<1, 2>()) * p3.swizzle<2, 1>());
}
f32x2 noise(Context &__ctx, f32x2 p) {
  const f32x2 k = f32x2(0.0f, 1.0f);
  f32x2 i = floor(p);
  f32x2 f = fract(p);
  f32x2 h00 = hash22(__ctx, i + k.swizzle<0, 0>());
  f32x2 h01 = hash22(__ctx, i + k.swizzle<0, 1>());
  f32x2 h10 = hash22(__ctx, i + k.swizzle<1, 0>());
  f32x2 h11 = hash22(__ctx, i + k.swizzle<1, 1>());
  f = f * f * f * (f * (6.0f * f - 15.0f) + 10.0f);
  return mix(mix(h00, h10, f.data[0]), mix(h01, h11, f.data[0]), f.data[1]);
}
f32x2 fbm(Context &__ctx, f32x2 p) {
  f32x2x2 m = f32x2x2(-0.73736887f, 0.67549029f, 0.67549029f, 0.73736887f);
  f32x2 f = f32x2(0.0f);
  float s = 0.5f;
  for (int i = 0; i < 4; i++) {
    f += noise(__ctx, p) * s;
    s *= 0.5f;
    p = (p + f32x2(0.03f, 0.07f)) * 2.01f * m;
  }
  return f;
}
float sdEllipsoid(Context &__ctx, f32x2 p, f32x2 r) {
  float k0 = length(p / r);
  float k1 = length(p / (r * r));
  return k0 * (k0 - 1.0f) / k1;
}
float sdCapsule(Context &__ctx, f32x2 p, f32x2 a, f32x2 b, float r) {
  f32x2 d = b - a;
  float l = length(d);
  d = normalize(d);
  f32x2 v = p - a;
  float h = min(l, max(0.0f, dot(v, d)));
  return length(v - d * h) - r;
}
float smin(Context &__ctx, float a, float b, float k) {
  float h = max(k - abs(a - b), 0.0f);
  return min(a, b) - h * h * 0.25f / k;
}
f32x2 leaf(Context &__ctx, f32x2 p) {
  float d;
  p.data[0] = abs(p.data[0]);
  float d0 = max(sdEllipsoid(__ctx, p, f32x2(0.85f, 1.0f)), -p.data[1]);
  float d1 = sdCapsule(__ctx, p, f32x2(0.0f, 0.7f), f32x2(0.0f, 1.0f), 0.02f);
  float d2 = sdEllipsoid(__ctx, p - f32x2(0.8f, 0.0f), f32x2(0.78f, 0.3f));
  float d4 = sdCapsule(__ctx, p, f32x2(0.0f, 0.1f), f32x2(0.0f, -0.8f), 0.02f);
  d = -smin(__ctx, -d0, d1, 0.15f);
  d = -smin(__ctx, -d, d2, 0.1f);
  d = smin(__ctx, d, d4, 0.05f);
  float r = length(p);
  float th = asin(p.data[1] / r);
  float h = (p.data[1] < 0.0f) ? 1.0f : abs(sin(th * 60.0f));
  h = max(h, 1.0f - pow(length(p) * 2.0f, 3.0f));
  return f32x2(d, h);
}
f32x3 h2c(Context &__ctx, float h) {
  const f32x3 c0 = f32x3(0.18f, 0.96f, 0.93f);
  const f32x3 c1 = f32x3(0.14f, 0.96f, 0.98f);
  f32x3 c = mix(c0, c1, h);
  f32x4 K = f32x4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
  f32x3 p = abs(fract(c.swizzle<0, 0, 0>() + K.swizzle<0, 1, 2>()) * 6.0f -
                K.swizzle<3, 3, 3>());
  return c.data[2] * mix(K.swizzle<0, 0, 0>(),
                         clamp(p - K.swizzle<0, 0, 0>(), 0.0f, 1.0f),
                         c.data[1]);
}
float roadh(Context &__ctx, f32x2 p, f32x4 &m) {
  const float xs = 2.0f;
  p *= f32x2(1.0f / xs, 1.0f);
  f32x2 ip = floor(p);
  f32x2 ap = p + f32x2(ip.data[1] * 0.5f, 0.0f);
  f32x2 fp = fract(ap);
  float d;
  fp = min(fp, 1.0f - fp) * 2.0f;
  d = min(1.0f, min(fp.data[0] * xs, fp.data[1]) * 8.0f);
  f32x2 n = fbm(__ctx, p * 8.0f);
  float d0 = n.data[0];
  float d1 = 1.0f - min(1.0f, n.data[1] * 3.5f);
  d -= d0 * 0.2f;
  d -= d1 * 0.4f;
  if (m.data[0] > 0.0f) {
    f32x2 iap = floor(ap);
    m.set_swizzle<0, 1>(hash22(__ctx, iap));
    m.set_swizzle<1, 2>(iap);
  }
  return d;
}
f32x3 road(Context &__ctx, f32x2 p) {
  const f32x2 eps = f32x2(1e-2f, 0.0f);
  f32x4 m;
  m = f32x4(0.0f);
  f32x3 n;
  n.data[0] = roadh(__ctx, p - eps.swizzle<0, 1>(), m) -
              roadh(__ctx, p + eps.swizzle<0, 1>(), m);
  n.data[1] = roadh(__ctx, p - eps.swizzle<1, 0>(), m) -
              roadh(__ctx, p + eps.swizzle<1, 0>(), m);
  n.data[2] = eps.data[0] * 3.0f;
  n = normalize(n);
  m = f32x4(1.0f);
  float d = roadh(__ctx, p, m);
  float ao = 1.0f - pow(max(0.0f, 1.0f - d), 8.0f);
  ao = mix(0.6f, 1.0f, max(0.0f, ao));
  f32x3 l = normalize(f32x3(0.5f, 0.7f, 2.0f));
  float dfs = dot(l, n);
  dfs = mix(0.8f, 1.0f, pow(clamp(dfs * 3.0f, 0.0f, 1.0f), 4.0f));
  f32x3 col;
  const f32x3 c0 = f32x3(0.74f, 0.72f, 0.49f);
  const f32x3 c1 = f32x3(0.66f, 0.64f, 0.42f);
  col = mix(c0, c1, m.data[0]);
  f32x2 f = fbm(__ctx, p * 2.0f);
  const f32x3 c2 = f32x3(0.73f, 0.68f, 0.44f);
  col = mix(col, c2, f.data[0]);
  col *= dfs * ao;
  return col;
}
f32x3 spreadleaf(Context &__ctx, f32x2 p, float R) {
  const int N = 2;
  f32x2 ip = floor(p);
  float h = -1.0f;
  f32x3 col = road(__ctx, p);
  f32x2 f = fbm(__ctx, p * 4.0f);
  for (int j = -N; j <= N; j++) {
    float y = float(j);
    for (int i = -N; i <= N; i++) {
      float x = float(i);
      f32x2 cp = ip + f32x2(x, y);
      f32x2 ch = hash22(__ctx, cp);
      cp += (ch * 2.0f - 1.0f) * R;
      f32x2 lp = cp - p + (f * 2.0f - 1.0f) * 0.05f;
      ch = hash22(__ctx, ch * 123.4567f);
      float angl = ch.data[0] * 6.28318530f;
      lp = rotZ(__ctx, f32x3(lp, 0.0f), angl).swizzle<0, 1>();
      f32x2 lfd = leaf(__ctx, lp * 1.2f);
      col = mix(col, col * f32x3(0.4f), smoothstep(0.04f, -0.02f, lfd.data[0]));
      f32x3 tint = mix(f32x3(0.75f, 0.7f, 0.6f), f32x3(1.0f), lfd.data[1]);
      col = mix(col, h2c(__ctx, ch.data[1]) * tint,
                smoothstep(0.0f, -0.01f, lfd.data[0]));
    }
  }
  return col;
}
f32x3 render(Context &__ctx, f32x2 sp) {
  f32x3 op = f32x3(0.0f, 0.0f, 4.0f);
  f32x3 dv = normalize(f32x3(sp, -2.0f));
  f32x3 angle =
      f32x3(0.8f + sin(iTime * 1.2f) * 0.02f, sin(iTime * 1.5f) * 0.05f, 0.0f);
  op = rotZ(__ctx, rotY(__ctx, rotX(__ctx, op, angle.data[0]), angle.data[1]),
            angle.data[2]);
  dv = rotZ(__ctx, rotY(__ctx, rotX(__ctx, dv, angle.data[0]), angle.data[1]),
            angle.data[2]);
  op += f32x3(0.0f, 1.0f + abs(sin(iTime * 6.0f)) * 0.06f, 0.0f);
  f32x3 gp = dv * op.data[1] / dv.data[1];
  f32x3 col;
  col =
      spreadleaf(__ctx, gp.swizzle<0, 2>() * 2.0f + f32x2(0.0f, -iTime), 0.5f);
  f32x3 vgn = exp(-pow(length(sp), 2.0f) * f32x3(0.5f, 0.6f, 1.0f) * 0.2f);
  col -= 1.0f - vgn;
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p = (fragCoord.swizzle<0, 1>() * 2.0f - iResolution.swizzle<0, 1>()) /
            min(iResolution.data[0], iResolution.data[1]);
  f32x3 c = render(__ctx, p);
  c = pow(clamp(c, 0.0f, 1.0f), f32x3(1.0f / 1.8f));
  fragColor = f32x4(c, 1.0f);
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
