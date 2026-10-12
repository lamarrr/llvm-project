// Port of https://www.shadertoy.com/view/fttyWN; author: jorge2017a2.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fttyWN.json.
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
float sdBox(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.0f)) + min(max(d.data[0], d.data[1]), 0.0f);
}
float sdCircle(Context &__ctx, f32x2 p, float r) { return length(p) - r; }
float sdSegment(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h);
}
float random(Context &__ctx, f32x2 v) {
  return fract(sin(dot(v, f32x2(114.0f, 514.0f))) * 1919810.0f) * 2.0f - 1.0f;
}
float random1d(Context &__ctx, float v) {
  return fract(sin(dot(f32x2(v, v), f32x2(114.0f, 514.0f))) * 1919810.0f) *
             2.0f -
         1.0f;
}
float noise(Context &__ctx, f32x2 v, float octave) {
  f32x2 maj = floor(v * pow(2.0f, octave));
  f32x2 sub = fract(v * pow(2.0f, octave));
  float v00 = random(__ctx, maj);
  float v10 = random(__ctx, maj + f32x2(1.0f, 0.0f));
  float v01 = random(__ctx, maj + f32x2(0.0f, 1.0f));
  float v11 = random(__ctx, maj + f32x2(1.0f, 1.0f));
  f32x2 u = smoothstep(0.f, 1.f, sub);
  return mix(v00, v10, u.data[0]) +
         (v01 - v00) * u.data[1] * (1.0f - u.data[0]) +
         (v11 - v10) * u.data[0] * u.data[1];
}
f32x3 nubeB(Context &__ctx, f32x2 fragCoord, f32x3 colOut) {
  f32x2 st = fragCoord;
  float v = 0.0f;
  for (int i = 0; i < 10; i++) {
    v += noise(__ctx, st, float(i)) / pow(2.0f, float(i));
  }
  return f32x3(v) * 0.5f + 0.5f;
}
float opRep1D(Context &__ctx, float p, float c) {
  float q = mod(p + 0.5f * c, c) - 0.5f * c;
  return q;
}
float intersectSDF(Context &__ctx, float distA, float distB) {
  return max(distA, distB);
}
float unionSDF(Context &__ctx, float distA, float distB) {
  return min(distA, distB);
}
float differenceSDF(Context &__ctx, float distA, float distB) {
  return max(distA, -distB);
}
f32x3 DrawFigBorde(Context &__ctx, f32x3 pColObj, f32x3 colOut, float distObj) {
  colOut = mix(
      colOut, pColObj,
      smoothstep(8.0f * 1.0f / min(iResolution.data[1], iResolution.data[0]),
                 0.0f, distObj));
  colOut = mix(
      colOut, f32x3(0.0f),
      smoothstep(8.0f * 1.0f / min(iResolution.data[1], iResolution.data[0]),
                 0.0f, abs(distObj) - 0.05f));
  return colOut;
}
f32x2 rotatev2(Context &__ctx, f32x2 p, float ang) {
  float c = cos(ang);
  float s = sin(ang);
  return f32x2(p.data[0] * c - p.data[1] * s, p.data[0] * s + p.data[1] * c);
}
f32x3 Luna(Context &__ctx, f32x2 p, f32x3 colOut) {
  float d1 = sdCircle(__ctx, p, 2.0f);
  colOut = DrawFigBorde(__ctx, f32x3(0.7f), colOut, d1);
  return colOut;
}
f32x3 nubeA(Context &__ctx, f32x2 p, f32x3 colOut) {
  p = f32x2(p.data[1] - 2.0f, p.data[0]);
  float d1 = 3.0f * sin(p.data[0] * 0.25f) + 1.0f * sin(p.data[0] * 0.5f);
  float d2 = 6.0f * cos(p.data[0] * 4.0f);
  float d3 = sdCircle(__ctx, p - f32x2(d1, d2), 1.0f);
  d1 += p.data[1] + d3;
  d2 += p.data[1] + 2.0f + 1.5f * sin(p.data[1] + p.data[0] + iTime);
  float duni = differenceSDF(__ctx, d1, d2);
  float aspect = iResolution.data[0] / iResolution.data[1];
  float px = 2.0f * aspect / iResolution.data[0];
  float sh3 = smoothstep(px, -px, duni);
  colOut = mix(colOut, f32x3(0.0f), sh3);
  return colOut;
}
f32x3 drawGradient(Context &__ctx, f32x2 v_p) {
  return mix(f32x3(0.0f, 0.3f, 0.7f), f32x3(1.0f, 0.8f, 0.7f),
             1.0f - v_p.data[1] + 0.7f);
}
f32x3 GraficarEdificioCol(Context &__ctx, f32x2 p, f32x3 colIn, f32x3 colOut) {
  float t = 0.0f;
  float x = p.data[0];
  float fx1 = 2.f + 2.f * sin(floor(x + t) * 4321.f);
  float y = p.data[1] + fx1;
  f32x3 col = DrawFigBorde(__ctx, colIn, colOut, y);
  return col;
}
f32x3 getColor(Context &__ctx, float c) {
  float r = cos((c - 0.75f) * 3.14159265f);
  float g = cos((c - 0.55f) * 3.14159265f);
  float b = cos((c - 0.25f) * 3.14159265f);
  return f32x3(r, g, b);
}
f32x3 estrella(Context &__ctx, f32x2 p, f32x3 colOut) {
  float r = 5.5f;
  f32x2 uv = p;
  float d1 = sdCircle(__ctx, p, 0.015f);
  float v = r / (d1);
  f32x3 luna = getColor(__ctx, 0.916f) * v;
  colOut = DrawFigBorde(__ctx, luna, colOut, d1);
  return luna;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      -1.0f + 2.0f * fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  uv -= f32x2(0.0f, -0.5f);
  float esc = 8.0f;
  uv *= esc;
  f32x2 uv0 = uv;
  f32x2 st =
      fragCoord.swizzle<0, 1>() / min(iResolution.data[0], iResolution.data[1]);
  float t = iTime;
  f32x2 st0 = st;
  st += f32x2(t * 0.5f, 0.0f + 0.15f * sin(t));
  f32x2 p = st;
  f32x3 col = f32x3(1.0f);
  col = nubeB(__ctx, st, col);
  col *= drawGradient(__ctx, p + f32x2(0.0f, 0.75f));
  col = Luna(__ctx, uv - f32x2(0.0f, 8.0f), col);
  col = nubeA(__ctx, uv - f32x2(0.0f, 8.0f), col);
  uv.data[1] += 2.0f * sin(t + cos(t));
  uv.data[0] = opRep1D(__ctx, uv.data[0] + t * 2.0f, 20.0f);
  col = nubeA(__ctx, uv - f32x2(0.0f, 8.0f), col);
  uv = uv0;
  float ts = 3.0f * sin(t);
  if (ts > 0.0f)
    col = (col + estrella(__ctx, uv - f32x2(0.0f, 8.0f), col)) / 2.0f;
  col = GraficarEdificioCol(__ctx, uv - f32x2(0.0f - iTime, 2.5f),
                            f32x3(0.25f, 0.2f, 1.0f) * 0.5f, col);
  col = GraficarEdificioCol(__ctx, uv - f32x2(0.0f, 2.0f),
                            f32x3(0.5f, 0.2f, 0.5f), col);
  col = GraficarEdificioCol(__ctx, uv - f32x2(0.0f, 1.0f),
                            f32x3(0.25f, 0.2f, 1.0f), col);
  col = pow(col, f32x3(0.554545f));
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
