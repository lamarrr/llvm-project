// Port of https://www.shadertoy.com/view/ctfXRs; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctfXRs.json.
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
struct Context {
  Texture2D channel0;
  Texture2D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float hash(Context &__ctx, float a, float b) {
  int x = float_bits_to_int(a * a / 7.f) ^ float_bits_to_int(a),
      y = float_bits_to_int(b * b / 7.f) ^ float_bits_to_int(b);
  return float((x * x + y) * (y * y - x) - x) / 2.14e9f;
}
f32x3 erot(Context &__ctx, f32x3 p, f32x3 ax, float ro) {
  return mix(dot(ax, p) * ax, p, cos(ro)) + cross(ax, p) * sin(ro);
}
float cc(Context &__ctx, float a, float b) {
  float f = tanh(a * cos(b)) / tanh(a);
  return sign(f) * pow(abs(f), 0.25f);
}
float cs(Context &__ctx, float a, float b) {
  float f = tanh(a * sin(b)) / tanh(a);
  return sign(f) * pow(abs(f), 0.25f);
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
  return floor(b - .5f) + .5f +
         .5f * tanh(a * (fract(b - .5f) - .5f)) / tanh(.5f * a);
}
f32x2 smin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.f, 1.f);
  return f32x2(mix(b, a, h) - k * h * (1.f - h), h);
}
float smax(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f - 0.5f * (b - a) / k, 0.f, 1.f);
  return mix(b, a, h) + k * h * (1.f - h);
}
const float Semitone = 1.05946309436f;
float KeyToFrequency(Context &__ctx, float n) {
  return pow(Semitone, (n - 8.f)) * 440.f / 48000.f;
}
f32x3 ori(Context &__ctx) {
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float r = mix(100.f, 12.f, tanh(.2f * iTime));
  f32x3 ro =
      f32x3(r * cos(.8f * iTime), cos(1.7f * iTime), r * sin(.8f * iTime));
  return ro;
}
f32x2 map(Context &__ctx, f32x3 p) {
  f32x3 p1 = p;
  p1.set_swizzle<0, 1>(p1.swizzle<0, 1>() *
                       (f32x2x2(cos(0.5f * iTime), -sin(0.5f * iTime),
                                sin(0.5f * iTime), cos(0.5f * iTime))));
  p1.set_swizzle<1, 2>(p1.swizzle<1, 2>() *
                       (f32x2x2(cos(0.215f * iTime), -sin(0.215f * iTime),
                                sin(0.215f * iTime), cos(0.215f * iTime))));
  float d1 = length(p1.swizzle<0, 1>()) - 0.5f;
  float td = length(f32x2(p1.data[2], d1));
  float sd = length(p) - 1.25f;
  float mx =
      .5f + .5f * tanh(5.f * cos(0.f * length(p) + 0.5f * iTime)) / tanh(5.f);
  float mx2 =
      .5f + .5f * tanh(5.f * cos(0.f * length(p) + 0.4f * iTime)) / tanh(5.f);
  float sc = smin(__ctx, td, sd, 1.f).data[1];
  sc *= 0.25f * 8.f;
  sc -= 0.5f * mix(td, sd, mix(0.f, 2.f, mx2));
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() *
                      (f32x2x2(cos(0.4f * iTime), -sin(0.4f * iTime),
                               sin(0.4f * iTime), cos(0.4f * iTime))));
  f32x3 p2 = mod(p - 0.5f * sc, sc) - 0.5f * sc;
  float d =
      mix(length(p2) + 0.2f * sc, length(p2.swizzle<0, 2>()) + 0.12f * sc, mx);
  float s = texture(__ctx.channel1, f32x2(14.f, 0.25f)).data[0];
  d = min(d, length(p) - 2.f * s);
  d = smax(__ctx, d, length(p) - 10.f, 1.f);
  return f32x2(d, sc);
}
f32x3 march(Context &__ctx, f32x3 ro, f32x3 rd, float z) {
  float d = 0.f;
  float s = sign(z);
  int steps = 0;
  float mat = 0.f;
  for (int i = 0; i < 400; i++) {
    f32x3 p = ro + rd * d;
    f32x2 m = map(__ctx, p);
    m.data[0] *=
        0.7f + 0.3f * hash(__ctx, hash(__ctx, p.data[0], p.data[2]), p.data[1]);
    if (s != sign(m.data[0])) {
      z *= 0.5f;
      s = sign(m.data[0]);
    }
    if (abs(m.data[0]) < .001f || d > 100.f) {
      steps = i + 1;
      mat = m.data[1];
      break;
    }
    d += m.data[0] * z;
  }
  return f32x3(min(d, 100.f), steps, mat);
}
f32x3 norm(Context &__ctx, f32x3 p) {
  float d = map(__ctx, p).data[0];
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = d - f32x3(map(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                      map(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                      map(__ctx, p - e.swizzle<1, 1, 0>()).data[0]);
  return normalize(n);
}
f32x3 dir(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i);
  return d;
}
float AO(Context &__ctx, f32x3 p, f32x3 n) {
  float occ = 0.f;
  float sc = 1.f;
  for (float i = 0.f; i < 5.f; i++) {
    float h = 0.015f + 0.015f * i;
    float d = map(__ctx, p + h * n).data[0];
    occ += (h - d) * sc;
    sc *= 0.95f;
  }
  return clamp(1.f - 3.f * occ, 0.f, 1.f);
}
float shadow(Context &__ctx, f32x3 ro, f32x3 rd) {
  float res = 1.f;
  float iTime = .001f;
  for (int i = 0; i < 24; i++) {
    float h = map(__ctx, ro + rd * iTime).data[0];
    float s = clamp(32.f * h / iTime, 0.f, 1.f);
    res = min(res, s);
    iTime += clamp(h, 0.01f, 0.2f);
    if (res < .001f || iTime > 100.f)
      break;
  }
  res = clamp(res, 0.0f, 1.0f);
  return smoothstep(0.f, 1.f, res);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 ro = ori(__ctx);
  f32x3 rd = dir(__ctx, uv, ro, f32x3(0), 1.6f);
  f32x3 col = f32x3(0);
  f32x3 m = march(__ctx, ro, rd, 1.f);
  float d = m.data[0];
  f32x3 p = ro + rd * d;
  float l = length(p);
  f32x3 bg = f32x3(244, 242, 199) / 255.f;
  if (d < 100.f) {
    f32x3 n = norm(__ctx, p);
    f32x3 ld = -normalize(p);
    float dif = dot(n, ld) * .5f + .5f;
    float spec = pow(dif, 40.f);
    float fres = pow(1.f + dot(rd, n), 5.f);
    f32x3 an = abs(n);
    f32x3 c1 =
        texture(__ctx.channel0, 0.14f * p.swizzle<0, 1>()).swizzle<0, 1, 2>();
    f32x3 c2 =
        texture(__ctx.channel0, 0.14f * p.swizzle<1, 2>()).swizzle<0, 1, 2>();
    f32x3 c3 =
        texture(__ctx.channel0, 0.14f * p.swizzle<2, 0>()).swizzle<0, 1, 2>();
    col = an.data[2] * c1 + an.data[0] * c2 + an.data[1] * c3;
    float sd = shadow(__ctx, p + 10.f * .001f * n, ld);
    col *= .5f + .5f * sd;
    col = clamp(col + spec, 0.f, 1.f);
    float ao = AO(__ctx, p + 10.f * .001f * n, n);
    col = mix(col, f32x3(1, 0.5f, 0),
              .2f + .2f * tanh(4.f * cos(8.f * ao)) / tanh(4.f));
    float s = smoothstep(3.9f, 5.f, l);
    col = mix(col, bg, s * fres);
  } else
    col = bg;
  float xp = exp(-0.077f * l);
  f32x3 pl = .5f + .5f * cos(2.f * 3.1415926535f *
                             (.5f * log(l) + .73f + xp * m.data[2] +
                              (1.f - xp) * f32x3(.5f, 1, 2) / 3.f));
  col = mix(col, pl, xp);
  col = 1.f - col;
  col = pow(col, f32x3(1.f / 2.2f));
  col = 1.f - col;
  float o = 2.f * 3.1415926535f / 3.f;
  f32x3 off = 0.4f * tanh(0.2f * iTime) * cos(iTime + f32x3(-o, o, 0));
  col = tanh(f32x3(.1f, 0, 0) + (f32x3(3.5f, 3.35f, 2.5f) + off) * col);
  fragColor = f32x4(col, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
               __binding(1, 0) sampler sampling0,
               __binding(0, 1) texture_2d<f32x4> channel1,
               __binding(1, 1) sampler sampling1) {
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
