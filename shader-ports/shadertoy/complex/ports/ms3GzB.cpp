// Port of https://www.shadertoy.com/view/ms3GzB; author: PauloFalcao.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ms3GzB.json.
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
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
float sdf_test(Context &__ctx, f32x3 p) {
  f32x3 d = f32x3(0.6f, 0.6f, 2.0f);
  return min(min(sdBox(__ctx, p, d), sdBox(__ctx, p, d.swizzle<0, 2, 1>())),
             sdBox(__ctx, p, d.swizzle<2, 0, 1>()));
}
float bevel_sdf(Context &__ctx, f32x3 p, float b) {
  f32x3 k = f32x3(1.0f, 0.0f, -1.0f) * b;
  float x1 = sdf_test(__ctx, p + k.swizzle<0, 1, 1>());
  float x2 = sdf_test(__ctx, p + k.swizzle<2, 1, 1>());
  float y1 = sdf_test(__ctx, p + k.swizzle<1, 0, 1>());
  float y2 = sdf_test(__ctx, p + k.swizzle<1, 2, 1>());
  float z1 = sdf_test(__ctx, p + k.swizzle<1, 1, 0>());
  float z2 = sdf_test(__ctx, p + k.swizzle<1, 1, 2>());
  return min(min(max((x1 + x2) / 2.0f, (y1 + y2) / 2.0f),
                 max((y1 + y2) / 2.0f, (z1 + z2) / 2.0f)),
             max((z1 + z2) / 2.0f, (x1 + x2) / 2.0f));
}
f32x2 rot(Context &__ctx, f32x2 p, float f) {
  float s = sin(f);
  float c = cos(f);
  return p * f32x2x2(c, -s, s, c);
}
float sdf_a(Context &__ctx, f32x3 p) {
  p.set_swizzle<1, 2>(rot(__ctx, p.swizzle<1, 2>(), sin(iTime * 0.5f) * 2.0f));
  p.set_swizzle<0, 2>(rot(__ctx, p.swizzle<0, 2>(), sin(iTime * 0.2f) * 0.5f));
  p.set_swizzle<0, 1>(rot(__ctx, p.swizzle<0, 1>(), sin(iTime * 0.03f) * 2.0f));
  float b = bevel_sdf(__ctx, p, sin(iTime) * 0.2f + 0.2f);
  return b;
}
f32x3 tex3d_a(Context &__ctx, f32x3 p) { return f32x3(0.9f, 0.6f, 0.3f); }
float sdf_b(Context &__ctx, f32x3 p) {
  return max(p.data[1] + 2.0f, length(p) - 8.0f);
}
f32x3 tex3d_b(Context &__ctx, f32x3 p) {
  p *= 2.0f;
  f32x2 w =
      max(abs(ddx(p.swizzle<0, 2>())), abs(ddy(p.swizzle<0, 2>()))) + 1e-5f;
  if (max(w.data[0], w.data[1]) > 0.5f) {
    return f32x3(mod(floor(p.data[0]) + floor(p.data[2]), 2.0f));
  }
  f32x2 i =
      ((1.f - abs(2.f * fract((p.swizzle<0, 2>() + .5f * w) / 2.f) - 1.f)) -
       (1.f - abs(2.f * fract((p.swizzle<0, 2>() - .5f * w) / 2.f) - 1.f))) /
      w;
  return f32x3(.5f - .5f * i.data[0] * i.data[1]);
}
f32x2 sdf(Context &__ctx, f32x3 p) {
  float sdfa = sdf_a(__ctx, p);
  float sdfb = sdf_b(__ctx, p);
  if (sdfa < sdfb) {
    return f32x2(sdfa, 0.0f);
  } else {
    return f32x2(sdfb, 1.0f);
  }
}
f32x3 raymarch(Context &__ctx, f32x2 uv, f32x3 camera, float cameraZoom,
               f32x3 lookAt, float cameraDistance, f32x3 sun,
               float ambOcclusion, float shadowIntensity,
               float ambLightIntensity, float lightPow, float lightSpecular,
               float reflection);
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>() - 0.5f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 camera = f32x3(sin(sin(iTime * 0.3f) * 0.5f + 0.5f) * 4.0f, 2.0f,
                       cos(cos(iTime * 0.2f) * 0.5f + 0.5f) * 4.0f);
  float cameraZoom = 1.5f;
  f32x3 lookAt = f32x3(0.0f, 0.0f, 0.0f);
  float cameraDistance = 1.1f;
  f32x3 sun = f32x3(0.0f, 2.0f, 2.5f);
  float ambOcclusion = 1.0f;
  float shadowIntensity = 1.0f;
  float ambLightIntensity = 0.25f;
  float lightPow = 128.0f;
  float lightSpecular = 0.7f;
  float reflection = 0.2f;
  f32x3 col = raymarch(__ctx, uv, camera, cameraZoom, lookAt, cameraDistance,
                       sun, ambOcclusion, shadowIntensity, ambLightIntensity,
                       lightPow, lightSpecular, reflection);
  fragColor = f32x4(pow(col, f32x3(1.0f / 2.2f)), 1.0f);
}
f32x3 normal(Context &__ctx, f32x3 p) {
  const f32x2 e = f32x2(0.001f, -0.001f);
  float x = sdf(__ctx, p + e.swizzle<0, 1, 1>()).data[0];
  float y = sdf(__ctx, p + e.swizzle<1, 0, 1>()).data[0];
  float z = sdf(__ctx, p + e.swizzle<1, 1, 0>()).data[0];
  return normalize(f32x3(x - y - z, -x + y - z, -x - y + z) +
                   sdf(__ctx, p + e.swizzle<0, 0, 0>()).data[0]);
}
void march(Context &__ctx, float &d, f32x3 &p, f32x2 &dS, f32x3 ro, f32x3 rd) {
  for (int i = 0; i < 100; i++) {
    p = ro + rd * d;
    dS = sdf(__ctx, p);
    d += dS.data[0];
    if (d > 20.0f || abs(dS.data[0]) < 0.0001f)
      break;
  }
}
float calcAO(Context &__ctx, f32x3 pos, f32x3 nor) {
  float occ = 0.0f;
  float sca = 1.0f;
  for (int i = 0; i < 5; i++) {
    float h = 0.001f + 0.25f * float(i) / 4.0f;
    float d = sdf(__ctx, pos + h * nor).data[0];
    occ += (h - d) * sca;
    sca *= 0.98f;
  }
  return clamp(1.0f - 1.6f * occ, 0.0f, 1.0f);
}
float calcSoftshadow(Context &__ctx, f32x3 ro, f32x3 rd, float mint,
                     float tmax) {
  float res = 1.0f;
  float t = mint;
  float ph = 1e10f;
  for (int i = 0; i < 32; i++) {
    float h = sdf(__ctx, ro + rd * t).data[0];
    res = min(res, 10.0f * h / t);
    t += h;
    if (res < 0.0001f || t > tmax)
      break;
  }
  return clamp(res, 0.0f, 1.0f);
}
f32x3 hdri(Context &__ctx, f32x3 p, float v) {
  return pow(texture(__ctx.channel0, p).swizzle<0, 1, 2>(), f32x3(2.2f)) +
         pow(texture(__ctx.channel0, p).swizzle<0, 0, 0>(), f32x3(16.0f)) * v;
}
f32x3 raymarch(Context &__ctx, f32x2 uv, f32x3 camera, float cameraZoom,
               f32x3 lookAt, float cameraDistance, f32x3 sun,
               float ambOcclusion, float shadowIntensity,
               float ambLightIntensity, float lightPow, float lightSpecular,
               float reflection) {
  f32x3 cam = camera * cameraZoom;
  f32x3 ray = normalize(lookAt - cam);
  f32x3 cX = normalize(cross(f32x3(0.0f, 1.0f, 0.0f), ray));
  f32x3 cY = normalize(cross(ray, cX));
  f32x3 rd =
      normalize(ray * cameraDistance + cX * uv.data[0] + cY * uv.data[1]);
  f32x3 ro = cam;
  float d = 0.f;
  f32x3 p = f32x3(0.f);
  f32x2 dS = f32x2(0.f);
  march(__ctx, d, p, dS, ro, rd);
  f32x3 color = f32x3(0.f);
  f32x3 objColor = (dS.data[1] < 0.5f) ? tex3d_a(__ctx, p) : tex3d_b(__ctx, p);
  f32x3 light = normalize(sun);
  if (d < 20.0f) {
    f32x3 n = normal(__ctx, p);
    float l = clamp(dot(-light, -n), 0.f, 1.0f);
    f32x3 ref = normalize(reflect(rd, -n));
    float r = clamp(dot(ref, light), 0.f, 1.0f);
    float cAO = mix(1.0f, calcAO(__ctx, p, n), ambOcclusion);
    float shadow =
        mix(1.0f, calcSoftshadow(__ctx, p, light, 0.05f, 5.f), shadowIntensity);
    color =
        min(f32x3(max(shadow, ambLightIntensity)), max(l, ambLightIntensity)) *
            max(cAO, ambLightIntensity) * objColor +
        pow(r, lightPow) * lightSpecular;
    d = 0.01f;
    march(__ctx, d, p, dS, p, ref);
    f32x3 objColorRef = f32x3(0.f);
    if (d < 20.0f) {
      objColorRef = (dS.data[1] < 0.5f) ? tex3d_a(__ctx, p) : tex3d_b(__ctx, p);
      n = normal(__ctx, p);
      l = clamp(dot(-light, -n), 0.f, 1.f);
      objColorRef = max(l, ambLightIntensity) * objColorRef;
    } else {
      objColorRef = hdri(__ctx, ref.swizzle<2, 1, 0>(), 1.0f);
    }
    color = mix(color, objColorRef, reflection);
  } else {
    color = hdri(__ctx, rd.swizzle<2, 1, 0>(), 0.0f);
  }
  return color;
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
