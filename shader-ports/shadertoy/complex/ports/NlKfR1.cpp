// Port of https://www.shadertoy.com/view/NlKfR1; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NlKfR1.json.
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
float cc(Context &__ctx, float a, float b) {
  float f = tanh(a * cos(b)) / tanh(a);
  return sign(f) * pow(abs(f), 0.25f);
}
float cs(Context &__ctx, float a, float b) {
  float f = tanh(a * sin(b)) / tanh(a);
  return sign(f) * pow(abs(f), 0.25f);
}
f32x3 pal(Context &__ctx, float t, f32x3 d) {
  return 0.5f + 0.5f * cos(2.f * 3.14159f * (0.5f * t + d));
}
f32x3 pal(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(2.f * 3.14159f * (c * t + d));
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
float sfloor(Context &__ctx, float a, float b) {
  return floor(b) + 0.5f + 0.5f * tanh(a * (fract(b) - 0.5f)) / tanh(0.5f * a);
}
float smin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.f, 1.f);
  return mix(b, a, h) - k * h * (1.f - h);
}
float smax(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f - 0.5f * (b - a) / k, 0.f, 1.f);
  return mix(b, a, h) + k * h * (1.f - h);
}
float hash(Context &__ctx, float a, float b) {
  int x = floatBitsToInt(a * a / 7.f) ^ floatBitsToInt(a),
      y = floatBitsToInt(b * b / 7.f) ^ floatBitsToInt(b);
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
float sdBox(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.f)) + min(max(d.data[0], d.data[1]), 0.f);
}
f32x3 GetRayOrigin(Context &__ctx) {
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float r = 4.f;
  float a = 0.125f * iTime;
  f32x3 ro = f32x3(r * cos(a), 0, r * sin(a));
  return ro;
}
float GetDist(Context &__ctx, f32x3 p) {
  float sd = length(p) - 0.5f;
  for (int j = 0; j < 16; j++) {
    float io = 2.f * 3.14159f * float(j) / 16.f;
    f32x3 q = normalize(p.swizzle<1, 2, 0>());
    p.set_swizzle<0, 2>(
        abs(p.swizzle<2, 0>() *
                f32x2x2(cos(0.25f * iTime + io), -sin(0.25f * iTime + io),
                        sin(0.25f * iTime + io), cos(0.25f * iTime + io)) -
            0.25f * abs(p.data[2] - p.data[0])) -
        0.0f);
    p.set_swizzle<1, 2>(
        abs(p.swizzle<1, 2>() *
                f32x2x2(cos(0.25f * iTime + io), -sin(0.25f * iTime + io),
                        sin(0.25f * iTime + io), cos(0.25f * iTime + io)) +
            .25f * abs(p.data[2] - p.data[1])) -
        0.01f);
  }
  return max(sd, 0.1f * abs(p.data[0] + p.data[1] + p.data[2]) - 0.01f);
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd, float z) {
  float dO = 0.f;
  float s = sign(z);
  for (int i = 0; i < 400; i++) {
    f32x3 p = ro + rd * dO;
    float dS = GetDist(__ctx, p);
    if (s != sign(dS)) {
      z *= 0.5f;
      s = sign(dS);
    }
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
  f32x3 ro = GetRayOrigin(__ctx);
  f32x3 rd =
      GetRayDir(__ctx, uv, ro, f32x3(0), 1.f + 5.f * tanh(0.25f * iTime));
  f32x3 col = f32x3(0);
  float d = RayMarch(__ctx, ro, rd, 1.f);
  f32x3 p = ro + rd * d;
  float IOR = 1.05f;
  if (d < 100.f) {
    f32x3 n = GetNormal(__ctx, p);
    f32x3 r = reflect(rd, n);
    f32x3 pIn = p - 4.f * .001f * n;
    f32x3 rdIn = refract(rd, n, 1.f / IOR);
    float dIn = RayMarch(__ctx, pIn, rdIn, -1.f);
    f32x3 pExit = pIn + dIn * rdIn;
    f32x3 nExit = -GetNormal(__ctx, pExit);
    float dif = dot(n, normalize(p)) * .5f + .5f;
    float dif2 = dot(n, normalize(abs(r - p))) * .5f + .5f;
    col = f32x3(dif);
    float fres = pow(1.f + dot(rd, n), 4.f);
    float fog = 1.f - exp(-length(p));
    float spec = pow(dif, 32.f);
    float spec2 = pow(dif2, 1.f);
    col *= 0.5f + 0.5f * n;
    col *= pal(__ctx, n.data[1] * 0.5f + 0.1f * iTime, f32x3(0, 1, 2) / 3.f);
    col *= 1.f + 0.75f * n.data[1];
    col += spec;
    col = pal(__ctx, dif + p.data[1] * 0.5f + 0.5f * iTime,
              0.25f * f32x3(2, 1, 1));
    col += spec;
    col += p.data[1] - 0.0f;
    float th =
        .6f +
        .4f * tanh(8.f * cos(12.f * log(length(p)) - 1.5f * 3.14159f * iTime)) /
            tanh(8.f);
    col *= th * exp(-12.f * abs(fract(log(length(p)) - 1.f * iTime) - 0.5f)) *
           abs(n.data[0] + n.data[1] + n.data[2]);
    col *= abs(n.data[0] + n.data[1] + n.data[2]);
  }
  col = pow(col, f32x3(.4545f));
  fragColor = f32x4(col, 1.0f);
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
