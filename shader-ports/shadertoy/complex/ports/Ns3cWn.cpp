// Port of https://www.shadertoy.com/view/Ns3cWn; author: antonOTI.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Ns3cWn.json.
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
int GLYPHS = 255;
f32x2x2 makem2(Context &__ctx, float theta) {
  float c = cos(theta);
  float s = sin(theta);
  return f32x2x2(c, -s, s, c);
}
float noise(Context &__ctx, f32x2 x) {
  return texture(__ctx.channel0, x * .01f).data[0];
}
float fbm(Context &__ctx, f32x2 p) {
  float z = 2.f;
  float rz = 0.f;
  f32x2 bp = p;
  for (float i = 1.f; i < 6.f; i++) {
    float soundBias =
        texture(__ctx.channel1, f32x2(1.f - (i / 6.f), 0.f)).data[0];
    rz += abs((noise(__ctx, p) * (1.f - soundBias * .2f) - 0.5f) * 2.f) / z +
          soundBias * .02f;
    soundBias = soundBias * soundBias;
    soundBias = 1.2f - soundBias;
    z = z * 2.f * soundBias;
    p = p * 2.f;
  }
  return rz;
}
float eldenRing(Context &__ctx, f32x2 uv) {
  float fontSize = .015f;
  float circleSep = .05f;
  float sf = 4.f;
  float f = 0.f;
  if ((GLYPHS & 1) != 0)
    f = max(f, (1.f -
                smoothstep(-1.f / iResolution.data[1] * sf,
                           1.f / iResolution.data[1] * sf,
                           abs(length(uv) - .155f) - (fontSize * .25f / 2.f))));
  if ((GLYPHS & 2) != 0)
    f = max(f, (1.f - smoothstep(-1.f / iResolution.data[1] * sf,
                                 1.f / iResolution.data[1] * sf,
                                 abs(length(uv - f32x2(0, circleSep)) - .14f) -
                                     (fontSize / 2.f))));
  if ((GLYPHS & 4) != 0)
    f = max(f, (1.f - smoothstep(-1.f / iResolution.data[1] * sf,
                                 1.f / iResolution.data[1] * sf,
                                 abs(length(uv - f32x2(circleSep * -0.86602f,
                                                       circleSep * -0.49999f)) -
                                     .14f) -
                                     (fontSize / 2.f))));
  if ((GLYPHS & 8) != 0)
    f = max(f, (1.f - smoothstep(-1.f / iResolution.data[1] * sf,
                                 1.f / iResolution.data[1] * sf,
                                 abs(length(uv - f32x2(circleSep * 0.86602f,
                                                       circleSep * -0.49999f)) -
                                     .14f) -
                                     (fontSize / 2.f))));
  if ((GLYPHS & 16) != 0)
    f = max(f, (1.f - smoothstep(-1.f / iResolution.data[1] * sf,
                                 1.f / iResolution.data[1] * sf,
                                 abs(uv.data[0]) - (fontSize * .5f))) *
                   step(abs(uv.data[1]), .25f));
  float parSize = fontSize * 2.6f;
  if ((GLYPHS & 32) != 0)
    f = max(f, 1.f - smoothstep(
                         -1.f / iResolution.data[1] * sf,
                         1.f / iResolution.data[1] * sf,
                         abs((.25f + 1.3f * abs(uv.data[0]) * abs(uv.data[0])) -
                             uv.data[1]) -
                             (parSize * (.0125f * sf -
                                         abs(uv.data[0] * iResolution.data[1] /
                                             iResolution.data[0])))));
  if ((GLYPHS & 64) != 0)
    f = max(f,
            1.f - smoothstep(
                      -1.f / iResolution.data[1] * sf,
                      1.f / iResolution.data[1] * sf,
                      abs((-.09f + 1.3f * abs(uv.data[0]) * abs(uv.data[0])) -
                          uv.data[1]) -
                          (parSize * 1.2f *
                           (.0125f * sf - abs(uv.data[0] * iResolution.data[1] /
                                              iResolution.data[0])))));
  if ((GLYPHS & 128) != 0)
    f = max(f,
            1.f - smoothstep(
                      -1.f / iResolution.data[1] * sf,
                      1.f / iResolution.data[1] * sf,
                      abs((-.25f + 1.2f * abs(uv.data[0]) * abs(uv.data[0])) -
                          uv.data[1]) -
                          (parSize *
                           (.025f * sf - abs(uv.data[0] * iResolution.data[1] /
                                             iResolution.data[0])))));
  return f;
}
float radagon(Context &__ctx, f32x2 uv) {
  uv = f32x2(uv.data[0] * 0.7071f - uv.data[1] * 0.7071f,
             uv.data[0] * 0.7071f + uv.data[1] * 0.7071f);
  uv = abs(mod(uv * 12.f, 1.f) - .5f);
  return (.25f - pow(max(uv.data[0] * uv.data[1], 0.0001f), .75f)) * 4.f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.data[1] -
             f32x2(iResolution.data[0] / iResolution.data[1] * .5f, .5f);
  f32x2 disp = f32x2(fbm(__ctx, uv), fbm(__ctx, uv * .7f - iTime * 0.0125f));
  disp = pow(disp, f32x2(2.f)) * .025f;
  uv += disp;
  float f = eldenRing(__ctx, uv);
  f += radagon(__ctx, uv) * pow(max(.7f - length(uv), .0001f), 2.2f) * 3.5f *
       fbm(__ctx, uv * .8f - .5f);
  f *= (1.f + length(disp) * 400.f) / 2.f;
  fragColor = f32x4(2.f * f, 2.f * f * f * f, f * f * f * f, 1.f);
  fragColor = pow(fragColor, f32x4(2.2f));
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0,
               __binding__(0, 1) texture_2d<f32x4> channel1,
               __binding__(1, 1) sampler sampling1) {
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
