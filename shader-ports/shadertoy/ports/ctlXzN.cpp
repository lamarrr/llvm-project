// Port of https://www.shadertoy.com/view/ctlXzN; author: mrange.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctlXzN.json.
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
const f32x4 hsv2rgb_K = f32x4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
f32x3 hsv2rgb(Context &__ctx, f32x3 c) {
  f32x3 p =
      abs(fract(c.swizzle<0, 0, 0>() + hsv2rgb_K.swizzle<0, 1, 2>()) * 6.0f -
          hsv2rgb_K.swizzle<3, 3, 3>());
  return c.data[2] * mix(hsv2rgb_K.swizzle<0, 0, 0>(),
                         clamp(p - hsv2rgb_K.swizzle<0, 0, 0>(), 0.0f, 1.0f),
                         c.data[1]);
}
const float hoff = 0.0f;
const f32x3 skyCol =
    (f32x3(hoff + 0.57f, 0.70f, 0.25f).data[2] *
     mix(hsv2rgb_K.swizzle<0, 0, 0>(),
         clamp(abs(fract(f32x3(hoff + 0.57f, 0.70f, 0.25f).swizzle<0, 0, 0>() +
                         hsv2rgb_K.swizzle<0, 1, 2>()) *
                       6.0f -
                   hsv2rgb_K.swizzle<3, 3, 3>()) -
                   hsv2rgb_K.swizzle<0, 0, 0>(),
               0.0f, 1.0f),
         f32x3(hoff + 0.57f, 0.70f, 0.25f).data[1]));
const f32x3 glowCol =
    (f32x3(hoff + 0.025f, 0.85f, 0.5f).data[2] *
     mix(hsv2rgb_K.swizzle<0, 0, 0>(),
         clamp(abs(fract(f32x3(hoff + 0.025f, 0.85f, 0.5f).swizzle<0, 0, 0>() +
                         hsv2rgb_K.swizzle<0, 1, 2>()) *
                       6.0f -
                   hsv2rgb_K.swizzle<3, 3, 3>()) -
                   hsv2rgb_K.swizzle<0, 0, 0>(),
               0.0f, 1.0f),
         f32x3(hoff + 0.025f, 0.85f, 0.5f).data[1]));
const f32x3 sunCol1 =
    (f32x3(hoff + 0.60f, 0.50f, 0.5f).data[2] *
     mix(hsv2rgb_K.swizzle<0, 0, 0>(),
         clamp(abs(fract(f32x3(hoff + 0.60f, 0.50f, 0.5f).swizzle<0, 0, 0>() +
                         hsv2rgb_K.swizzle<0, 1, 2>()) *
                       6.0f -
                   hsv2rgb_K.swizzle<3, 3, 3>()) -
                   hsv2rgb_K.swizzle<0, 0, 0>(),
               0.0f, 1.0f),
         f32x3(hoff + 0.60f, 0.50f, 0.5f).data[1]));
const f32x3 sunCol2 =
    (f32x3(hoff + 0.05f, 0.75f, 25.0f).data[2] *
     mix(hsv2rgb_K.swizzle<0, 0, 0>(),
         clamp(abs(fract(f32x3(hoff + 0.05f, 0.75f, 25.0f).swizzle<0, 0, 0>() +
                         hsv2rgb_K.swizzle<0, 1, 2>()) *
                       6.0f -
                   hsv2rgb_K.swizzle<3, 3, 3>()) -
                   hsv2rgb_K.swizzle<0, 0, 0>(),
               0.0f, 1.0f),
         f32x3(hoff + 0.05f, 0.75f, 25.0f).data[1]));
const f32x3 diffCol =
    (f32x3(hoff + 0.60f, 0.75f, 0.25f).data[2] *
     mix(hsv2rgb_K.swizzle<0, 0, 0>(),
         clamp(abs(fract(f32x3(hoff + 0.60f, 0.75f, 0.25f).swizzle<0, 0, 0>() +
                         hsv2rgb_K.swizzle<0, 1, 2>()) *
                       6.0f -
                   hsv2rgb_K.swizzle<3, 3, 3>()) -
                   hsv2rgb_K.swizzle<0, 0, 0>(),
               0.0f, 1.0f),
         f32x3(hoff + 0.60f, 0.75f, 0.25f).data[1]));
