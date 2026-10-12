// Port of https://www.shadertoy.com/view/Nt2yRd; author: Spi3lot.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Nt2yRd.json.
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
float iterations = -1.0f;
float sdf(Context &__ctx, f32x3 pos) {
  f32x3 z = pos;
  float dr = 1.0f;
  float r;
  for (int i = 0; i < 50; i++) {
    r = length(z);
    if (r > 2.0f) {
      iterations = float(i);
      break;
    }
    float theta = 2.0f * acos(z.data[2] / r) - 3.1415926535f * 0.5f;
    float phi = 2.0f * atan(z.data[1], z.data[0]);
    dr = pow(r, 2.0f - 1.0f) * 2.0f * dr + 1.0f;
    float zr = pow(r, 2.0f);
    z = zr * f32x3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
    z += pos;
  }
  return 0.5f * log(r) * r / dr;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  const float eps = 5.0f;
  const f32x2 h = f32x2(eps, 0);
  f32x3 a = f32x3(sdf(__ctx, p - h.swizzle<0, 1, 1>()),
                  sdf(__ctx, p - h.swizzle<1, 0, 1>()),
                  sdf(__ctx, p - h.swizzle<1, 1, 0>()));
  f32x3 b = f32x3(sdf(__ctx, p + h.swizzle<0, 1, 1>()),
                  sdf(__ctx, p + h.swizzle<1, 0, 1>()),
                  sdf(__ctx, p + h.swizzle<1, 1, 0>()));
  return normalize(b - a);
}
f32x3 GetLight(Context &__ctx, f32x3 pos) {
  return normalize(10.0f * f32x3(cos(iTime * 1.0f), sin(iTime * 1.0f), -1) -
                   pos);
}
f32x4 march(Context &__ctx, f32x3 ro, f32x3 rd) {
  rd = normalize(rd);
  float minDist = 1.0f / 0.0f;
  for (int i = 0; i < 100; i++) {
    float dist = sdf(__ctx, ro);
    if (dist < minDist)
      minDist = dist;
    if (dist <= 0.001f) {
      f32x3 normal = GetNormal(__ctx, ro);
      f32x3 ro2Light = GetLight(__ctx, ro);
      float light = clamp(dot(normal, ro2Light), 0.0f, 1.0f);
      float ambientExposure = 1.0f;
      ro -= rd * 0.001f * 2.0f;
      for (int j = 0; j < 100; j++) {
        float distLightSource = length(
            10.0f * f32x3(cos(iTime * 1.0f), sin(iTime * 1.0f), -1) - ro);
        if (distLightSource <= 0.001f) {
          ambientExposure = 1.0f;
          break;
        }
        float distMandel = sdf(__ctx, ro);
        if (distMandel <= 0.001f) {
          ambientExposure = 0.5f;
          break;
        }
        ro += ro2Light * min(distLightSource, distMandel);
      }
      f32x4 col = f32x4(1);
      return light * ambientExposure * col;
    }
    ro += rd * dist;
  }
  return f32x4(1e-3f / minDist);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 norm = uv - 0.5f;
  f32x2 fov = f32x2(0.4f * 3.1415926535f / 1.0f, 0.4f * 3.1415926535f / 1.0f *
                                                     iResolution.data[1] /
                                                     iResolution.data[0]);
  f32x3 ro = f32x3(
      10.0f * f32x3(cos(iTime * 1.0f), sin(iTime * 1.0f), -1).swizzle<0, 1>() /
          20.0f,
      -4);
  f32x3 rd = f32x3(norm * fov, 1.0f);
  f32x4 col;
  col = march(__ctx, ro, rd);
  fragColor = col;
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
