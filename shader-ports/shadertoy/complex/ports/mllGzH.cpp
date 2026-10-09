// Port of https://www.shadertoy.com/view/mllGzH; author: iq.
// Dataset license metadata: mit; original retained in upstream/mllGzH.json.
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
f32x4 iWedge(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 s) {
  f32x3 m = 1.0f / rd;
  f32x3 z = f32x3(rd.data[0] >= 0.0f ? 1.0f : -1.0f,
                  rd.data[1] >= 0.0f ? 1.0f : -1.0f,
                  rd.data[2] >= 0.0f ? 1.0f : -1.0f);
  f32x3 k = s * z;
  f32x3 t1 = (-ro - k) * m;
  f32x3 t2 = (-ro + k) * m;
  float tn = max(max(t1.data[0], t1.data[1]), t1.data[2]);
  float tf = min(min(t2.data[0], t2.data[1]), t2.data[2]);
  if (tn > tf)
    return f32x4(-1.0f);
  float k1 = s.data[1] * ro.data[0] - s.data[0] * ro.data[1];
  float k2 = s.data[0] * rd.data[1] - s.data[1] * rd.data[0];
  float tp = k1 / k2;
  if (k1 > tn * k2)
    return f32x4(tn, -step(tn, t1) * z);
  if (tp > tn && tp < tf)
    return f32x4(tp, normalize(f32x3(-s.data[1], s.data[0], 0.0f)));
  return f32x4(-1.0f);
}
f32x3 pattern(Context &__ctx, f32x2 uv) {
  return f32x3(0.6f + 0.4f * smoothstep(-0.01f, 0.01f,
                                        cos(uv.data[0] * 0.5f) *
                                            cos(uv.data[1] * 0.5f))) *
         smoothstep(-1.0f, -0.98f, cos(uv.data[0])) *
         smoothstep(-1.0f, -0.98f, cos(uv.data[1]));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float an = 0.2f * iTime;
  f32x3 ro = f32x3(1.0f * sin(an), 0.4f * sin(1.6f * an), 1.0f * cos(an));
  f32x3 ta = f32x3(0.0f, 0.01f, 0.0f);
  f32x3 ww = normalize(ta - ro);
  f32x3 uu = normalize(cross(ww, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 vv = normalize(cross(uu, ww));
  const f32x3 siz = f32x3(0.5f, 0.2f, 0.4f);
  f32x3 tot = f32x3(0.0f);
  for (int m = 0; m < 3; m++)
    for (int n = 0; n < 3; n++) {
      f32x2 o = f32x2(float(m), float(n)) / float(3) - 0.5f;
      f32x2 p = (-iResolution.swizzle<0, 1>() + 2.0f * (fragCoord + o)) /
                iResolution.data[1];
      f32x3 rd = normalize(p.data[0] * uu + p.data[1] * vv + 1.5f * ww);
      f32x3 col = f32x3(0.08f + 0.02f * rd.data[1]) * (1.0f - 0.3f * length(p));
      f32x4 tnor = iWedge(__ctx, ro, rd, siz);
      if (tnor.data[0] > 0.0f) {
        float t = tnor.data[0];
        f32x3 pos = ro + t * rd;
        f32x3 nor = tnor.swizzle<1, 2, 3>();
        f32x3 mor = abs(nor);
        f32x2 uv = (mor.data[0] > mor.data[1] && mor.data[0] > mor.data[2])
                       ? pos.swizzle<1, 2>()
                   : (mor.data[1] > mor.data[2]) ? pos.swizzle<2, 0>()
                                                 : pos.swizzle<0, 1>();
        col = pattern(__ctx, 47.0f * uv);
        f32x3 lig = normalize(f32x3(0.7f, 0.6f, 0.3f));
        f32x3 hal = normalize(-rd + lig);
        float dif = clamp(dot(nor, lig), 0.0f, 1.0f);
        float amb = clamp(0.6f + 0.4f * nor.data[1], 0.0f, 1.0f);
        col *= f32x3(0.2f, 0.3f, 0.4f) * amb + f32x3(1.0f, 0.9f, 0.7f) * dif;
        col += 0.4f * pow(clamp(dot(hal, nor), 0.0f, 1.0f), 12.0f) * dif;
      }
      col = sqrt(col);
      tot += col;
    }
  tot /= float(3 * 3);
  tot += fract(sin(fragCoord.data[0] * f32x3(13, 17, 11) +
                   fragCoord.data[1] * f32x3(1, 7, 5)) *
               158.391832f) /
         255.0f;
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