const f32x3 sunDir1 = normalize(f32x3(3.f, 3.0f, -7.0f));
f32x3x3 g_rot;
float g_anim;
f32x2x2 g_rot_yx;
f32x2x2 g_rot_xz;
f32x3 sRGB(Context &__ctx, f32x3 t) {
  return mix(1.055f * pow(t, f32x3(1.f / 2.4f)) - 0.055f, 12.92f * t,
             step(t, f32x3(0.0031308f)));
}
f32x3 aces_approx(Context &__ctx, f32x3 v) {
  v = max(v, 0.0f);
  v *= 0.6f;
  float a = 2.51f;
  float b = 0.03f;
  float c = 2.43f;
  float d = 0.59f;
  float e = 0.14f;
  return clamp((v * (a * v + b)) / (v * (c * v + d) + e), 0.0f, 1.0f);
}
f32x3x3 rot_z(Context &__ctx, float a) {
  float c = cos(a);
  float s = sin(a);
  return f32x3x3(c, s, 0, -s, c, 0, 0, 0, 1);
}
f32x3x3 rot_y(Context &__ctx, float a) {
  float c = cos(a);
  float s = sin(a);
  return f32x3x3(c, 0, s, 0, 1, 0, -s, 0, c);
}
f32x3x3 rot_x(Context &__ctx, float a) {
  float c = cos(a);
  float s = sin(a);
  return f32x3x3(1, 0, 0, 0, c, s, 0, -s, c);
}
float rayPlane(Context &__ctx, f32x3 ro, f32x3 rd, f32x4 p) {
  return -(dot(ro, p.swizzle<0, 1, 2>()) + p.data[3]) /
         dot(rd, p.swizzle<0, 1, 2>());
}
float box(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.0f)) + min(max(d.data[0], d.data[1]), 0.0f);
}
float torus(Context &__ctx, f32x3 p, f32x2 t) {
  f32x2 q = f32x2(length(p.swizzle<0, 2>()) - t.data[0], p.data[1]);
  return length(q) - t.data[1];
}
float df(Context &__ctx, f32x3 p) {
  const float zz = 6.0f;
  const float r0 = 2.0f - 0.5f;
  const float r1 = r0 / zz;
  const float r2 = r1 / zz;
  const float r3 = r2 / zz;
  float anim = g_anim;
  float angle = anim * 3.141592654f;
  float z = mix(zz, 1.0f, anim);
  f32x3 p0 = p;
  p0 *= g_rot;
  p0.set_swizzle<1, 2>(p0.swizzle<1, 2>() * (g_rot_yx));
  p0.data[0] -= mix(r0 * zz, 0.0f, anim);
  p0 /= z;
  f32x3 p1 = p0;
  p1.data[2] = abs(p1.data[2]);
  p1.set_swizzle<0, 2>(p1.swizzle<0, 2>() * (g_rot_xz));
  f32x3 p2 = p1;
  p2.data[2] = abs(p2.data[2]);
  p2.data[2] -= r0;
  p2 = p2.swizzle<2, 0, 1>();
  f32x3 p3 = p0;
  float rr = mix(r3, 0.0f, anim);
  float d0 = torus(__ctx, p0, f32x2(r0, r1 + rr));
  d0 = abs(d0) - r2;
  float d1 = p1.data[0];
  float d2 = torus(__ctx, p2, f32x2(r1, r2 + rr));
  float d3 = p3.data[0];
  float d = d0;
  d = max(d, d1);
  d = min(d, d2);
  if (angle < 3.141592654f / 4.0f)
    d = max(d, d3);
  if (angle > ((2.0f * 3.141592654f) - 3.141592654f / 4.0f))
    d = max(d, -d3);
  return d * z;
}
f32x3 normal(Context &__ctx, f32x3 pos) {
  f32x2 eps = f32x2(0.001f, 0.0f);
  f32x3 nor;
  nor.data[0] = df(__ctx, pos + eps.swizzle<0, 1, 1>()) -
                df(__ctx, pos - eps.swizzle<0, 1, 1>());
  nor.data[1] = df(__ctx, pos + eps.swizzle<1, 0, 1>()) -
                df(__ctx, pos - eps.swizzle<1, 0, 1>());
  nor.data[2] = df(__ctx, pos + eps.swizzle<1, 1, 0>()) -
                df(__ctx, pos - eps.swizzle<1, 1, 0>());
  return normalize(nor);
}
float rayMarch(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 &gcol) {
  float t = 0.0f;
  const float tol = 0.0001f;
  f32x2 dti = f32x2(1e10f, 0.0f);
  int i = 0;
  f32x3 gc = f32x3(0.0f);
  for (i = 0; i < 60; ++i) {
    float d = df(__ctx, ro + rd * t);
    gc += (0.0125f * glowCol) / d;
    if (d < dti.data[0]) {
      dti = f32x2(d, t);
    }
    if (d < 0.0001f || t > 22.0f) {
      break;
    }
    t += d;
  }
  gcol = gc;
  if (i == 60) {
    t = dti.data[1];
  };
  return t;
}
f32x3 render0(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x3 col = f32x3(0.0f);
  float sd = max(dot(sunDir1, rd), 0.0f);
  float sf = 1.0001f - sd;
  col += clamp(f32x3(0.0025f / abs(rd.data[1])) * glowCol, 0.0f, 1.0f);
  col += 0.75f * skyCol * pow((1.0f - abs(rd.data[1])), 8.0f);
  col += 2.0f * sunCol1 * pow(sd, 100.0f);
  col += sunCol2 * pow(sd, 800.0f);
  float tp1 = rayPlane(__ctx, ro, rd, f32x4(f32x3(0.0f, -1.0f, 0.0f), -6.0f));
  if (tp1 > 0.0f) {
    f32x3 pos = ro + tp1 * rd;
    f32x2 pp = pos.swizzle<0, 2>();
    float db = box(__ctx, pp, f32x2(5.0f, 9.0f)) - 3.0f;
    col += f32x3(4.0f) * skyCol * rd.data[1] * rd.data[1] *
           smoothstep(0.25f, 0.0f, db);
    col += f32x3(0.8f) * skyCol * exp(-0.5f * max(db, 0.0f));
    col += 0.25f * sqrt(skyCol) * max(-db, 0.0f);
  }
  return clamp(col, 0.0f, 10.0f);
  ;
}
f32x3 render1(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x3 gcol;
  int iter;
  float t = rayMarch(__ctx, ro, rd, gcol);
  f32x3 col = render0(__ctx, ro, rd);
  f32x3 p = ro + rd * t;
  f32x3 n = normal(__ctx, p);
  f32x3 r = reflect(rd, n);
  float fre = 1.0f + dot(rd, n);
  fre *= fre;
  float dif = dot(sunDir1, n);
  if (t < 22.0f) {
    col = f32x3(0.0f);
    col += sunCol1 * dif * dif * diffCol * 0.25f;
    col += mix(0.33f, 1.0f, fre) * render0(__ctx, p, r);
  } else {
    col += gcol;
  }
  return col;
}
f32x3 effect(Context &__ctx, f32x2 p) {
  float tm = iTime * 0.5f + 10.0f;
  g_rot = rot_x(__ctx, 0.11f * tm) * rot_y(__ctx, 0.23f * tm) *
          rot_z(__ctx, 0.35f * tm);
  g_anim = (0.5f + 0.5f * sin(fract(0.1f * iTime) * 3.141592654f -
                              3.141592654f / 2.0f));
  g_rot_yx = f32x2x2(cos(g_anim * (3.141592654f * 0.5f)),
                     sin(g_anim * (3.141592654f * 0.5f)),
                     -sin(g_anim * (3.141592654f * 0.5f)),
                     cos(g_anim * (3.141592654f * 0.5f)));
  g_rot_xz = f32x2x2(cos(3.141592654f / 2.0f - g_anim * 3.141592654f),
                     sin(3.141592654f / 2.0f - g_anim * 3.141592654f),
                     -sin(3.141592654f / 2.0f - g_anim * 3.141592654f),
                     cos(3.141592654f / 2.0f - g_anim * 3.141592654f));
  f32x3 ro = 2.0f * f32x3(5.0f, 1.0f, 0.f);
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() *
                       (f32x2x2(cos(-0.1f * tm), sin(-0.1f * tm),
                                -sin(-0.1f * tm), cos(-0.1f * tm))));
  const f32x3 la = f32x3(0.0f, 0.0f, 0.0f);
  const f32x3 up = normalize(f32x3(0.0f, 1.0f, 0.0f));
  f32x3 ww = normalize(la - ro);
  f32x3 uu = normalize(cross(up, ww));
  f32x3 vv = (cross(ww, uu));
  const float fov = tan((2.0f * 3.141592654f) / 6.f);
  f32x3 rd = normalize(-p.data[0] * uu + p.data[1] * vv + fov * ww);
  f32x3 col = render1(__ctx, ro, rd);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 q = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 p = -1.f + 2.f * q;
  f32x2 pp = p;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 col = f32x3(0.0f);
  col = effect(__ctx, p);
  col *= smoothstep(1.5f, 0.5f, length(pp));
  col = aces_approx(__ctx, col);
  col = sRGB(__ctx, col);
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
