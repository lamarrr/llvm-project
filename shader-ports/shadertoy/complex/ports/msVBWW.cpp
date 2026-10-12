// Port of https://www.shadertoy.com/view/msVBWW; author: ianertson.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/msVBWW.json.
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
struct Data {
  f32x3 p;
  f32x3 n;
  float d;
  int id;
};
struct Light {
  f32x3 p;
  f32x3 c;
  float s;
};
f32x2x2 rot(Context &__ctx, float a) {
  float c = cos(a);
  float s = sin(a);
  return f32x2x2(c, s, -s, c);
}
f32x3 pointRot(Context &__ctx, f32x3 p, f32x3 r) {
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (rot(__ctx, r.data[0])));
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (rot(__ctx, r.data[1])));
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (rot(__ctx, r.data[2])));
  return p;
}
float boxSDF(Context &__ctx, f32x3 p, f32x3 s) {
  p = abs(p) - s;
  return length(max(p, 0.0f)) +
         min(max(p.data[0], max(p.data[1], p.data[2])), 0.0f);
}
float planeSDF(Context &__ctx, f32x3 p, f32x2 s) {
  return max(max(p.data[1], abs(p.data[0]) - s.data[0]),
             abs(p.data[2]) - s.data[1]) +
         0.019f * cos(p.data[0] * 10.f + p.data[2] * 10.f + iTime * 10.f) +
         0.025f * sin(p.data[0] + p.data[2] * 10.f + iTime * 10.f) +
         0.025f * exp(cos(p.data[0] * p.data[2] * 10.f +
                          sin(iTime + p.data[0] * 3.14f + p.data[2])) -
                      1.f) -
         0.025f *
             exp(sin((iTime * 2.2f) + (p.data[0] * 10.f - p.data[2] * 10.f)) -
                 1.f);
}
float sdf(Context &__ctx, f32x3 p, Data &data) {
  float box =
      boxSDF(__ctx,
             pointRot(
                 __ctx,
                 p - f32x3(0, (0.19f) + 0.25f * (0.5f + 0.5f * sin(iTime)), 0),
                 f32x3(iTime, iTime, 0)),
             f32x3(0.1f)) -
      0.01f;
  float ground = planeSDF(__ctx, p, f32x2(2, 2));
  float d = 32.0f;
  if (box < d) {
    d = box;
    data.id = 0;
  }
  if (ground < d) {
    d = ground;
    data.id = 1;
  }
  return d / 1.1f;
}
bool march(Context &__ctx, f32x3 ro, f32x3 rd, Data &data) {
  float d = 0.0f;
  for (int i = min(0, iFrame); i < 100; i++) {
    f32x3 p = ro + rd * d;
    float next = sdf(__ctx, p, data);
    if (abs(next) <= 0.003f)
      break;
    if (abs(d) >= 32.0f)
      return false;
    d += next;
  }
  f32x3 p = data.p = ro + rd * d;
  data.d = d;
  f32x2 e = f32x2(0.003f, 0.0f);
  data.n = normalize(sdf(__ctx, p, data) -
                     f32x3(sdf(__ctx, p - e.swizzle<0, 1, 1>(), data),
                           sdf(__ctx, p - e.swizzle<1, 0, 1>(), data),
                           sdf(__ctx, p - e.swizzle<1, 1, 0>(), data)));
  return true;
}
float getShadow(Context &__ctx, f32x3 ro, f32x3 rd, Data data, Light light) {
  f32x3 L = normalize(light.p);
  f32x3 N = data.n;
  ro = data.p + N * 0.003f * 3.f;
  rd = L;
  float d = 2.5f * max(0.0f, -dot(N, L));
  for (int i = min(0, iFrame); i < 8; i++) {
    f32x3 p = ro + rd * d;
    float r = float(i) / float(8);
    p += N * (1.0f / (1.0f + r)) * 0.1f;
    float next = sdf(__ctx, p, data);
    if (abs(next) <= 0.003f || abs(d) >= 32.0f)
      break;
    d += next;
  }
  float s = (d / (1.0f + d));
  return clamp(smoothstep(0.0f, 0.5f, s * s), 0.07f * 2.f, 1.0f);
}
f32x3 getColor(Context &__ctx, Data data) {
  switch (data.id) {
  case 0:
    return f32x3(1, 0, 0);
    break;
  default:
    return f32x3(1.0f);
    break;
  }
  return f32x3(1.0f);
}
f32x3 forEachLight(Context &__ctx, f32x3 ro, f32x3 rd, Data &data,
                   Light light) {
  f32x3 L = normalize(light.p);
  f32x3 N = data.n;
  f32x3 ref = reflect(L, N);
  float NdotL = max(0.07f, dot(N, L));
  float VdotR = max(0.0f, dot(rd, ref));
  float spec = pow(VdotR, 16.0f);
  float shadow =
      NdotL <= (0.07f * 0.03f) ? 0.07f : getShadow(__ctx, ro, rd, data, light);
  f32x3 diffuse = getColor(__ctx, data) / 3.1415f;
  return (NdotL * light.c + spec) * shadow * diffuse;
}
f32x3 render(Context &__ctx, f32x3 ro, f32x3 rd, Data &data) {
  Light light = Light(f32x3(1, 2.f, -3.f), f32x3(0.97f, 0.69f, 0.51f), 2.0f);
  f32x3 L = normalize(light.p);
  float VdotL = max(0.0f, dot(rd, L));
  f32x3 col = f32x3(0.0f);
  col += pow(VdotL, 4.f) * light.c;
  if (march(__ctx, ro, rd, data)) {
    col += forEachLight(__ctx, ro, rd, data, light);
  }
  return col;
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 fc) {
  f32x3 col = f32x3(0.0f);
  f32x2 uv = (fc.swizzle<0, 1>() -
              0.5f * (iResolution.swizzle<0, 1>()).swizzle<0, 1>()) /
             (iResolution.swizzle<0, 1>()).data[1];
  f32x4 m = f32x4((iMouse.swizzle<0, 1>() -
                   0.5f * (iResolution.swizzle<0, 1>()).swizzle<0, 1>()) /
                      (iResolution.swizzle<0, 1>()).data[1],
                  iMouse.swizzle<2, 3>());
  f32x3 ro = f32x3(0.0f, 0.2f, -1.f);
  f32x3 rd = normalize(f32x3(uv.swizzle<0, 1>(), 1.0f));
  if (m.data[2] < 0.01f) {
    m.data[0] = (iTime + 4.0f) * 0.09f;
    m.data[1] = -0.09f;
  }
  rd.set_swizzle<1, 2>(rd.swizzle<1, 2>() *
                       (rot(__ctx, m.data[1] * (3.1415f * 2.0f))));
  rd.set_swizzle<0, 2>(rd.swizzle<0, 2>() *
                       (rot(__ctx, m.data[0] * (3.1415f * 2.0f))));
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() *
                       (rot(__ctx, m.data[1] * (3.1415f * 2.0f))));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() *
                       (rot(__ctx, m.data[0] * (3.1415f * 2.0f))));
  Data data = Data(f32x3(0.0f), f32x3(0.0f), 32.0f, -1);
  col += render(__ctx, ro, rd, data);
  float depth = data.d / 32.0f;
  col += smoothstep(0.02f, 0.2f, depth);
  col += col * col;
  col = pow(col, f32x3(1.0f / 2.2f));
  O = f32x4(col, 1.0f);
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
