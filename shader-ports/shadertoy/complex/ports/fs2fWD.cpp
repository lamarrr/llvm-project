// Port of https://www.shadertoy.com/view/fs2fWD; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fs2fWD.json.
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
f32x3 pal(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(6.28318f * (c * t + d));
}
float h21(Context &__ctx, f32x2 a) {
  return fract(sin(dot(a.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
float mlength(Context &__ctx, f32x2 uv) {
  return max(abs(uv.data[0]), abs(uv.data[1]));
}
float mlength(Context &__ctx, f32x3 uv) {
  return max(max(abs(uv.data[0]), abs(uv.data[1])), abs(uv.data[2]));
}
float dlength(Context &__ctx, f32x2 uv) {
  return abs(uv.data[0]) + abs(uv.data[1]);
}
float dlength(Context &__ctx, f32x3 uv) {
  return abs(uv.data[0]) + abs(uv.data[1]) + abs(uv.data[2]);
}
float smin(Context &__ctx, float a, float b) {
  float k = 0.12f;
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float hash(Context &__ctx, float a, float b) {
  int x = float_bits_to_int(a * a / 7.f) ^ float_bits_to_int(a),
      y = float_bits_to_int(b * b / 7.f) ^ float_bits_to_int(b);
  return float((x * x + y) * (y * y - x) - x) / 2.14e9f;
}
f32x3 erot(Context &__ctx, f32x3 p, f32x3 ax, float ro) {
  return mix(dot(ax, p) * ax, p, cos(ro)) + cross(ax, p) * sin(ro);
}
f32x3 face(Context &__ctx, f32x3 p) {
  f32x3 a = abs(p);
  return step(a.swizzle<1, 2, 0>(), a.swizzle<0, 1, 2>()) *
         step(a.swizzle<2, 0, 1>(), a.swizzle<0, 1, 2>()) * sign(p);
}
float sdBox(Context &__ctx, f32x3 p, f32x3 s) {
  p = abs(p) - s;
  return length(max(p, 0.f)) +
         min(max(p.data[0], max(p.data[1], p.data[2])), 0.f);
}
f32x3 getRo(Context &__ctx) {
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 ro = f32x3(0, 0.01f, 2.2f);
  if (iMouse.data[2] > 0.f) {
    ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() *
                         (Rot(__ctx, -m.data[1] * 3.14f + 1.f)));
    ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() *
                         (Rot(__ctx, -m.data[0] * 6.2831f)));
  } else
    ro.data[1] += 0.3f * cos(0.8f * iTime);
  return ro;
}
float GetDist(Context &__ctx, f32x3 p) {
  float a = atan(p.data[0], p.data[2]);
  float r = length(p.swizzle<0, 2>());
  float sc = 3.f / 3.14159f;
  f32x2 uv = sc * f32x2(a - 0.25f * iTime, p.data[1]);
  uv.data[0] = (fract(uv.data[0]) - 0.5f) * r;
  float d = 10.f;
  float m = 0.25f;
  float n = 6.f;
  for (float i = 0.f; i < n; i++) {
    float io = 2.f * 3.14159f * i / n;
    d = min(d, dlength(__ctx, uv) - m);
    uv = abs(uv) - m;
    m *= 0.6f;
  }
  d = min(d, dlength(__ctx, uv) - m);
  float d2 = abs(length(p.swizzle<0, 2>()) - 1.f) - 0.03f;
  float d3 = d2;
  p.data[1] += 0.05f * cos(3.f * a + iTime);
  float h = 0.35f * pow(abs(cos(0.5f * a + 0.5f * iTime)), 4.f);
  d2 = max(d2, abs(p.data[1]) - h);
  float h2 = 0.02f;
  d3 = max(d3, abs(abs(p.data[1]) - h - 0.5f * h2) - h2);
  d2 = -min(-d2, d);
  d2 = min(d2, d3);
  return 0.8f * d2;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd, float z) {
  float dO = 0.f;
  for (int i = 0; i < 400; i++) {
    f32x3 p = ro + rd * dO;
    float dS = GetDist(__ctx, p);
    if (abs(dS) < .001f || dO > 100.f)
      break;
    dO += dS * z;
  }
  return min(dO, 100.f);
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p);
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()));
  return normalize(n);
}
f32x3 GetRayDir(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i);
  return d;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 ro = getRo(__ctx);
  float a = 0.5f * 3.14159f * uv.data[0] + 0.18f * iTime;
  float zm = mix(
      1.2f, 4.f,
      0.5f + 0.5f *
                 tanh(5.f * cos(0.f * 3.14159f * uv.data[0] + 0.5f * iTime)) /
                 tanh(5.f));
  zm = 1.2f;
  f32x3 rd = GetRayDir(__ctx, uv, ro, f32x3(0), zm);
  f32x3 col = f32x3(0);
  float d = RayMarch(__ctx, ro, rd, 1.f);
  float IOR = -1.f;
  if (d < 100.f) {
    f32x3 p = ro + rd * d;
    f32x3 n = GetNormal(__ctx, p);
    f32x3 r = reflect(rd, n);
    f32x3 pIn = p + 1000.f * .001f * n;
    f32x3 rdIn = refract(rd, n, 1.f / IOR);
    float dIn = RayMarch(__ctx, pIn, rdIn, 1.f);
    f32x3 pExit = pIn + dIn * rdIn;
    f32x3 nExit = -GetNormal(__ctx, pExit);
    float dif = dot(n, normalize(f32x3(1, 2, 3))) * .5f + .5f;
    col = f32x3(dif);
    float v = exp(-0.85f * pow(abs(p.data[1]), 0.25f));
    float fresnel = pow(1.f + dot(p, n), 5.f);
    v = smoothstep(0.f, 1.f, v);
    v = clamp(1.5f * v * v, 0.f, 1.f);
    f32x3 e = f32x3(1);
    col = v * pal(__ctx, 0.32f + v, 0.8f * e, 0.5f * e, 0.5f * e,
                  0.8f * f32x3(0, 1, 2) / 3.f);
    col *= dif;
    col += 0.08f * n.data[1];
    col = smoothstep(0.f, 1.f, col);
  }
  col = pow(col, f32x3(.4545f));
  col += 0.15f;
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
