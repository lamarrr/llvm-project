// Port of https://www.shadertoy.com/view/ml3Xz7; author: z0rg.
// Dataset license metadata: cc-by-nc-4.0; original retained in
// upstream/ml3Xz7.json.
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
  Texture2D channel0;
  Texture2D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float _sqr(Context &__ctx, f32x2 uv, f32x2 s) {
  f32x2 l = abs(uv) - s;
  return max(l.data[0], l.data[1]);
}
float hash(Context &__ctx, float seed) {
  return fract(sin(seed * 123.456f) * 123.456f);
}
f32x2x2 r2d(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, -s, s, c);
}
float smin(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  return mix(d2, d1, h) - k * h * (1.0f - h);
}
f32x3 rdrtree(Context &__ctx, f32x2 uv, f32x3 col, f32x3 light) {
  float sharp = iResolution.data[0] * .5f;
  uv.data[0] += sin(uv.data[1] * 5.f + uv.data[0] * 2.f + iTime) * .02f *
                clamp(uv.data[1], 0.f, 1.f);
  f32x2 p = uv;
  float trunk = 1000.f;
  for (float i = 0.f; i < 6.f; ++i) {
    float off = mix(.3f, .05f, i / 7.f);
    p += f32x2(0.f, -off * .5f);
    p *= r2d(__ctx, .5f * clamp(i, 0.f, 1.f));
    p += f32x2(0.f, -off * .5f);
    p.data[0] += sin(p.data[1] * 20.f) * .02f;
    p.data[0] = abs(p.data[0]);
    f32x2 pos = p;
    float shape = _sqr(__ctx, pos, f32x2(.01f, off * .45f));
    shape -= .0001f - texture(__ctx.channel0, uv * .15f).data[0] * .007f;
    if (i < 4.5f) {
      trunk = smin(__ctx, trunk, shape, .05f);
    } else {
      float cir = length(p) - .06f;
      col = mix(col, f32x3(0.957f, 0.333f, 0.286f) * .7f + light,
                1.f - clamp(cir * sharp, 0.f, 1.f));
    }
  }
  col = mix(col, f32x3(0.086f, 0.173f, 0.145f) * .05f + light * .75f,
            1.f - clamp(trunk * sharp, 0.f, 1.f));
  return col;
}
float cloud(Context &__ctx, f32x2 uv) {
  return (texture(__ctx.channel0, uv * .05f).data[0] * .5f +
          texture(__ctx.channel0,
                  uv * .1f * r2d(__ctx, iTime * .05f + uv.data[0] * .2f))
                  .data[0] *
              .2f +
          texture(__ctx.channel0,
                  uv * .2f * r2d(__ctx, -iTime * .02f + uv.data[1] * .5f))
                  .data[0] *
              .2f +
          texture(__ctx.channel0, uv * .33f).data[0] * .07f) *
         clamp(texture(__ctx.channel0, uv * .01f).data[0] - .2f, 0.f, 1.f);
}
f32x3 rdr(Context &__ctx, f32x2 uv) {
  float sharp = iResolution.data[0] * .5f;
  f32x3 col =
      f32x3(0.110f, 0.153f, 0.133f) * (1.f - clamp(length(uv - .5f), 0.f, 1.f));
  float fins = clamp(200.f * sin(uv.data[1] * 200.f), 0.f, 1.f);
  float shadow =
      rdrtree(__ctx, uv * 2.f - f32x2(1.f, -0.5f), f32x3(1.f), f32x3(0.f))
          .data[2];
  float boxwin = _sqr(__ctx, uv - f32x2(.4f, .5f), f32x2(.2f, .4f));
  col = mix(col, f32x3(0.957f, 0.333f, 0.286f),
            clamp(.7f + fins, 0.f, 1.f) * shadow *
                (1.f - clamp(boxwin * sharp, 0.f, 1.f)));
  float boxwin2 = _sqr(__ctx, uv - f32x2(.4f, .5f) - .2f, f32x2(.2f, .4f));
  f32x3 light = 2.f * f32x3(0.957f, 0.333f, 0.286f) *
                (1.f - clamp(boxwin2 * sharp, 0.f, 1.f));
  col = rdrtree(__ctx, uv * 2.f - f32x2(1.2f, 0.2f), col,
                .25f * light * clamp(.7f + fins, 0.f, 1.f));
  float box = _sqr(__ctx, uv - f32x2(.6f, .1f), f32x2(.05f, .1f));
  col = mix(col, f32x3(0.f), 1.f - clamp(box * sharp, 0.f, 1.f));
  col += .5f * f32x3(0.871f, 0.647f, 0.627f) * cloud(__ctx, uv) *
         clamp(.7f * length(uv - .5f), 0.f, 1.f);
  float boxwin3 = _sqr(__ctx, uv - f32x2(.4f, .5f) - .25f, f32x2(.2f, .4f));
  float finssmoke =
      mix(1.f, clamp((sin(uv.data[1] * 150.f) * .5f + .5f) + .5f, 0.f, 1.f),
          1.f - clamp(boxwin3 * sharp * .1f, 0.f, 1.f));
  float beat = texture(__ctx.channel1, f32x2(0.1f, 0.f)).data[0];
  col += mix(0.5f, 2.f, beat) * finssmoke * 2.f *
         f32x3(0.631f, 0.690f, 0.871f) * cloud(__ctx, -uv * 1.f) *
         (1.f - clamp(1.7f * length(uv - .5f), 0.f, 1.f));
  col += 2.f * f32x3(0.957f, 0.333f, 0.286f) *
         (1.f - clamp(length(uv - .5f) * 1.5f, 0.f, 1.f)) * .5f;
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord - .5f * iResolution.swizzle<0, 1>()) /
             iResolution.swizzle<0, 0>();
  f32x3 col = f32x3(0.f);
  col = rdr(__ctx, uv * 1.5f + .5f);
  col = pow(col, f32x3(1.5f));
  fragColor = f32x4(col, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
               __binding(1, 0) sampler sampling0,
               __binding(0, 1) texture_2d<f32x4> channel1,
               __binding(1, 1) sampler sampling1) {
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
  Context __ctx{{channel0, sampling0}, {channel1, sampling1}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
