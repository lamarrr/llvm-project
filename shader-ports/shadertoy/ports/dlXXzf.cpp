// Port of https://www.shadertoy.com/view/dlXXzf; author: larserik.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dlXXzf.json.
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
f32x4 permute(Context &__ctx, f32x4 x) {
  return mod(((x * 34.0f) + 1.0f) * x, 289.0f);
}
f32x4 taylorInvSqrt(Context &__ctx, f32x4 r) {
  return 1.79284291400159f - 0.85373472095314f * r;
}
float snoise(Context &__ctx, f32x3 v) {
  const f32x2 C = f32x2(1.0f / 6.0f, 1.0f / 3.0f);
  const f32x4 D = f32x4(0.0f, 0.5f, 1.0f, 2.0f);
  f32x3 i = floor(v + dot(v, C.swizzle<1, 1, 1>()));
  f32x3 x0 = v - i + dot(i, C.swizzle<0, 0, 0>());
  f32x3 g = step(x0.swizzle<1, 2, 0>(), x0.swizzle<0, 1, 2>());
  f32x3 l = 1.0f - g;
  f32x3 i1 = min(g.swizzle<0, 1, 2>(), l.swizzle<2, 0, 1>());
  f32x3 i2 = max(g.swizzle<0, 1, 2>(), l.swizzle<2, 0, 1>());
  f32x3 x1 = x0 - i1 + 1.0f * C.swizzle<0, 0, 0>();
  f32x3 x2 = x0 - i2 + 2.0f * C.swizzle<0, 0, 0>();
  f32x3 x3 = x0 - 1.f + 3.0f * C.swizzle<0, 0, 0>();
  i = mod(i, 289.0f);
  f32x4 p = permute(
      __ctx,
      permute(__ctx,
              permute(__ctx,
                      i.data[2] + f32x4(0.0f, i1.data[2], i2.data[2], 1.0f)) +
                  i.data[1] + f32x4(0.0f, i1.data[1], i2.data[1], 1.0f)) +
          i.data[0] + f32x4(0.0f, i1.data[0], i2.data[0], 1.0f));
  float n_ = 1.0f / 7.0f;
  f32x3 ns = n_ * D.swizzle<3, 1, 2>() - D.swizzle<0, 2, 0>();
  f32x4 j = p - 49.0f * floor(p * ns.data[2] * ns.data[2]);
  f32x4 x_ = floor(j * ns.data[2]);
  f32x4 y_ = floor(j - 7.0f * x_);
  f32x4 x = x_ * ns.data[0] + ns.swizzle<1, 1, 1, 1>();
  f32x4 y = y_ * ns.data[0] + ns.swizzle<1, 1, 1, 1>();
  f32x4 h = 1.0f - abs(x) - abs(y);
  f32x4 b0 = f32x4(x.swizzle<0, 1>(), y.swizzle<0, 1>());
  f32x4 b1 = f32x4(x.swizzle<2, 3>(), y.swizzle<2, 3>());
  f32x4 s0 = floor(b0) * 2.0f + 1.0f;
  f32x4 s1 = floor(b1) * 2.0f + 1.0f;
  f32x4 sh = -step(h, f32x4(0.0f));
  f32x4 a0 = b0.swizzle<0, 2, 1, 3>() +
             s0.swizzle<0, 2, 1, 3>() * sh.swizzle<0, 0, 1, 1>();
  f32x4 a1 = b1.swizzle<0, 2, 1, 3>() +
             s1.swizzle<0, 2, 1, 3>() * sh.swizzle<2, 2, 3, 3>();
  f32x3 p0 = f32x3(a0.swizzle<0, 1>(), h.data[0]);
  f32x3 p1 = f32x3(a0.swizzle<2, 3>(), h.data[1]);
  f32x3 p2 = f32x3(a1.swizzle<0, 1>(), h.data[2]);
  f32x3 p3 = f32x3(a1.swizzle<2, 3>(), h.data[3]);
  f32x4 norm = taylorInvSqrt(
      __ctx, f32x4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
  p0 *= norm.data[0];
  p1 *= norm.data[1];
  p2 *= norm.data[2];
  p3 *= norm.data[3];
  f32x4 m = max(
      0.6f - f32x4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0f);
  m = m * m;
  return 42.0f *
         dot(m * m, f32x4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}
float speed = .02f;
float scale = .01f;
int octaves = 8;
int repeats = 0;
float height = 20.f;
f32x3 sky = f32x3(.9f, .93f, .99f);
f32x3 lp = f32x3(100, 50, -100);
float fbm(Context &__ctx, f32x3 st) {
  float value = 0.0f;
  float amplitude = 1.f;
  float total = 0.f;
  for (int i = 0; i < octaves; i++) {
    total += amplitude;
    value += amplitude * snoise(__ctx, st);
    st *= 2.f;
    amplitude *= 0.5f;
  }
  return value / total;
}
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
f32x2 sdFloor(Context &__ctx, f32x3 uv) {
  float time = iTime * speed;
  f32x2 scaled = uv.swizzle<0, 2>() * scale;
  scaled -= time;
  float n = fbm(__ctx, f32x3(scaled, 1.f));
  for (int i = 0; i < repeats; i++) {
    float ang = n * 3.1415f;
    n = fbm(__ctx, f32x3(scaled + f32x2(cos(ang), sin(ang)) * n, 1.f));
  }
  n = n;
  return f32x2(uv.data[1] + 10.f - n * height, n);
}
f32x2 GetDist(Context &__ctx, f32x3 p) {
  f32x2 d = sdFloor(__ctx, p);
  return d;
}
f32x2 RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x2 dO = f32x2(0.f);
  for (int i = 0; i < 100; i++) {
    float j = 1.f - length(dO.data[0]) / 150.f;
    octaves = 2 + int(8.f * pow(j, .25f));
    f32x3 p = ro + rd * dO.data[0];
    f32x2 dS = GetDist(__ctx, p);
    dO.data[0] += dS.data[0];
    dO.data[1] = dS.data[1];
    if (dO.data[0] > 150.f || abs(dS.data[0]) < .01f)
      break;
  }
  return dO;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p).data[0];
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()).data[0]);
  return normalize(n);
}
f32x3 GetRayDir(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i);
  return d;
}
float GetLight(Context &__ctx, f32x3 lp, f32x3 p, f32x3 n) {
  f32x3 l = normalize(lp - p);
  float dif = clamp(dot(n, l) * .4f + .39f, 0.f, 1.f);
  f32x2 dh = RayMarch(__ctx, p + n * .01f * 6.f, l);
  float d = dh.data[0];
  if (d < length(lp)) {
    dif *= .5f;
  }
  return dif;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 ro = f32x3(0, 20.5f, 0);
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() *
                       (Rot(__ctx, -m.data[1] * 3.14f + 1.f)));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (Rot(__ctx, -m.data[0] * 6.2831f)));
  ro.data[1] = max(.01f, ro.data[1]);
  f32x3 la = f32x3(0, 3.5f, 0);
  f32x3 rd = GetRayDir(__ctx, uv, ro, la, 1.f);
  f32x3 col = f32x3(.9f, .93f, .99f);
  f32x2 d = RayMarch(__ctx, ro, rd);
  if (d.data[0] < 150.f) {
    f32x3 p = ro + rd * d.data[0];
    f32x3 n = GetNormal(__ctx, p);
    f32x3 r = reflect(rd, n);
    col =
        mix(f32x3(.05f, .53f, .17f), f32x3(.73f, .77f, .71f), d.data[1] * 2.f);
    float dif = GetLight(__ctx, lp, p, n);
    col *= f32x3(dif);
    float off = pow(1.f - (d.data[0] / 150.f), .4f);
    col = mix(sky, col, off);
  }
  col = pow(col, f32x3(.9545f));
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
