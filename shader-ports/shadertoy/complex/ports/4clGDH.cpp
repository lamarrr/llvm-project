// Port of https://www.shadertoy.com/view/4clGDH; author: spqr.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/4clGDH.json.
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

f32x3 r;
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
float fractal(Context &__ctx, f32x3 p) {
  float d = 0.5f;
  float f = 0.0f;
  for (int i = 0; i < 5; ++i) {
    f += noise(__ctx, p / d) * d;
    d *= .5f;
  }
  return f;
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
  f32x3 q = abs(p) - s;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
float hash(Context &__ctx, f32x2 p) {
  float h = dot(p, f32x2(127.1f, 311.7f));
  return -1.0f + 2.0f * fract(sin(h) * 43758.5453123f);
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
float aabb(Context &__ctx, f32x3 p, f32x3 d) {
  f32x3 neg = -(d / 2.f - p) / r;
  f32x3 pos = (d / 2.f - p) / r;
  f32x3 bot = min(neg, pos);
  float top = max(max(bot.data[0], bot.data[1]), bot.data[2]);
  return max(0.0f, top);
}
float field(Context &__ctx, f32x3 p, float scale) {
  p *= scale;
  f32x3 id = floor(p) - .5f;
  p = fract(p) - .5f;
  float f1 = hash31(__ctx, id);
  float shape = box(__ctx, p, f32x3(f1 * .46f));
  float bound = aabb(__ctx, p, f32x3(scale * 1.01f));
  return min(bound, shape) / scale;
}
float hash(Context &__ctx, float n) {
  return fract(sin(n * 3553.2352f) * 452.322f);
}
float map(Context &__ctx, f32x3 p) {
  f32x3 q = p;
  p = mod(p, 20.f) - 10.f;
  float pos = box(__ctx, p, f32x3(5));
  for (float i = 1.f; i < 10.f; i++) {
    pos = max(pos, -field(__ctx, q, pow(1.3f, i) / 16.f));
  }
  return pos;
}
f32x3 norm(Context &__ctx, f32x3 p) {
  f32x2 off = f32x2(0.01f, 0.0f);
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
  float tt = iTime * .3f;
  f32x3 s = f32x3(0);
  f32x3 arm = f32x3(0, 0, 1);
  arm.set_swizzle<0, 2>(arm.swizzle<0, 2>() *
                        (rot(__ctx, cos(tick(__ctx, tt)))));
  arm.set_swizzle<2, 1>(arm.swizzle<2, 1>() *
                        (rot(__ctx, cos(tick(__ctx, tt * .6f)))));
  f32x3 fwd = lattice(__ctx, tick(__ctx, tt)) * 400.f;
  s += fwd;
  f32x3 t = s + arm;
  f32x3 cz = normalize(t - s);
  f32x3 cx = normalize(cross(cz, f32x3(0, 1, 0)));
  f32x3 cy = -normalize(cross(cz, cx));
  r = normalize(cx * uv.data[0] + cy * uv.data[1] + cz);
  f32x3 p = s;
  float i = 0.f;
  float dd = 0.f;
  float d;
  bool hit = false;
  for (float z = 0.f; z < 200.f; ++z) {
    i = z;
    d = abs(map(__ctx, p) * .7f);
    if (d < 0.001f) {
      hit = true;
      break;
    }
    if (dd > 500.f)
      break;
    dd += d;
    p += r * d;
  }
  f32x3 n = norm(__ctx, p);
  f32x3 light = normalize(f32x3(-1, 2, 3));
  float diff = dot(n, light) * .5f + .5f;
  f32x3 col = f32x3(0);
  col += diff;
  if (!hit) {
    col = f32x3(1) - pow(r.data[1], .2f);
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
