// Port of https://www.shadertoy.com/view/mlByzG; author: totetmatt.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mlByzG.json.
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
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x3 pal(Context &__ctx, float t) {
  return .5f + .5f * cos(6.28f * (t + f32x3(0, .3f, .7f)));
}
float diam2(Context &__ctx, f32x2 p, float s) {
  p = abs(p);
  return (p.data[0] + p.data[1] - s) * inversesqrt(3.f);
}
float timer;
float bpm;
f32x3 erot(Context &__ctx, f32x3 p, f32x3 ax, float t) {
  return mix(dot(ax, p) * ax, p, cos(t)) + cross(ax, p) * sin(t);
}
float tru(Context &__ctx, f32x3 p) {
  f32x3 id = floor(p) + .5f;
  f32x3 gv = p - id;
  gv.data[0] *=
      fract(452.6f * sin(dot(id, f32x3(452.5f, 985.5f, 487.56f)))) > .5f ? -1.f
                                                                         : 1.f;
  gv.set_swizzle<0, 2>(gv.swizzle<0, 2>() -
                       (.5f * (gv.data[0] > -gv.data[2] ? 1.f : -1.f)));
  return max(abs(gv.data[1]) - .05f,
             abs(diam2(__ctx, gv.swizzle<0, 2>(), .5f) * 4.f) - .05f);
}
f32x3 path(Context &__ctx, float t) {
  f32x3 o = f32x3(0);
  o.data[0] += asin(sin(t * .45f)) * .5f;
  o.data[0] += asin(cos(t * .75f)) * .45f;
  o.data[1] += asin(cos(t * .95f)) * .33f;
  o.data[1] += asin(sin(t * .35f)) * .44f;
  return o;
}
f32x2 sdf(Context &__ctx, f32x3 p) {
  f32x2 h;
  f32x3 hp = p;
  hp.data[2] -= timer;
  hp += path(__ctx, floor((hp.data[2] * .025f + .5f)) + timer);
  h.data[0] =
      length(hp) - 1.f -
      .2f * mix(0.f, dot(sin(hp + iTime), cos(hp.swizzle<2, 0, 1>() * 5.f)),
                tanh(sin(bpm + iTime) * 10.f) * .5f + .5f);
  ;
  h.data[1] = 1.f;
  f32x2 t;
  f32x3 tp = p;
  tp += path(__ctx, tp.data[2]);
  tp /= 4.f;
  t.data[0] = min(tru(__ctx, tp.swizzle<2, 0, 1>()),
                  min(tru(__ctx, tp), tru(__ctx, tp.swizzle<1, 2, 0>())));
  t.data[1] = 2.f;
  h = t.data[0] < h.data[0] ? t : h;
  return h;
}
f32x3 norm(Context &__ctx, f32x3 p, float ee) {
  f32x2 e = f32x2(-ee, ee);
  return normalize(
      e.swizzle<0, 1, 1>() * sdf(__ctx, p + e.swizzle<0, 1, 1>()).data[0] +
      e.swizzle<1, 0, 1>() * sdf(__ctx, p + e.swizzle<1, 0, 1>()).data[0] +
      e.swizzle<1, 1, 0>() * sdf(__ctx, p + e.swizzle<1, 1, 0>()).data[0] +
      e.swizzle<0, 0, 0>() * sdf(__ctx, p + e.swizzle<0, 0, 0>()).data[0]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  float rnd =
      float(
          ((floatBitsToInt(uv.data[0]) * floatBitsToInt(fragCoord.data[1])) ^
           (floatBitsToInt(uv.data[1]) * floatBitsToInt(fragCoord.data[0])))) /
      2.19e9f;
  bpm = floor(iTime * 115.f / 60.f) + fract(iTime * 115.f / 60.f);
  bpm += rnd * .1f;
  bpm = floor(bpm) + smoothstep(.0f, 1.f, pow(fract(bpm), .4f));
  timer += iTime + bpm;
  f32x3 col = f32x3(0.f);
  f32x2 puv = uv * fragCoord.data[0] / iResolution.data[0];
  puv += .5f;
  float q =
      texture(__ctx.channel0, fract(floor(puv.swizzle<0, 0>() * 10.f) / 10.f))
          .data[0] *
      .0225f;
  float st;
  col += sqrt((st = step(-(abs(uv.data[1]) - .5f), sqrt(q))) * sqrt(q));
  if (st > .00f)
    uv *= (1.f + sqrt(q) * 5.f);
  f32x3 ro = f32x3(0, 0, -5), rt = f32x3(0);
  ro = erot(__ctx, ro, f32x3(0.f, 1.f, 0), bpm * .1f);
  ro.data[2] += timer - tanh(cos(bpm) * 5.f);
  ro += path(__ctx, ro.data[2]) * 2.f;
  rt.data[2] += timer;
  rt += path(__ctx, ro.data[2]);
  f32x3 z = normalize(rt - ro), x = normalize(cross(z, f32x3(0.f, -1.f, 0))),
        y = cross(z, x);
  f32x3 rd =
      f32x3x3(x, y, z) * normalize(f32x3(uv, 1.f + .5f * tanh(sin(bpm) * 5.f)));
  f32x3 rp = ro;
  f32x3 light = f32x3(1.f, 2.f, -3.f + timer);
  f32x3 acc = f32x3(0.f);
  for (float i = 0.f; i++ < 128.f;) {
    f32x2 d = sdf(__ctx, rp);
    if (d.data[1] == 2.f) {
      acc += f32x3(.03f, .04f, .05f) * exp(10.f * -abs(d.data[0])) /
             (20.f - 19.f * exp(-3.f * fract(iTime + rp.data[2])));
      d.data[0] = max(.001f, abs(d.data[0]));
    }
    rp += rd * d.data[0];
    if (d.data[0] < .001f) {
      f32x3 n = norm(__ctx, rp, .001f);
      f32x3 nl = normalize(light - rp);
      float dif = max(0.f, dot(nl, n));
      float spc = pow(max(0.f, dot(rd, reflect(nl, n))), 16.f);
      col = f32x3(.75f) * dif + spc;
      if (d.data[1] == 1.f) {
        col = col * (col);
        rd = reflect(rd, n);
        rp += rd * .1f;
        continue;
      }
      break;
    }
  }
  fragColor = f32x4(sqrt(col + acc), 1.f);
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
