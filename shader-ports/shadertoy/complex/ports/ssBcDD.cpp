// Port of https://www.shadertoy.com/view/ssBcDD; author: gelami.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ssBcDD.json.
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
f32x4 iPrism(Context &__ctx, f32x3 ro, f32x3 rd, float ra, float he, int si) {
  f32x4 tN = f32x4(-1e3f, 0, 0, 0);
  float tF = 1e3f;
  float s = 3.14159265359f * 2.f / float(si);
  float a = 3.14159265359f / 2.f;
  ra = ra * cos(s * 0.5f);
  for (int i = 0; i < 12 + 2; i++) {
    if (i > si + 1)
      break;
    f32x3 nd = i < si ? f32x3(cos(a), 0, sin(a))
                      : f32x3(0, float(i - si) * 2.f - 1.f, 0);
    float c = dot(ro, nd) + (i < si ? ra : he);
    float d = dot(rd, nd);
    float nt = -c / d > 0.f ? -c / d : -1e3f;
    nt = d < 0.f ? -nt : nt;
    if (nt > tN.data[0])
      tN = f32x4(nt, -nd);
    if (d < 0.f && -nt < tF)
      tF = -nt;
    a += s;
  }
  if (tN.data[0] > tF)
    return f32x4(1e3f);
  return tN;
}
f32x3x3 getCamMat(Context &__ctx, f32x3 ro, f32x3 lo) {
  f32x3 w = normalize(lo - ro);
  f32x3 u = normalize(cross(w, f32x3(0, 1, 0)));
  f32x3 v = normalize(cross(u, w));
  return f32x3x3(u, v, w);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float t = iTime * 0.4f;
  f32x3 ro = f32x3(0, 1.2f, 1.8f);
  f32x3 lo = f32x3(0, 0, 0);
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() *
                       (f32x2x2(cos(sin(t) * .15f), -sin(sin(t) * .15f),
                                sin(sin(t) * .15f), cos(sin(t) * .15f))));
  ro.set_swizzle<0, 2>(
      ro.swizzle<0, 2>() *
      (f32x2x2(
          cos(iMouse.data[2] > 0.f ? m.data[0] * 2.f * 3.14159265359f : t),
          -sin(iMouse.data[2] > 0.f ? m.data[0] * 2.f * 3.14159265359f : t),
          sin(iMouse.data[2] > 0.f ? m.data[0] * 2.f * 3.14159265359f : t),
          cos(iMouse.data[2] > 0.f ? m.data[0] * 2.f * 3.14159265359f : t))));
  float fn = iMouse.data[2] > 0.f
                 ? m.data[1]
                 : (cos(iTime * 3.14159265359f / 2.5f) * 0.5f + 0.5f);
  int n = int(fn * float(12 - 3) * .9999f + 3.f);
  f32x3x3 cm = getCamMat(__ctx, ro, lo);
  f32x3 tot = f32x3(0);
  for (int p = 0; p < 2; p++)
    for (int v = 0; v < 2; v++) {
      f32x2 o = f32x2(float(p), float(v)) / float(2) - 0.5f;
      f32x2 pv = (2.0f * (fragCoord + o) - iResolution.swizzle<0, 1>()) /
                 iResolution.data[1];
      f32x3 col = f32x3(0);
      f32x3 rd = normalize(cm * f32x3(pv, 1.5f));
      f32x4 d = iPrism(__ctx, ro, rd, .8f, .8f, n);
      if (d.data[0] < 1e3f) {
        f32x3 p = ro + rd * d.data[0];
        f32x3 n = d.swizzle<1, 2, 3>();
        f32x3 l = normalize(f32x3(1.f, .6f, .5f));
        f32x2 uv =
            abs(n.data[1]) > .5f
                ? p.swizzle<0, 2>()
                : f32x2(atan(p.data[2], p.data[0]) / 3.14159265359f * 2.f,
                        p.data[1] * 1.2f);
        f32x3 env =
            textureLod(__ctx.channel0, reflect(rd, n), 3.f).swizzle<0, 1, 2>() *
                .6f +
            .4f;
        float ch = mod(dot(floor(uv * 2.5f), f32x2(1)), 2.f) * .4f + .6f;
        float dif = max(dot(n, l), .1f);
        col =
            f32x3(.1f, .2f, .3f) * .6f + f32x3(.9f, .8f, .6f) * dif * env * ch;
      } else {
        col = f32x3(0.12f) * (1.f - 0.45f * length(pv));
      }
      tot += col;
    }
  tot /= float(2 * 2);
  tot = max(tot, f32x3(0));
  tot = tot / (1.f + tot);
  tot = pow(tot, f32x3(1.f / 2.2f));
  tot += fract(sin(fragCoord.data[0] * f32x3(32, 11, 2) +
                   fragCoord.data[1] * f32x3(3, 31, 21)) *
               232.323f) /
         255.f;
  fragColor = f32x4(tot, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_cube<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
