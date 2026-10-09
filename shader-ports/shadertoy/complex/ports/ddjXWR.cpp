// Port of https://www.shadertoy.com/view/ddjXWR; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ddjXWR.json.
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
  TextureCube channel0;
  TextureCube channel1;
};
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
f32x3 rayOri(Context &__ctx) {
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float t = 0.5f * iTime;
  f32x3 ro = f32x3(4.f * cos(t), 4.f * cos(0.8f * t), 4.f * sin(t));
  return ro;
}
float sdBoxFrame(Context &__ctx, f32x3 p, f32x3 b, float e) {
  p = abs(p) - b;
  f32x3 q = abs(p + e) - e;
  return min(min(length(max(f32x3(p.data[0], q.data[1], q.data[2]), 0.0f)) +
                     min(max(p.data[0], max(q.data[1], q.data[2])), 0.0f),
                 length(max(f32x3(q.data[0], p.data[1], q.data[2]), 0.0f)) +
                     min(max(q.data[0], max(p.data[1], q.data[2])), 0.0f)),
             length(max(f32x3(q.data[0], q.data[1], p.data[2]), 0.0f)) +
                 min(max(q.data[0], max(q.data[1], p.data[2])), 0.0f));
}
float dist(Context &__ctx, f32x3 p, float o) {
  float sd = length(p - rayOri(__ctx)) - 0.8f;
  p += cos(f32x3(0.25f * p.swizzle<2, 0, 1>() + iTime));
  p += o;
  p.data[2] -= 1.5f * iTime;
  f32x3 fpos = mod(p, 4.f) - 2.f;
  f32x3 fpos2 = mod(p + 2.f, 4.f) - 2.f;
  float o2 =
      .2f + .05f * tanh(2.f * cos(10.f * p.data[1] - 0.5f * iTime)) / tanh(2.f);
  float d1 = sdBoxFrame(__ctx, fpos, f32x3(2), 0.05f) - o2 + 0.1f * o;
  float d2 = sdBoxFrame(__ctx, fpos2, f32x3(2), 0.05f) - o2 + 0.1f * o;
  return -min(sd, -min(d1, d2));
}
float march(Context &__ctx, f32x3 ro, f32x3 rd, float z, float o) {
  float dO = 0.f;
  float s = sign(z);
  for (int i = 0; i < 200; i++) {
    f32x3 p = ro + rd * dO;
    float dS = dist(__ctx, p, o);
    if (s != sign(dS)) {
      z *= 0.5f;
      s = sign(dS);
    }
    if (abs(dS) < .02f || dO > 15.f)
      break;
    dO += dS * z;
  }
  return min(dO, 15.f);
}
f32x3 normal(Context &__ctx, f32x3 p, float o) {
  float d = dist(__ctx, p, o);
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = d - f32x3(dist(__ctx, p - e.swizzle<0, 1, 1>(), o),
                      dist(__ctx, p - e.swizzle<1, 0, 1>(), o),
                      dist(__ctx, p - e.swizzle<1, 1, 0>(), o));
  return normalize(n);
}
f32x3 rayDir(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i);
  return d;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 ro = rayOri(__ctx);
  f32x3 rd = rayDir(__ctx, uv, ro, f32x3(0), 0.6f);
  f32x3 col = f32x3(0);
  float d1 = march(__ctx, ro, rd, 1.f, 1.f);
  float d2 = march(__ctx, ro, rd, 1.f, 0.f);
  f32x3 p1 = ro + rd * d1;
  f32x3 p2 = ro + rd * d2;
  f32x3 n1 = normal(__ctx, p1, 1.f);
  f32x3 n2 = normal(__ctx, p2, 0.f);
  f32x3 r1 = reflect(rd, n1);
  f32x3 r2 = reflect(rd, n2);
  f32x3 ld = normalize(f32x3(1, 2, 3));
  float dif1 = .5f + .5f * dot(abs(n1), ld);
  float dif2 = .5f + .5f * dot(abs(n2), ld);
  float spec1 = pow(dif1, 256.f);
  float spec2 = pow(dif2, 256.f);
  float sp = -0.1f * (spec1 + spec2);
  f32x3 pl1 = pal(__ctx, sp + -0.2f * length(p1) + 0.1f * d1,
                  0.5f * f32x3(.5f, 1, 2) / 3.f);
  f32x3 pl2 = pal(__ctx, sp + 0.5f + 0.1f * d2, 0.5f * f32x3(.5f, 1, 2) / 3.f);
  col = clamp(col, 0.f, 1.f);
  col =
      mix(f32x3(1.f - 1.f / cosh(-.2f * d1)),
          f32x3(1.f - 1.f / cosh(-.2f * d2) * pl2), clamp(d1 / d2, 0.f, 1.5f));
  col *= pl1;
  col *= .96f +
         .02f * step(mod(fragCoord.data[0] + fragCoord.data[1] + iTime, 14.f),
                     7.f) +
         .02f * step(mod(fragCoord.data[0] - fragCoord.data[1] + iTime, 14.f),
                     7.f);
  col *= 1.f - 0.2f * abs(dif1 - dif2);
  col *= 1.f + 0.4f * f32x3(spec1 + spec2);
  f32x3 tx = texture(__ctx.channel0, (r1 + r2)).swizzle<0, 1, 2>();
  f32x3 tx2 = texture(__ctx.channel1, (r1 + r2)).swizzle<0, 1, 2>();
  tx = mix(tx, f32x3(1), 1.f - 1.f / cosh(-0.1f * (d1 + d2)));
  tx2 = mix(tx2, f32x3(1), 1.f - 1.f / cosh(-0.1f * (d1 + d2)));
  col *= 1.4f * tx2;
  col = mix(col, tx,
            .5f + .5f * tanh(4.f * cos(0.25f * iTime + 0.1f * p1.data[1])) /
                      tanh(4.f));
  col = pow(col, f32x3(1.f / 2.4f));
  fragColor = f32x4(col, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_cube<f32x4> channel0,
               __binding__(1, 0) sampler sampling0,
               __binding__(0, 1) texture_cube<f32x4> channel1,
               __binding__(1, 1) sampler sampling1) {
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
  Context __ctx{{channel0, sampling0}, {channel1, sampling1}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
