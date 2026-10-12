// Port of https://www.shadertoy.com/view/DllXR4; author: byt3_m3chanic.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/DllXR4.json.
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
struct Context {};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x3 hit, hitPoint;
f32x4 gtile, stile;
float mid, sid;
f32x2x2 flip, turn;
f32x3 colorA = f32x3(.65f);
f32x3 colorB = f32x3(.40f);
f32x3 colorC = f32x3(.80f);
const f32x3 size = f32x3(1.75f, 2.5f, 1.75f);
const f32x3 hlf = size / 2.f;
const f32x3 bs = f32x3(hlf * .9f);
const f32x3 grid = f32x3(1, 1, 1);
const float thick = .45f / size.data[0];
float u_hash = .6938f;
const f32x3 d = f32x3(0.957f, 0.439f, 0.043f);
f32x3 hue(Context &__ctx, float t) {
  return .5f + .4f * cos(6.28318530718f * t * (f32x3(.95f, .97f, .98f) * d));
}
f32x2x2 rot(Context &__ctx, float a) {
  return f32x2x2(cos(a), sin(a), -sin(a), cos(a));
}
float hash21(Context &__ctx, f32x2 p) {
  return fract(sin(dot(p, f32x2(23.43f, 84.21f))) * 4832.3234f);
}
float box(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.f);
}
float cap(Context &__ctx, f32x3 p, float r, float h) {
  f32x2 d = abs(f32x2(length(p.swizzle<0, 1>()), p.data[2])) - f32x2(h, r);
  return min(max(d.data[0], d.data[1]), 0.f) + length(max(d, 0.f));
}
float trs(Context &__ctx, f32x3 p, f32x2 t) {
  f32x2 q = f32x2(length(p.swizzle<2, 0>()) - t.data[0], p.data[1]);
  return length(q) - t.data[1];
}
f32x2 map(Context &__ctx, f32x3 p) {
  f32x2 res = f32x2(1e5f, 0.f);
  p.data[0] += iTime * .25f;
  float pd = floor((p.data[0] + 4.f) / 8.f);
  mid = pd;
  p.data[0] = mod(p.data[0] + 4.f, 8.f) - 4.f;
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (flip));
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (turn));
  f32x3 q = p;
  f32x3 id = floor((q + hlf) / size) - grid;
  id += pd;
  q = q - size * clamp(round(q / size), -grid, grid);
  f32x3 fq = q;
  float hs = hash21(__ctx, id.swizzle<0, 2>() + id.data[1] + u_hash);
  if (hs > .5f)
    q.data[2] *= -1.f;
  f32x2 d2 = f32x2(length(q.swizzle<0, 2>() - hlf.swizzle<0, 2>()),
                   length(q.swizzle<0, 2>() + hlf.swizzle<0, 2>()));
  f32x2 gx = d2.data[0] < d2.data[1]
                 ? f32x2(q.swizzle<0, 2>() - hlf.swizzle<0, 2>())
                 : f32x2(q.swizzle<0, 2>() + hlf.swizzle<0, 2>());
  f32x3 tq = f32x3(gx.data[0], q.data[1], gx.data[1]);
  float xhs = fract(2.f * hs + id.data[1]);
  float rhs = fract(hs + id.data[0]);
  float trh = 1e5f;
  if (fract(xhs + id.data[0] * 33.32f) > .75f)
    q.set_swizzle<2, 0>(q.swizzle<2, 0>() * (rot(__ctx, 1.5707f)));
  if (rhs > .9f) {
    trh = length(f32x3(abs(q.data[0]), q.swizzle<1, 2>()) -
                 f32x3(hlf.data[0], .25f, 0)) -
          thick;
    trh = min(length(f32x3(q.swizzle<0, 1>(), abs(q.data[2])) -
                     f32x3(0, .25f, hlf.data[2])) -
                  thick,
              trh);
  } else if (rhs > .7f) {
    float qy = .15f + .15f * sin((q.data[2] + .5f) * size.data[2] * 2.f);
    float qx = .15f + .15f * sin((q.data[0] + .5f) * size.data[0] * 2.f);
    trh = cap(__ctx, q - f32x3(0, .25f + qy, 0), hlf.data[0], thick);
    trh = min(cap(__ctx, q.swizzle<2, 1, 0>() - f32x3(0, .25f - qx, 0),
                  hlf.data[2], thick),
              trh);
  } else {
    trh = trs(__ctx, tq - f32x3(0, .25f, 0), f32x2(hlf.data[0], thick));
  }
  trh = max(abs(trh) - .075f, -trh);
  trh = max(trh, box(__ctx, q, bs));
  if (trh < res.data[0]) {
    float mt = xhs > .725f ? 5.f : xhs > .61f ? 4.f : xhs > .25f ? 2.f : 3.f;
    gtile.set_swizzle<0, 1, 2>(f32x3(id.swizzle<0, 2>(), 1.f));
    gtile.data[3] = mt;
    res = f32x2(trh, mt);
    hit = tq;
  }
  float bls =
      cap(__ctx, f32x3(q.swizzle<0, 1>(), abs(q.data[2])) - f32x3(0, .25f, hlf),
          thick * .4f, thick * 1.1f);
  bls = min(cap(__ctx,
                f32x3(q.swizzle<2, 1>(), abs(q.data[0])) - f32x3(0, .25f, hlf),
                thick * .4f, thick * 1.1f),
            bls);
  float crt =
      cap(__ctx, f32x3(q.swizzle<0, 1>(), abs(q.data[2])) - f32x3(0, .25f, hlf),
          thick * .34f, thick * 1.45f);
  crt = min(cap(__ctx,
                f32x3(q.swizzle<2, 1>(), abs(q.data[0])) - f32x3(0, .25f, hlf),
                thick * .34f, thick * 1.45f),
            crt);
  bls = max(crt, -bls);
  if (bls < res.data[0]) {
    res = f32x2(bls, 1.f);
    hit = q;
  }
  return res;
}
f32x3 normal(Context &__ctx, f32x3 p, float t) {
  float e = 1e-4f * t;
  f32x2 h = f32x2(1, -1) * .5773f;
  f32x3 n =
      h.swizzle<0, 1, 1>() * map(__ctx, p + h.swizzle<0, 1, 1>() * e).data[0] +
      h.swizzle<1, 1, 0>() * map(__ctx, p + h.swizzle<1, 1, 0>() * e).data[0] +
      h.swizzle<1, 0, 1>() * map(__ctx, p + h.swizzle<1, 0, 1>() * e).data[0] +
      h.swizzle<0, 0, 0>() * map(__ctx, p + h.swizzle<0, 0, 0>() * e).data[0];
  return normalize(n);
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 F) {
  float ftx = 276.f;
  f32x2 uv = (2.f * F.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]);
  uv = floor(uv * (ftx * iResolution.data[0] / iResolution.data[1])) /
       (ftx * iResolution.data[0] / iResolution.data[1]);
  f32x3 ro = f32x3(0, 0, 10);
  f32x3 rd = normalize(f32x3(uv, -1));
  float x =
      iMouse.data[2] < 0.f || iMouse.swizzle<0, 1>() == f32x2(0)
          ? 0.0f
          : (iMouse.data[1] / iResolution.data[1] * 2.f - 1.f) * 3.14159265358f;
  float y =
      iMouse.data[2] > 0.f
          ? (iMouse.data[0] / iResolution.data[0] * 2.f - 1.f) * 3.14159265358f
          : iTime * .063f;
  f32x2x2 rx = rot(__ctx, -.65f);
  f32x2x2 ry = rot(__ctx, .0f);
  flip = rot(__ctx, x);
  turn = rot(__ctx, y);
  ro.set_swizzle<2, 1>(ro.swizzle<2, 1>() * (rx));
  rd.set_swizzle<2, 1>(rd.swizzle<2, 1>() * (rx));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (ry));
  rd.set_swizzle<0, 2>(rd.swizzle<0, 2>() * (ry));
  f32x3 C = f32x3(.0f);
  float m = 0.f;
  float d = 0.f;
  f32x3 p = ro;
  for (int i = 0; i < 164; i++) {
    p = ro + rd * d;
    f32x2 ray = map(__ctx, p);
    if (ray.data[0] < d * 1e-5f || d > 35.f)
      break;
    d += i < 42 ? ray.data[0] * .25f : ray.data[0] * .5f;
    m = ray.data[1];
  }
  sid = mid;
  colorA = hue(__ctx, .00f + u_hash + sid);
  colorB = hue(__ctx, .15f + u_hash + sid);
  colorC = hue(__ctx, .60f + u_hash + sid);
  float alpha = 1.f;
  if (d < 35.f) {
    hitPoint = hit;
    alpha *= 1e-5f;
    f32x3 n = normal(__ctx, p, d);
    f32x3 lpos = f32x3(5, 5, 9.f);
    f32x3 l = normalize(lpos - p);
    float diff = clamp(dot(n, l), 0.f, 1.f);
    float spec = pow(max(dot(reflect(l, n), rd), .1f), 32.f) * .75f;
    float shdw = 1.0f;
    float t = .01f;
    for (int i = 0; i < 64; i++) {
      float h = map(__ctx, p + l * t).data[0];
      if (h < 1e-4f) {
        shdw = 0.f;
        break;
      }
      shdw = min(shdw, 18.f * h / t);
      t += h;
      if (shdw < 1e-4f || t > 35.f)
        break;
    }
    diff = mix(diff, diff * shdw, .5f);
    f32x3 h = f32x3(.0f);
    if (m == 1.f) {
      h = f32x3(.35f);
    }
    if (m == 2.f) {
      h = colorB;
    }
    if (m == 3.f) {
      h = colorC;
    }
    if (m == 4.f) {
      h = colorA;
    }
    if (m == 5.f) {
      h = f32x3(.05f);
    }
    float xiff = diff;
    if (diff < .6f && (mod(F.data[0], 4.f) == mod(F.data[1], 4.f)))
      xiff *= .3f;
    if (diff < .4f && (mod(F.data[0], 3.f) == mod(F.data[1], 3.f)))
      xiff *= .3f;
    if (diff < .2f && (mod(F.data[0], 2.f) == mod(F.data[1], 2.f)))
      xiff *= .3f;
    if (spec > .25f)
      xiff = clamp(xiff * 1.15f, 0.f, 1.f);
    if (spec > .5f && (mod(F.data[0], 2.f) == mod(F.data[1], 2.f)))
      xiff = clamp(xiff * 1.5f, 0.f, 1.f);
    C = h * xiff;
  }
  C = mix(C, mix(f32x3(.025f), f32x3(.95f), (uv.data[1] + .5f) * .25f),
          1.f - exp(-.0005f * d * d * d));
  C = clamp(C, f32x3(1e-3f), f32x3(1));
  C = pow(C, f32x3(.4545f));
  O = f32x4(C, 1.0f);
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
