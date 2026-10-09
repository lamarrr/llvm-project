// Port of https://www.shadertoy.com/view/csBfWd; author: flockaroo.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/csBfWd.json.
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
  TextureCube channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float linstep(Context &__ctx, float a, float b, float x) {
  return clamp((x - a) / (b - a), 0.f, 1.f);
}
float smin(Context &__ctx, float a, float b, float k) {
  return mix(a, b, linstep(__ctx, -2.f * k, 2.f * k, a - b)) -
         k * linstep(__ctx, -2.f * k, 0.f, -abs(a + b));
}
float smax(Context &__ctx, float a, float b, float k) {
  return -smin(__ctx, -a, -b, k);
}
float distX(Context &__ctx, f32x3 p) {
  float W = .8f;
  float D = .15f;
  float H = 1.f;
  float w1 = .15f;
  float w2 = .04f;
  float w3 = .03f;
  float sr = .007f;
  float dX11 = .015f;
  float d = 10000.f;
  float d1, d2, dh, dThick;
  dh = abs(p.data[1]) - H * .5f;
  dThick = abs(p.data[2]) - D * .5f;
  f32x3 Xn;
  Xn = normalize(
      f32x3(H + .05f - clamp(-cos(iTime * .3f) + .5f, 0.f, 1.f) * .03f,
            W + clamp(-cos(iTime * .3f) + .5f, 0.f, 1.f) * .03f, 0));
  float d10 = dot(p, Xn);
  d1 = abs(d10) - w1 * .5f;
  d1 = smax(__ctx, d1, dh, sr);
  Xn = normalize(
      f32x3(-(H - .05f + clamp(-cos(iTime * .3f) + .5f, 0.f, 1.f) * .03f),
            W - clamp(-cos(iTime * .3f) + .5f, 0.f, 1.f) * .03f, 0));
  float d20 = dot(p, Xn);
  d2 = abs(d20 + clamp(-cos(iTime * .3f) + .5f, 0.f, 1.f) * (w2 + dX11) * .5f *
                     sign(d10)) -
       w2 * .5f;
  d2 = smax(__ctx, d2, dh, sr);
  d = smin(__ctx, d1, d2, sr);
  d = smax(__ctx, d, -d1 - w3 - clamp(-cos(iTime * .3f) + .5f, 0.f, 1.f) * w1,
           sr);
  d = smax(__ctx, d, dThick, sr);
  d = smax(__ctx, d,
           -abs(d20) + (dX11 * (-1.5f + 2.f * clamp(-cos(iTime * .3f) + .5f,
                                                    0.f, 1.f))),
           sr);
  return d;
}
float dist(Context &__ctx, f32x3 pos) {
  return min(distX(__ctx, pos.swizzle<0, 2, 1>() * f32x3(1, 1, 1) * .75f) /
                 .75f,
             pos.data[2] + .7f);
}
f32x3 getGrad(Context &__ctx, f32x3 pos, float eps) {
  f32x2 d = f32x2(eps, 0);
  float d0 = dist(__ctx, pos);
  return f32x3(dist(__ctx, pos + d.swizzle<0, 1, 1>()) - d0,
               dist(__ctx, pos + d.swizzle<1, 0, 1>()) - d0,
               dist(__ctx, pos + d.swizzle<1, 1, 0>()) - d0) /
         eps;
}
f32x4 march(Context &__ctx, f32x3 &pos, f32x3 dir) {
  float eps = 0.001f;
  for (int cnt = 0; cnt < 100; cnt++) {
    float d = dist(__ctx, pos);
    pos += d * dir;
    if (d < eps) {
      break;
    }
  }
  f32x3 n = getGrad(__ctx, pos, .001f);
  return f32x4(n, 0.f);
}
f32x3x3 rotX(Context &__ctx, float ang) {
  float c = cos(ang), s = sin(ang);
  return f32x3x3(1, 0, 0, 0, c, s, 0, -s, c);
}
f32x3x3 rotZ(Context &__ctx, float ang) {
  float c = cos(ang), s = sin(ang);
  return f32x3x3(c, s, 0, -s, c, 0, 0, 0, 1);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 sc =
      (fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>()) * 2.f - 1.f;
  f32x3 pos = f32x3(0, -3.5f, 0);
  f32x3 dir =
      normalize(2.f * normalize(-pos) +
                f32x3(sc.data[0], 0,
                      sc.data[1] * iResolution.data[1] / iResolution.data[0]));
  float phi = iMouse.data[0] / iResolution.data[0] * 7.f;
  float th = iMouse.data[1] / iResolution.data[1] * 7.f;
  if (iMouse.data[0] == 0.f) {
    phi = sin(iTime * .5f) * .5f;
    th = .3f * sin(iTime * .3f) - .15f;
  }
  f32x3x3 rx = rotX(__ctx, th);
  f32x3x3 rz = rotZ(__ctx, phi);
  pos = rz * (rx * pos);
  dir = rz * (rx * dir);
  f32x4 n = march(__ctx, pos, dir);
  float bg = n.data[3];
  float ao = 1.f;
  ao *= dist(__ctx, pos + n.swizzle<0, 1, 2>() * .02f) / .02f;
  ao *= dist(__ctx, pos + n.swizzle<0, 1, 2>() * .05f) / .05f;
  ao *= dist(__ctx, pos + n.swizzle<0, 1, 2>() * .10f) / .10f;
  ao = pow(ao, .4f);
  f32x3 R = pos - 2.0f * dot(pos, n.swizzle<0, 1, 2>()) * n.swizzle<0, 1, 2>();
  R = -((R * rz) * rx).swizzle<1, 2, 0>();
  f32x3 c = f32x3(1);
  c += n.swizzle<0, 1, 2>() * .1f + .1f;
  c *= texture(__ctx.channel1, R).swizzle<0, 1, 2>() * 1.5f + .4f;
  c *= clamp(-dot(dir, pos) * .7f + .7f, .2f, 1.f);
  c *= ao;
  if (bg >= .5f)
    c = f32x3(.5f, .6f, .75f) - .17f;
  float vign = (1.1f - .3f * length(sc.swizzle<0, 1>()));
  fragColor = f32x4(c * vign, 1);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 1) texture_cube<f32x4> channel1,
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
  Context __ctx{{channel1, sampling1}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
