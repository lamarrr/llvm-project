// Port of https://www.shadertoy.com/view/NdfBDs; author: drschizzo.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NdfBDs.json.
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
  TextureCube channel0;
  Texture2D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float opSmoothIntersection(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f - 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  return mix(d2, d1, h) + k * h * (1.0f - h);
}
float gyro(Context &__ctx, f32x3 p) {
  float size = 10.f * (.8f + .6f * sin(iTime / 2.47f + 1.6f));
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (f32x2x2(cos(iTime), sin(iTime),
                                                   -sin(iTime), cos(iTime))));
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (f32x2x2(cos(iTime), sin(iTime),
                                                   -sin(iTime), cos(iTime))));
  p *= size;
  return abs(.5f * dot(sin(p), cos(p.swizzle<1, 2, 0>())) / size) - .02f;
}
bool isgyro = true;
float map(Context &__ctx, f32x3 p) {
  float d = p.data[1] + 1.1f;
  float c2 = abs(length(p) - 1.f) - .05f;
  c2 = opSmoothIntersection(__ctx, c2, gyro(__ctx, p), .02f);
  d = abs(length(p) - 6.f) - .1f;
  if (d < c2 - .5f)
    isgyro = false;
  return min(d, c2);
}
f32x3 normal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(0, .001f);
  float a = map(__ctx, p);
  return (normalize(a - f32x3(map(__ctx, p - e.swizzle<1, 0, 0>()),
                              map(__ctx, p - e.swizzle<0, 1, 0>()),
                              map(__ctx, p - e.swizzle<0, 0, 1>()))));
}
f32x3x3 cam(Context &__ctx, f32x3 ro, f32x3 lp) {
  f32x3 cd = normalize(lp - ro);
  f32x3 cr = normalize(cross(f32x3(0, 1, 0), cd));
  f32x3 cu = normalize(cross(cd, cr));
  return f32x3x3(-cr, cu, -cd);
}
f32x3 normalmap(Context &__ctx, f32x2 p) {
  float v = texture(__ctx.channel1, p).data[0];
  float vx = v - texture(__ctx.channel1, p - f32x2(0.001f, 0)).data[0];
  float vy = v - texture(__ctx.channel1, p - f32x2(0.f, 0.001f)).data[0];
  return normalize(f32x3(vx, 0.f, vy));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col = 0.5f + 0.5f * cos(iTime + uv.swizzle<0, 1, 0>() + f32x3(0, 2, 4));
  f32x3 ro = f32x3(0.f, 1.f, 3.f);
  f32x3 rd = normalize(cam(__ctx, ro, f32x3(0.f, 0.f, 0.f)) *
                       normalize(f32x3(uv, -1)));
  float dt = 0.f;
  for (float i = 0.f; i < 200.f; i++) {
    float d = map(__ctx, ro + dt * rd);
    dt += d;
    if (d < .001f || d > 200.f)
      break;
  }
  float dust = 0.f;
  f32x3 pos = ro;
  for (float i = 0.f; i < 20.f; i++) {
    float s = gyro(__ctx, normalize(pos));
    s *= 50.f * smoothstep(.8f, 1.f, length(pos));
    s = clamp(0.f, 1.f, s);
    dust += s * (1.f / max(1.f, (length(pos))));
    dust += pow(smoothstep(.4f, .001f, length(pos)) * 10.f, 3.f);
    pos += rd;
  }
  if (dt < 200.f) {
    f32x3 lpos = f32x3(0, 0, 0);
    f32x3 p = ro + dt * rd;
    if (!isgyro) {
      f32x3 n = normal(__ctx, p);
      float s = gyro(__ctx, normalize(-p));
      float w = .04f * length(p / 3.f);
      s = smoothstep(-w, w * 2.f, s) * .3f;
      float diff = dot(n, normalize(lpos - p));
      f32x3 r = f32x3(1.f);
      fragColor = f32x4((f32x3(.4f, .8f, .9f) * diff * s), 1.f);
    } else {
      f32x3 n = normal(__ctx, p);
      float diff = dot(n, normalize(lpos - p));
      f32x3 colext = f32x3(.2f, .3f, .9f) *
                     texture(__ctx.channel0, normalize(reflect((rd), n)))
                         .swizzle<0, 1, 2>() *
                     .2f;
      f32x3 colint = f32x3(.95f, .2f, .1f) * clamp((diff), .0f, 1.f);
      if (diff < 0.f) {
      }
      dust = dust * smoothstep(-.4f, .4f, diff);
      fragColor = f32x4(mix(colext, colint, smoothstep(-.2f, .2f, diff)), 1.f);
    }
  } else
    fragColor = f32x4(f32x3(0.f), 1.0f);
  fragColor = pow((fragColor + f32x4(dust * .05f) * f32x4(.9f, .7f, .7f, 1.f)),
                  f32x4(.45f));
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_cube<f32x4> channel0,
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
