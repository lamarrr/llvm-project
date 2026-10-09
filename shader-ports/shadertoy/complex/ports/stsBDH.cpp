// Port of https://www.shadertoy.com/view/stsBDH; author: abrasaxxx93.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/stsBDH.json.
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
float smin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (a - b) / k, 0.0f, 1.0f);
  return mix(a, b, h) - k * h * (1.0f - h);
}
float dot2(Context &__ctx, f32x3 v) { return dot(v, v); }
f32x3 hsbColor(Context &__ctx, float h, float s, float b) {
  clamp(h, 0.f, 1.f);
  clamp(s, 0.f, 1.f);
  clamp(b, 0.f, 1.f);
  float six = 6.f * h;
  float red;
  float gre;
  float blu;
  if (six < 3.f) {
    if (six < 1.f) {
      red = 1.f;
      gre = six;
      blu = 0.f;
    } else if (six < 2.f) {
      red = 2.f - six;
      gre = 1.f;
      blu = 0.f;
    } else {
      red = 0.f;
      gre = 1.f;
      blu = six - 2.f;
    }
  } else {
    if (six < 4.f) {
      red = 0.f;
      gre = 4.f - six;
      blu = 1.f;
    } else if (six < 5.f) {
      red = six - 4.f;
      gre = 0.f;
      blu = 1.f;
    } else {
      red = 1.f;
      gre = 0.f;
      blu = 6.f - six;
    }
  }
  red = 1.f - s * (1.f - red);
  gre = 1.f - s * (1.f - gre);
  blu = 1.f - s * (1.f - blu);
  red *= b;
  gre *= b;
  blu *= b;
  return f32x3(red, gre, blu);
}
float r(Context &__ctx, f32x2 p) {
  return fract(sin(dot(p, f32x2(12.9898f, 78.233f))) * 43758.5453f);
}
float dCuboid(Context &__ctx, f32x3 pos, f32x3 s, f32x3 p) {
  p -= pos;
  return length(max(abs(p) - s, 0.f));
}
float dSphere(Context &__ctx, f32x3 pos, float r, f32x3 p) {
  return length(p - pos) - r;
}
float dTriangle(Context &__ctx, f32x3 a, f32x3 b, f32x3 c, f32x3 p) {
  f32x3 ba = b - a;
  f32x3 pa = p - a;
  f32x3 cb = c - b;
  f32x3 pb = p - b;
  f32x3 ac = a - c;
  f32x3 pc = p - c;
  f32x3 nor = cross(ba, ac);
  return sqrt(
      (sign(dot(cross(ba, nor), pa)) + sign(dot(cross(cb, nor), pb)) +
           sign(dot(cross(ac, nor), pc)) <
       2.0f)
          ? min(min(dot2(__ctx,
                         ba * clamp(dot(ba, pa) / dot2(__ctx, ba), 0.0f, 1.0f) -
                             pa),
                    dot2(__ctx,
                         cb * clamp(dot(cb, pb) / dot2(__ctx, cb), 0.0f, 1.0f) -
                             pb)),
                dot2(__ctx,
                     ac * clamp(dot(ac, pc) / dot2(__ctx, ac), 0.0f, 1.0f) -
                         pc))
          : dot(nor, pa) * dot(nor, pa) / dot2(__ctx, nor));
}
float dCapsule(Context &__ctx, f32x3 a, f32x3 b, float r, f32x3 p) {
  f32x3 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h) - r;
}
float dWing(Context &__ctx, f32x3 pos, float s, f32x3 p) {
  float d =
      dTriangle(__ctx, pos - f32x3(-s / 16.f, 0, 0), pos - f32x3(s / 4.f, 0, 0),
                pos - f32x3(-s / 16.f, 0, -s / 1.5f), p);
  d = min(d, dTriangle(__ctx, pos - f32x3(s / 8.f, 0, -s / 1.5f),
                       pos - f32x3(s / 4.f, 0, 0),
                       pos - f32x3(-s / 16.f, 0, -s / 1.5f), p));
  d = min(d, dCapsule(__ctx, pos - f32x3(s / 8.f, 0, -s / 1.5f),
                      pos - f32x3(-s / 16.f, 0, -s / 1.5f), s / 64.f, p));
  return d;
}
float dAirPlane(Context &__ctx, f32x3 pos, float s, f32x3 p) {
  p.data[2] = abs(p.data[2]);
  float d = dCapsule(__ctx, pos + f32x3(s / 4.f, 0, 0),
                     pos - f32x3(s / 4.f, 0, 0), s / 16.f, p);
  d = min(d, dWing(__ctx, pos, s, p));
  d = min(d, 2.f);
  return d;
}
float dTerrain(Context &__ctx, f32x3 pos, f32x3 p) {
  p -= pos;
  int o = 0;
  float r = pow(.5f, float(o)) * .25f;
  float dSpheres = length(p - f32x3(0, 0, 0)) - r;
  return max(dSpheres,
             dCuboid(__ctx, f32x3(0, .25f, 0), f32x3(.5f, .25f, .5f), p));
}
float sdf(Context &__ctx, f32x3 p) {
  float dCube = dCuboid(__ctx, f32x3(0, 0, 0), f32x3(.5f), p);
  float dTerr = dTerrain(__ctx, f32x3(0, .5f, 0), p);
  return min(dCube, dTerr);
}
f32x3 normal(Context &__ctx, f32x3 p) {
  f32x2 h = f32x2(0.000001f, 0.f);
  return normalize(f32x3(sdf(__ctx, p + h.swizzle<0, 1, 1>()) -
                             sdf(__ctx, p - h.swizzle<0, 1, 1>()),
                         sdf(__ctx, p + h.swizzle<1, 0, 1>()) -
                             sdf(__ctx, p - h.swizzle<1, 0, 1>()),
                         sdf(__ctx, p + h.swizzle<1, 1, 0>()) -
                             sdf(__ctx, p - h.swizzle<1, 1, 0>())));
}
float light(Context &__ctx, f32x3 lo, f32x3 n, f32x3 p) {
  return max(dot(normalize(lo - p), n), 0.f);
}
float rayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float t = 0.f;
  float dt = 0.f;
  int steps = 0;
  f32x3 p = ro;
  do {
    t += dt;
    p += dt * rd;
    dt = sdf(__ctx, p);
    steps++;
  } while (dt >= 0.0001f && t < 10.f && steps < 100);
  return t;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (2.f * fragCoord / iResolution.swizzle<1, 1>()) -
             f32x2(iResolution.data[0] / iResolution.data[1], 1.f);
  f32x3 ro = f32x3(2.f, 2.f, 2.f);
  float zoom = .5f;
  f32x3 lookat = f32x3(0.f);
  f32x3 rd = normalize(lookat - ro);
  f32x3 right = cross(f32x3(0.f, 1.f, 0.f), rd);
  f32x3 up = cross(rd, right);
  ro += (uv.data[0] * right + uv.data[1] * up) / zoom;
  f32x3 col = f32x3(0);
  float d = rayMarch(__ctx, ro, rd);
  if (d < 10.f) {
    f32x3 p = ro + d * rd;
    f32x3 n = normal(__ctx, p);
    f32x3 lo = f32x3(3.f, 3.f, 3.f);
    float li = light(__ctx, lo, n, p);
    col = f32x3(5.f * li / (d * d));
  }
  fragColor = f32x4(col, 1.f);
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
