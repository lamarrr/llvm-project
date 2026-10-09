// Port of https://www.shadertoy.com/view/dl3BWn; author: DrieStone.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/dl3BWn.json.
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
f32x4 getCol(Context &__ctx, f32x2 pos) {
  f32x2 uv = pos / iResolution.swizzle<0, 1>().swizzle<0, 1>();
  f32x4 c1 = texture(__ctx.channel0, uv);
  f32x4 e = smoothstep(f32x4(-0.05f), f32x4(-0.0f), f32x4(uv, f32x2(1) - uv));
  c1 =
      mix(f32x4(1, 1, 1, 0), c1, e.data[0] * e.data[1] * e.data[2] * e.data[3]);
  float d =
      clamp(dot(c1.swizzle<0, 1, 2>(), f32x3(-.5f, 1.f, -.5f)), 0.0f, 1.0f);
  f32x4 c2 = f32x4(.7f);
  return min(mix(c1, c2, 1.8f * d), .7f);
}
f32x4 getColHT(Context &__ctx, f32x2 pos) {
  return smoothstep(0.795f, 1.05f, getCol(__ctx, pos) * .8f + .2f + 1.0f);
}
float getVal(Context &__ctx, f32x2 pos) {
  f32x4 c = getCol(__ctx, pos);
  return pow(dot(c.swizzle<0, 1, 2>(), f32x3(.333f)), 1.f) * 1.f;
}
f32x2 getGrad(Context &__ctx, f32x2 pos, float eps) {
  f32x2 d = f32x2(eps, 0.f);
  return f32x2(getVal(__ctx, pos + d.swizzle<0, 1>()) -
                   getVal(__ctx, pos - d.swizzle<0, 1>()),
               getVal(__ctx, pos + d.swizzle<1, 0>()) -
                   getVal(__ctx, pos - d.swizzle<1, 0>())) /
         eps / 2.f;
}
float lum(Context &__ctx, f32x3 c) { return dot(c, f32x3(0.3f, 0.59f, 0.11f)); }
f32x3 clipcolor(Context &__ctx, f32x3 c) {
  float l = lum(__ctx, c);
  float n = min(min(c.data[0], c.data[1]), c.data[2]);
  float x = max(max(c.data[0], c.data[1]), c.data[2]);
  if (n < 0.0f) {
    c.data[0] = l + ((c.data[0] - l) * l) / (l - n);
    c.data[1] = l + ((c.data[1] - l) * l) / (l - n);
    c.data[2] = l + ((c.data[2] - l) * l) / (l - n);
  }
  if (x > 1.25f) {
    c.data[0] = l + ((c.data[0] - l) * (1.0f - l)) / (x - l);
    c.data[1] = l + ((c.data[1] - l) * (1.0f - l)) / (x - l);
    c.data[2] = l + ((c.data[2] - l) * (1.0f - l)) / (x - l);
  }
  return c;
}
f32x3 setlum(Context &__ctx, f32x3 c, float l) {
  float d = l - lum(__ctx, c);
  c = c + f32x3(d);
  return clipcolor(__ctx, 0.85f * c);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 pos = fragCoord;
  f32x3 col = f32x3(0);
  f32x3 col2 = f32x3(0);
  float sum = 0.f;
  for (int i = 0; i < 3; i++) {
    float ang = 6.28318530717959f / float(3) * (float(i) + 0.8f);
    f32x2 v = f32x2(cos(ang), sin(ang));
    for (int j = 0; j < 9; j++) {
      f32x2 dpos = v.swizzle<1, 0>() * f32x2(1, -1) * float(j) *
                   iResolution.data[1] / 920.f;
      f32x2 dpos2 = 5.0f * (v.swizzle<0, 1>() * float(j * j) / float(9) * .5f *
                            iResolution.data[1] / 920.f);
      f32x2 g;
      float fact;
      float fact2;
      float s = 3.5f;
      f32x2 pos2 = pos + s * dpos + dpos2;
      g = getGrad(__ctx, pos2, 0.08f);
      fact = dot(g, v) - .5f * abs(dot(g, v.swizzle<1, 0>() * f32x2(1, -1)));
      fact2 =
          dot(normalize(g + f32x2(.0001f)), v.swizzle<1, 0>() * f32x2(1, -1));
      fact = clamp(fact, 0.f, .05f);
      fact2 = abs(fact2);
      fact *= 1.f - float(j) / float(9);
      col += fact;
      col2 += fact2;
      sum += fact2;
    }
  }
  col /= float(9 * 3) * 0.65f / sqrt(iResolution.data[1]);
  col2 /= sum;
  col.data[0] *= 1.6f;
  col.data[0] = 1.f - col.data[0];
  col.data[0] *= col.data[0] * col.data[0];
  f32x2 s = sin(pos.swizzle<0, 1>() * .1f / sqrt(iResolution.data[1] / 720.f));
  f32x3 karo = f32x3(1);
  karo -= .75755f * f32x3(.25f, .1f, .1f) * dot(exp(-s * s * 80.f), f32x2(1.f));
  float r =
      length(pos - iResolution.swizzle<0, 1>() * .5f) / iResolution.data[0];
  float vign = 1.f - r * r * r;
  fragColor = f32x4(f32x3(col.data[0] * col2 * karo * vign), 1);
  f32x4 origCol = texture(__ctx.channel0, gl_FragCoord.swizzle<0, 1>() /
                                              iResolution.swizzle<0, 1>());
  f32x4 overlayColor = f32x4(0.3755f, 0.05f, 0.f, 0.0f) * origCol;
  fragColor = f32x4(setlum(__ctx, 1.25f * overlayColor.swizzle<0, 1, 2>(),
                           lum(__ctx, fragColor.swizzle<0, 1, 2>())) *
                        1.0f,
                    0.f);
  fragColor.set_swizzle<0, 1, 2>(
      fragColor.swizzle<0, 1, 2>() -
      (0.75f - clamp(origCol.data[0] + origCol.data[1] + origCol.data[2], 0.0f,
                     0.75f)));
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
