// Port of https://www.shadertoy.com/view/ctcyWn; author: yasuo.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctcyWn.json.
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
float random(Context &__ctx, f32x2 p) {
  return fract(sin(dot(p.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
f32x3 graphicItem0(Context &__ctx, f32x2 p, f32x3 col) {
  f32x2 prevP = p;
  float d = length(p - f32x2(0.0f, 0.12f)) - 0.06f;
  float d2 = max(abs(p).data[0] - f32x2(0.025f, 0.1f).data[0],
                 abs(p).data[1] - f32x2(0.025f, 0.1f).data[1]);
  d = min(d, d2);
  d2 = length(p - f32x2(0.0f, -0.098f)) - 0.0256f;
  d = abs(min(d, d2)) - 0.007f;
  d2 = length(p - f32x2(0.0f, 0.12f)) - 0.02f;
  d = min(d, d2);
  d2 = length(p - f32x2(0.0f, 0.24f)) - 0.015f;
  d = min(d, d2);
  d2 = abs(length(p - f32x2(0.0f, 0.24f)) - 0.03f) - 0.002f;
  d = min(d, d2);
  col = mix(col, f32x3(0.8f),
            smoothstep(1.5f / min(iResolution.data[1], iResolution.data[0]),
                       -1.5f / min(iResolution.data[1], iResolution.data[0]),
                       d - 0.0f));
  return col;
}
f32x3 graphicItem0Group(Context &__ctx, f32x2 p, f32x3 col) {
  f32x2 prevP = p;
  col = graphicItem0(__ctx, p, col);
  p.data[0] = abs(p.data[0]) - 0.12f;
  p.data[1] -= 0.1f;
  col = graphicItem0(__ctx, p, col);
  return col;
}
f32x3 graphicItem0Layer(Context &__ctx, f32x2 p, f32x3 col) {
  f32x2 prevP = p;
  p.data[0] = abs(p.data[0]);
  p.data[1] += iTime * 0.1f;
  p *= 2.5f;
  f32x2 id = floor(p);
  f32x2 gr = fract(p) - 0.5f;
  float n = random(__ctx, id);
  gr.data[0] += sin(n * 2.f) * 0.25f;
  gr.data[1] += sin(n * 2.f) * 0.3f + mix(0.0f, 1.f, step(0.9f, n));
  gr *= clamp(n * 1.5f, 0.85f, 1.5f);
  col = graphicItem0Group(__ctx, gr, col);
  return col;
}
f32x3 graphicItem0Layer2(Context &__ctx, f32x2 p, f32x3 col) {
  f32x2 prevP = p;
  p.data[0] = abs(p.data[0]) - 0.12f;
  p.data[1] += iTime * 0.12f;
  p *= 2.f;
  f32x2 id = floor(p);
  f32x2 gr = fract(p) - 0.5f;
  float n = random(__ctx, id);
  gr.data[0] += sin(n * 2.f) * 0.25f;
  gr.data[1] += sin(n * 2.f) * 0.3f + mix(0.0f, 1.f, step(0.9f, n));
  gr *= clamp(n * 1.5f, 0.6f, 1.5f);
  col = graphicItem0(__ctx, gr, col);
  return col;
}
float hexagon(Context &__ctx, f32x2 p, float animate) {
  p *= f32x2x2(cos(radians(-30.f * iTime) * animate),
               -sin(radians(-30.f * iTime) * animate),
               sin(radians(-30.f * iTime) * animate),
               cos(radians(-30.f * iTime) * animate));
  float size = 0.1f;
  float d = max(abs(p).data[0] - f32x2(size - 0.02f, 0.1f).data[0],
                abs(p).data[1] - f32x2(size - 0.02f, 0.1f).data[1]);
  p.data[0] = abs(p.data[0]) - size;
  p.data[1] = abs(p.data[1]) - size * 0.35f;
  float a = radians(-120.f);
  d = max(-dot(p, f32x2(cos(a), sin(a))), d);
  return abs(d) - 0.003f;
}
f32x3 graphicItem1(Context &__ctx, f32x2 p, f32x3 col) {
  f32x2 prevP = p;
  float d = hexagon(__ctx, p, 0.0f);
  p.data[1] = abs(p.data[1]) - 0.19f;
  float d2 = hexagon(__ctx, p, 0.0f);
  d = min(d, d2);
  p = prevP;
  p.data[0] = abs(p.data[0]) - 0.16f;
  p.data[1] = abs(p.data[1]) - 0.09f;
  d2 = hexagon(__ctx, p, 1.f);
  d = min(d, d2);
  col = mix(col, f32x3(0.7f),
            smoothstep(1.5f / min(iResolution.data[1], iResolution.data[0]),
                       -1.5f / min(iResolution.data[1], iResolution.data[0]),
                       d - 0.0f));
  return col;
}
f32x3 graphicItem1Layer(Context &__ctx, f32x2 p, f32x3 col) {
  f32x2 prevP = p;
  p.data[0] = abs(p.data[0]) - 0.3f;
  p.data[1] += iTime * 0.15f;
  p.data[1] += 0.2f;
  p *= 2.1f;
  f32x2 id = floor(p);
  f32x2 gr = fract(p) - 0.5f;
  float n = random(__ctx, id);
  gr.data[0] += sin(n * 10.f) * 0.1f;
  gr.data[1] += sin(n * 10.f) * 0.1f + mix(0.0f, 1.f, step(0.9f, n));
  gr *= f32x2x2(cos(n * 2.f), -sin(n * 2.f), sin(n * 2.f), cos(n * 2.f));
  gr *= clamp(n * 2.5f, 0.85f, 2.5f);
  col = graphicItem1(__ctx, gr, col);
  return col;
}
f32x3 graphicItem2(Context &__ctx, f32x2 p, f32x3 col) {
  float d = 10.f;
  for (float i = 0.f; i < 3.f; i++) {
    p = abs(p) - 0.01f;
    p *= f32x2x2(cos(radians(45.0f + (30.0f * iTime))),
                 -sin(radians(45.0f + (30.0f * iTime))),
                 sin(radians(45.0f + (30.0f * iTime))),
                 cos(radians(45.0f + (30.0f * iTime))));
    p.data[1] += 0.05f;
    p *= 1.6f;
    f32x2 prevP = p;
    p.data[1] += 0.2f;
    p.data[0] = abs(p.data[0]);
    p.data[0] -= 0.22f;
    p *= f32x2x2(cos(radians(30.0f)), -sin(radians(30.0f)), sin(radians(30.0f)),
                 cos(radians(30.0f)));
    d = length(p) - 0.4f;
    d = max(-(length(p - f32x2(0.15f, 0.0f)) - 0.31f), d);
    p = prevP;
    p.data[1] -= 0.4f;
    float d2 = length(p) - 0.4f;
    d2 = max(-(length(p - f32x2(0.0f, 0.15f)) - 0.31f), d2);
    d = min(d, d2);
    p = prevP;
    p.data[1] -= 0.05f;
    d2 = abs(length(p) - 0.35f) - 0.05f;
    d = abs(min(d, d2)) - 0.01f;
  }
  col = mix(col, f32x3(0.4f),
            smoothstep(1.5f / min(iResolution.data[1], iResolution.data[0]),
                       -1.5f / min(iResolution.data[1], iResolution.data[0]),
                       d - 0.0f));
  return col;
}
f32x3 graphicItem2Layer(Context &__ctx, f32x2 p, f32x3 col) {
  f32x2 prevP = p;
  p *= 0.9f;
  p.data[0] += 0.5f;
  p.data[1] += iTime * 0.2f;
  p.data[1] += 0.5f;
  f32x2 id = floor(p);
  f32x2 gr = fract(p) - 0.5f;
  float n = random(__ctx, id);
  gr.data[1] += sin(n * 50.f) * 0.2f;
  gr *= clamp(n * 1.f, 0.6f, 1.f);
  col = graphicItem2(__ctx, gr, col);
  return col;
}
float circleAnimation(Context &__ctx, f32x2 p, float size, float lineWidth,
                      float dir, float b) {
  f32x2 prevP = p;
  p *= f32x2x2(
      cos(radians(20.f * iTime * dir)), -sin(radians(20.f * iTime * dir)),
      sin(radians(20.f * iTime * dir)), cos(radians(20.f * iTime * dir)));
  float d = abs(abs(length(p) - 0.5f) - size) - lineWidth;
  p = length(p) * cos(mod(atan(p.data[1], p.data[0]) + 6.28f / (b * 8.0f),
                          6.28f / ((b * 8.0f) * 0.5f)) +
                      (b - 1.f) * 6.28f / (b * 8.0f) + f32x2(0, 11));
  p -= 0.007f;
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  float d2 = max(abs(p).data[0] - f32x2(0.02f, 1.f).data[0],
                 abs(p).data[1] - f32x2(0.02f, 1.f).data[1]);
  d = max(-d2, d);
  return d;
}
float graphicItem3(Context &__ctx, f32x2 p) {
  float d = max(abs(p).data[0] - f32x2(0.008f, 0.08f).data[0],
                abs(p).data[1] - f32x2(0.008f, 0.08f).data[1]);
  float d2 = max(abs(p).data[0] - f32x2(0.08f, 0.008f).data[0],
                 abs(p).data[1] - f32x2(0.08f, 0.008f).data[1]);
  d = min(d, d2);
  return d;
}
f32x3 lineGraphicsLayer(Context &__ctx, f32x2 p, f32x3 col) {
  f32x2 prevP = p;
  p.data[0] = abs(p.data[0]);
  p.data[1] += iTime * 0.08f;
  p *= 4.f;
  f32x2 id = floor(p);
  f32x2 gr = fract(p) - 0.5f;
  float n = random(__ctx, id);
  gr *= f32x2x2(cos(radians(mix(0.0f, 90.f, step(0.5f, n)))),
                -sin(radians(mix(0.0f, 90.f, step(0.5f, n)))),
                sin(radians(mix(0.0f, 90.f, step(0.5f, n)))),
                cos(radians(mix(0.0f, 90.f, step(0.5f, n)))));
  float lineWidth = 0.008f;
  f32x3 lineColor = f32x3(0.3f);
  gr *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
                cos(radians(45.f)));
  gr.data[0] = abs(gr.data[0]) - 0.707f;
  float d = circleAnimation(__ctx, gr, 0.14f, lineWidth, -1.0f, 2.f);
  col = mix(col, lineColor,
            smoothstep(1.5f / min(iResolution.data[1], iResolution.data[0]),
                       -1.5f / min(iResolution.data[1], iResolution.data[0]),
                       d - -0.001f));
  d = abs(abs(length(gr) - 0.5f) - 0.09f) - lineWidth;
  col = mix(col, lineColor,
            smoothstep(1.5f / min(iResolution.data[1], iResolution.data[0]),
                       -1.5f / min(iResolution.data[1], iResolution.data[0]),
                       d - -0.001f));
  d = circleAnimation(__ctx, gr, 0.03f, lineWidth, 1.0f, 2.f);
  col = mix(col, lineColor,
            smoothstep(1.5f / min(iResolution.data[1], iResolution.data[0]),
                       -1.5f / min(iResolution.data[1], iResolution.data[0]),
                       d - -0.001f));
  d = abs(abs(length(gr) - 0.5f) - 0.19f) - lineWidth;
  col = mix(col, lineColor,
            smoothstep(1.5f / min(iResolution.data[1], iResolution.data[0]),
                       -1.5f / min(iResolution.data[1], iResolution.data[0]),
                       d - -0.001f));
  return col;
}
f32x3 lineGraphicsLayer2(Context &__ctx, f32x2 p, f32x3 col) {
  f32x2 prevP = p;
  p.data[1] += iTime * 0.08f;
  p *= 4.f;
  p -= 0.5f;
  f32x2 id = floor(p);
  f32x2 gr = fract(p) - 0.5f;
  float n = random(__ctx, id);
  float lineWidth = 0.008f;
  f32x3 lineColor = f32x3(0.6f);
  float d = mix(0.0f, step(0.5f, n),
                circleAnimation(__ctx, gr, 0.35f, lineWidth, 1.f, 1.5f));
  float d2 = mix(0.0f, step(0.5f, n),
                 circleAnimation(__ctx, gr, 0.3f, lineWidth, -1.f, 1.5f));
  d = min(d, d2);
  d2 = mix(step(0.5f, n), 1.0f, graphicItem3(__ctx, gr));
  d = min(d, d2);
  col = mix(col, lineColor,
            smoothstep(1.5f / min(iResolution.data[1], iResolution.data[0]),
                       -1.5f / min(iResolution.data[1], iResolution.data[0]),
                       d - -0.002f));
  return col;
}
f32x3 drawGraphics(Context &__ctx, f32x2 p, f32x3 col) {
  f32x2 prevP = p;
  col = graphicItem2Layer(__ctx, p, col);
  col = lineGraphicsLayer(__ctx, p, col);
  col = lineGraphicsLayer2(__ctx, p, col);
  col = graphicItem0Layer(__ctx, p, col);
  col = graphicItem0Layer2(__ctx, p, col);
  col = graphicItem1Layer(__ctx, p, col);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col = f32x3(0.0f);
  col = drawGraphics(__ctx, p, col);
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
