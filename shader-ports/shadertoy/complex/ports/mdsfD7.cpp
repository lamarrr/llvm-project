// Port of https://www.shadertoy.com/view/mdsfD7; author: fishy.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mdsfD7.json.
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
float sdSegment(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h);
}
float sdRing(Context &__ctx, f32x2 p, f32x2 o, float r) {
  return abs(distance(p, o) - r);
}
float sdPoint(Context &__ctx, f32x2 p, f32x2 o) { return distance(p, o); }
float sdPointedEggRing(Context &__ctx, f32x2 p, f32x2 o, float r, float h) {
  return abs(
      p.data[1] - o.data[1] > 0.0f
          ? length(f32x2(abs(p.data[0]), p.data[1]) - o + f32x2(h, 0.0f)) -
                (r + h)
          : length(p - o) - r);
}
float sdCubicBezier(Context &__ctx, f32x2 uv, f32x2 s, f32x2 c1, f32x2 c2,
                    f32x2 e, float res) {
  float r = 1e20f;
  float st = 1.f / res;
  f32x2 prev = s;
  for (float i = 0.f; i < 1.f; i += st) {
    float j = 1.f - i;
    f32x2 point = j * j * j * s + 3.f * j * j * i * c1 + 3.f * j * i * i * c2 +
                  i * i * i * e;
    r = min(r, sdSegment(__ctx, uv, prev, point));
    prev = point;
  }
  r = min(r, sdSegment(__ctx, uv, prev, e));
  return r;
}
float sdGlpyh(Context &__ctx, f32x2 uv, int type) {
  float r = 1e30f;
  r = min(r, sdRing(__ctx, uv, f32x2(0), 1.0f));
  switch (type) {
  case 0:
    r = min(r, sdSegment(__ctx, f32x2(abs(uv.data[0]), uv.data[1]), f32x2(0, 1),
                         f32x2(0.15f, 0.7f)));
    r = min(r, sdRing(__ctx, uv, f32x2(0, 0.66f), 0.154f));
    r = min(r, sdSegment(__ctx, f32x2(abs(uv.data[0]), uv.data[1]),
                         f32x2(0, 0.506f), f32x2(0.953f, -0.3f)));
    r = min(r,
            sdSegment(__ctx, uv, f32x2(0.953f, -0.3f), f32x2(-0.953f, -0.3f)));
    r = min(r, sdSegment(__ctx, uv, f32x2(0, 0.506f), f32x2(0, -1)));
    r = min(r, sdSegment(__ctx, uv, f32x2(-0.1f, 0), f32x2(0.1f, 0.1f)));
    r = min(r, sdSegment(__ctx, uv, f32x2(-0.1f, -0.1f), f32x2(0.1f, 0)));
    return r;
  case 1:
    r = min(r, sdSegment(__ctx, f32x2(abs(uv.data[0]), uv.data[1]), f32x2(0, 1),
                         f32x2(0.4f, 0.6f)));
    r = min(r, sdSegment(__ctx, f32x2(abs(uv.data[0]), uv.data[1]),
                         f32x2(0.4f, 0.6f), f32x2(0.0f, -0.3f)));
    r = min(r,
            sdSegment(__ctx, uv, f32x2(0.953f, -0.3f), f32x2(-0.953f, -0.3f)));
    r = min(r, sdCubicBezier(__ctx, f32x2(abs(uv.data[0]), uv.data[1]),
                             f32x2(0.0f, 0.2f), f32x2(0.2f, 0.2f),
                             f32x2(0.6f, 0.1f), f32x2(0.97f, -0.2f), 8.0f));
    r = min(r, sdSegment(__ctx, uv, f32x2(0, 1), f32x2(0, -0.3f)));
    r = min(r, sdSegment(__ctx, f32x2(abs(uv.data[0]), uv.data[1]),
                         f32x2(0, -0.3f), f32x2(0.15f, -0.45f)));
    r = min(r, sdSegment(__ctx, f32x2(abs(uv.data[0]), uv.data[1]),
                         f32x2(0.15f, -0.45f), f32x2(0, -0.6f)));
    return r;
  case 2:
    r = min(r, sdSegment(__ctx, f32x2(abs(uv.data[0]), uv.data[1]),
                         f32x2(0, -0.2f), f32x2(0.25f, 0.05f)));
    r = min(r, sdRing(__ctx, uv, f32x2(0, -0.5f), 0.3f));
    r = min(r, sdPoint(__ctx, uv, f32x2(0, -0.5f)));
    r = min(r, sdSegment(__ctx, f32x2(abs(uv.data[0]), uv.data[1]),
                         f32x2(0, 0.4f), f32x2(0.25f, 0.65f)));
    r = min(r, sdSegment(__ctx, uv, f32x2(0.1f, 0.5f), f32x2(-0.1f, 0.5f)));
    r = min(r, sdSegment(__ctx, uv, f32x2(-0.25f, 0.65f), f32x2(0.25f, 0.65f)));
    r = min(r, sdSegment(__ctx, uv, f32x2(0, -0.2f), f32x2(0, 0.4f)));
    return r;
  case 3:
    r = min(r, sdPointedEggRing(__ctx, uv, f32x2(0.0f, -0.5f), 0.5f, 0.95f));
    r = min(r, sdRing(__ctx, uv, f32x2(0.0f, -0.5f), 0.5f));
    r = min(r, sdPoint(__ctx, uv, f32x2(0.0f, -0.5f)));
    r = min(r, sdPointedEggRing(__ctx, uv, f32x2(0.0f, 0.3f), 0.3f, 0.65f));
    r = min(r, sdRing(__ctx, uv, f32x2(0.0f, 0.3f), 0.3f));
    return r;
  }
}
float map(Context &__ctx, f32x2 uv) {
  return sdGlpyh(__ctx, uv, int(iTime) % 4);
}
f32x3 colorSdf(Context &__ctx, f32x2 uv, f32x4 m) {
  float d = map(__ctx, uv);
  float px = fwidth(d);
  float md = abs(map(__ctx, m.swizzle<0, 1>()));
  f32x3 col = mix(f32x3(0.094f, 0.494f, 0.988f), f32x3(1.000f, 0.082f, 0.980f),
                  sign(d) * 0.5f + 0.5f);
  col *=
      smoothstep(-px * 200.f, px * 200.f, cos(d * 200.f)) * 0.2f + 1.f - 0.2f;
  col = mix(col, f32x3(1), smoothstep(0.01f + px, 0.01f - px, abs(d)));
  if (m.data[2] > 0.f) {
    float mdc = smoothstep(
        px, -px,
        min(abs(distance(m.swizzle<0, 1>(), uv) - md + 0.005f) - 0.005f,
            distance(m.swizzle<0, 1>(), uv) - 0.005f * 2.f));
    col = mix(col, f32x3(1.000f, 0.875f, 0.000f), mdc);
  }
  col /= d + 1.f;
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord * 2.f - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x4 m = f32x4((iMouse.swizzle<0, 1>() * 2.f - iResolution.swizzle<0, 1>()) /
                      iResolution.data[1],
                  iMouse.swizzle<2, 3>());
  float px = 2.0f / iResolution.data[1];
  f32x3 col = f32x3(colorSdf(__ctx, uv * 1.2f, m * 1.2f));
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
