// Port of https://www.shadertoy.com/view/fdGfDt; author: intrakits.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fdGfDt.json.
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
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float dBox(Context &__ctx, f32x3 p, f32x3 s) {
  return length(max(abs(p) - s, 0.f));
}
f32x2 colMin(Context &__ctx, f32x2 res, f32x2 obj) {
  res.data[0] = min(obj.data[0], res.data[0]);
  if (obj.data[0] == res.data[0]) {
    res.data[1] = obj.data[1];
  }
  return res;
}
f32x2 GetDist(Context &__ctx, f32x3 p) {
  f32x2 res = f32x2(p.data[1], 1);
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (Rot(__ctx, iTime)));
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (Rot(__ctx, iTime)));
  f32x2 box = f32x2(dBox(__ctx, p, f32x3(1, 1, 1)), 2);
  res = colMin(__ctx, res, box);
  return res;
}
f32x2 RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float width = .01f;
  f32x3 rd1 = rd + f32x3(width, 0, 0);
  f32x3 rd2 = rd + f32x3(-width, 0, 0);
  f32x3 rd3 = rd + f32x3(0, width, 0);
  f32x3 rd4 = rd + f32x3(0, -width, 0);
  f32x2 dO = f32x2(0.f, 0.f);
  f32x2 dO1 = f32x2(0.f, 0.f);
  f32x2 dO2 = f32x2(0.f, 0.f);
  f32x2 dO3 = f32x2(0.f, 0.f);
  f32x2 dO4 = f32x2(0.f, 0.f);
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + dO.data[0] * rd;
    f32x3 p1 = ro + dO1.data[0] * rd1;
    f32x3 p2 = ro + dO2.data[0] * rd2;
    f32x3 p3 = ro + dO3.data[0] * rd3;
    f32x3 p4 = ro + dO4.data[0] * rd4;
    f32x2 ds = GetDist(__ctx, p);
    f32x2 ds1 = GetDist(__ctx, p1);
    f32x2 ds2 = GetDist(__ctx, p2);
    f32x2 ds3 = GetDist(__ctx, p3);
    f32x2 ds4 = GetDist(__ctx, p4);
    dO += ds.data[0];
    dO1 += ds1.data[0];
    dO2 += ds2.data[0];
    dO3 += ds3.data[0];
    dO4 += ds4.data[0];
    if (ds.data[1] != ds1.data[1] || ds.data[1] != ds2.data[1] ||
        ds.data[1] != ds3.data[1] || ds.data[1] != ds4.data[1]) {
      dO.data[1] = 3.f;
    } else if (ds.data[0] < 0.01f) {
      dO.data[1] = ds.data[1];
      break;
    } else if (dO.data[0] > 100.f) {
      dO.data[1] = -1.f;
      break;
    }
  }
  return dO;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p).data[0];
  f32x2 e = f32x2(0.01f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()).data[0]);
  return normalize(n);
}
float GetLight(Context &__ctx, f32x3 p) {
  f32x3 lightPos = f32x3(4, 5.f, -3);
  lightPos.set_swizzle<0, 2>(lightPos.swizzle<0, 2>() +
                             (f32x2(sin(iTime), cos(iTime))));
  f32x3 l = normalize(lightPos - p);
  f32x3 n = GetNormal(__ctx, p);
  float diff = clamp(dot(n, l), 0.f, 1.f);
  float d = RayMarch(__ctx, p + n * 0.01f * 1.1f, l).data[0];
  if (d < length(lightPos - p)) {
    diff *= .1f;
  }
  return diff;
}
f32x3x3 setCamera(Context &__ctx, f32x3 ro, f32x3 ta, float cr) {
  f32x3 cw = normalize(ta - ro);
  f32x3 cp = f32x3(sin(cr), cos(cr), 0.0f);
  f32x3 cu = normalize(cross(cw, cp));
  f32x3 cv = (cross(cu, cw));
  return f32x3x3(cu, cv, cw);
}
f32x3 Render(Context &__ctx, f32x3 &ro, f32x3 &rd, float &reflVal) {
  f32x2 d = RayMarch(__ctx, ro, rd);
  f32x3 p = ro + rd * d.data[0];
  float diff = GetLight(__ctx, p);
  f32x3 col = f32x3(0);
  col = f32x3(diff);
  f32x3 n = GetNormal(__ctx, p);
  f32x3 r = reflect(rd, n);
  f32x3 refl = texture(__ctx.channel0, r).swizzle<0, 1, 2>();
  refl = refl * abs(n.data[1]) + refl * abs(n.data[2]) + refl * abs(n.data[0]);
  if (d.data[1] == 1.f) {
    col *= f32x3(1, 0, 0);
    reflVal = 0.f;
  } else if (d.data[1] == 2.f) {
    col *= f32x3(1);
    reflVal = 0.1f;
  } else if (d.data[1] == 3.f) {
    col = f32x3(0, 1, 0);
    reflVal = 0.1f;
  }
  ro = p + n * 0.01f * 3.f;
  rd = r;
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 mo = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 ta = f32x3(0, 1, 0);
  f32x3 ro = ta + f32x3(0, .5f, -5.f);
  f32x3x3 ca = setCamera(__ctx, ro, ta, 0.f);
  f32x2 pt =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 rd = ca * normalize(f32x3(pt, 2.5f));
  float reflVal = 1.f;
  f32x3 col = Render(__ctx, ro, rd, reflVal);
  f32x3 bounce = reflVal * Render(__ctx, ro, rd, reflVal);
  col += bounce;
  fragColor = f32x4(col, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_cube<f32x4> channel0,
               __binding(1, 0) sampler sampling0) {
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
