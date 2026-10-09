// Port of https://www.shadertoy.com/view/7dsfDl; author: jorge2017a2.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7dsfDl.json.
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
struct Ray {
  f32x3 ro;
  f32x3 rd;
};
float sdSphere(Context &__ctx, f32x3 p, float s) { return length(p) - s; }
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 d = abs(p) - b;
  return length(max(d, 0.0f)) +
         min(max(d.data[0], max(d.data[1], d.data[2])), 0.0f);
}
f32x3 rotate_x(Context &__ctx, f32x3 p, float phi) {
  float c = cos(phi);
  float s = sin(phi);
  return f32x3(p.data[0], c * p.data[1] - s * p.data[2],
               s * p.data[1] + c * p.data[2]);
}
f32x3 rotate_y(Context &__ctx, f32x3 p, float phi) {
  float c = cos(phi);
  float s = sin(phi);
  return f32x3(c * p.data[0] + s * p.data[2], p.data[1],
               c * p.data[2] - s * p.data[0]);
}
f32x3 rotate_z(Context &__ctx, f32x3 p, float phi) {
  float c = cos(phi);
  float s = sin(phi);
  return f32x3(c * p.data[0] - s * p.data[1], s * p.data[0] + c * p.data[1],
               p.data[2]);
}
f32x2 GetDist(Context &__ctx, f32x3 p) {
  f32x2 res = f32x2(9999.0f, -1.0f);
  f32x3 p0 = p;
  float planeDist1 = p.data[1] - 1.0f;
  res =
      (res.data[0] < f32x2(planeDist1, 1.0f).data[0] ? res
                                                     : f32x2(planeDist1, 1.0f));
  float sds1 = sdSphere(__ctx, p - f32x3(0.0f, 5.0f, 0.0f), 5.0f);
  float d2 =
      sdBox(__ctx, p - f32x3(-10.0f, 5.0f, -5.0f), f32x3(3.0f, 10.0f, 2.0f));
  float d3 =
      sdBox(__ctx, p - f32x3(10.0f, 15.0f, 5.0f), f32x3(3.0f, 15.0f, 5.0f));
  float d4 = sdSphere(__ctx, p - f32x3(0.0f, 14.0f, 0.0f), 3.0f);
  float d5 = sdBox(__ctx, p - f32x3(-10.0f, 3.5f, 10.0f), f32x3(2.0f));
  res = (res.data[0] < f32x2(sds1, 2.0f).data[0] ? res : f32x2(sds1, 2.0f));
  res = (res.data[0] < f32x2(d2, 3.0f).data[0] ? res : f32x2(d2, 3.0f));
  res = (res.data[0] < f32x2(d3, 4.0f).data[0] ? res : f32x2(d3, 4.0f));
  res = (res.data[0] < f32x2(d4, 5.0f).data[0] ? res : f32x2(d4, 5.0f));
  res = (res.data[0] < f32x2(d5, 6.0f).data[0] ? res : f32x2(d5, 6.0f));
  return res;
}
f32x2 RayMarch(Context &__ctx, f32x3 ro, f32x3 rd, float m) {
  float t;
  f32x2 d = f32x2(9999.0f, -1.0f);
  for (int i = 0; i < 200; i++) {
    d = GetDist(__ctx, ro + rd * t);
    if (d.data[0] < (0.001f + 0.0001f * t) || t >= m)
      break;
    t += d.data[0] * 0.67f;
  }
  return f32x2(t, d.data[1]);
}
f32x3 normal(Context &__ctx, f32x3 p) {
  f32x2 h = f32x2(0.001f, 0.0f);
  f32x3 n = f32x3(GetDist(__ctx, p + h.swizzle<0, 1, 1>()).data[0] -
                      GetDist(__ctx, p - h.swizzle<0, 1, 1>()).data[0],
                  GetDist(__ctx, p + h.swizzle<1, 0, 1>()).data[0] -
                      GetDist(__ctx, p - h.swizzle<1, 0, 1>()).data[0],
                  GetDist(__ctx, p + h.swizzle<1, 1, 0>()).data[0] -
                      GetDist(__ctx, p - h.swizzle<1, 1, 0>()).data[0]);
  return normalize(n);
}
float getSoftShadow(Context &__ctx, f32x3 p, f32x3 lightPos) {
  float res = 1.0f;
  float dist = 0.01f;
  float lightSize = 0.03f;
  for (int i = 0; i < 15; i++) {
    float hit = GetDist(__ctx, p + lightPos * dist).data[0];
    res = min(res, hit / (dist * lightSize));
    dist += hit;
    if (hit < 0.0001f || dist > 60.0f)
      break;
  }
  return clamp(res, 0.0f, 1.0f);
}
float ambOcclusion(Context &__ctx, f32x3 pos, f32x3 nor) {
  float sca = 2.0f, occ = 0.0f;
  for (int i = 0; i < 10; i++) {
    float hr = 0.01f + float(i) * 0.5f / 4.0f;
    float dd = GetDist(__ctx, nor * hr + pos).data[0];
    occ += (hr - dd) * sca;
    sca *= 0.6f;
  }
  return clamp(1.0f - occ, 0.0f, 1.0f);
}
f32x3x3 camera(Context &__ctx, f32x3 o, f32x3 l) {
  f32x3 w = normalize(l - o);
  f32x3 u = normalize(cross(f32x3(0, 1, 0), w));
  f32x3 v = normalize(cross(w, u));
  return f32x3x3(u, v, w);
}
f32x3 iluminacion(Context &__ctx, f32x3 pos, f32x3 rd, f32x3 nor, f32x3 ref,
                  f32x3 lig, float id_color) {
  f32x3 rcol = f32x3(0.0f);
  float occ = ambOcclusion(__ctx, pos, nor);
  float sha = getSoftShadow(__ctx, pos, normalize(lig));
  rcol += 0.2f * occ;
  rcol += clamp(dot(lig, nor), 0.0f, 1.0f) * occ * sha;
  rcol += pow(clamp(1.0f + dot(rd, nor), 0.0f, 1.0f), 2.0f) * occ;
  rcol += 2.0f * pow(clamp(dot(ref, lig), 0.0f, 1.0f), 30.0f) * occ;
  if (id_color == 1.0f) {
    rcol *= 0.2f +
            0.5f * mod(floor(pos.data[0] * 0.25f) + floor(pos.data[2] * 0.25f),
                       2.0f);
  }
  if (id_color == 2.0f) {
    rcol *= f32x3(0.2f, 0.7f, 0.7f);
  }
  if (id_color == 3.0f) {
    rcol *= f32x3(1.0f, 0.32f, 0.17f);
  }
  if (id_color == 4.0f) {
    rcol *= f32x3(1.0f);
  }
  if (id_color == 5.0f) {
    rcol *= f32x3(1.0f, 0.0f, 0.0f);
  }
  if (id_color == 6.0f) {
    rcol *= f32x3(1.0f, 1.0f, 0.0f);
  }
  return rcol;
}
f32x3 render(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x3 col = f32x3(0.45f, 0.8f, 1.0f);
  f32x3 lig1 = normalize(f32x3(10.8f, 20.0f, -25.6f));
  f32x3 lig2 = normalize(f32x3(0.0f, 10.f, 25.6f));
  f32x2 t;
  for (int i = 0; i < 3; i++) {
    t = RayMarch(__ctx, ro, rd, 50.0f);
    if (t.data[0] < 50.0f) {
      f32x3 rcol = f32x3(0);
      f32x3 pos = ro + rd * t.data[0];
      f32x3 nor = normal(__ctx, pos);
      f32x3 ref = reflect(rd, nor);
      rcol = iluminacion(__ctx, pos, rd, nor, ref, lig1, t.data[1]);
      rcol += iluminacion(__ctx, pos, rd, nor, ref, lig2, t.data[1]);
      rcol /= 2.0f;
      ro = pos + nor * 0.001f;
      if (t.data[1] < 5.0f)
        rd = ref;
      rcol = mix(rcol, f32x3(0.45f, 0.8f, 1.0f),
                 1.0f - exp(-0.00715f * t.data[0]));
      if (i == 0)
        col = rcol;
      else
        col *= mix(rcol, f32x3(1), 1.0f - exp(-0.8f * float(i)));
    }
  }
  return col;
}
f32x3 linear2srgb(Context &__ctx, f32x3 c) {
  return mix(12.92f * c, 1.055f * pow(c, f32x3(1.0f / 1.8f)) - 0.055f,
             step(f32x3(0.0031308f), c));
}
f32x3 exposureToneMapping(Context &__ctx, float exposure, f32x3 hdrColor) {
  return f32x3(1.0f) - exp(-hdrColor * exposure);
}
f32x3 ACESFilm(Context &__ctx, f32x3 x) {
  float a, b, c, d, e;
  a = 2.51f;
  b = 0.03f;
  c = 2.43f;
  d = 0.59f;
  e = 0.14f;
  return (x * (a * x + b)) / (x * (c * x + d) + e);
}
Ray RotarEnCirculo(Context &__ctx, f32x3 ro, f32x3 rd) {
  float r = 25.0f;
  float veltime = 30.0f;
  float tt = radians(-iTime * veltime);
  f32x3 pos = f32x3(0.0f, 0.0f, 20.0f);
  f32x3 rotation1 = pos + f32x3(r * sin(tt), 0.0f, r * cos(tt));
  ro += rotation1;
  rd = rotate_y(__ctx, f32x3(-rd.data[0], rd.data[1], -rd.data[2]), tt);
  return Ray(ro, rd);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 ro = f32x3(0.0f, 8.0f, -15.0f);
  f32x3 rd = normalize(f32x3(p, 1.0f));
  Ray ray = RotarEnCirculo(__ctx, ro, rd);
  ro = ray.ro;
  rd = ray.rd;
  f32x3 col = render(__ctx, ro, rd);
  col = (ACESFilm(__ctx, col) + linear2srgb(__ctx, col) + col +
         exposureToneMapping(__ctx, 3.0f, col)) /
        4.0f;
  fragColor = f32x4(col, 1);
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
