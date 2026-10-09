// Port of https://www.shadertoy.com/view/ftSBRw; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ftSBRw.json.
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
f32x3 pal(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(6.28318f * (c * t + d));
}
float mlength(Context &__ctx, f32x3 uv) {
  return max(max(abs(uv.data[0]), abs(uv.data[1])), abs(uv.data[2]));
}
float sdBox(Context &__ctx, f32x3 p, f32x3 s) {
  p = abs(p) - s;
  return length(max(p, 0.f)) +
         min(max(p.data[0], max(p.data[1], p.data[2])), 0.f);
}
f32x3 getRo(Context &__ctx) {
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float r = 4.f;
  float t = 0.2f * iTime;
  f32x3 ro = f32x3(r * cos(t), 1.5f * cos(0.8f * t), r * sin(t));
  return ro;
}
float GetDist(Context &__ctx, f32x3 p) {
  float o = 2.f * 3.14159f / 3.f;
  float d = length(p) - 1.3f + 0.12f * cos(p.data[1] * 4.f - iTime);
  float val = smoothstep(-0.5f, 0.5f, -length(p) + 1.15f);
  val = clamp(val, 0.f, 1.f);
  d -= 0.75f * val;
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() *
                      (f32x2x2(cos(1.f * iTime), -sin(1.f * iTime),
                               sin(1.f * iTime), cos(1.f * iTime))));
  float r1 = 1.f;
  float r2 = 0.5f;
  float d1 = length(p.swizzle<0, 2>()) - r1;
  float d2 = length(f32x2(p.data[1], d1)) - r2;
  d = mix(d, d2, 0.5f + 0.5f * tanh(5.f * cos(0.25f * iTime)) / tanh(5.f));
  return d;
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
  f32x3 ro = getRo(__ctx);
  f32x3 rd = GetRayDir(__ctx, uv, ro, f32x3(0), 1.f);
  f32x3 col = f32x3(0);
  float d = RayMarch(__ctx, ro, rd, 1.f);
  float IOR = 1.2f;
  if (d < 100.f) {
    f32x3 p = ro + rd * d;
    f32x3 n = GetNormal(__ctx, p);
    f32x3 r = reflect(rd, n);
    f32x3 pIn = p - 20.f * .001f * n;
    f32x3 rdIn = refract(rd, n, 1.f / IOR);
    float dIn = RayMarch(__ctx, pIn, rdIn, -1.f);
    f32x3 pExit = pIn + dIn * rdIn;
    f32x3 nExit = -GetNormal(__ctx, pExit);
    float dif = dot(n, normalize(f32x3(1, 2, 3))) * .5f + .5f;
    float fres = pow(1.f + dot(rd, n), 5.f);
    float c = 4.f;
    float I = 3.f;
    f32x3 n1 = 0.8f * (abs(r) - abs(n));
    float mx2 = exp(-5.f * length(abs(r) - abs(n1)));
    f32x3 r2 = c * (abs(r) - abs(n));
    f32x3 n2 = c * (abs(r2) - abs(n));
    float mx = exp(-I * length(abs(r2) - abs(n2)));
    float cl = 0.5f + 0.25f * dif + max(0.5f * fres, max(mx, mx2));
    f32x3 e = f32x3(0.5f);
    f32x3 col2 =
        pal(__ctx, 0.75f * length(p) - p.data[1] * 0.05f + 0.25f + 0.75f * mx,
            e, e, e, 0.5f * f32x3(0, 1, 2) / 3.f);
    f32x3 col3 =
        pal(__ctx, 0.75f * length(p) - p.data[1] * 0.05f + 0.25f + 1.5f * mx2,
            e, e, e, 0.5f * f32x3(0, 1, 2) / 3.f);
    float c1 = texture(__ctx.channel0, 0.05f * iTime + 3.f * p.swizzle<0, 1>())
                   .data[0];
    float c2 = texture(__ctx.channel0, 0.05f * iTime + 3.f * p.swizzle<1, 2>())
                   .data[0];
    float c3 = texture(__ctx.channel0, 0.05f * iTime + 3.f * p.swizzle<2, 0>())
                   .data[0];
    f32x3 n3 = abs(n);
    float c4 = n3.data[2] * c1 + n3.data[0] * c2 + n3.data[1] * c3;
    col = cl * col2;
    col *= 0.72f + 0.35f * c4;
    float mx3 = exp(-0.333f * dIn);
    col = mix(col, col3, mx3);
    col += 0.2f * n.data[1] + 0.1f * nExit.data[1];
    col += 0.075f * (exp(f32x3(abs(rd.data[0]), rd.data[1], abs(rd.data[2]))) +
                     exp(-r) + 0.1f * exp(r2) - 1.05f);
  } else {
    col = 0.5f * exp(f32x3(abs(rd.data[0]), rd.data[1], abs(rd.data[2])));
  }
  fragColor = f32x4(col, 1.0f);
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
