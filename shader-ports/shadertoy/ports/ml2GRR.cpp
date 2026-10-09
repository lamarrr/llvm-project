// Port of https://www.shadertoy.com/view/ml2GRR; author: oneshade.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ml2GRR.json.
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

f32x2x2 Rotate2D(Context &__ctx, float a) {
  float co = cos(a), si = sin(a);
  return f32x2x2(co, -si, si, co);
}
float Hash11(Context &__ctx, float p) {
  p = fract(p * 0.1031f);
  p *= p + 33.33f;
  p *= p + p;
  return fract(p);
}
f32x3 Hash33(Context &__ctx, f32x3 p) {
  p = fract(p * f32x3(0.1031f, 0.103f, 0.0973f));
  p += dot(p, p.swizzle<1, 0, 2>() + 33.33f);
  return fract((p.swizzle<0, 0, 1>() + p.swizzle<1, 0, 0>()) *
               p.swizzle<2, 1, 0>());
}
float map(Context &__ctx, f32x3 p) {
  p.data[1] += iTime;
  float w = 4.0f;
  f32x3 cell = floor(p / w) * w;
  p -= cell + 0.5f * w;
  p.set_swizzle<0, 2>(
      p.swizzle<0, 2>() *
      (Rotate2D(__ctx, iTime * Hash11(__ctx, cell.data[1]) * 2.0f)));
  p -= 2.0f * Hash33(__ctx, cell) - 1.0f;
  return 0.5f * (length(p) - 0.25f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 mouse = (iMouse.swizzle<0, 1>() - 0.5f * iResolution.swizzle<0, 1>()) /
                iResolution.data[1];
  f32x3 color = f32x3(0.0f);
  f32x3 ro = f32x3(10.0f * mouse, 5.0f);
  f32x3 fd = -normalize(ro);
  f32x3 rt = normalize(f32x3(-fd.data[2], 0.0f, fd.data[0]));
  f32x3 up = cross(rt, fd);
  f32x3 rd = normalize(uv.data[0] * rt + uv.data[1] * up + fd);
  color =
      f32x3(0.5f, 0.5f, 1.0f) *
      pow(1.0f - length(fragCoord / iResolution.swizzle<0, 1>() - 0.5f), 0.8f);
  if (rd.data[1] < 0.0f && ro.data[1] > -10.0f) {
    color = mix(color, f32x3(1.0f), pow(-rd.data[1], 0.4f));
  }
  bool inside = map(__ctx, ro) < 0.0f;
  float total = 0.0f;
  float t = 0.0f;
  for (int i = 0; i < 100; i++) {
    if (inside) {
      total += 0.03f;
      t += 0.02f;
      if (map(__ctx, ro + rd * t) >= 0.0f)
        inside = false;
    } else {
      t += map(__ctx, ro + rd * t);
      if (map(__ctx, ro + rd * t) < 1e-3f)
        inside = true;
    }
    if (total > 1.0f || t > 60.0f || ro.data[1] + rd.data[1] * t < -10.0f)
      break;
  }
  color = mix(color, f32x3(1.0f), total);
  fragColor = f32x4(color, 1.0f);
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
