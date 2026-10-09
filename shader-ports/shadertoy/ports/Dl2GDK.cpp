// Port of https://www.shadertoy.com/view/Dl2GDK; author: vinvinvin.
// Dataset license metadata: mit; original retained in upstream/Dl2GDK.json.
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
float sdRoundBox(Context &__ctx, f32x2 p, f32x2 b, f32x4 r) {
  r.set_swizzle<0, 1>((p.data[0] > 0.0f) ? r.swizzle<0, 1>()
                                         : r.swizzle<2, 3>());
  r.data[0] = (p.data[1] > 0.0f) ? r.data[0] : r.data[1];
  f32x2 q = abs(p) - b + r.data[0];
  return min(max(q.data[0], q.data[1]), 0.0f) + length(max(q, 0.0f)) -
         r.data[0];
}
float sdCircle(Context &__ctx, f32x2 p, float r) { return length(p) - r; }
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 cp = f32x2(2.0f * fragCoord.data[0] - (iMouse.data[0] * 2.0f),
                   2.0f * fragCoord.data[1] - (iMouse.data[1] * 2.0f)) /
             iResolution.data[1];
  f32x2 p =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = (2.0f * iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
            iResolution.data[1];
  float rbd = sdRoundBox(__ctx, p, f32x2(1.2f, 0.7f), f32x4(0.06f));
  float cd = sdCircle(__ctx, cp, 0.02f);
  float boxCircleD =
      1.0f - abs(sdRoundBox(__ctx, p - cp, f32x2(1.2f, 0.7f), f32x4(0.06f)));
  f32x3 bead = f32x3(1.0f, 0.8f, 0.8f);
  bead *= (pow((1.0f - (abs(rbd) * 1.0f)), 100.0f));
  bead *= pow(max(1.0f - (cd * 0.8f), 0.0f), 100.0f);
  bead *= 0.5f;
  f32x3 redOutline = f32x3(1.0f, 0.05f, 0.1f);
  redOutline *= (pow((1.0f - (abs(rbd * 0.8f))), 100.0f));
  redOutline *= pow(max(1.0f - cd * 0.5f, 0.0f), 10.0f);
  redOutline *= 0.5f;
  f32x3 whiteOutline = f32x3(1.0f, 0.7f, 0.8f);
  whiteOutline *= (pow((1.0f - (abs(rbd * 0.9f))), 250.0f));
  whiteOutline *= pow(max(1.0f - (cd * 0.4f), 0.0f), 6.0f);
  whiteOutline *= 0.8f;
  f32x3 haloColor = f32x3(0.8f, 0.0f, 0.1f);
  f32x3 haloFalloff = f32x3(0.0f);
  haloFalloff += haloColor * pow(max(1.0f - (cd * 6.0f), 0.0f), 4.0f) * 0.25f;
  haloFalloff += haloColor * pow(max(1.0f - (cd * 4.0f), 0.0f), 2.0f) * 0.1f;
  haloFalloff += haloColor * pow(max(1.0f - (cd * 0.8f), 0.0f), 2.0f) * 0.2f;
  haloFalloff += haloColor * pow(max(1.0f - (cd * 0.3f), 0.0f), 2.0f) * 0.1f;
  f32x3 col = f32x3(0.0f);
  col += redOutline;
  col += haloFalloff * pow(boxCircleD, 100.0f);
  col += whiteOutline * pow(boxCircleD, 20.0f);
  col += bead * pow(boxCircleD, 100.0f);
  fragColor = f32x4(col, 1.0f);
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
