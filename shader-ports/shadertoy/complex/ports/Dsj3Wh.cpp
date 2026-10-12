// Port of https://www.shadertoy.com/view/Dsj3Wh; author: scry.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Dsj3Wh.json.
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
f32x2x2 r2d(Context &__ctx, float a) {
  return f32x2x2(cos(a), sin(a), -sin(a), cos(a));
}
f32x3 cpos(Context &__ctx) {
  f32x3 c = f32x3(0.f, 0.f, -2.f);
  c.set_swizzle<0, 2>(c.swizzle<0, 2>() *
                      (r2d(__ctx, acos(-1.f) / 180.f * 40.f)));
  c.set_swizzle<1, 2>(c.swizzle<1, 2>() *
                      (r2d(__ctx, acos(-1.f) / 180.f * -40.f)));
  c.set_swizzle<0, 2>(
      c.swizzle<0, 2>() *
      (r2d(__ctx, sin(iTime * acos(-1.f) / 10.f * 2.f) * 0.2f - 0.1f)));
  c.set_swizzle<1, 2>(
      c.swizzle<1, 2>() *
      (r2d(__ctx, cos(iTime * acos(-1.f) / 10.f * 2.f) * 0.2f - 0.1f)));
  c.set_swizzle<0, 2>(c.swizzle<0, 2>() *
                      (r2d(__ctx, iTime * acos(-1.f) / 10.f * 2.f)));
  return c;
}
f32x4x4 eye(Context &__ctx) {
  f32x3 c = cpos(__ctx);
  f32x3 l = f32x3(0.f);
  f32x3 f = normalize(l - c);
  f32x3 r = normalize(cross(f32x3(0.f, 1.f, 0.f), f));
  f32x3 u = cross(f, r);
  return f32x4x4(r, 0.f, u, 0.f, f, 0.f, f32x3(0.f), 1.f);
}
float sdBox(Context &__ctx, f32x3 p, f32x3 s) {
  f32x3 q = abs(p) - s;
  return max(q.data[2], max(q.data[0], q.data[1]));
}
f32x4 map(Context &__ctx, f32x3 p) {
  f32x3 o = p;
  p = (eye(__ctx) * f32x4(p, 1.f)).swizzle<0, 1, 2>();
  p += cpos(__ctx);
  float d = sdBox(__ctx, p, f32x3(0.5f));
  d = min(d, sdBox(__ctx, p - f32x3(0.4f), f32x3(0.2f)));
  return f32x4(p, d);
}
f32x4 dbmap(Context &__ctx, f32x3 p) {
  f32x3 o = p;
  p = (eye(__ctx) * f32x4(p, 1.f)).swizzle<0, 1, 2>();
  p += cpos(__ctx);
  float d = map(__ctx, o).data[3];
  d = abs(p.data[2]) - 0.01f;
  return f32x4(p, d);
}
f32x2 RM(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.f;
  float ii = 0.f;
  for (int i = 0; i < 30; i++) {
    f32x3 p = ro + rd * dO;
    float dS = map(__ctx, p).data[3];
    dO += dS;
    ii += 0.01f;
    if (dO > 1000.f || dS < 0.001f) {
      break;
    }
  }
  return f32x2(dO, ii);
}
f32x2 dbRM(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.f;
  float ii = 0.f;
  for (int i = 0; i < 30; i++) {
    f32x3 p = ro + rd * dO;
    float dS = dbmap(__ctx, p).data[3];
    dO += dS;
    ii += 0.01f;
    if (dO > 1000.f || dS < 0.001f) {
      break;
    }
  }
  return f32x2(dO, ii);
}
f32x3 gradient(Context &__ctx, f32x3 p) {
  f32x2 h = f32x2(0.01f, 0.f);
  return f32x3(map(__ctx, p + h.swizzle<0, 1, 1>()).data[3] -
                   map(__ctx, p - h.swizzle<0, 1, 1>()).data[3],
               map(__ctx, p + h.swizzle<1, 0, 1>()).data[3] -
                   map(__ctx, p - h.swizzle<1, 0, 1>()).data[3],
               map(__ctx, p + h.swizzle<1, 1, 0>()).data[3] -
                   map(__ctx, p - h.swizzle<1, 1, 0>()).data[3]) /
         0.01f / 2.f;
}
f32x4 vRM(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.f;
  float ii = 0.f;
  f32x3 col = f32x3(0.f);
  for (int i = 0; i < 420; i++) {
    f32x3 p = ro + rd * dO;
    float dS = map(__ctx, p).data[3];
    dO += abs(dS * 0.004f) + 0.01f / 2.f;
    ii += 0.01f;
    f32x3 cp = (eye(__ctx) * f32x4(p, 1.f)).swizzle<0, 1, 2>() + cpos(__ctx);
    f32x3 mp = map(__ctx, p).swizzle<0, 1, 2>();
    float e = map(__ctx, p + gradient(__ctx, p) * (-dS)).data[3];
    float e_ = map(__ctx, p + gradient(__ctx, p) * (dO)).data[3];
    float ec = smoothstep(0.01f, 0.f, abs(e));
    float ee = smoothstep(0.f, 0.01f, 0.5f - (e_));
    col += ee * clamp(-dS, 0.f, 1.f);
    if (dO > 1000.f) {
      break;
    }
  }
  return f32x4(col, ii);
}
f32x3 calcNorm(Context &__ctx, f32x3 p) {
  f32x2 h = f32x2(0.001f, 0.f);
  return normalize(f32x3(map(__ctx, p + h.swizzle<0, 1, 1>()).data[3] -
                             map(__ctx, p - h.swizzle<0, 1, 1>()).data[3],
                         map(__ctx, p + h.swizzle<1, 0, 1>()).data[3] -
                             map(__ctx, p - h.swizzle<1, 0, 1>()).data[3],
                         map(__ctx, p + h.swizzle<1, 1, 0>()).data[3] -
                             map(__ctx, p - h.swizzle<1, 1, 0>()).data[3]));
}
f32x3 colo(Context &__ctx, f32x3 p, f32x3 n, f32x2 d, f32x3 ro, f32x3 rd) {
  f32x3 col = f32x3(0.f);
  f32x2 db = dbRM(__ctx, ro, rd);
  f32x4 dv = vRM(__ctx, ro, rd);
  f32x3 pdb = ro + rd * db.data[0];
  float dbx = abs(map(__ctx, pdb).data[3]);
  float e = map(__ctx, p + gradient(__ctx, p)).data[3];
  col += sin(f32x3(1.f, 2.f, 3.f) + d.data[1] * 30.f -
             iTime * acos(-1.f) / 10.f * 2.f) *
         0.1f;
  f32x3 vp = ro + rd * dv.data[3];
  col *= 0.1f;
  col += dv.swizzle<0, 1, 2>();
  if (d.data[0] > 100.f) {
    col *= 0.05f;
    col += dv.data[3] - 4.5f;
  }
  col += (1.f - clamp(d.data[0] * 0.1f, 0.f, 1.f)) * 0.1f;
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 R = iResolution.swizzle<0, 1>();
  float ar = R.data[0] / R.data[1];
  f32x2 tv = uv;
  uv -= 0.5f;
  uv.data[0] *= ar;
  f32x3 col = f32x3(0.f);
  f32x3 ro = f32x3(0.f);
  f32x3 rd = normalize(f32x3(uv, 1.f));
  f32x2 d = RM(__ctx, ro, rd);
  f32x3 p = ro + rd * d.data[0];
  f32x3 n = calcNorm(__ctx, p);
  col += colo(__ctx, p, n, d, ro, rd);
  fragColor = f32x4(col, 1.0f);
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
