// Port of https://www.shadertoy.com/view/mdfyz8; author: henrmota.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mdfyz8.json.
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

float random(Context &__ctx, f32x2 _st) {
  return fract(sin(dot(_st.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
float noise(Context &__ctx, f32x2 _st) {
  f32x2 i = floor(_st);
  f32x2 f = fract(_st);
  float a = random(__ctx, i);
  float b = random(__ctx, i + f32x2(1.0f, 0.0f));
  float c = random(__ctx, i + f32x2(0.0f, 1.0f));
  float d = random(__ctx, i + f32x2(1.0f, 1.0f));
  f32x2 u = f * f * (3.0f - 2.0f * f);
  return mix(a, b, u.data[0]) + (c - a) * u.data[1] * (1.0f - u.data[0]) +
         (d - b) * u.data[0] * u.data[1];
}
float fbm(Context &__ctx, f32x2 _st) {
  float v = 0.0f;
  float a = 0.5f;
  f32x2 shift = f32x2(100.0f);
  f32x2x2 rot = f32x2x2(cos(0.5f), sin(0.5f), -sin(0.5f), cos(0.50f));
  for (int i = 0; i < 5; ++i) {
    v += a * noise(__ctx, _st);
    _st = rot * _st * 2.0f + shift;
    a *= 0.5f;
  }
  return v;
}
f32x2x2 rot(Context &__ctx, float angle) {
  float c = cos(angle), s = sin(angle);
  return f32x2x2(c, -s, s, c);
}
float pingpong(Context &__ctx, float x, float a, float p) {
  return a / p * (p - abs(mod(x, (2.f * p)) - p));
}
float knittedPattern(Context &__ctx, f32x2 uv) {
  f32x2 scale = f32x2(1.5f, 1.f);
  f32x2 scaled = uv * scale;
  scaled.data[1] += pingpong(__ctx, scaled.data[0], 1.f, 1.f);
  f32x2 guv = fract(scaled) - 0.5f;
  float d = smoothstep(1.f, 0.f, length(guv));
  d -= .3f * smoothstep(.2f, 0.1f,
                        abs(cos((guv.data[0] - guv.data[1]) * 40.f +
                                cos((uv.data[0] + uv.data[1]) * 100.f)) *
                            0.5f));
  d = max(0.f, d);
  return d;
}
float sdCapsule(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h) - r;
}
float smin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
float arm(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float rotation) {
  p.data[0] = abs(p.data[0]);
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (rot(__ctx, rotation)));
  float radius =
      mix(.15f, 0.3f, smoothstep(a.data[0], b.data[0], abs(p.data[0])));
  radius *= mix(.9f, 1.f, smoothstep(0.f, .3f, abs(p.data[1] - 0.25f)));
  radius *= mix(1.f, .7f, smoothstep(2.f, 0.15f, p.data[1]));
  radius +=
      0.015f * knittedPattern(
                   __ctx, f32x2(atan(p.data[2], p.data[1]), p.data[0]) * 8.f) -
      0.05f;
  p.data[0] = abs(p.data[0]);
  return sdCapsule(__ctx, p, a, b, radius);
}
f32x2 map(Context &__ctx, f32x3 p) {
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (rot(__ctx, 0.7f * iTime)));
  float radius = 1.f;
  radius *= mix(.9f, 1.f, smoothstep(0.f, .3f, abs(p.data[1] - 0.25f)));
  radius *= mix(1.f, .7f, smoothstep(2.f, 0.15f, p.data[1]));
  radius +=
      0.025f * knittedPattern(
                   __ctx, f32x2(atan(p.data[2], p.data[0]), p.data[1]) * 8.f) -
      0.05f;
  float b =
      sdCapsule(__ctx, p, f32x3(0.f, 1.f, 0), f32x3(0.f, -1.f, 0.f), radius);
  float a = arm(__ctx, p, f32x3(0.5f, 0.f, 0.1f), f32x3(1.5f, 0.f, 0.2f), 0.3f);
  float d = smin(__ctx, b, a, 0.15f);
  float l = arm(__ctx, p, f32x3(.5f, -1.f, 0.f), f32x3(1.5f, -1.5f, 0.f), 0.5f);
  d = smin(__ctx, d, l, 0.2f);
  float bodyId = 1.f;
  f32x3 q = p;
  q.data[0] = abs(q.data[0]);
  float eyeD = length(q - f32x3(0.28f, 1.f, .73f)) - 0.2f;
  d = smin(__ctx, d, eyeD + 0.1f, 0.3f);
  d = min(d, eyeD);
  if (d == eyeD)
    bodyId = 2.f;
  q = p;
  q -= f32x3(0.f, 0.6f, .73f);
  q.data[1] -= abs(q.data[0]) * 2.f;
  float noseD = (length(q) - 0.2f) * 0.4f;
  d = smin(__ctx, d, noseD + 0.1f, 0.3f);
  d = min(d, noseD);
  if (d == noseD)
    bodyId = 3.f;
  return f32x2(d, bodyId);
}
f32x2 rayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float t = 0.f;
  float id = -1.f;
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + rd * t;
    f32x2 d = map(__ctx, p);
    t += d.data[0];
    id = d.data[1];
    if (d.data[0] < 0.01f || t > 100.f)
      break;
  }
  return f32x2(t, id);
}
f32x3 getNormal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(0.01f, 0.f);
  return normalize(f32x3(map(__ctx, p + e.swizzle<0, 1, 1>()).data[0] -
                             map(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                         map(__ctx, p + e.swizzle<1, 0, 1>()).data[0] -
                             map(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                         map(__ctx, p + e.swizzle<1, 1, 0>()).data[0] -
                             map(__ctx, p - e.swizzle<1, 1, 0>()).data[0]));
}
f32x3 getLight(Context &__ctx, f32x3 p, f32x3 n, f32x3 rd,
               float specularFactor) {
  f32x3 light = f32x3(1.f, 1.f, 1.f) * 5.f;
  f32x3 sky = f32x3(0.1f, 0.2f, 0.8f);
  f32x3 ground = f32x3(0.4f, 0.2f, 0.4f);
  f32x3 hemiMix = mix(ground, sky, smoothstep(-.5f, .5f, n.data[1]));
  f32x3 lightDir = normalize(f32x3(.5f, .5f, .5f));
  float diffusePower = max(0.f, dot(n, lightDir));
  f32x3 diffuse = diffusePower * light * 0.7f;
  f32x3 r = normalize(reflect(-lightDir, n));
  float specularPower = pow(max(0.f, dot(rd, r)), 16.f);
  f32x3 specular = specularPower * light * specularFactor;
  float fresnel = pow(1.f - max(0.f, dot(rd, n)), 10.f);
  return 0.5f * fresnel * light + diffuse + specular;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col = f32x3(0.f);
  f32x3 ro = f32x3(0.f, 0.f, 5.f);
  f32x3 rd = normalize(f32x3(uv, -1.f));
  f32x2 di = rayMarch(__ctx, ro, rd);
  float d = di.data[0];
  f32x3 p = ro + d * rd;
  if (d < 100.f) {
    f32x3 n = getNormal(__ctx, p);
    f32x3 q = p;
    q.set_swizzle<0, 2>(q.swizzle<0, 2>() * (rot(__ctx, 0.7f * iTime)));
    if (di.data[1] == 1.f) {
      col += f32x3(.3f, 0.2f, 0.2f) *
             getLight(__ctx, p, n, normalize(ro - p), 0.f);
      col -= 0.3f * noise(__ctx, q.swizzle<0, 1>() * 10.f) *
             noise(__ctx, q.swizzle<0, 1>() * 20.f);
    }
    if (di.data[1] == 2.f) {
      col += fbm(__ctx, q.swizzle<0, 2>() * 50.f - iTime) *
             f32x3(0.1f, 0.1f, 0.1f) *
             getLight(__ctx, p, n, normalize(ro - p), 3.f);
    }
    if (di.data[1] == 3.f) {
      col += f32x3(.3f, 0.05f, 0.05f) *
             getLight(__ctx, p, n, normalize(ro - p), 3.f);
    }
  } else {
    p = fract(rd * 10.f) - 0.5f;
    p.set_swizzle<0, 1>(p.swizzle<0, 1>() + (iTime * 0.1f));
    p.data[2] += 0.2f * iTime;
    f32x3 color = f32x3(
        dot(cos(p * 10.f), sin(p.swizzle<2, 0, 1>() * 15.f)) * 0.5f + 0.5f);
    col = (0.1f / abs(color) - .2f);
    col *= f32x3(1.f, 1.f, 1.f) * 0.1f * rd * rd;
    col *= 0.5f;
  }
  col = pow(col, f32x3(1.f / 2.2f));
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
