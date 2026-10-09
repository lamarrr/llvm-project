// Port of https://www.shadertoy.com/view/NlK3Rc; author: z0rg.
// Dataset license metadata: cc-by-nc-4.0; original retained in
// upstream/NlK3Rc.json.
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
f32x2x2 r2d(Context &__ctx, float a) {
  float c = cos(a);
  float s = sin(a);
  return f32x2x2(c, -s, s, c);
}
f32x3 getCam(Context &__ctx, f32x3 rd, f32x2 uv) {
  float fov = 1.f;
  f32x3 r = normalize(cross(rd, f32x3(0.f, 1.f, 0.f)));
  f32x3 u = normalize(cross(rd, r));
  return normalize(rd + (r * uv.data[0] + u * uv.data[1]) * fov);
}
f32x2 _min(Context &__ctx, f32x2 a, f32x2 b) {
  if (a.data[0] < b.data[0])
    return a;
  return b;
}
float _cube(Context &__ctx, f32x3 p, f32x3 s) {
  f32x3 l = abs(p) - s;
  l.set_swizzle<0, 2>(l.swizzle<0, 2>() * (r2d(__ctx, iTime * .1f)));
  l += f32x3(.5f, 0.1f, 0.f);
  l.set_swizzle<0, 1>(l.swizzle<0, 1>() *
                      (r2d(__ctx, iTime * .1f + sin(p.data[0]))));
  l = abs(l) - .5f;
  return max(l.data[0], max(l.data[1], l.data[2]));
}
f32x3 accCol;
f32x2 map(Context &__ctx, f32x3 p) {
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (r2d(__ctx, iTime * .25f)));
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (r2d(__ctx, iTime * .25f)));
  float acc = 1000.f;
  for (int i = 0; i < 16; ++i) {
    float fi = float(i) * .25f;
    f32x3 pc = p;
    float r = 5.f;
    pc.data[0] += sin(iTime * .25f + fi * 3.3f) * r;
    pc.data[1] += cos(iTime * .5f + fi * 2.f) * r;
    pc.data[2] += abs(sin(iTime * .1f + fi * 2.f));
    pc.set_swizzle<0, 1>(pc.swizzle<0, 1>() * (r2d(__ctx, iTime + fi)));
    pc.set_swizzle<0, 2>(pc.swizzle<0, 2>() *
                         (r2d(__ctx, iTime * .5f + fi * 2.f)));
    acc = min(acc, _cube(__ctx, pc, fi + f32x3(5.5f) * sin(fi + iTime)));
  }
  f32x2 main = f32x2(_cube(__ctx, p, f32x3(.5f)), 0.f);
  return _min(__ctx, main, f32x2(acc, 1.f));
}
f32x3 getNorm(Context &__ctx, float d, f32x3 p) {
  f32x2 e = f32x2(0.01f, 0.f);
  return normalize(f32x3(d) -
                   f32x3(map(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                         map(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                         map(__ctx, p - e.swizzle<1, 1, 0>()).data[0]));
}
f32x3 trace(Context &__ctx, f32x3 ro, f32x3 rd, int steps) {
  accCol = f32x3(0.f);
  f32x3 p = ro;
  for (int i = 0; i < steps; ++i) {
    float fi = float(i);
    f32x2 res = map(__ctx, p);
    if (res.data[0] < 0.01f) {
      return f32x3(res.data[0], distance(p, ro), res.data[1]);
    }
    f32x3 rgb = f32x3(1.f);
    if (res.data[1] == 1.f)
      rgb = f32x3(1.f, .5f, .25f);
    accCol += rgb * (rd + .5f) *
              f32x3(sin(fi) * .5f + .5f, cos(fi) * .5f + .5f, .5f) *
              pow(1.f - clamp(res.data[0] / 0.75f, 0.f, 1.f), 2.f) * .15f;
    p += rd * res.data[0];
  }
  return f32x3(-1.f);
}
f32x3 rdr(Context &__ctx, f32x2 uv) {
  f32x3 ro = f32x3(sin(iTime * .25f), 5.f * sin(iTime), -15.f * cos(iTime));
  f32x3 ta = f32x3(0.f, 0.f, 0.f);
  f32x3 rd = normalize(ta - ro);
  f32x3 col = pow(texture(__ctx.channel0, uv * 5.f).data[0], 5.f) * f32x3(2.f) +
              mix(f32x3(.5f, .5f, .6f) * .25f, f32x3(.95f, .56f, .34f),
                  (1.f - clamp(abs(uv.data[1] * 5.f), 0.f, 1.f)) *
                      clamp(length(uv), 0.f, 1.f));
  rd = getCam(__ctx, rd, uv);
  f32x3 res = trace(__ctx, ro, rd, 32);
  if (res.data[1] > 0.f) {
    f32x3 p = ro + rd * res.data[1];
    f32x3 n = getNorm(__ctx, res.data[0], p);
    col = n * .5f + .5f;
    if (res.data[2] == 1.f) {
      col = f32x3(.25f);
      col += f32x3(.75f, 0.2f, .14f) * pow(clamp(-dot(rd, n), 0.f, 1.f), 5.f);
      if (res.data[2] == 0.f)
        col = f32x3(0.f);
    }
  }
  col += accCol.swizzle<2, 0, 1>();
  return col;
}
f32x3 rdr2(Context &__ctx, f32x2 uv) {
  f32x2 dir = normalize(f32x2(1.f));
  float strength = .01f;
  f32x3 col = f32x3(0.f);
  col.data[0] = rdr(__ctx, uv + strength * dir).data[0];
  col.data[1] = rdr(__ctx, uv).data[1];
  col.data[2] = rdr(__ctx, uv - strength * dir).data[2];
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord.swizzle<0, 1>() - f32x2(.5f) * iResolution.swizzle<0, 1>()) /
      iResolution.swizzle<0, 0>();
  f32x3 col = f32x3(0.f);
  col = rdr2(__ctx, uv) * .5f;
  float stp = 0.025f;
  uv = floor(uv / stp) * stp;
  col = mix(rdr(__ctx, uv), col,
            1.f - clamp(length(uv * 2.f), 0.f, 1.f) *
                      clamp(abs(uv.data[1] * 3.f), 0.f, 1.f));
  col +=
      (1.f - clamp(length(uv * 2.f), 0.f, 1.f)) * f32x3(.1f, .2f, .3f) * 5.f *
      (clamp((abs(sin((uv.data[0] + uv.data[1]) * 5.f + iTime)) - .95f) * 400.f,
             0.f, 1.f));
  fragColor = f32x4(col, 1.f);
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
