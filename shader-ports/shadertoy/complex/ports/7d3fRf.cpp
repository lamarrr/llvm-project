// Port of https://www.shadertoy.com/view/7d3fRf; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7d3fRf.json.
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
f32x3 getRo(Context &__ctx) {
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 ro = f32x3(80.f - 70.f * tanh(0.75f * iTime * iTime),
                   5.f * tanh(1.f * cos(0.81f * iTime)) / tanh(1.f),
                   5.f * tanh(1.f * sin(iTime)) / tanh(1.f));
  return ro;
}
float GetDist(Context &__ctx, f32x3 p) {
  p.data[1] += 0.25f * tanh(5.f * cos(1.25f * length(p) - iTime)) / tanh(5.f);
  ;
  p.data[0] += 0.5f * cos(0.5f * p.data[1] + iTime);
  float y = fract(p.data[1]) - 0.5f;
  float d = length(f32x2(p.data[0], y)) - 0.25f +
            0.25f * tanh(1.f * cos(4.f * abs(p.data[2]))) / tanh(1.f);
  float y2 = fract(p.data[1] + 0.5f) - 0.5f;
  float d2 =
      length(f32x2(p.data[0], y2)) - 0.25f +
      0.25f * tanh(1.f * cos(4.f * abs(p.data[2]) + 3.14159f)) / tanh(1.f);
  d = min(d, d2);
  return 0.75f * d;
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
f32x3 bg(Context &__ctx, f32x3 rd, f32x3 p) {
  return 0.5f +
         0.5f *
             tanh(4.f * cos(0.5f * length(p) +
                            0.125f * 3.14159f * h21(__ctx, p.swizzle<0, 2>()) +
                            rd.data[1] - 0.5f * iTime)) /
             tanh(4.f) * f32x3(1);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 ro = getRo(__ctx);
  f32x3 rd = GetRayDir(
      __ctx, uv, ro, f32x3(0),
      1.5f + 0.75f * tanh(4.f * cos(0.25f * log(length(uv)) - 0.5f * iTime)) /
                 tanh(4.f));
  f32x3 col = f32x3(0);
  float d = RayMarch(__ctx, ro, rd, 1.f);
  float IOR = 1.05f;
  f32x3 p = ro + rd * d;
  if (d < 100.f) {
    f32x3 n = GetNormal(__ctx, p);
    f32x3 r = reflect(rd, n);
    f32x3 pIn = p - 4.f * .001f * n;
    f32x3 rdIn = refract(rd, n, 1.f / IOR);
    float dIn = RayMarch(__ctx, pIn, rdIn, -1.f);
    f32x3 pExit = pIn + dIn * rdIn;
    f32x3 nExit = -GetNormal(__ctx, pExit);
    float dif = dot(n, normalize(f32x3(1, 2, 3))) * .5f + .5f;
    col = f32x3(dif);
    float pw = 5.f;
    float fres =
        1.f -
        pow(abs(1.25f + 0.25f * cos(10.f * p.data[1] + iTime) + dot(rd, -n)),
            pw);
    float fres2 = pow(abs(0.f + dot(rdIn, rd)), 100.f);
    f32x3 fog = f32x3(1.f - exp(-0.3f * dIn));
    col +=
        mix(f32x3(0.25f),
            pal(__ctx, 0.1f * iTime - length(p) * 0.15f, f32x3(0, 1, 2) / 3.f),
            cos(fres * 0.025f) * 0.5f + 0.5f);
    col = mix(
        f32x3(1), col,
        0.5f + 0.5f *
                   tanh(2.f * cos(length(p.swizzle<1, 2>()) * 3.14159f / 8.f)) /
                   tanh(2.f));
    col *= pal(__ctx, 0.1f * iTime - length(p) * 0.15f, f32x3(0, 1, 2) / 3.f);
    col = mix(bg(__ctx, rd, p), col, 1.f / cosh(0.5f * length(p)));
  } else {
    col = bg(__ctx, rd, p);
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
