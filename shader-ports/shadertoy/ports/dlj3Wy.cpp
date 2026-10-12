// Port of https://www.shadertoy.com/view/dlj3Wy; author: cinmin.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dlj3Wy.json.
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
struct Context {
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

f32x3 hand = f32x3(0, 1.3f, 3.4f);
f32x3 hand2 = f32x3(0, 1.3f, 3.4f);
f32x3 normalMap(Context &__ctx, f32x3 p, f32x3 n);
float calcAO(Context &__ctx, f32x3 pos, f32x3 nor);
float noise_3(Context &__ctx, f32x3 p);
f32x3 FlameColour(Context &__ctx, float f);
float bnoise(Context &__ctx, float x);
f32x2 SphereMap(Context &__ctx, f32x3 ray) {
  f32x2 st;
  ray = normalize(ray);
  float radius = length(ray);
  st.data[1] = acos(ray.data[1] / radius) / 3.141592654f;
  if (ray.data[2] >= 0.0f)
    st.data[0] =
        acos(ray.data[0] / (radius * sin(3.141592654f * (st.data[1])))) /
        6.283185308f;
  else
    st.data[0] =
        1.0f - acos(ray.data[0] / (radius * sin(3.141592654f * (st.data[1])))) /
                   6.283185308f;
  return st;
}
f32x4 warpcolor(Context &__ctx, f32x2 uv, float t) {
  float strength = 0.4f;
  f32x3 col = f32x3(0);
  f32x2 pos = uv * 2.0f - 1.0f;
  for (int i = 1; i < 6; i++) {
    pos.data[0] +=
        strength * sin(2.0f * t + float(i) * 1.5f * pos.data[1]) + t * 0.5f;
    pos.data[1] += strength * cos(2.0f * t + float(i) * 1.5f * pos.data[0]);
  }
  col += 0.5f + 0.5f * cos(t + pos.swizzle<0, 1, 0>() + f32x3(0, 2, 4));
  col = pow(col, f32x3(0.4545f));
  return f32x4(col, 1.0f);
}
f32x3 warpSky(Context &__ctx, f32x3 e) {
  f32x2 ST = SphereMap(__ctx, e);
  f32x4 color = warpcolor(__ctx, ST, iTime * 0.1f);
  return color.swizzle<0, 1, 2>();
}
f32x3 phong(Context &__ctx, f32x3 p, f32x3 n, f32x3 v) {
  f32x3 final = f32x3(0.0f);
  f32x3 ambient_color = f32x3(0.351f, 0.900f, 0.955f);
  f32x2 mouse =
      (iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>()) * 2.0f - 1.0f;
  {
    f32x3 light_pos = f32x3(mouse.data[0], mouse.data[1], 2.0f);
    f32x3 light_color = f32x3(0.245f, 0.194f, 0.241f);
    f32x3 light_vector = normalize(light_pos - p);
    f32x3 reflection = normalize(reflect(-light_pos, n));
    float ka = 0.620f, ks = 1.5f, kd = 2.f;
    float shineness = 5.160f;
    f32x3 diffuse, specular, ambient;
    ambient = ka * ambient_color;
    diffuse = kd * dot(light_vector, n) * light_color;
    specular = ks * pow(max(dot(reflection, v), 0.f), shineness) * light_color;
    final = ambient_color + diffuse + specular;
  }
  {
    f32x3 light_pos = f32x3(-5.f, 15.f, 2.0f);
    f32x3 light_color = f32x3(0.920f, 0.266f, 0.258f);
    f32x3 light_vector = normalize(light_pos - p);
    f32x3 reflection = normalize(reflect(-light_pos, n));
    float ka = 0.320f, ks = 0.8f, kd = 0.8f;
    float shineness = 2.568f;
    f32x3 diffuse, specular, ambient;
    ambient = ka * ambient_color;
    diffuse = kd * dot(light_vector, n) * light_color;
    specular = ks * pow(max(dot(reflection, v), 0.f), shineness) * light_color;
    final += (diffuse + specular) * 0.6f;
  }
  f32x3 refl = reflect(-v, n);
  f32x3 refl_clr = warpSky(__ctx, refl);
  float F = 1.0f - dot(n, v);
  final = mix(final, refl_clr, F);
  return final;
}
float sdSphere(Context &__ctx, f32x3 p, float s) { return length(p) - s; }
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 d = abs(p) - b;
  return min(max(d.data[0], max(d.data[1], d.data[2])), 0.0f) +
         length(max(d, 0.0f));
}
float sdTorus(Context &__ctx, f32x3 p, f32x2 t) {
  f32x2 q = f32x2(length(p.swizzle<0, 1>()) - t.data[0], p.data[2]);
  return length(q) - t.data[1];
}
float udRoundBox(Context &__ctx, f32x3 p, f32x3 b, float r) {
  return length(max(abs(p) - b, 0.0f)) - r;
}
float map(Context &__ctx, f32x3 p) {
  float bump = -0.288f * (noise_3(__ctx, p * 3.0f) * 2.0f - 1.0f);
  float bump_h1 =
      -0.008f * hand.data[2] * (noise_3(__ctx, p * 3.0f) * 2.0f - 1.0f);
  float bump_h2 =
      -0.008f * hand2.data[2] * (noise_3(__ctx, p * 3.0f) * 2.0f - 1.0f);
  f32x3 p1 = p;
  float t1 = sdTorus(__ctx, p1 + hand + bump_h1, f32x2(0.05f, 0.025f));
  float t2 = sdTorus(__ctx, p1 + hand2 + bump_h2, f32x2(0.05f, 0.025f));
  return min(t1, t2);
}
f32x3 gradient(Context &__ctx, f32x3 p) {
  const float d = 0.001f;
  f32x3 grad =
      f32x3(map(__ctx, p + f32x3(d, 0, 0)) - map(__ctx, p - f32x3(d, 0, 0)),
            map(__ctx, p + f32x3(0, d, 0)) - map(__ctx, p - f32x3(0, d, 0)),
            map(__ctx, p + f32x3(0, 0, d)) - map(__ctx, p - f32x3(0, 0, d)));
  return grad;
}
float trace(Context &__ctx, f32x3 o, f32x3 r, f32x3 &p) {
  float d = 0.0f, t = 0.0f;
  for (int i = 0; i < 15; ++i) {
    p = o + r * t;
    d = map(__ctx, p);
    if (d < 0.0f)
      break;
    t += d * 0.3f;
  }
  return t;
}
float fbm(Context &__ctx, f32x2 uv);
f32x3 getSkyFBM(Context &__ctx, f32x3 e) {
  f32x3 f = e;
  float m = 2.0f * sqrt(f.data[0] * f.data[0] + f.data[1] * f.data[1] +
                        f.data[2] * f.data[2]);
  f32x2 st = f32x2(-f.data[0] / m + .5f, -f.data[1] / m + .5f);
  float fog =
      fbm(__ctx, 0.6f * st + f32x2(-0.2f * iTime, -0.02f * iTime)) * 0.5f +
      0.3f;
  return f32x3(fog);
}
f32x3 sky_color(Context &__ctx, f32x3 e) {
  e.data[1] = max(e.data[1], 0.0f);
  f32x3 ret;
  ret = FlameColour(__ctx, e.data[1]);
  return ret;
}
f32x3 getSkyALL(Context &__ctx, f32x3 e) { return sky_color(__ctx, e); }
f32x3x3 setCamera(Context &__ctx, f32x3 ro, f32x3 ta, float cr) {
  f32x3 cw = normalize(ta - ro);
  f32x3 cp = f32x3(sin(cr), cos(cr), 0.0f);
  f32x3 cu = normalize(cross(cw, cp));
  f32x3 cv = normalize(cross(cu, cw));
  return f32x3x3(cu, cv, cw);
}
f32x3x3 fromEuler(Context &__ctx, f32x3 ang) {
  f32x2 a1 = f32x2(sin(ang.data[0]), cos(ang.data[0]));
  f32x2 a2 = f32x2(sin(ang.data[1]), cos(ang.data[1]));
  f32x2 a3 = f32x2(sin(ang.data[2]), cos(ang.data[2]));
  f32x3 m0 =
      f32x3(a1.data[1] * a3.data[1] + a1.data[0] * a2.data[0] * a3.data[0],
            a1.data[1] * a2.data[0] * a3.data[0] + a3.data[1] * a1.data[0],
            -a2.data[1] * a3.data[0]);
  f32x3 m1 =
      f32x3(-a2.data[1] * a1.data[0], a1.data[1] * a2.data[1], a2.data[0]);
  f32x3 m2 =
      f32x3(a3.data[1] * a1.data[0] * a2.data[0] + a1.data[1] * a3.data[0],
            a1.data[0] * a3.data[0] - a1.data[1] * a3.data[1] * a2.data[0],
            a2.data[1] * a3.data[1]);
  return f32x3x3(m0, m1, m2);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  hand = f32x3(bnoise(__ctx, iTime / 8.f + 100.f) * 2.5f,
               (bnoise(__ctx, iTime / 5.f + 15.f)) * 0.9f,
               (bnoise(__ctx, iTime / 6.f) + 1.f) / 2.f * 7.f);
  hand2 = f32x3(bnoise(__ctx, iTime / 9.f + 50.f) * 2.f,
                (bnoise(__ctx, iTime / 6.f + 35.f)) * 0.9f,
                (bnoise(__ctx, iTime / 4.f + 60.f) + 1.f) / 2.f * 6.5f);
  f32x2 uv = gl_FragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  uv = uv * 2.0f - 1.0f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  uv.data[1] *= 1.0f;
  f32x2 mouse =
      (iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>()) * 2.0f - 1.0f;
  f32x3 CameraRot = f32x3(0.0f, 0.f, 0.f);
  f32x3 ro = f32x3(0.0f, 0.0f, 2.0f) * fromEuler(__ctx, CameraRot);
  f32x3 ta = f32x3(0.0f, 0.0f, 0.0f);
  f32x3x3 ca = setCamera(__ctx, ro, ta, 0.0f);
  f32x3 RayDir = ca * normalize(f32x3(uv, 2.0f));
  f32x3 RayOri = ro;
  f32x3 p, n;
  float t = trace(__ctx, RayOri, RayDir, p);
  n = normalize(gradient(__ctx, p));
  f32x3 bump = normalMap(__ctx, p * 1.652f, n);
  n = n + bump * 0.05f;
  float edge = dot(-RayDir, n);
  edge = smoothstep(-0.072f, 0.956f, edge);
  f32x3 result = n;
  f32x3 ao = f32x3(calcAO(__ctx, p, n));
  result = phong(__ctx, p, n, -RayDir) * ao;
  f32x3 refl = reflect(RayDir, n);
  f32x3 refl_clr = warpSky(__ctx, refl);
  f32x3 BG = refl_clr;
  if (t < 2.5f)
    fragColor = f32x4(f32x3(result), 1.0f);
  else
    fragColor = f32x4(BG, 1.0f);
}
float bnoise(Context &__ctx, float x) {
  float i = floor(x);
  float f = fract(x);
  float s = sign(fract(x / 2.0f) - 0.5f);
  float k = fract(i * .1731f);
  return s * f * (f - 1.0f) * ((16.0f * k - 4.0f) * f * (f - 1.0f) - 1.0f);
}
float hash11(Context &__ctx, float p) {
  return fract(sin(p * 727.1f) * 43758.5453123f);
}
float hash12(Context &__ctx, f32x2 p) {
  float h = dot(p, f32x2(127.1f, 311.7f));
  return fract(sin(h) * 43758.5453123f);
}
f32x3 hash31(Context &__ctx, float p) {
  f32x3 h = f32x3(1275.231f, 4461.7f, 7182.423f) * p;
  return fract(sin(h) * 43758.543123f);
}
float noise_3(Context &__ctx, f32x3 p) {
  f32x3 i = floor(p);
  f32x3 f = fract(p);
  f32x3 u = f * f * (3.0f - 2.0f * f);
  f32x2 ii = i.swizzle<0, 1>() + i.data[2] * f32x2(5.0f);
  float a = hash12(__ctx, ii + f32x2(0.0f, 0.0f));
  float b = hash12(__ctx, ii + f32x2(1.0f, 0.0f));
  float c = hash12(__ctx, ii + f32x2(0.0f, 1.0f));
  float d = hash12(__ctx, ii + f32x2(1.0f, 1.0f));
  float v1 = mix(mix(a, b, u.data[0]), mix(c, d, u.data[0]), u.data[1]);
  ii += f32x2(5.0f);
  a = hash12(__ctx, ii + f32x2(0.0f, 0.0f));
  b = hash12(__ctx, ii + f32x2(1.0f, 0.0f));
  c = hash12(__ctx, ii + f32x2(0.0f, 1.0f));
  d = hash12(__ctx, ii + f32x2(1.0f, 1.0f));
  float v2 = mix(mix(a, b, u.data[0]), mix(c, d, u.data[0]), u.data[1]);
  return max(mix(v1, v2, u.data[2]), 0.0f);
}
float glow(Context &__ctx, float d, float str, float thickness) {
  return thickness / pow(d, str);
}
f32x2 hash2(Context &__ctx, f32x2 x) {
  const f32x2 k = f32x2(0.3183099f, 0.3678794f);
  x = x * k + k.swizzle<1, 0>();
  return -1.0f +
         2.0f * fract(16.0f * k *
                      fract(x.data[0] * x.data[1] * (x.data[0] + x.data[1])));
}
float gnoise(Context &__ctx, f32x2 p) {
  f32x2 i = floor(p);
  f32x2 f = fract(p);
  f32x2 u = f * f * (3.0f - 2.0f * f);
  return mix(
      mix(dot(hash2(__ctx, i + f32x2(0.0f, 0.0f)), f - f32x2(0.0f, 0.0f)),
          dot(hash2(__ctx, i + f32x2(1.0f, 0.0f)), f - f32x2(1.0f, 0.0f)),
          u.data[0]),
      mix(dot(hash2(__ctx, i + f32x2(0.0f, 1.0f)), f - f32x2(0.0f, 1.0f)),
          dot(hash2(__ctx, i + f32x2(1.0f, 1.0f)), f - f32x2(1.0f, 1.0f)),
          u.data[0]),
      u.data[1]);
}
float fbm(Context &__ctx, f32x2 uv) {
  float f;
  f32x2x2 m = f32x2x2(1.6f, 1.2f, -1.2f, 1.6f);
  f = 0.5000f * gnoise(__ctx, uv);
  uv = m * uv;
  f += 0.2500f * gnoise(__ctx, uv);
  uv = m * uv;
  f += 0.1250f * gnoise(__ctx, uv);
  uv = m * uv;
  f += 0.0625f * gnoise(__ctx, uv);
  uv = m * uv;
  return f;
}
f32x3 smoothSampling2(Context &__ctx, f32x2 uv) {
  const float T_RES = 32.0f;
  return f32x3(gnoise(__ctx, uv * T_RES));
}
float triplanarSampling(Context &__ctx, f32x3 p, f32x3 n) {
  float fTotal = abs(n.data[0]) + abs(n.data[1]) + abs(n.data[2]);
  return (abs(n.data[0]) * smoothSampling2(__ctx, p.swizzle<1, 2>()).data[0] +
          abs(n.data[1]) * smoothSampling2(__ctx, p.swizzle<0, 2>()).data[0] +
          abs(n.data[2]) * smoothSampling2(__ctx, p.swizzle<0, 1>()).data[0]) /
         fTotal;
}
const f32x2x2 m2 = f32x2x2(0.90f, 0.44f, -0.44f, 0.90f);
float triplanarNoise(Context &__ctx, f32x3 p, f32x3 n) {
  const float BUMP_MAP_UV_SCALE = 0.2f;
  float fTotal = abs(n.data[0]) + abs(n.data[1]) + abs(n.data[2]);
  float f1 = triplanarSampling(__ctx, p * BUMP_MAP_UV_SCALE, n);
  p.set_swizzle<0, 1>(m2 * p.swizzle<0, 1>());
  p.set_swizzle<0, 2>(m2 * p.swizzle<0, 2>());
  p *= 2.1f;
  float f2 = triplanarSampling(__ctx, p * BUMP_MAP_UV_SCALE, n);
  p.set_swizzle<1, 0>(m2 * p.swizzle<1, 0>());
  p.set_swizzle<1, 2>(m2 * p.swizzle<1, 2>());
  p *= 2.3f;
  float f3 = triplanarSampling(__ctx, p * BUMP_MAP_UV_SCALE, n);
  return f1 + 0.5f * f2 + 0.25f * f3;
}
f32x3 normalMap(Context &__ctx, f32x3 p, f32x3 n) {
  float d = 0.005f;
  float po = triplanarNoise(__ctx, p, n);
  float px = triplanarNoise(__ctx, p + f32x3(d, 0, 0), n);
  float py = triplanarNoise(__ctx, p + f32x3(0, d, 0), n);
  float pz = triplanarNoise(__ctx, p + f32x3(0, 0, d), n);
  return normalize(f32x3((px - po) / d, (py - po) / d, (pz - po) / d));
}
float calcAO(Context &__ctx, f32x3 pos, f32x3 nor) {
  float ao = 0.0f;
  f32x3 v = normalize(f32x3(0.7f, -0.1f, 0.1f));
  for (int i = 0; i < 12; i++) {
    float h = abs(sin(float(i)));
    f32x3 kv = v + 2.0f * nor * max(0.0f, -dot(nor, v));
    ao += clamp(map(__ctx, pos + nor * 0.01f + kv * h * 0.08f) * 3.0f, 0.0f,
                1.0f);
    v = v.swizzle<1, 2, 0>();
  }
  ao /= 12.0f;
  ao = ao + 2.0f * ao * ao;
  return clamp(ao * 5.0f, 0.0f, 1.0f);
}
f32x3 hsv2rgb_smooth(Context &__ctx, f32x3 c) {
  f32x3 rgb = clamp(
      abs(mod(c.data[0] * 6.0f + f32x3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f,
      0.0f, 1.0f);
  rgb = rgb * rgb * (3.0f - 2.0f * rgb);
  return c.data[2] * mix(f32x3(1.0f), rgb, c.data[1]);
}
f32x3 hsv2rgb_trigonometric(Context &__ctx, f32x3 c) {
  f32x3 rgb = 0.5f + 0.5f * cos((c.data[0] * 6.0f + f32x3(0.0f, 4.0f, 2.0f)) *
                                3.14159f / 3.0f);
  return c.data[2] * mix(f32x3(1.0f), rgb, c.data[1]);
}
f32x3 FlameColour(Context &__ctx, float f) {
  return hsv2rgb_smooth(__ctx, f32x3((f - (2.25f / 6.f)) * (1.25f / 6.f),
                                     f * 1.25f + .2f, f * .95f));
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
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
