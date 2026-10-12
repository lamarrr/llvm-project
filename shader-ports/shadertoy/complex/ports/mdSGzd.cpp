// Port of https://www.shadertoy.com/view/mdSGzd; author: rubioh.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mdSGzd.json.
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
f32x2 hash22(Context &__ctx, f32x2 p) {
  f32x2 a = f32x2(94.86f, 43.46f);
  f32x2 b = f32x2(72.67f, 13.48f);
  p = f32x2(dot(p, a), dot(p, b));
  return fract(sin(p * 10.29f) * 48.47f);
}
f32x3 hash23(Context &__ctx, f32x2 p) {
  f32x2 a = f32x2(92.8f, 438.7f);
  f32x2 b = f32x2(73.6f, 12.8f);
  f32x2 c = f32x2(34.7f, 73.18f);
  f32x3 r = f32x3(dot(p, a), dot(p, b), dot(p, c));
  return fract(sin(r * 10.29f) * 48.47f);
}
f32x3 analytic_sphere(Context &__ctx, f32x2 p, float r) {
  float l = length(p);
  float h = r * cos(l * 3.14159f / 2.f / (r * 1.5f));
  p = normalize(p / r) * asin(length(p / r)) / 3.14159f;
  return f32x3(p, h);
}
f32x3 calcNormal(Context &__ctx, f32x2 p, float r) {
  f32x2 e = f32x2(1.0f, -1.0f) * 0.001f;
  return normalize(
      e.swizzle<0, 1, 0>() * analytic_sphere(__ctx, p + e.swizzle<0, 1>(), r) +
      e.swizzle<1, 0, 0>() * analytic_sphere(__ctx, p + e.swizzle<1, 0>(), r) +
      e.swizzle<0, 0, 0>() * analytic_sphere(__ctx, p + e.swizzle<0, 0>(), r));
}
f32x2 voro(Context &__ctx, f32x2 uv, float ntiles, f32x3 &normal,
           f32x3 &col_boule) {
  f32x2 uv_id = floor(uv * ntiles);
  f32x2 uv_st = fract(uv * ntiles);
  f32x2 m_neighbor, m_diff, m_neighbor_id;
  float m_dist_s, m_dist = 10.f;
  f32x2 idx = f32x2(0.f, 0.f);
  int K = 1;
  f32x2 point;
  for (int j = -K; j <= K; j++) {
    for (int i = -K; i <= K; i++) {
      f32x2 neighbor = f32x2(float(i), float(j));
      if (uv_id + neighbor == f32x2(0.f, 0.f)) {
        point = (iMouse.swizzle<0, 1>() - .5f * iResolution.swizzle<0, 1>()) /
                iResolution.data[1] * ntiles;
      } else {
        point = hash22(__ctx, uv_id + neighbor);
        point = float(K) / 2.f +
                float(K) / 2.f * sin(2.f * 3.14159f * point + iTime / 1.f);
      }
      f32x2 diff = neighbor + point - uv_st;
      float dist = length(diff);
      if (dist < m_dist) {
        m_dist = dist;
        m_dist_s = dist;
        m_diff = diff;
        m_neighbor = neighbor + point;
        m_neighbor_id = neighbor;
      }
    }
  }
  f32x2 neighbor_id = m_neighbor_id + uv_id;
  f32x2 neighbor_coord = m_neighbor + uv_id - neighbor_id;
  m_dist = 100.f;
  for (int j = -K - 1; j <= K + 1; j++) {
    for (int i = -K - 1; i <= K + 1; i++) {
      f32x2 neighbor = f32x2(float(i), float(j));
      if (neighbor_id + neighbor == f32x2(0.f, 0.f)) {
        point = (iMouse.swizzle<0, 1>() - .5f * iResolution.swizzle<0, 1>()) /
                iResolution.data[1] * ntiles;
      } else {
        point = hash22(__ctx, neighbor_id + neighbor);
        point = float(K) / 2.f +
                float(K) / 2.f * sin(2.f * 3.14159f * point + iTime / 1.f);
      }
      f32x2 new_neighbor = point + neighbor;
      f32x2 diff = new_neighbor - neighbor_coord;
      float dist = length(diff);
      if (dist < m_dist && length(neighbor_coord - new_neighbor) > .0001f)
        m_dist = dist;
    }
  }
  float neighbor_radius = m_dist / 2.f;
  float d = 1.f - smoothstep(-.02f, -0.f,
                             length(m_neighbor - uv_st) - neighbor_radius);
  float r = neighbor_radius;
  f32x2 p = m_neighbor - uv_st;
  float l = length(p);
  f32x3 sphere = analytic_sphere(__ctx, p, r);
  float h = r * cos(l * 3.14159f / 2.f / r);
  col_boule = hash23(__ctx, neighbor_id);
  normal = calcNormal(__ctx, p, r);
  return f32x2(d, h);
}
float smooth_damier(Context &__ctx, f32x2 uv) {
  float a = (.32035f * atan(sin(3.14159f * uv.data[0]) / 0.01f) + .5f);
  float b = (.32035f * atan(sin(3.14159f * uv.data[1]) / 0.01f) + .5f);
  return a * (1.f - b) + b * (1.f - a);
}
f32x3 phong(Context &__ctx, f32x3 lightDir, f32x3 normal, f32x3 rd, f32x3 col) {
  f32x3 ambient = col * (.5f + .5f * normal.data[2]);
  float dotLN = clamp(dot(lightDir, normal), 0.f, 1.f);
  f32x3 diffuse = col * dotLN;
  float dotRV = clamp(dot(reflect(lightDir, normal * .8f), -rd), 0.f, 1.f);
  f32x3 specular = 2.f * col * pow(dotRV, 12.f);
  return ambient * .4f + diffuse + specular;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 normal;
  f32x3 col_boule;
  f32x2 v = voro(__ctx, uv, 4.f, normal, col_boule);
  f32x3 rd = f32x3(uv, 2.f);
  f32x3 ld = f32x3(-.4f, -.4f, 2.f);
  f32x3 rr = refract(rd, normal, .1f);
  f32x3 ro = f32x3(uv, v.data[1]);
  float depth = -ro.data[2] / rr.data[2];
  f32x2 st = (v.data[0] > 0.06f) ? (ro + rr * depth).swizzle<0, 1>() : uv;
  f32x3 col = v.data[1] * 1.f *
              max(f32x3(0.f),
                  v.data[0] * phong(__ctx, ld, normal, rd, col_boule + .3f));
  float damier = smooth_damier(__ctx, st * 4.f);
  col += v.data[1] * 1.f *
         max(f32x3(0.f), v.data[0] * phong(__ctx, ld, normal, rd,
                                           f32x3(damier * .4f) * v.data[0]));
  float dotLN_damier =
      clamp(dot(ld, f32x3(0.f, 0.f, 1.f)), 0.f, 1.f) * (1.f - v.data[0]);
  float dotRV_damier =
      clamp(dot(reflect(ld, f32x3(0.f, 0.f, 1.f) * .8f), -rd), 0.f, 1.f) *
      (1.f - v.data[0]);
  col += .5f * damier * pow(dotRV_damier * .99f, 10.f) +
         .4f * damier * dotLN_damier;
  fragColor = f32x4(col * .8f, 1.0f);
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
