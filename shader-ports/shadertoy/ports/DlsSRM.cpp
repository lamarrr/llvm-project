// Port of https://www.shadertoy.com/view/DlsSRM; author: totetmatt.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DlsSRM.json.
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
struct Context {
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

float box3(Context &__ctx, f32x3 p, f32x3 b) {
  p = abs(p) - b;
  return length(max(f32x3(0.f), p)) +
         min(0.f, max(p.data[0], max(p.data[1], p.data[2])));
}
f32x3 erot(Context &__ctx, f32x3 p, f32x3 ax, float t) {
  return mix(dot(ax, p) * ax, p, cos(t)) + cross(ax, p) * sin(t);
}
float txt(Context &__ctx, f32x2 uv) {
  float d =
      min(length(fract(uv.data[0]) - .5f), length(fract(uv.data[1]) - .5f)) -
      .1f;
  return max(.05f, smoothstep(sqrt(ddx(d) * ddx(d) + ddy(d) * ddy(d)), 0.f,
                              abs(d) - .01f));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord.swizzle<0, 1>() - .5f * iResolution.swizzle<0, 1>()) /
             iResolution.data[1];
  float tt = texture(__ctx.channel0, f32x2(.1f)).data[0] * .5f;
  f32x3 col = f32x3(0.f);
  f32x3 p, d = normalize(f32x3(uv, 1.f + (-tt + .8f) * sqrt(length(uv))));
  for (float i = 0.f, e = 0.f, g = 0.f; i++ < 50.f;) {
    p = d * g;
    float z = p.data[2];
    p.data[2] -= 10.f;
    ;
    p = erot(__ctx, p,
             normalize(f32x3(2.f * cos(iTime), 1.f, sin(iTime) * -.66f)),
             floor(iTime * .2f) + pow(fract(iTime * .2f), .25f));
    float mj = 1.f;
    for (float j = 0.f; j++ < 5.f;) {
      p = abs(p) - f32x3(1.7f);
      p *= 1.4f;
      mj *= 1.4f;
      p = erot(__ctx, p, normalize(f32x3(-.5f, 5.f, 1.5f * sin(j))),
               j * j * .785f);
    }
    float h = box3(__ctx, p, f32x3(1.f));
    h = min(h, min(min(length(p.swizzle<1, 2>()), length(p.swizzle<0, 2>())),
                   length(p.swizzle<0, 1>())) -
                   0.1f);
    g += e = max(.001f, abs(h / mj));
    float dd = txt(__ctx, p.swizzle<0, 1>()) + txt(__ctx, p.swizzle<0, 2>()) +
               txt(__ctx, p.swizzle<1, 2>());
    col += (erot(__ctx, f32x3(.1f, .2f, .3f),
                 normalize(f32x3(.3f, .2f + sin(i * 5.f), .3f)),
                 i + iTime + p.data[2]) *
            (.05f + dd * sin(z + iTime))) /
           exp(i * i * e);
  }
  fragColor = f32x4(sqrt(col), 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
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
