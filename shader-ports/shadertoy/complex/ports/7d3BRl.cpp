// Port of https://www.shadertoy.com/view/7d3BRl; author: iq.
// Dataset license metadata: mit; original retained in upstream/7d3BRl.json.
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
f32x4 iEllipsoid(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 r) {
  f32x3 r2 = r * r;
  float a = dot(rd, rd / r2);
  float b = dot(ro, rd / r2);
  float c = dot(ro, ro / r2);
  float h = b * b - a * (c - 1.0f);
  if (h < 0.0f)
    return f32x4(-1.0f);
  float t = (-b - sqrt(h)) / a;
  f32x3 n = normalize((ro + t * rd) / r2);
  return f32x4(t, n);
}
f32x3 pattern(Context &__ctx, f32x2 uv) {
  f32x3 col = f32x3(0.6f);
  col += 0.4f * smoothstep(-0.01f, 0.01f,
                           cos(uv.data[0] * 0.5f) * cos(uv.data[1] * 0.5f));
  col *= smoothstep(-1.0f, -0.98f, cos(uv.data[0])) *
         smoothstep(-1.0f, -0.98f, cos(uv.data[1]));
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float an = 0.5f * iTime;
  f32x3 ro = f32x3(1.0f * cos(an), 0.4f, 1.0f * sin(an));
  f32x3 ta = f32x3(0.0f, 0.0f, 0.0f);
  f32x3 ww = normalize(ta - ro);
  f32x3 uu = normalize(cross(ww, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 vv = normalize(cross(uu, ww));
  f32x3 tot = f32x3(0.0f);
  for (int m = 0; m < 3; m++)
    for (int n = 0; n < 3; n++) {
      f32x2 o = f32x2(float(m), float(n)) / float(3) - 0.5f;
      f32x2 p = (2.0f * (fragCoord + o) - iResolution.swizzle<0, 1>()) /
                iResolution.data[1];
      f32x3 rd = normalize(p.data[0] * uu + p.data[1] * vv + 1.5f * ww);
      f32x3 ra = f32x3(0.6f, 0.5f, 0.3f);
      f32x4 tnor = iEllipsoid(__ctx, ro, rd, ra);
      float t = tnor.data[0];
      f32x3 col = f32x3(0.08f) * (1.0f - 0.3f * length(p)) + 0.02f * rd.data[1];
      if (t > 0.0f) {
        f32x3 pos = ro + t * rd;
        f32x3 nor = tnor.swizzle<1, 2, 3>();
        col = pattern(__ctx,
                      f32x2(16.0f, 64.0f) *
                          f32x2(atan(pos.data[1], pos.data[2]), pos.data[0]));
        f32x3 lig = normalize(f32x3(0.7f, 0.6f, 0.3f));
        f32x3 hal = normalize(-rd + lig);
        float dif = clamp(dot(nor, lig), 0.0f, 1.0f);
        float amb =
            clamp(0.5f + 0.5f * dot(nor, f32x3(0.0f, 1.0f, 0.0f)), 0.0f, 1.0f);
        col *= f32x3(0.2f, 0.3f, 0.4f) * amb + f32x3(1.0f, 0.9f, 0.7f) * dif;
        col += 0.4f * pow(clamp(dot(hal, nor), 0.0f, 1.0f), 12.0f) * dif;
      }
      col = sqrt(col);
      tot += col;
    }
  tot /= float(3 * 3);
  tot += fract(sin(fragCoord.data[0] * f32x3(13, 1, 11) +
                   fragCoord.data[1] * f32x3(1, 7, 5)) *
               158.391832f) /
         255.0f;
  fragColor = f32x4(tot, 1.0f);
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
