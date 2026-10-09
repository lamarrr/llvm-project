// Port of https://www.shadertoy.com/view/ddGBzt; author: Monetenbube.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ddGBzt.json.
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
struct MarchResult {
  float d;
  f32x3 c;
};
void repeatXZ(Context &__ctx, f32x3 &ray, f32x2 r) {
  f32x2 tmp = mod(ray.swizzle<0, 2>(), r) - 0.5f * r;
  ray = f32x3(tmp.data[0], ray.data[1], tmp.data[1]);
}
void repeatXYZ(Context &__ctx, f32x3 &ray, f32x3 r) {
  ray = mod(ray, r) - 0.5f * r;
}
MarchResult opU(Context &__ctx, MarchResult a, MarchResult b) {
  if (a.d < b.d) {
    return a;
  }
  return b;
}
MarchResult opSU(Context &__ctx, MarchResult b, MarchResult a, float k) {
  float smin = -log2(exp2(-k * a.d) + exp2(-k * b.d)) / k;
  float t = clamp(b.d - a.d, -1.0f, 1.0f) / 2.0f + 0.5f;
  a.c = a.c * t + b.c * (1.0f - t);
  a.d = smin;
  return a;
}
MarchResult sdSphere(Context &__ctx, f32x3 ray, f32x3 p, float r, f32x3 c) {
  return MarchResult(distance(ray, p) - r, c);
}
MarchResult sdHPlane(Context &__ctx, f32x3 ray, float h, f32x3 c) {
  return MarchResult(ray.data[1] - h, c);
}
MarchResult sdMandelBulb(Context &__ctx, f32x3 pos, float power, int max_i,
                         f32x3 c) {
  f32x3 z = pos;
  float dr = 1.0f;
  float r = 0.0f;
  for (int i = 0; i < max_i; i++) {
    r = length(z);
    if (r > 4.0f)
      break;
    float theta = acos(z.data[2] / r);
    float phi = atan(z.data[1], z.data[0]);
    dr = pow(r, power - 1.0f) * power * dr + 1.0f;
    float zr = pow(r, power);
    theta = theta * power;
    phi = phi * power;
    z = zr * f32x3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
    z += pos;
  }
  return MarchResult(0.5f * log(r) * r / dr, c);
}
MarchResult sdScene(Context &__ctx, f32x3 ray) {
  return sdMandelBulb(__ctx, ray, 8.0f, min(10, int(iTime / 10.0f + 3.0f)),
                      f32x3(1.0f, 1.0f, 1.0f));
}
f32x3 calculate_normal(Context &__ctx, f32x3 p) {
  const f32x3 s = f32x3(0.001f, 0.0f, 0.0f);
  float gradient_x = sdScene(__ctx, p + s.swizzle<0, 1, 1>()).d -
                     sdScene(__ctx, p - s.swizzle<0, 1, 1>()).d;
  float gradient_y = sdScene(__ctx, p + s.swizzle<1, 0, 1>()).d -
                     sdScene(__ctx, p - s.swizzle<1, 0, 1>()).d;
  float gradient_z = sdScene(__ctx, p + s.swizzle<1, 1, 0>()).d -
                     sdScene(__ctx, p - s.swizzle<1, 1, 0>()).d;
  f32x3 normal = f32x3(gradient_x, gradient_y, gradient_z);
  return normalize(normal);
}
MarchResult marchRay(Context &__ctx, f32x3 ray, f32x3 rd) {
  for (int i = 0; i < 1000; i++) {
    MarchResult res = sdScene(__ctx, ray);
    if (res.d > 100.0f) {
      res.c = f32x3(0.3f, 0.8f, 1.0f);
      return res;
    } else if (res.d < 0.001f) {
      f32x3 light_position = f32x3(5.0f, 5.0f, 5.0f);
      f32x3 dir_to_light = normalize(light_position - ray);
      f32x3 normal = calculate_normal(__ctx, ray);
      float diffuse_intensity = max(0.3f, dot(normal, dir_to_light));
      res.c *= diffuse_intensity;
      MarchResult closest = MarchResult(100.0f, f32x3(0.0f));
      ray += dir_to_light * 0.9f;
      for (int j = 0; j < 1000; j++) {
        MarchResult sres = sdScene(__ctx, ray);
        if (sres.d < closest.d) {
          closest = sres;
        }
        if (sres.d > 100.0f) {
          break;
        } else if (res.d < 0.001f) {
          break;
        }
        ray += dir_to_light * sres.d;
      }
      float shadow_intensity = closest.d / 0.9f;
      res.c *= shadow_intensity;
      return res;
    } else {
      ray += rd * res.d;
    }
  }
  return MarchResult(100.0f, f32x3(0.3f, 0.8f, 1.0f));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>() * 2.0f - 1.0f;
  f32x3 ro = f32x3(cos(iTime / 2.0f) * 2.5f, -sin(iTime / 2.0f) * 2.5f,
                   sin(iTime / 2.0f) * 2.5f);
  f32x3 ta = f32x3(0.0f);
  f32x3 cw = normalize(ta - ro);
  f32x3 cp = f32x3(sin(0.0f), cos(0.0f), 0.0f);
  f32x3 cu = normalize(cross(cw, cp));
  f32x3 cv = (cross(cu, cw));
  f32x3x3 camera_matrix = f32x3x3(cu, cv, cw);
  f32x3 rd = camera_matrix *
             normalize(f32x3(
                 uv.data[0],
                 uv.data[1] * iResolution.data[1] / iResolution.data[0], 1.0f));
  MarchResult res = marchRay(__ctx, ro, rd);
  fragColor = f32x4(res.c, 1.0f);
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
