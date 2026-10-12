// Port of https://www.shadertoy.com/view/ctlGDs; author: remaindeer.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctlGDs.json.
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
struct Context {};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x2x2 rotmat2(Context &__ctx, float angle) {
  float c = cos(angle), s = sin(angle);
  return f32x2x2(c, -s, s, c);
}
float opUnion(Context &__ctx, float d1, float d2) { return min(d1, d2); }
float opOnion(Context &__ctx, float sdf, float thickness) {
  return abs(sdf) - thickness;
}
float opExtrusion(Context &__ctx, float p, float d, float h) {
  f32x2 w = f32x2(d, abs(p) - h);
  return min(max(w.data[0], w.data[1]), 0.0f) + length(max(w, 0.0f));
}
float sdSegment(Context &__ctx, f32x2 p, f32x2 a, f32x2 b, float r) {
  f32x2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h) - r;
}
float sdBox(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.0f)) + min(max(d.data[0], d.data[1]), 0.0f);
}
const float sixto = 1.0f / 6.0f;
float sdMonogram(Context &__ctx, f32x2 p, float t, f32x4 dann) {
  float d = 1.0f;
  if (bool(dann.data[0])) {
    d = sdSegment(__ctx, p, f32x2(0, -1.0f * sixto), f32x2(0, +3.0f * sixto),
                  t);
    d = opUnion(
        __ctx, d,
        opOnion(__ctx, sdBox(__ctx, p + f32x2(+sixto, 0), f32x2(sixto)), t));
  }
  if (bool(dann.data[1])) {
    d = opUnion(__ctx, d,
                sdSegment(__ctx, p, f32x2(-1.0f * sixto, 1.0f * sixto),
                          f32x2(-1.0f * sixto, 2.0f * sixto), t));
    d = opUnion(__ctx, d,
                sdSegment(__ctx, p, f32x2(-1.0f * sixto, 2.0f * sixto),
                          f32x2(1.0f * sixto, 2.0f * sixto), t));
    d = opUnion(__ctx, d,
                sdSegment(__ctx, p, f32x2(1.0f * sixto, 2.0f * sixto),
                          f32x2(1.0f * sixto, 0.0f * sixto), t));
    d = opUnion(
        __ctx, d,
        opOnion(__ctx, sdBox(__ctx, p + f32x2(0, +sixto), f32x2(sixto)), t));
  }
  if (bool(dann.data[2]) || bool(dann.data[3])) {
    d = opUnion(
        __ctx, d,
        sdSegment(__ctx, p, f32x2(0, 0), f32x2(0.0f, -3.0f * sixto), t));
    d = opUnion(__ctx, d,
                sdSegment(__ctx, p, f32x2(0, -sixto),
                          f32x2(2.0f * sixto,
                                (bool(dann.data[3]) ? -1.0f : -3.0f) * sixto),
                          t));
    d = opUnion(__ctx, d,
                sdSegment(__ctx, p, f32x2(2.0f * sixto, -3.0f * sixto),
                          f32x2(2.0f * sixto, -sixto), t));
  }
  return d;
}
float sdSigil(Context &__ctx, f32x2 p, float t, f32x4 dann) {
  float d = sdMonogram(__ctx, p + f32x2(sixto, -4.0f * sixto), t, dann);
  d = opUnion(__ctx, d,
              sdMonogram(__ctx,
                         rotmat2(__ctx, radians(090.0f)) * p +
                             f32x2(sixto, -4.0f * sixto),
                         t, dann));
  d = opUnion(__ctx, d,
              sdMonogram(__ctx,
                         rotmat2(__ctx, radians(180.0f)) * p +
                             f32x2(sixto, -4.0f * sixto),
                         t, dann));
  d = opUnion(__ctx, d,
              sdMonogram(__ctx,
                         rotmat2(__ctx, radians(270.0f)) * p +
                             f32x2(sixto, -4.0f * sixto),
                         t, dann));
  return d;
}
float scene(Context &__ctx, f32x3 p) {
  float t = 0.125f * sixto;
  float y = abs(-cos(iTime / 2.0f) * 5.0f);
  p.set_swizzle<0, 1>(rotmat2(__ctx, iTime / 3.0f) * p.swizzle<0, 1>());
  float d = sdSigil(__ctx, p.swizzle<0, 1>(), t,
                    f32x4(y > 1.0f, y > 2.0f, y > 3.0f, y > 4.0f));
  return opExtrusion(__ctx, p.data[2], d, 0.25f);
}
float march(Context &__ctx, f32x3 ro, f32x3 rd) {
  float depth = 0.0f;
  float d;
  for (int i = 0; i < 255; i++) {
    f32x3 p = ro + depth * rd;
    d = scene(__ctx, p);
    depth += d;
    if (d < 0.001f || depth > 100.0f)
      break;
  }
  d = depth;
  return d;
}
f32x3 normal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(1, -1) * 0.0005f;
  return normalize(
      e.swizzle<0, 1, 1>() * scene(__ctx, p + e.swizzle<0, 1, 1>()) +
      e.swizzle<1, 1, 0>() * scene(__ctx, p + e.swizzle<1, 1, 0>()) +
      e.swizzle<1, 0, 1>() * scene(__ctx, p + e.swizzle<1, 0, 1>()) +
      e.swizzle<0, 0, 0>() * scene(__ctx, p + e.swizzle<0, 0, 0>()));
}
f32x3x3 camera(Context &__ctx, f32x3 cameraPos, f32x3 lookAtPoint) {
  f32x3 cd = normalize(lookAtPoint - cameraPos);
  f32x3 cr = normalize(cross(f32x3(0, 1, 0), cd));
  f32x3 cu = normalize(cross(cd, cr));
  return f32x3x3(-cr, cu, -cd);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.data[1];
  if (m == f32x2(0.0f))
    m = f32x2(0.5f);
  f32x3 col = f32x3(0);
  f32x3 lp = f32x3(0);
  f32x3 ro = f32x3(0, 0, 3);
  float camRadius = 1.0f;
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() * camRadius *
                       rotmat2(__ctx, mix(-3.14159265359f / 2.f,
                                          3.14159265359f / 2.f, m.data[1])));
  ro.set_swizzle<0, 2>(
      ro.swizzle<0, 2>() *
          rotmat2(__ctx, mix(-3.14159265359f, 3.14159265359f, m.data[0])) +
      f32x2(lp.data[0], lp.data[2]));
  f32x3 rd = camera(__ctx, ro, lp) * normalize(f32x3(p, -1));
  float d = march(__ctx, ro, rd);
  if (d > 100.0f) {
    col = f32x3(0, 0, 0);
  } else {
    f32x3 p = ro + rd * d;
    f32x3 norm = normal(__ctx, p);
    f32x3 lightPos = f32x3(0, 2, 2);
    f32x3 lightDir = normalize(lightPos - p) * 0.65f;
    float diff = clamp(dot(norm, lightDir), 0.0f, 1.0f) * 0.5f + 0.5f;
    f32x3 rnd =
        0.5f + 0.5f * cos(iTime + p.swizzle<0, 1, 0>() + f32x3(0, 2, 4));
    col = f32x3(diff) + rnd;
  }
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
