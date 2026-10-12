// Port of https://www.shadertoy.com/view/cttGRH; author: NinjaKoala.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cttGRH.json.
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

float cubic_bezier_normal_iteration(Context &__ctx, float t, f32x2 a0, f32x2 a1,
                                    f32x2 a2, f32x2 a3) {
  f32x2 a_2 = a2 + t * a3;
  f32x2 a_1 = a1 + t * a_2;
  f32x2 b_2 = a_2 + t * a3;
  f32x2 uv_to_p = a0 + t * a_1;
  f32x2 tang = a_1 + t * b_2;
  float l_tang = dot(tang, tang);
  return t - dot(tang, uv_to_p) / l_tang;
}
float cubic_bezier_dis(Context &__ctx, f32x2 uv, f32x2 p0, f32x2 p1, f32x2 p2,
                       f32x2 p3) {
  f32x2 a3 = (-p0 + 3.f * p1 - 3.f * p2 + p3);
  f32x2 a2 = (3.f * p0 - 6.f * p1 + 3.f * p2);
  f32x2 a1 = (-3.f * p0 + 3.f * p1);
  f32x2 a0 = p0;
  float d0 = 1e38f;
  float t0 = 0.f;
  float t;
  for (int i = 0; i < 3; i++) {
    t = t0;
    for (int j = 0; j < 3; j++) {
      t = cubic_bezier_normal_iteration(__ctx, t, a0 - uv, a1, a2, a3);
    }
    t = clamp(t, 0.f, 1.f);
    f32x2 uv_to_p = (((a3 * t) + a2) * t + a1) * t + a0 - uv;
    d0 = min(d0, dot(uv_to_p, uv_to_p));
    t0 += .5f;
  }
  return sqrt(d0);
}
float sdBox(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.0f)) + min(max(d.data[0], d.data[1]), 0.0f);
}
f32x2x2 rot(Context &__ctx, float t) {
  return f32x2x2(cos(t), sin(t), -sin(t), cos(t));
}
float line_segment_dis(Context &__ctx, f32x2 p, f32x2 a, f32x2 b, float fac) {
  f32x2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, fac);
  return length(pa - ba * h);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  uv -= f32x2(.5f, .55f);
  uv.data[1] *= iResolution.data[1] / iResolution.data[0];
  float border = 2.f / iResolution.data[0];
  float zoom = 3.f;
  uv *= zoom;
  border *= zoom;
  float dis = 1e38f;
  dis = min(dis, abs(length(uv - f32x2(.2f, 0)) - .1f) - .01f);
  dis = min(dis, abs(length(uv + f32x2(.2f, 0)) - .1f) - .01f);
  dis = min(dis, line_segment_dis(__ctx, uv, f32x2(-.2f, 0), f32x2(-.11f, .17f),
                                  1.f) -
                     .01f);
  dis =
      min(dis, line_segment_dis(__ctx, uv, f32x2(.2f, 0), f32x2(.05f, 0), 1.f) -
                   .01f);
  dis = min(
      dis, line_segment_dis(__ctx, uv, f32x2(.2f, 0), f32x2(.125f, .17f), 1.f) -
               .01f);
  dis = min(dis, line_segment_dis(__ctx, uv, f32x2(.05f, 0), f32x2(.125f, .17f),
                                  1.2f) -
                     .01f);
  dis = min(dis, line_segment_dis(__ctx, uv, f32x2(-.11f, .17f),
                                  f32x2(.125f, .17f), 1.f) -
                     .01f);
  dis = min(dis, line_segment_dis(__ctx, uv, f32x2(-.12f, .15f), f32x2(.05f, 0),
                                  1.f) -
                     .01f);
  dis = min(dis, line_segment_dis(__ctx, uv, f32x2(.11f, .21f),
                                  f32x2(.16f, .21f), 1.f) -
                     .01f);
  dis = min(dis, cubic_bezier_dis(__ctx, uv, f32x2(-.11f, .17f),
                                  f32x2(-.11f, .17f) + .4f * f32x2(.09f, .17f),
                                  f32x2(-.1f, .24f), f32x2(-.03f, .24f)) -
                     .01f);
  f32x3 color =
      mix(f32x3(.08f, .28f, .53f), f32x3(1),
          smoothstep(0.f, border, length(uv - f32x2(0, .05f)) - .37f));
  color = mix(f32x3(1), color, smoothstep(0.f, border, dis));
  dis = sdBox(__ctx, uv + f32x2(.3f, .4f), f32x2(.07f, .015f));
  dis = min(dis, sdBox(__ctx, uv + f32x2(.3f, .6f), f32x2(.07f, .015f)));
  dis = min(dis, sdBox(__ctx, (uv + f32x2(.3f, .5f)) * rot(__ctx, -.548f),
                       f32x2(.015f, .11f)));
  dis = min(dis,
            cubic_bezier_dis(__ctx, uv, -f32x2(.185f, .5f), -f32x2(.185f, .64f),
                             -f32x2(.045f, .64f), -f32x2(.045f, .5f)) -
                .015f);
  dis = min(dis,
            cubic_bezier_dis(__ctx, uv, -f32x2(.185f, .5f), -f32x2(.185f, .36f),
                             -f32x2(.045f, .36f), -f32x2(.045f, .5f)) -
                .015f);
  dis = min(dis, sdBox(__ctx, uv - f32x2(.17f, -.5f), f32x2(.015f, .116f)));
  dis = min(dis, sdBox(__ctx, uv - f32x2(.02f, -.5f), f32x2(.015f, .116f)));
  dis = min(dis, sdBox(__ctx, (uv - f32x2(.095f, -.5f)) * rot(__ctx, .595f),
                       f32x2(.015f, .13f)));
  dis = min(dis,
            abs(sdBox(__ctx, uv + f32x2(0, .08f), f32x2(.6f)) - .02f) - .02f);
  dis = min(dis, sdBox(__ctx, uv - f32x2(.3f, -.4f), f32x2(.07f, .015f)));
  dis = min(dis, sdBox(__ctx, uv - f32x2(.3f, -.6f), f32x2(.07f, .015f)));
  dis = min(dis, sdBox(__ctx, uv - f32x2(.29f, -.5f), f32x2(.06f, .015f)));
  dis = min(dis, sdBox(__ctx, uv - f32x2(.245f, -.5f), f32x2(.015f, .09f)));
  color = mix(f32x3(0), color, smoothstep(0.f, border, dis));
  fragColor = f32x4(color, 1);
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
