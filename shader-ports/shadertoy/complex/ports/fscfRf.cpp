// Port of https://www.shadertoy.com/view/fscfRf; author: Del.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fscfRf.json.
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
float opExtrusion(Context &__ctx, f32x3 p, float dd, float h) {
  float d = dd;
  f32x2 w = f32x2(d, abs(p.data[1]) - h);
  return min(max(w.data[0], w.data[1]), 0.0f) + length(max(w, 0.0f));
}
f32x2x2 rotate(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, s, -s, c);
}
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 d = abs(p) - b;
  return min(max(d.data[0], max(d.data[1], d.data[2])), 0.0f) +
         length(max(d, 0.0f));
}
float smin(Context &__ctx, float a, float b, float r) {
  float f = max(0.f, 1.f - abs(b - a) / r);
  return min(a, b) - r * .25f * f * f;
}
f32x2 hash2(Context &__ctx, f32x2 p) {
  return fract(
      sin(f32x2(dot(p, f32x2(127.1f, 311.7f)), dot(p, f32x2(269.5f, 183.3f)))) *
      43758.5453f);
}
f32x4 VoronoiGrid(Context &__ctx, f32x2 x, f32x2 &id, float _smooth) {
  f32x2 n = floor(x);
  f32x2 f = fract(x);
  f32x2 mg, mr;
  float md = 8.0f;
  for (int j = -1; j <= 1; j++)
    for (int i = -1; i <= 1; i++) {
      f32x2 g = f32x2(float(i), float(j));
      f32x2 o = hash2(__ctx, n + g);
      f32x2 r = g + o - f;
      float d = dot(r, r);
      if (d < md) {
        md = d;
        mr = r;
        mg = g;
      }
    }
  md = 8.0f;
  for (int j = -2; j <= 2; j++)
    for (int i = -2; i <= 2; i++) {
      f32x2 g = mg + f32x2(float(i), float(j));
      f32x2 o = hash2(__ctx, n + g);
      f32x2 r = g + o - f;
      if (dot(mr - r, mr - r) > 0.00001f)
        if (_smooth > 0.0f)
          md =
              smin(__ctx, md, dot(0.5f * (mr + r), normalize(r - mr)), _smooth);
        else
          md = min(md, dot(0.5f * (mr + r), normalize(r - mr)));
    }
  id = (n + mg) + f32x2(0.5f);
  md = max(md, 0.0f);
  return f32x4(md, length(mr), mr);
}
f32x2 smoothRot(Context &__ctx, f32x2 p, float s, float m, float c, float d) {
  s *= 0.5f;
  float k = length(p);
  float x = asin(sin(atan(p.data[0], p.data[1]) * s) * (1.0f - m)) * k;
  float ds = k * s;
  float y = mix(ds, 2.0f * ds - sqrt(x * x + ds * ds), c);
  return f32x2(x / s, y / s - d);
}
float map(Context &__ctx, f32x3 pos) {
  f32x3 pp = pos;
  float d = 100000.0f;
  float bd = sdBox(__ctx, pp, f32x3(4.0f, 0.5f, 4.0f));
  if (iMouse.data[2] < 0.5f)
    pp.set_swizzle<0, 2>(
        smoothRot(__ctx, pp.swizzle<0, 2>(), 10.0f, 0.3f, 0.5f, 0.0f));
  float scale = 1.0f + (sin(iTime * 0.325f) * 0.7f);
  f32x2 id = f32x2(0.0f);
  f32x2 uv = pp.swizzle<0, 2>();
  f32x4 h = VoronoiGrid(__ctx, uv * scale, id, 0.1f);
  uv = id / scale;
  float height = 0.2f;
  float dd = h.data[0] / scale;
  dd = opExtrusion(__ctx, pos, dd, height);
  dd = dd - 0.05f;
  dd = smin(__ctx, dd, pos.data[1] + 0.2f, 0.1f);
  dd = max(dd, bd);
  d = min(d, dd);
  return d - 0.05f;
}
f32x3 calcNormal(Context &__ctx, f32x3 pos) {
  f32x2 e = f32x2(1.0f, -1.0f) * 0.5773f;
  const float eps = 0.001f;
  return normalize(
      e.swizzle<0, 1, 1>() * map(__ctx, pos + e.swizzle<0, 1, 1>() * eps) +
      e.swizzle<1, 1, 0>() * map(__ctx, pos + e.swizzle<1, 1, 0>() * eps) +
      e.swizzle<1, 0, 1>() * map(__ctx, pos + e.swizzle<1, 0, 1>() * eps) +
      e.swizzle<0, 0, 0>() * map(__ctx, pos + e.swizzle<0, 0, 0>() * eps));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float an = 0.5f * iTime;
  f32x3 ro = f32x3(6.f * cos(an), 4.0f + sin(iTime * 0.9f), 6.0f * sin(an));
  f32x3 ta = f32x3(0.0f, 0.0f, 0.0f);
  f32x3 ww = normalize(ta - ro);
  f32x3 uu = normalize(cross(ww, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 vv = normalize(cross(uu, ww));
  f32x3 tot = f32x3(0.0f);
  f32x2 p =
      (-iResolution.swizzle<0, 1>() + 2.0f * fragCoord) / iResolution.data[1];
  f32x3 rd = normalize(p.data[0] * uu + p.data[1] * vv + 1.8f * ww);
  const float tmax = 50.0f;
  float t = 0.0f;
  for (int i = 0; i < 128; i++) {
    f32x3 pos = ro + t * rd;
    float h = map(__ctx, pos);
    if (h < 0.0001f || t > tmax)
      break;
    t += h;
  }
  float v = 1.0f - abs(p.data[1]);
  f32x3 col = f32x3(v * 0.1f);
  if (t < tmax) {
    f32x3 pos = ro + t * rd;
    f32x3 nor = calcNormal(__ctx, pos);
    float dif = clamp(dot(nor, f32x3(0.7f, 0.6f, 0.4f)), 0.0f, 1.0f);
    float amb = 0.5f + 0.5f * dot(nor, f32x3(0.0f, 0.8f, 0.6f));
    col = f32x3(0.2f, 0.3f, 0.4f) * amb + f32x3(0.8f, 0.7f, 0.5f) * dif;
  }
  col = sqrt(col);
  tot += col;
  fragColor = f32x4(tot, 1.0f);
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
