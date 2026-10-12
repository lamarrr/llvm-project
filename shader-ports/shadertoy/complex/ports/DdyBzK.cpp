// Port of https://www.shadertoy.com/view/DdyBzK; author: spqr.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DdyBzK.json.
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
f32x2x2 rot(Context &__ctx, float a) {
  float ca = cos(a);
  float sa = sin(a);
  return f32x2x2(ca, sa, -sa, ca);
}
float cylcap(Context &__ctx, f32x3 p, float h, float r) {
  f32x2 d = abs(f32x2(length(p.swizzle<0, 2>()), p.data[1])) - f32x2(h, r);
  return min(max(d.data[0], d.data[1]), 0.0f) + length(max(d, 0.0f));
}
float hash11(Context &__ctx, float t) {
  return fract(sin(t * 7361.994f) * 4518.442f);
}
f32x3 lattice(Context &__ctx, float t) {
  float m = t;
  float mx = floor((m - 2.f) / 3.f);
  float my = floor((m - 1.f) / 3.f);
  float mz = floor((m - 0.f) / 3.f);
  float n = m + 1.f;
  float nx = floor((n - 2.f) / 3.f);
  float ny = floor((n - 1.f) / 3.f);
  float nz = floor((n - 0.f) / 3.f);
  f32x3 a = mix(f32x3(hash11(__ctx, mx) - .5f, hash11(__ctx, my) - .5f,
                      hash11(__ctx, mz) - .5f),
                f32x3(hash11(__ctx, nx) - .5f, hash11(__ctx, ny) - .5f,
                      hash11(__ctx, nz) - .5f),
                fract(t));
  return a;
}
float hash31(Context &__ctx, f32x3 p3) {
  p3 = fract(p3 * .1031f);
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
float hash21(Context &__ctx, f32x2 p2) {
  p2 = fract(p2 * .1031f);
  p2 += dot(p2, p2.swizzle<0, 1>() + 33.33f);
  return fract((p2.data[0] + p2.data[1]));
}
float closestWall(Context &__ctx, f32x3 p) {
  f32x3 d = min(fract(p), 1.0f - fract(p));
  return min(min(d.data[0], d.data[1]), d.data[2]);
}
float box(Context &__ctx, f32x3 p, f32x3 s) {
  f32x3 ap = abs(p) - s;
  return length(max(f32x3(0.f), ap)) +
         min(0.f, max(ap.data[0], max(ap.data[1], ap.data[2])));
}
float hash(Context &__ctx, f32x2 p) {
  float h = dot(p, f32x2(127.1f, 311.7f));
  return -1.0f + 2.0f * fract(sin(h) * 43758.5453123f);
}
float noise(Context &__ctx, f32x3 p) {
  f32x3 ip = floor(p);
  p = fract(p);
  p = smoothstep(0.0f, 1.0f, p);
  f32x3 st = f32x3(7, 37, 289);
  f32x4 pos =
      dot(ip, st) + f32x4(0, st.data[1], st.data[2], st.data[1] + st.data[2]);
  f32x4 val = mix(fract(sin(pos) * 7894.552f),
                  fract(sin(pos + st.data[0]) * 7894.552f), p.data[0]);
  f32x2 val2 = mix(val.swizzle<0, 2>(), val.swizzle<1, 3>(), p.data[1]);
  return mix(val2.data[0], val2.data[1], p.data[2]);
}
float fbm(Context &__ctx, f32x2 a) {
  f32x3 p = f32x3(a, 1.f);
  float f = 0.0f;
  f += 0.5000f * noise(__ctx, p);
  p = p * 2.02f;
  f += 0.2500f * noise(__ctx, p);
  p = p * 2.03f;
  f += 0.1250f * noise(__ctx, p);
  p = p * 2.01f;
  return f;
}
float sph(Context &__ctx, f32x3 p, float n) { return length(p) - n; }
float field(Context &__ctx, f32x3 p, float scale) {
  float tt = iTime * .1f;
  scale *= .25f;
  p *= scale;
  f32x3 id = floor(p) - .5f;
  p = fract(p) - .5f;
  float f1 = hash31(__ctx, id);
  float ball = box(__ctx, p, pow(f1, sin(tt * .5f) * .5f + .5f) * f32x3(.49f));
  float bound = closestWall(__ctx, p + .5f);
  return min(bound + .1f, ball) / scale;
}
float map(Context &__ctx, f32x3 p) {
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (rot(__ctx, iTime * .1f)));
  float thing = box(__ctx, p, f32x3(3));
  for (float i = 0.f; i < 4.f; i++) {
    thing = max(thing, -field(__ctx, p, pow(2.f, i)));
  }
  return thing;
}
f32x3 norm(Context &__ctx, f32x3 p) {
  f32x2 off = f32x2(0.001f, 0.0f);
  return normalize(map(__ctx, p) -
                   f32x3(map(__ctx, p - off.swizzle<0, 1, 1>()),
                         map(__ctx, p - off.swizzle<1, 0, 1>()),
                         map(__ctx, p - off.swizzle<1, 1, 0>())) +
                   0.000001f);
}
float tick(Context &__ctx, float t) {
  float i = floor(t);
  float f = fract(t);
  f = smoothstep(0.f, 1.f, f);
  return i + f;
}
float getao(Context &__ctx, f32x3 p, f32x3 n, float dist) {
  return clamp(map(__ctx, p + n * dist) / dist, 0.0f, 1.0f);
}
float path(Context &__ctx, float t) {
  float a = sin(sin(t) + .3f * t);
  return a;
}
float pathfbm(Context &__ctx, float t) {
  float a = 0.f;
  a += path(__ctx, (t + 0.f));
  a += path(__ctx, (t + 21.f) * 2.f) / 2.1f;
  a += path(__ctx, (t + 211.f) * 5.f) / 5.1f;
  return a / 2.f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = f32x2(fragCoord.data[0] / iResolution.data[0],
                   fragCoord.data[1] / iResolution.data[1]);
  uv -= 0.5f;
  uv /= f32x2(iResolution.data[1] / iResolution.data[0], 1);
  f32x3 s = f32x3(0, 0, -4);
  s *= (sin(iTime * .3f) * .5f + .5f) + .2f;
  f32x3 t = f32x3(0);
  float tt = iTime * .3f;
  s.set_swizzle<0, 2>(s.swizzle<0, 2>() * (rot(__ctx, tt * .4f)));
  s.set_swizzle<0, 1>(s.swizzle<0, 1>() * (rot(__ctx, tt * .5f)));
  s.set_swizzle<2, 1>(s.swizzle<2, 1>() * (rot(__ctx, tt * .7f)));
  f32x3 cz = normalize(t - s);
  f32x3 cx = normalize(cross(cz, f32x3(0, 1, 0)));
  f32x3 cy = -normalize(cross(cz, cx));
  f32x3 r = normalize(cx * uv.data[0] + cy * uv.data[1] + cz);
  f32x3 col = f32x3(0);
  f32x3 p = s;
  float dd = 0.f;
  float side = sign(map(__ctx, p));
  for (int i = 0; i < 400; i++) {
    float d = map(__ctx, p) * side;
    if (d < 0.001f) {
      f32x3 n = norm(__ctx, p) * side;
      f32x3 l = normalize(f32x3(-1));
      if (dot(n, l) < 0.f) {
      }
      f32x3 h = normalize(l - r);
      float diff = max(0.f, dot(n, l));
      float spec = (0.4f + pow(max(0.f, dot(n, h)), 100.f));
      f32x3 remaining = f32x3(1) - col;
      col += remaining * spec * .2f;
      side = -side;
      d = .1f;
    }
    if (dd > 10.f) {
      break;
    }
    dd += d;
    p += r * d;
  }
  fragColor = f32x4(col, 1);
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
