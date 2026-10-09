// Port of https://www.shadertoy.com/view/slVBWV; author: yasuo.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/slVBWV.json.
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
float segBase(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float size = 0.02f;
  float padding = 0.05f;
  float w = padding * 3.0f;
  float h = padding * 5.0f;
  p = mod(p, 0.05f) - 0.025f;
  float thickness = 0.005f;
  float gridMask = min(abs(p.data[0]) - thickness, abs(p.data[1]) - thickness);
  p = prevP;
  float d = max(abs(p).data[0] - f32x2(w * 0.5f, h * 0.5f).data[0],
                abs(p).data[1] - f32x2(w * 0.5f, h * 0.5f).data[1]);
  float a = radians(45.0f);
  p.data[0] = abs(p.data[0]) - 0.1f;
  p.data[1] = abs(p.data[1]) - 0.05f;
  float d2 = dot(p, f32x2(cos(a), sin(a)));
  d = max(d2, d);
  d = max(-gridMask, d);
  return d;
}
float seg0(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = segBase(__ctx, p);
  float size = 0.03f;
  float mask = max(abs(p).data[0] - f32x2(size, size * 2.7f).data[0],
                   abs(p).data[1] - f32x2(size, size * 2.7f).data[1]);
  d = max(-mask, d);
  return d;
}
float seg2(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = segBase(__ctx, p);
  float size = 0.03f;
  p.data[0] += size;
  p.data[1] -= 0.05f;
  float mask = max(abs(p).data[0] - f32x2(size * 2.f, size).data[0],
                   abs(p).data[1] - f32x2(size * 2.f, size).data[1]);
  d = max(-mask, d);
  p = prevP;
  p.data[0] -= size;
  p.data[1] += 0.05f;
  mask = max(abs(p).data[0] - f32x2(size * 2.f, size).data[0],
             abs(p).data[1] - f32x2(size * 2.f, size).data[1]);
  d = max(-mask, d);
  return d;
}
f32x2 combine(Context &__ctx, f32x2 val1, f32x2 val2) {
  return (val1.data[0] < val2.data[0]) ? val1 : val2;
}
f32x2x2 Rot0(Context &__ctx, f32x2 p) {
  return f32x2x2(cos(radians(90.f)), -sin(radians(90.f)), sin(radians(90.f)),
                 cos(radians(90.f)));
}
float ring(Context &__ctx, f32x3 p, float size) {
  return max(abs(p.data[1]) - 1.f,
             abs(length(p.swizzle<0, 2>()) - size) - 0.001f);
}
f32x2 GetDist(Context &__ctx, f32x3 p) {
  f32x3 prevP = p;
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (Rot0(__ctx, p.swizzle<1, 2>())));
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (Rot0(__ctx, p.swizzle<0, 1>())));
  float d = ring(__ctx, p, 0.4f);
  f32x2 res = f32x2(d, 0);
  p.data[2] -= 0.85f;
  d = ring(__ctx, p, 0.4f);
  f32x2 res2 = f32x2(d, 1);
  p.data[2] += 1.7f;
  d = ring(__ctx, p, 0.4f);
  f32x2 res3 = f32x2(d, 2);
  return combine(__ctx, res, combine(__ctx, res2, res3));
}
f32x2 RayMarch(Context &__ctx, f32x3 ro, f32x3 rd, float side, int stepnum) {
  f32x2 dO = f32x2(0.0f);
  for (int i = 0; i < stepnum; i++) {
    f32x3 p = ro + rd * dO.data[0];
    f32x2 dS = GetDist(__ctx, p);
    dO.data[0] += dS.data[0] * side;
    dO.data[1] = dS.data[1];
    if (dO.data[0] > 100.f || abs(dS.data[0]) < .0005f)
      break;
  }
  return dO;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p).data[0];
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()).data[0]);
  return normalize(n);
}
f32x3 R(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = p + f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i - p);
  return d;
}
f32x3 tex(Context &__ctx, f32x3 p, float dir) {
  f32x2 uv =
      f32x2(0.9f * atan(p.data[0], p.data[2]) / 6.2832f, p.data[1] / 3.f);
  uv *= 1.5f;
  uv.data[1] *= -1.f;
  uv.data[0] += iTime * 0.1f * dir;
  uv.data[0] = mod(uv.data[0], 0.3f) - 0.15f;
  uv *= f32x2x2(cos(radians(90.f)), -sin(radians(90.f)), sin(radians(90.f)),
                cos(radians(90.f)));
  float d = seg2(__ctx, uv - f32x2(-0.3f, 0.f));
  float d2 = seg0(__ctx, uv - f32x2(-0.1f, 0.f));
  d = min(d, d2);
  d2 = seg2(__ctx, uv - f32x2(0.1f, 0.f));
  d = min(d, d2);
  d2 = seg2(__ctx, uv - f32x2(0.3f, 0.f));
  d = min(d, d2);
  return mix(f32x3(0.0f), f32x3(1.8f),
             smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                        0.0f, d));
}
f32x3 materials(Context &__ctx, int mat, f32x3 n, f32x3 rd, f32x3 p,
                f32x3 col) {
  f32x3 prevP = p;
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (Rot0(__ctx, p.swizzle<1, 2>())));
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (Rot0(__ctx, p.swizzle<0, 1>())));
  if (mat == 0) {
    col = tex(__ctx, p, 1.f);
  } else if (mat == 1) {
    p.data[2] -= 0.85f;
    col = tex(__ctx, p, -1.f);
  } else if (mat == 2) {
    p.data[2] += 0.85f;
    col = tex(__ctx, p, -1.f);
  }
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 ro = f32x3(0, 0, -1.35f);
  f32x3 rd = R(__ctx, uv, ro, f32x3(0, 0.0f, 0), 1.0f);
  f32x2 d = RayMarch(__ctx, ro, rd, 1.f, 100);
  f32x3 col = f32x3(.0f);
  if (d.data[0] < 100.f) {
    f32x3 p = ro + rd * d.data[0];
    f32x3 n = GetNormal(__ctx, p);
    int mat = int(d.data[1]);
    col = materials(__ctx, mat, n, rd, p, col);
  }
  col = pow(col, f32x3(0.9545f));
  fragColor = f32x4(col, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output) {
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
