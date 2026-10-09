// Port of https://www.shadertoy.com/view/7ddyzn; author: Spi3lot.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7ddyzn.json.
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
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float sdTorus(Context &__ctx, f32x3 p, f32x2 t) {
  f32x2 q = f32x2(length(p.swizzle<0, 2>()) - t.data[0], p.data[1]);
  return length(q) - t.data[1];
}
float sdDisk(Context &__ctx, f32x3 p) {
  float torus = sdTorus(__ctx, p, f32x2(0.0f, 4.0f));
  float plane = abs(p.data[1]);
  return max(torus, plane);
}
float sdBlackHole(Context &__ctx, f32x3 p) {
  return length(p - f32x3(0, 0, 0)) - 0.8f;
}
float GetDist(Context &__ctx, f32x3 p) {
  float disk = sdDisk(__ctx, p);
  float bhole = sdBlackHole(__ctx, p);
  return min(disk, bhole);
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p);
  f32x2 e = f32x2(0.001f, 0);
  return normalize(d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()),
                             GetDist(__ctx, p - e.swizzle<1, 0, 1>()),
                             GetDist(__ctx, p - e.swizzle<1, 1, 0>())));
}
f32x4 RayMarch(Context &__ctx, f32x3 ro, f32x3 &rd, float side) {
  f32x3 p = ro;
  float minDist = 1e20f;
  float traveled = 0.0f;
  for (int i = 0; i < 10000; i++) {
    float dS = side * GetDist(__ctx, p);
    traveled += 0.1f * dS;
    if (abs(dS) < minDist)
      minDist = abs(dS);
    if (abs(dS) <= 0.001f || traveled > 1000.0f)
      break;
    f32x3 d = p - f32x3(0, 0, 0);
    float r = dot(d, d);
    float bend = 0.5f / r;
    f32x3 n = d * inversesqrt(r);
    rd = normalize(rd - n * (bend * 0.1f * dS));
    p += rd * 0.1f * dS;
  }
  return f32x4(p, minDist);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord - iResolution.swizzle<0, 1>() * 0.5f) /
             min(iResolution.data[0], iResolution.data[1]);
  f32x2 mouse;
  if (iMouse.data[0] == 0.0f && iMouse.data[1] == 0.0f)
    mouse = f32x2(0.5f, 0.48f);
  else
    mouse = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float yaw = mouse.data[0] * 2.0f * 3.1415926535f;
  float pitch = mouse.data[1] * 3.1415926535f;
  f32x4 cs = f32x4(cos(yaw), sin(yaw), cos(pitch), sin(pitch));
  f32x3 ro = 7.0f * cs.swizzle<0, 2, 1>() * f32x3(cs.data[3], 1, cs.data[3]);
  f32x3 lookAt = f32x3(0, 0, 0), f = normalize(lookAt - ro),
        r = normalize(cross(f, f32x3(0, 1, 0))), u = cross(r, f),
        c = ro + f * 1.0f, i = c + r * uv.data[0] + u * uv.data[1],
        rd = normalize(i - ro);
  f32x3 col = f32x3(0);
  float j;
  float iterations;
  iterations = float(10);
  for (j = 0.f; j < iterations; j++) {
    f32x4 p_minDist = RayMarch(__ctx, ro, rd, 1.0f);
    f32x3 p = p_minDist.swizzle<0, 1, 2>();
    if (sdBlackHole(__ctx, p) <= 0.001f) {
      break;
    } else if (sdDisk(__ctx, p) <= 0.001f) {
      ro = p;
      ro.data[1] -= sign(p.data[1]) * 2.0f * 0.001f;
      float a = 0.5f + 0.5f * atan(p.data[2], p.data[0]) / 3.1415926535f;
      float d = 1.0f - sdBlackHole(__ctx, p) / 4.0f;
      float motion = iTime * 0.25f;
      float u = a + motion;
      float v = d;
      f32x3 tex = texture(__ctx.channel0, f32x2(u, v)).swizzle<0, 1, 2>();
      float brightness = dot(tex, f32x3(0.3f, 0.6f, 0.1f));
      col += exp2(-j) * 4.0f * d * brightness * f32x3(1.0f, 0.3f, 0.0f);
      col += 2.0f * d * d;
    } else {
      break;
    }
  }
  fragColor = f32x4(col, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0,
               __binding__(0, 1) texture_2d<f32x4> channel1,
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
  Context __ctx{{channel0, sampling0}, {channel1, sampling1}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
