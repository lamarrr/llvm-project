// Port of https://www.shadertoy.com/view/fltcWM; author: kilianovski.
// Dataset license metadata: mit; original retained in upstream/fltcWM.json.
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
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
const int MAT_BALL = 0;
const int MAT_THREAD = 1;
const int MAT_UP_BASE = 2;
const int MAT_DOWN_BASE = 3;
const int N_PENDULUMS = 15;
const float FIRST_L = 15.f;
const float Tmax = 60.f;
const float ball_r = .5f;
const float base_y = 3.f;
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float pendulumPeriod(Context &__ctx, float L) {
  return 2.f * 3.141592f * sqrt(L / 9.81f);
}
float pendulumTheta(Context &__ctx, float t, float A, float iTime) {
  return A * cos((2.f * 3.141592f / iTime) * t);
}
float sdBox(Context &__ctx, f32x3 p, f32x3 s) {
  p = abs(p) - s;
  return length(max(p, 0.f)) +
         min(max(p.data[0], max(p.data[1], p.data[2])), 0.f);
}
float sdCapsule(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 ab = b - a, ap = p - a;
  float t = dot(ap, ab) / dot(ab, ab);
  t = clamp(t, 0.f, 1.f);
  f32x3 c = a + t * ab;
  float d = length(p - c) - r;
  return d;
}
f32x2 sdPend(Context &__ctx, f32x3 p, float l, float t) {
  p.data[1] -= (base_y - (l + ball_r));
  f32x3 base = f32x3(0, (l + ball_r), 0);
  float A = 3.141592f / 4.f;
  float iTime = pendulumPeriod(__ctx, l);
  float a = pendulumTheta(__ctx, t, A, iTime);
  p -= base;
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (Rot(__ctx, a)));
  p += base;
  float line = sdCapsule(__ctx, p, base, f32x3(0.f), .03f);
  float ball = length(p) - ball_r;
  float d = min(line, ball);
  int mat = d == line ? MAT_THREAD : MAT_BALL;
  return f32x2(d, mat);
}
float get_k(Context &__ctx) {
  float d = sqrt(FIRST_L / 9.81f) * 2.f * 3.141592f;
  float k = Tmax / d - 2.f;
  return k;
}
float getL(Context &__ctx, float Tmax, float k, float n) {
  float x = pow(Tmax / (2.f * 3.141592f * (k + n + 1.f)), 2.f);
  return 9.81f * x;
}
f32x2 Min(Context &__ctx, f32x2 a, f32x2 b) {
  return a.data[0] > b.data[0] ? b : a;
}
f32x2 GetDist(Context &__ctx, f32x3 p) {
  float ball_horizontal_offset = 3.f * ball_r;
  float base_length = ball_horizontal_offset * float(N_PENDULUMS - 1);
  float base = sdCapsule(__ctx, p - f32x3(0.f, base_y + .5f, 0.f), f32x3(0.f),
                         f32x3(0, 0, base_length), .5f);
  f32x2 d = f32x2(base, MAT_UP_BASE);
  float t = iTime * 1.5f;
  t -= 3.f;
  t = max(t, 0.f);
  float k = get_k(__ctx);
  for (int i = 0; i < N_PENDULUMS; i++) {
    float L = getL(__ctx, Tmax, k, float(i + 1));
    f32x3 pendulum_offset = f32x3(0.f, 0.f, ball_horizontal_offset * float(i));
    f32x2 p_dist = sdPend(__ctx, p - pendulum_offset, L, t);
    d = Min(__ctx, d, p_dist);
  }
  float downbase = sdBox(__ctx, p + f32x3(0.f, FIRST_L * 1.f, -base_length),
                         f32x3(10.f, .1f, base_length * 1.5f));
  d = Min(__ctx, d, f32x2(downbase, MAT_DOWN_BASE));
  return d;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.f;
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + rd * dO;
    float dS = GetDist(__ctx, p).data[0];
    dO += dS;
    if (dO > 100.f || abs(dS) < .001f)
      break;
  }
  return dO;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = GetDist(__ctx, p).data[0] -
            f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                  GetDist(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                  GetDist(__ctx, p - e.swizzle<1, 1, 0>()).data[0]);
  return normalize(n);
}
f32x3 GetRayDir(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u;
  return normalize(i);
}
f32x3 Render(Context &__ctx, f32x3 &ro, f32x3 &rd) {
  f32x3 col = f32x3(0);
  float d = RayMarch(__ctx, ro, rd);
  f32x3 background = texture(__ctx.channel0, rd).swizzle<0, 1, 2>();
  if (d < 100.f) {
    f32x3 p = ro + rd * d;
    f32x3 n = GetNormal(__ctx, p);
    int mat = int(GetDist(__ctx, p).data[1]);
    float dif = dot(n, normalize(f32x3(1, 2, 3))) * .5f + .5f;
    f32x3 r = reflect(rd, n);
    f32x3 ref = texture(__ctx.channel0, r).swizzle<0, 1, 2>();
    if (mat == MAT_UP_BASE)
      col = ref * .05f;
    else if (mat == MAT_BALL)
      col = ref * .95f;
    else if (mat == MAT_THREAD)
      col = f32x3(.4f);
    else if (mat == MAT_DOWN_BASE) {
      col = f32x3(.8f, .6f, .3f);
      col = f32x3(0.f);
      rd = r;
      ro = p + .001f * 2.f;
    } else
      col = f32x3(p);
  } else
    col = background * .3f;
  col = pow(col, f32x3(.4545f));
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 ro = f32x3(0, 0, -15);
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() *
                       (Rot(__ctx, -m.data[1] * 3.141592f + 1.f)));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() *
                       (Rot(__ctx, -m.data[0] * 6.283185f)));
  f32x3 rd = GetRayDir(__ctx, uv, ro, f32x3(0, 0.f, 0), 1.f);
  f32x3 col = f32x3(0.f);
  for (int i = 0; i < 2; i++) {
    col += Render(__ctx, ro, rd);
  }
  col /= float(2);
  fragColor = f32x4(col, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_cube<f32x4> channel0,
               __binding(1, 0) sampler sampling0) {
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
  Context __ctx{{channel0, sampling0}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
