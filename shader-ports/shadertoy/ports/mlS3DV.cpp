// Port of https://www.shadertoy.com/view/mlS3DV; author: dsa8.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mlS3DV.json.
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
float hash(Context &__ctx, float p) {
  p = fract(p * 0.011f);
  p *= p + 7.5f;
  p *= p + p;
  return fract(p);
}
f32x2x2 rot(Context &__ctx, float a) {
  float ca = cos(a);
  float sa = sin(a);
  return f32x2x2(ca, sa, -sa, ca);
}
float sminCubic(Context &__ctx, float a, float b, float k) {
  float h = max(k - abs(a - b), 0.0f) / k;
  return min(a, b) - h * h * h * k * (1.0f / 6.0f);
}
float opExtrussion(Context &__ctx, f32x3 p, float sdf, float h) {
  f32x2 w = f32x2(sdf, abs(p.data[2]) - h);
  return min(max(w.data[0], w.data[1]), 0.0f) + length(max(w, 0.0f));
}
f32x2 smoothrepeat_asin_sin(Context &__ctx, f32x2 p, float smooth_size,
                            float size) {
  p /= size;
  p = asin(sin(p) * (1.0f - smooth_size));
  return p * size;
}
f32x2 dKifs(Context &__ctx, f32x2 p, f32x2 c, float s, float t1, float t2) {
  float r2 = p.data[0] * p.data[0] + p.data[1] * p.data[1];
  float i;
  for (i = 0.f; i < 6.f && r2 < 1000.f; i++) {
    p *= rot(__ctx, t1);
    p = abs(p);
    if (p.data[0] - p.data[1] < 0.f) {
      float x1 = p.data[1];
      p.data[1] = p.data[0];
      p.data[0] = x1;
    }
    p *= rot(__ctx, t2);
    p.data[0] = s * p.data[0] - c.data[0] * (s - 1.f);
    p.data[1] = s * p.data[1];
    if (p.data[1] > 0.5f * c.data[1] * (s - 1.f))
      p.data[1] -= c.data[1] * (s - 1.f);
    p.data[1] += .1f * s;
    p.data[1] *= p.data[1] * .45f;
    p.data[0] -= .2f * s;
    r2 = p.data[0] * p.data[0] + p.data[1] * p.data[1];
  }
  return f32x2((sqrt(r2) - 2.f) * pow(s, -i), i);
}
f32x3 camPos;
f32x2 map(Context &__ctx, f32x3 p) {
  float d = 3.402823466e+38f;
  float mat = -1.f;
  f32x2 f;
  {
    f32x3 q = p;
    q.set_swizzle<0, 1>(
        smoothrepeat_asin_sin(__ctx, p.swizzle<0, 1>(), .04f, 4.0f));
    float c = 15.f;
    float zId = ceil((q.data[2] + .5f * c) / c);
    q.data[2] = mod(q.data[2] + .5f * c, c) - .5f * c;
    float r1 =
        0.2f + hash(__ctx, zId * 1234.f) * .75f +
        smoothstep(0.f, .2f,
                   sin((camPos.data[2] + 1.5f) * 3.14159265359f / 15.f)) *
            .15f;
    float r2 = mix(0.2f, 0.5f, hash(__ctx, zId * 4312.f));
    f = dKifs(__ctx, q.swizzle<0, 1>() * .2f, f32x2(1.f), 3.f, r1, r2);
    if (f.data[0] < d)
      mat = 0.f;
    d = min(d, opExtrussion(__ctx, q, f.data[0], .5f)) - 0.002f;
  }
  {
    f32x3 q = p;
    float c = 15.f;
    q.data[2] = mod(q.data[2] + .5f * c, c) - .5f * c;
    q.set_swizzle<0, 1>(q.swizzle<0, 1>() * (.5f));
    float s = 1.0f;
    float t = 0.0f;
    for (int i = 0; i < 2; i++) {
      t += s * .5f *
           (cos(6.2831f * q.data[0] + iTime * 5.f) +
            cos(6.2831f * q.data[1] + iTime * 2.f));
      ;
      s *= 0.5f + 0.1f * t;
      q.set_swizzle<0, 1>(0.97f * f32x2x2(1.6f, -1.2f, 1.2f, 1.6f) *
                              q.swizzle<0, 1>() +
                          (t - 0.5f) * 0.2f);
    }
    float dP = abs(q.data[2] - 0.01f -
                   (cos(q.data[0] * 4.f + iTime) * .5f + .5f) * .02f);
    if (dP - .3f < d)
      mat = 1.f;
    d = sminCubic(__ctx, d, dP, .6f);
  }
  return f32x2(d, mat);
}
f32x3 calcNormal(Context &__ctx, f32x3 pos) {
  f32x2 e = f32x2(1.0f, -1.0f) * 0.5773f;
  const float eps = 0.0005f;
  return normalize(e.swizzle<0, 1, 1>() *
                       map(__ctx, pos + e.swizzle<0, 1, 1>() * eps).data[0] +
                   e.swizzle<1, 1, 0>() *
                       map(__ctx, pos + e.swizzle<1, 1, 0>() * eps).data[0] +
                   e.swizzle<1, 0, 1>() *
                       map(__ctx, pos + e.swizzle<1, 0, 1>() * eps).data[0] +
                   e.swizzle<0, 0, 0>() *
                       map(__ctx, pos + e.swizzle<0, 0, 0>() * eps).data[0]);
}
float ao(Context &__ctx, f32x3 v, f32x3 n) {
  float sum = 0.0f;
  float att = 3.0f;
  float aoStep = .1f;
  float aoScale = .3f;
  float len = aoStep;
  for (int i = 0; i < 4; i++) {
    sum += (len - map(__ctx, v + n * len).data[0]) * att;
    len += aoStep;
    att *= 0.5f;
  }
  return max(1.0f - sum * aoScale, 0.0f);
}
f32x3 sRGB(Context &__ctx, f32x3 t) {
  return mix(1.055f * pow(t, f32x3(1.f / 2.4f)) - 0.055f, 12.92f * t,
             step(t, f32x3(0.0031308f)));
}
f32x3 aces_approx(Context &__ctx, f32x3 v) {
  v = max(v, 0.0f);
  v *= 0.6f;
  float a = 2.51f;
  a = 2.f;
  float b = 0.03f;
  float c = 2.43f;
  c = 1.f;
  float d = 0.59f;
  float e = 0.14f;
  return clamp((v * (a * v + b)) / (v * (c * v + d) + e), 0.0f, 1.0f);
}
f32x3 postProcess(Context &__ctx, f32x3 col) {
  col -= .1f;
  col += col * col * .4f;
  col = aces_approx(__ctx, col);
  col = sRGB(__ctx, col);
  return col;
}
f32x2 march(Context &__ctx, f32x3 ro, f32x3 rd, float tmax, float &t, int iMax,
            float e, float hStep) {
  f32x2 h;
  for (int i = 0; i < iMax; i++) {
    f32x3 pos = ro + t * rd;
    h = map(__ctx, pos);
    if (h.data[0] < e || t > tmax)
      break;
    t += h.data[0] * hStep;
  }
  return h;
}
f32x3 shade(Context &__ctx, f32x3 pos, f32x3 nor, f32x3 ro, f32x3 rd, float t) {
  f32x3 col = f32x3(0.15f, 0.25f, 0.4f);
  float m = mod(pos.data[2], 15.f);
  col *= pow(1.f - mod(pos.data[2], 15.f) / 15.f, 80.f) * 6.5f;
  float fractal = 1.f - step(m, .1f);
  f32x3 lDir = f32x3(0.57703f);
  lDir = normalize(f32x3(0.3f, .6f, .6f));
  float dif = clamp(dot(nor, lDir), 0.0f, 1.0f);
  f32x3 ref = reflect(rd, nor);
  float spec = max(dot(ref, lDir), 0.0f);
  col += (spec * .5f + dif * f32x3(.1f, .15f, .25f) * .5f) * fractal * .8f;
  float plane = step(m, .3f);
  col = clamp(col + -.2f * plane, 0.f, 1.f);
  col += plane * col * col * .7f;
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 ro = f32x3(0.f, 0.f, 15.f - iTime * 2.5f);
  camPos = ro;
  f32x3 ta = f32x3(0.0f, 0.f, 0.0f - iTime * 2.5f);
  f32x3 ww = normalize(ta - ro);
  f32x3 uu = normalize(cross(ww, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 vv = (cross(uu, ww));
  f32x3 tot = f32x3(0.0f);
  for (int m = 0; m < 2; m++)
    for (int n = 0; n < 2; n++) {
      f32x2 o = f32x2(float(m), float(n)) / float(2) - 0.5f;
      f32x2 p = (-iResolution.swizzle<0, 1>() + 2.0f * (fragCoord + o)) /
                iResolution.data[1];
      f32x3 rd = normalize(p.data[0] * uu + p.data[1] * vv + 1.5f * ww);
      rd.set_swizzle<0, 1>(
          rd.swizzle<0, 1>() *
          (rot(__ctx, sin(iTime * 3.14159265359f / 7.5f) * .4f)));
      rd.set_swizzle<0, 2>(
          rd.swizzle<0, 2>() *
          (rot(__ctx, sin(iTime * 3.14159265359f / 7.5f * 2.f) * .15f)));
      float tmax = 40.0f;
      float t = 0.0f;
      f32x2 h = march(__ctx, ro, rd, tmax, t, 40, 0.005f, .8f);
      f32x3 col = f32x3(0.f);
      if (t < tmax) {
        f32x3 pos = ro + t * rd;
        f32x3 nor = calcNormal(__ctx, pos);
        col = shade(__ctx, pos, nor, ro, rd, t);
        float ao = clamp(pow(ao(__ctx, pos, nor), 5.f) * 3.f, 0.f, 1.f);
        col = f32x3(ao) * col;
        if (h.data[1] == 1.f) {
          f32x3 ro2 = pos - nor * .5f;
          float ior =
              mix(1.2f, 1.f,
                  smoothstep(0.9f, 1.f,
                             cos(ro.data[2] * 3.14159265359f / 7.5f - .3f)));
          f32x3 rd2 = refract(rd, nor, ior);
          tmax = 60.f;
          t = 0.f;
          f32x2 h2 = march(__ctx, ro2, rd2, tmax, t, 50, 0.01f, 1.f);
          f32x3 col2 = f32x3(0.f);
          if (t < tmax) {
            f32x3 pos2 = ro2 + t * rd2;
            f32x3 nor2 = calcNormal(__ctx, pos2);
            col2 = shade(__ctx, pos2, nor2, ro2, rd2, t);
          }
          col = mix(col, col2, .5f);
        }
      }
      col = postProcess(__ctx, col);
      tot += col;
    }
  tot /= float(2 * 2);
  fragColor = f32x4(tot, 1.0f);
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
