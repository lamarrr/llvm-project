// Port of https://www.shadertoy.com/view/cdjSDc; author: phreax.
// Dataset license metadata: cc-by-nc-4.0; original retained in
// upstream/cdjSDc.json.
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
float tt;
float curve(Context &__ctx, float t, float d) {
  t /= d;
  return mix(floor(t), floor(t) + 1.f,
             pow(smoothstep(0.f, 1.f, fract(t)), 5.f));
}
float flower(Context &__ctx, f32x2 uv) {
  float n = 3.f;
  float a = atan(uv.data[1], uv.data[0]);
  float d = length(uv) - cos(a * n);
  float emph = .1f * (smoothstep(20.6f, 20.6f + .2f, iChannelTime[0]) -
                      smoothstep(21.f, 21.f + 2.f, iChannelTime[0])) +
               .07f * ((smoothstep(35.4f, 35.4f + .2f, iChannelTime[0]) -
                        smoothstep(35.4f, 35.4f + 1.5f, iChannelTime[0])));
  emph += .03f * (smoothstep(50.4f, 50.4f + .2f, iChannelTime[0]) -
                  smoothstep(50.4f, 50.4f + 1.f, iChannelTime[0]));
  emph += .05f * ((smoothstep(64.f, 64.f + .2f, iChannelTime[0]) -
                   smoothstep(64.f, 64.f + 1.f, iChannelTime[0])));
  emph += .07f * ((smoothstep(65.f, 65.f + .2f, iChannelTime[0]) -
                   smoothstep(65.f, 65.f + 1.f, iChannelTime[0])));
  emph += .1f * ((smoothstep(79.8f, 79.8f + .2f, iChannelTime[0]) -
                  smoothstep(79.8f, 79.8f + 1.f, iChannelTime[0])));
  emph += .1f * ((smoothstep(88.8f, 88.8f + .2f, iChannelTime[0]) -
                  smoothstep(88.8f, 88.8f + 1.f, iChannelTime[0])));
  emph += .15f * ((smoothstep(57.4f, 57.4f + .2f, iChannelTime[0]) -
                   smoothstep(57.4f, 57.4f + 1.f, iChannelTime[0])));
  emph += .1f * ((smoothstep(72.4f, 72.4f + .2f, iChannelTime[0]) -
                  smoothstep(72.4f, 72.4f + 1.f, iChannelTime[0])));
  emph += .1f * ((smoothstep(93.4f, 93.4f + .2f, iChannelTime[0]) -
                  smoothstep(93.4f, 93.4f + 1.f, iChannelTime[0])));
  emph += .15f * ((smoothstep(109.3f, 109.3f + .2f, iChannelTime[0]) -
                   smoothstep(109.3f, 109.3f + 1.f, iChannelTime[0])));
  emph += .1f * ((smoothstep(118.3f, 118.3f + .2f, iChannelTime[0]) -
                  smoothstep(118.3f, 118.3f + 1.f, iChannelTime[0])));
  emph += .1f * ((smoothstep(124.f, 124.f + .2f, iChannelTime[0]) -
                  smoothstep(124.f, 124.f + 1.f, iChannelTime[0])));
  emph += .1f * ((smoothstep(133.3f, 133.3f + .2f, iChannelTime[0]) -
                  smoothstep(133.3f, 133.3f + 1.f, iChannelTime[0])));
  emph += .1f * ((smoothstep(138.8f, 138.8f + .2f, iChannelTime[0]) -
                  smoothstep(138.8f, 138.8f + 1.f, iChannelTime[0])));
  emph += .1f * ((smoothstep(153.7f, 153.7f + .2f, iChannelTime[0]) -
                  smoothstep(153.7f, 153.7f + 1.f, iChannelTime[0])));
  emph += .1f * ((smoothstep(183.f, 183.f + .2f, iChannelTime[0]) -
                  smoothstep(183.f, 183.f + 1.f, iChannelTime[0])));
  emph += .1f * ((smoothstep(257.f, 257.f + .2f, iChannelTime[0]) -
                  smoothstep(257.f, 257.f + 1.5f, iChannelTime[0])));
  emph += .07f * ((smoothstep(271.8f, 271.8f + .2f, iChannelTime[0]) -
                   smoothstep(271.8f, 271.8f + 1.f, iChannelTime[0])));
  return smoothstep(fwidth(d) + .01f, -.07f, abs(d + .2f) - emph);
}
f32x3 pal(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(6.28318f * (c * t + d));
}
f32x3 getPal(Context &__ctx, int id, float t) {
  id = id % 7;
  f32x3 col = pal(__ctx, t, f32x3(.5f, 0.5f, 0.5f), f32x3(0.5f, 0.5f, 0.5f),
                  f32x3(1.0f, 1.0f, 1.0f), f32x3(0.0f, -0.33f, 0.33f));
  if (id == 1)
    col = pal(__ctx, t, f32x3(0.5f, 0.5f, 0.5f), f32x3(0.5f, 0.5f, 0.5f),
              f32x3(1.0f, 1.0f, 1.0f), f32x3(0.0f, 0.10f, 0.20f));
  if (id == 2)
    col = pal(__ctx, t, f32x3(0.5f, 0.5f, 0.5f), f32x3(0.5f, 0.5f, 0.5f),
              f32x3(1.0f, 1.0f, 1.0f), f32x3(0.3f, 0.20f, 0.20f));
  if (id == 3)
    col = pal(__ctx, t, f32x3(0.5f, 0.5f, 0.5f), f32x3(0.5f, 0.5f, 0.5f),
              f32x3(1.0f, 1.0f, 0.5f), f32x3(0.8f, 0.90f, 0.30f));
  if (id == 4)
    col = pal(__ctx, t, f32x3(0.5f, 0.5f, 0.5f), f32x3(0.5f, 0.5f, 0.5f),
              f32x3(1.0f, 0.7f, 0.4f), f32x3(0.0f, 0.15f, 0.20f));
  if (id == 5)
    col = pal(__ctx, t, f32x3(0.5f, 0.5f, 0.5f), f32x3(0.5f, 0.5f, 0.5f),
              f32x3(2.0f, 1.0f, 0.0f), f32x3(0.5f, 0.20f, 0.25f));
  if (id == 6)
    col = pal(__ctx, t, f32x3(0.8f, 0.5f, 0.4f), f32x3(0.2f, 0.4f, 0.2f),
              f32x3(2.0f, 1.0f, 1.0f), f32x3(0.0f, 0.25f, 0.25f));
  return col;
}
float entity(Context &__ctx, f32x2 uv) {
  uv *= mix(.1f, .7f, smoothstep(0.f, 14.f, tt));
  uv = mix(fract(uv * .2f - .5f) - .5f, uv, smoothstep(14.f, 14.f, tt));
  uv *= f32x2x2(cos(3.141592f / 2.f), sin(3.141592f / 2.f),
                -sin(3.141592f / 2.f), cos(3.141592f / 2.f));
  f32x2 bp = uv;
  uv = abs(uv) - .2f;
  uv *= f32x2x2(cos(-0.5f * tt), sin(-0.5f * tt), -sin(-0.5f * tt),
                cos(-0.5f * tt));
  uv = abs(uv - .2f * sin(tt) * .5f + .5f) - .2f;
  uv *= f32x2x2(cos(0.93f * tt), sin(0.93f * tt), -sin(0.93f * tt),
                cos(0.93f * tt));
  if (tt < 14.f)
    bp.data[0] = abs(bp.data[0]);
  uv += smoothstep(0.0f, 1.f, bp.data[0]);
  float r = length(uv);
  float a = atan(uv.data[1], uv.data[0]);
  float f = cos(a * 4.f);
  return smoothstep(f, (f + .1f), r);
}
f32x2 kalei(Context &__ctx, f32x2 uv) {
  f32x2 bp = uv;
  float n = 10.f;
  float r = 2.f * 3.141592f / n;
  float at = entity(__ctx, uv);
  for (float i = 0.f; i < n; i++) {
    uv = abs(uv);
    uv.data[0] -= (.2f * i * at) + .2f;
    uv *= f32x2x2(
        cos(r * i - .2f * tt - .1f * at), sin(r * i - .2f * tt - .1f * at),
        -sin(r * i - .2f * tt - .1f * at), cos(r * i - .2f * tt - .1f * at));
  }
  uv = abs(uv) - (sin(.15f * tt) + 1.2f);
  return uv;
}
f32x3 spiral(Context &__ctx, f32x2 uv, float i) {
  uv *= f32x2x2(cos(i * 3.141592f + tt * .3f), sin(i * 3.141592f + tt * .3f),
                -sin(i * 3.141592f + tt * .3f), cos(i * 3.141592f + tt * .3f));
  uv += .6f * sin(5.f * uv.swizzle<1, 0>());
  return flower(__ctx, uv) *
         ((getPal(__ctx, 0, i + tt * .2f)) * (getPal(__ctx, 0, i + tt * .2f)));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col = f32x3(0);
  if (iChannelTime[0] < 20.f)
    tt = .7f * iChannelTime[0];
  else
    tt = (.7f * 20.f) + (iChannelTime[0] * 0.99f - 20.f) * .5f;
  if (iChannelTime[0] > 50.35f && iChannelTime[0] < 79.8f) {
    tt += 2.f * curve(__ctx, iChannelTime[0] + .3f, 1.f);
  }
  uv = kalei(__ctx, uv * 5.f);
  float s = 1.f / 40.f;
  for (float i = 0.f; i < 1.f; i += s) {
    float z = fract(i - .1f * tt);
    float fade = smoothstep(0.f, .2f, 1.f - abs(2.f * z - 1.f));
    col += spiral(__ctx, uv * z, i) * fade;
  }
  col = sqrt(col);
  fragColor = f32x4(col, 1.f);
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
