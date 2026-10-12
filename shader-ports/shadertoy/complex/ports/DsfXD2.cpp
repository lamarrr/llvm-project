// Port of https://www.shadertoy.com/view/DsfXD2; author: chrisjj.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DsfXD2.json.
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

float random(Context &__ctx, f32x2 st) {
  return fract(sin(dot(st.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
f32x2 random2(Context &__ctx, f32x2 st) {
  st = f32x2(dot(st, f32x2(127.1f, 311.7f)), dot(st, f32x2(269.5f, 183.3f)));
  return fract(abs(sin(st) * 43758.5453123f));
}
float noise(Context &__ctx, f32x2 st) {
  f32x2 i = floor(st);
  f32x2 f = fract(st);
  float a = random(__ctx, i);
  float b = random(__ctx, i + f32x2(1.0f, 0.0f));
  float c = random(__ctx, i + f32x2(0.0f, 1.0f));
  float d = random(__ctx, i + f32x2(1.0f, 1.0f));
  f32x2 u = f * f * (3.0f - 2.0f * f);
  return mix(a, b, u.data[0]) + (c - a) * u.data[1] * (1.0f - u.data[0]) +
         (d - b) * u.data[0] * u.data[1];
}
float fbm(Context &__ctx, f32x2 st) {
  float value = 0.0f;
  float amplitude = .6f;
  float frequency = 0.f;
  for (int i = 0; i < 4; i++) {
    value += amplitude * noise(__ctx, st);
    st *= 2.f;
    amplitude *= .5f;
  }
  return value;
}
float dist(Context &__ctx, f32x3 a, f32x3 b) {
  return sqrt(pow(a.data[0] - b.data[0], 2.f) +
              pow(a.data[1] - b.data[1], 2.f) +
              pow(a.data[2] - b.data[2], 2.f));
}
f32x3 sum(Context &__ctx, f32x3 a, f32x3 b) {
  return f32x3(a.data[0] + b.data[0], a.data[1] + b.data[1],
               a.data[2] + b.data[2]);
}
f32x3 sub(Context &__ctx, f32x3 a, f32x3 b) {
  return f32x3(a.data[0] - b.data[0], a.data[1] - b.data[1],
               a.data[2] - b.data[2]);
}
f32x3 mul(Context &__ctx, f32x3 a, float value) {
  return f32x3(a.data[0] * value, a.data[1] * value, a.data[2] * value);
}
float smin(Context &__ctx, float a, float b, float k) {
  float m = max(min(0.5f + 0.5f * (b - a) / k, 1.f), 0.f);
  return a * m + b * (1.f - m) - k * m * (1.f - m);
}
float getDistdotsentra(Context &__ctx, f32x3 p) {
  f32x3 sphere = f32x3(0.f, 0.f, 0.f);
  float sphereDist = dist(__ctx, p, sphere) - 1.5f;
  float d = sphereDist;
  return d;
}
f32x3 normalforlit(Context &__ctx, f32x3 p) {
  float d = getDistdotsentra(__ctx, p);
  float p1 = getDistdotsentra(__ctx, sub(__ctx, p, f32x3(0.001f, 0.f, 0.f)));
  float p2 = getDistdotsentra(__ctx, sub(__ctx, p, f32x3(0.f, 0.001f, 0.f)));
  float p3 = getDistdotsentra(__ctx, sub(__ctx, p, f32x3(0.f, 0.f, 0.001f)));
  f32x3 tri = f32x3(p1, p2, p3);
  f32x3 n = sub(__ctx, f32x3(d, d, d), tri);
  return normalize(n);
}
f32x2 getDist(Context &__ctx, f32x3 p) {
  f32x3 sphere = f32x3(0.f, 0.f, 0.f);
  f32x3 n = normalforlit(__ctx, p);
  f32x2 st = p.swizzle<0, 1>();
  st.data[1] += iTime * 0.1f;
  float mashtab = 15.f;
  st *= mashtab;
  f32x2 i_st = floor(st);
  f32x2 f_st = fract(st);
  float m_dist = 100.0f;
  for (int y = -1; y <= 1; y++) {
    for (int x = -1; x <= 1; x++) {
      f32x2 neighbor = f32x2(float(x), float(y));
      f32x2 point = random2(__ctx, i_st + neighbor);
      point = 0.5f + 0.5f * sin(iTime + 6.2831f * point);
      f32x2 diff = neighbor + point - f_st;
      float dist = length(diff) + 1.f * sin(point.data[0]);
      m_dist = min(m_dist, dist);
    }
  }
  float sphereDist = dist(__ctx, p, sphere) - m_dist * 0.1f - 1.3f;
  float fraktal_noise =
      dist(__ctx, p, sphere) -
      fbm(__ctx, f32x2(n.data[0] * 4.f + iTime * 0.7f, n.data[1] * 4.f)) *
          0.7f -
      1.f;
  float d = smin(__ctx, sphereDist, fraktal_noise, 0.1f);
  float t = 0.f;
  if (sphereDist > fraktal_noise)
    t = 1.f;
  return f32x2(d, t);
}
f32x3 normal(Context &__ctx, f32x3 p) {
  float d = getDist(__ctx, p).data[0];
  float p1 = getDist(__ctx, sub(__ctx, p, f32x3(0.001f, 0.f, 0.f))).data[0];
  float p2 = getDist(__ctx, sub(__ctx, p, f32x3(0.f, 0.001f, 0.f))).data[0];
  float p3 = getDist(__ctx, sub(__ctx, p, f32x3(0.f, 0.f, 0.001f))).data[0];
  f32x3 tri = f32x3(p1, p2, p3);
  f32x3 n = sub(__ctx, f32x3(d, d, d), tri);
  return normalize(n);
}
float light(Context &__ctx, f32x3 p) {
  f32x3 lightPos = f32x3(sin(iTime) * 3.f, 1.f, cos(iTime) * 3.f);
  f32x3 lightDir = normalize(sub(__ctx, lightPos, p));
  f32x3 n = normalforlit(__ctx, p);
  return dot(n, lightDir) * 0.5f + 0.5f;
}
f32x3 rayMarching(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x3 p = ro;
  for (int i = -0; i < 500; i++) {
    float d = getDist(__ctx, p).data[0];
    float t = getDist(__ctx, p).data[1];
    if (d > 10.f) {
      break;
    }
    p = sum(__ctx, p, mul(__ctx, rd, d));
    if (d < 0.0001f) {
      float l = light(__ctx, p);
      float n = 2.f;
      return f32x3(pow(l * t, 5.f), pow(l, n), pow(l * (1.f - t), n));
    }
  }
  return f32x3(0.f, 0.f, 0.f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  const int iteration = 500;
  const float max_dist = 10.0f;
  f32x3 ro = f32x3(0.f, 0.f, -2.5f);
  float x = (fragCoord.data[0] / iResolution.data[0]) * 2.f - 1.f;
  float y = (fragCoord.data[1] / iResolution.data[1]) * 2.f - 1.f;
  x *= iResolution.data[0] / iResolution.data[1];
  f32x3 rd = f32x3(x, y, 1.f);
  rd = normalize(rd);
  f32x3 color = rayMarching(__ctx, ro, rd);
  fragColor = f32x4(color, 1.0f);
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
