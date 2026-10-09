// Port of https://www.shadertoy.com/view/NldSW7; author: z0rg.
// Dataset license metadata: cc-by-nc-4.0; original retained in
// upstream/NldSW7.json.
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
  Texture2D channel2;
  Texture2D channel3;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x2x2 r2d(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, -s, s, c);
}
float _time;
f32x2 _min(Context &__ctx, f32x2 a, f32x2 b) {
  if (a.data[0] < b.data[0])
    return a;
  return b;
}
f32x2 _max(Context &__ctx, f32x2 a, f32x2 b) {
  if (a.data[0] > b.data[0])
    return a;
  return b;
}
float _cube(Context &__ctx, f32x3 p, f32x3 s) {
  f32x3 l = abs(p) - s;
  return max(l.data[0], max(l.data[1], l.data[2]));
}
f32x2 map(Context &__ctx, f32x3 p) {
  float ttunnel = _time * 55.f;
  f32x3 po = p;
  p.set_swizzle<2, 1>(p.swizzle<2, 1>() + (f32x2(_time * .5f, 0.f)));
  f32x2 acc = f32x2(1000.f, -1.f);
  acc = _min(__ctx, acc,
             f32x2(-p.data[1] + sin(p.data[0] + p.data[2] - _time) * .1f +
                       0.025f * sin((p.data[0] - p.data[2] * .5f) * 3.f -
                                    _time * 4.5f) +
                       0.015f * sin((p.data[0] + p.data[2]) * 12.3f + _time) +
                       0.01f * sin((p.data[0] - p.data[2]) * 9.f + _time) *
                           sin(p.data[0] * 5.f),
                   0.f));
  p.set_swizzle<2, 1>(p.swizzle<2, 1>() + (f32x2(ttunnel * .5f, 0.f)));
  float offWall =
      .2f *
      (1.f -
       clamp(pow(texture(__ctx.channel1, p.swizzle<2, 1>() * .1f).data[2], 5.f),
             0.f, 1.f));
  acc = _min(__ctx, acc, f32x2(-(abs(p.data[0]) - 1.f - offWall), 1.f));
  acc = _max(__ctx, acc, f32x2(abs(po.data[2]) - 20.f, 0.f));
  f32x3 cpos = po - f32x3(0, -1.f + sin(_time), -5.f);
  cpos.set_swizzle<2, 1>(cpos.swizzle<2, 1>() * (r2d(__ctx, _time)));
  cpos.set_swizzle<0, 2>(cpos.swizzle<0, 2>() * (r2d(__ctx, _time * .5f)));
  acc = _min(__ctx, acc, f32x2(_cube(__ctx, cpos, f32x3(.5f)), 2.f));
  return acc;
}
f32x3 _norm(Context &__ctx, float d, f32x3 p) {
  return normalize(-cross(dFdx(p), dFdy(p)));
  f32x2 e = f32x2(0.01f, 0.f);
  return normalize(f32x3(d) -
                   f32x3(map(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                         map(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                         map(__ctx, p - e.swizzle<1, 1, 0>()).data[0]));
}
f32x3 trace(Context &__ctx, f32x3 ro, f32x3 rd, int steps) {
  f32x3 p = ro;
  for (int i = 0; i < steps; ++i) {
    f32x2 res = map(__ctx, p);
    if (res.data[0] < 0.001f)
      return f32x3(res.data[0], distance(ro, p), res.data[1]);
    p += rd * res.data[0];
  }
  return f32x3(-1.f);
}
f32x3 getCam(Context &__ctx, f32x3 rd, f32x2 uv) {
  float fov = 2.5f;
  if (mod(iTime, 5.f) < 2.f)
    fov += 5.5f;
  if (mod(iTime, 10.f) < 5.f)
    fov *= .2f;
  fov -= pow(texelFetch(__ctx.channel2, i32x2(10, 0)).data[0], 5.f) * .5f;
  f32x3 r = normalize(cross(rd, f32x3(0.f, 1.f, 0.f)));
  f32x3 u = normalize(cross(rd, r));
  return normalize(rd + fov * (r * uv.data[0] + u * uv.data[1]));
}
float lenny(Context &__ctx, f32x2 v) { return abs(v.data[0]) + abs(v.data[1]); }
f32x3 rdr(Context &__ctx, f32x2 uv) {
  f32x3 col = f32x3(1.f);
  float dist =
      mix(-20.f, -15.f,
          mod(_time, 5.f) / 5.f +
              pow(texelFetch(__ctx.channel2, i32x2(10, 0)).data[0], 5.f));
  f32x3 ro = f32x3(sin(iTime + 3.1415f), -6.f + sin(_time) * 5.f, dist);
  f32x3 ta = f32x3(0.f, 0.f, 0.f);
  f32x3 rd = normalize(ta - ro);
  rd = getCam(__ctx, rd, uv);
  rd.set_swizzle<0, 1>(
      rd.swizzle<0, 1>() *
      (r2d(__ctx,
           sin(_time +
               pow(texelFetch(__ctx.channel2, i32x2(15, 0)).data[0], 5.f) *
                   5.f) *
               .25f)));
  f32x3 res = trace(__ctx, ro, rd, 256);
  if (res.data[1] > 0.f) {
    f32x3 p = ro + rd * res.data[1];
    f32x3 n = _norm(__ctx, res.data[0], p);
    if (res.data[2] == 0.f) {
      col = f32x3(.05f);
      n.set_swizzle<0, 2>(
          n.swizzle<0, 2>() +
          (0.2f * texture(__ctx.channel0, p.swizzle<0, 2>() * .1f +
                                              _time * .025f * f32x2(1.f, -1.f))
                      .data[0] +
           .1f * sin((p.data[0] + p.data[2] * .3f) * 8.f + _time) +
           .1f *
               sin(p.data[2] * 8.33f - pow(abs(p.data[0]) - .2f, 5.f) * 10.f) +
           .25f * sin(_time +
                      length((p.swizzle<0, 2>() - f32x2(0.f, 20.f)) * 2.f))));
      n = normalize(n);
      col += pow(clamp(-dot(normalize(f32x3(0.f, 1.f, 1.f) - rd), n), 0.f, 1.f),
                 250.05f) *
             f32x3(.5f) * length(uv * 5.f);
    }
    if (res.data[2] == 1.f) {
      col = f32x3(.05f);
      col *= texture(__ctx.channel0, p.swizzle<2, 1>() * f32x2(.01f, 2.f))
                 .swizzle<0, 0, 0>();
      col +=
          mix(f32x3(0.f),
              .15f * texture(__ctx.channel1,
                             p.swizzle<2, 1>() * .1f * f32x2(5.f, 1.f) -
                                 f32x2(-_time * .2f, 0.f) +
                                 sin((n.data[0] + p.data[1] + p.data[0]) * 3.f +
                                     _time * .3f))
                         .swizzle<0, 0, 0>(),
              texture(__ctx.channel0, p.swizzle<2, 1>() * f32x2(.1f, 1.f) * .5f)
                  .data[0]);
      col += .1f *
             texture(__ctx.channel1, p.swizzle<2, 1>() * f32x2(1.f, 1.f) * .5f -
                                         f32x2(-_time * .25f, 0.f))
                 .swizzle<0, 1, 2>();
    }
    if (res.data[2] == 2.f) {
      col = f32x3(0.0f);
      col += (n * .5f + .5f) * pow(1.f - clamp(-dot(n, rd), 0.f, 1.f), 5.f);
    }
    col += f32x3(.5f) * pow(1.f - clamp(-dot(rd, n), 0.f, 1.f), 5.f);
    col = clamp(col, 0.f, 1.f);
    col +=
        pow(1.f - clamp(lenny(__ctx, uv - f32x2(0.f, .15f)), 0.f, 1.f), 3.f) *
        1.2f;
    if (res.data[2] == 1.f) {
      col *=
          clamp(pow(clamp(-p.data[1] * .25f, 0.f, 1.f), .5f) + .4f, 0.f, 1.f);
    }
  }
  float waves = sin((uv.data[0] + uv.data[1]) * 55.f);
  float flicker = 0.1f;
  col = mix(col, 1.f - col,
            clamp(waves * 400.f, 0.f, 1.f) *
                pow(pow(texelFetch(__ctx.channel2, i32x2(50, 0)).data[0], 5.f),
                    .25f) *
                clamp(length(uv * .25f), 0.f, 1.f));
  return col;
}
f32x3 rdr2(Context &__ctx, f32x2 uv) {
  f32x2 dir = normalize(f32x2(1.f, 1.f));
  float strength =
      0.03f * pow(texelFetch(__ctx.channel2, i32x2(20, 0)).data[0], 5.f);
  f32x3 col;
  col.data[0] = rdr(__ctx, uv + dir * strength).data[0];
  col.data[1] = rdr(__ctx, uv).data[1];
  col.data[2] = rdr(__ctx, uv - dir * strength).data[2];
  col *= mix(
      f32x3(1.f), f32x3(0.180f, 1.000f, 0.863f),
      clamp(length(uv) * 1.5f - .3f, 0.f, 1.f) *
          clamp((sin(-iTime * 5.f + (uv.data[1] - uv.data[0]) * 50.f) - .9f) *
                    400.f,
                0.f, 1.f));
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  _time = iTime;
  f32x2 uv = (fragCoord - f32x2(.5f) * iResolution.swizzle<0, 1>()) /
             iResolution.swizzle<0, 0>();
  f32x3 col = rdr2(__ctx, uv);
  col *= mix(f32x3(0.5f), f32x3(1.f),
             1.f - clamp(lenny(__ctx, uv * 1.5f), 0.f, 1.f));
  col = pow(col, f32x3(1.2f));
  float flicker = 1.f / 12.f;
  float flickperiod = 2.f;
  col = mix(col, 1.f - col,
            pow(texelFetch(__ctx.channel2, i32x2(125, 0)).data[0], 5.f) *
                float(mod(iTime, flickperiod) < .5f) *
                clamp(mod(iTime, flicker) / flicker, 0.f, 1.f));
  col = mix(fwidth(col) * 10.f, col,
            clamp((sin(iTime * .5f) + .8f) * 400.f, 0.f, 1.f));
  col = mix(col, col.swizzle<2, 0, 1>() * f32x3(.1f, .1f, .3f) * 8.f,
            clamp(sin(iTime * .25f) * 400.f, 0.f, 1.f));
  col = col * 2.f *
        f32x3(clamp((sin(iTime * .25f) * .5f + .5f) + .5f, 0.f, 1.f) * .5f,
              texture(__ctx.channel0, f32x2(iTime * .05f)).data[0], .5f);
  col *= clamp(pow(pow(texelFetch(__ctx.channel2, i32x2(10, 0)).data[0], 5.f) +
                       .3f,
                   5.f) +
                   .5f,
               0.f, 1.f) *
         2.f;
  fragColor =
      f32x4(2.f * mix(col, col.swizzle<0, 0, 0>(), .5f) *
                mix(f32x3(.1f, .45f, .23f), f32x3(0.541f, 1.000f, 0.992f),
                    clamp(length(uv), 0.f, 1.f)),
            1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0,
               __binding__(0, 1) texture_2d<f32x4> channel1,
               __binding__(1, 1) sampler sampling1,
               __binding__(0, 2) texture_2d<f32x4> channel2,
               __binding__(1, 2) sampler sampling2,
               __binding__(0, 3) texture_2d<f32x4> channel3,
               __binding__(1, 3) sampler sampling3) {
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
  Context __ctx{{channel0, sampling0},
                {channel1, sampling1},
                {channel2, sampling2},
                {channel3, sampling3}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
