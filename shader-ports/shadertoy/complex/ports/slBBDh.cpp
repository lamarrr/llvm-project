// Port of https://www.shadertoy.com/view/slBBDh; author: droidkid.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/slBBDh.json.
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

f32x2 opU(Context &__ctx, f32x2 d1, f32x2 d2) {
  return (d1.data[0] < d2.data[0]) ? d1 : d2;
}
f32x3 pallete(Context &__ctx, float t) {
  f32x3 a = f32x3(0.2f);
  f32x3 b = f32x3(0.5f);
  f32x3 c = f32x3(1.0f);
  f32x3 d = f32x3(0.0f, 0.33f, 0.67f);
  return a + b * cos(6.28318f * (c * t + d));
}
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
f32x2 map(Context &__ctx, f32x3 p) {
  f32x2 d = f32x2(length(p - f32x3(0.0f, 0.2f + cos(iTime) * 0.05f,
                                   0.0f + sin(iTime) * 0.02f)) -
                      0.2f,
                  0.05f);
  d = opU(__ctx, d,
          f32x2(length(p - f32x3(0.5f, 0.05f + sin(1.4f * iTime + 2.0f) * 0.03f,
                                 0.0f + cos(iTime) * 0.03f)) -
                    0.1f,
                2.2f));
  d = opU(__ctx, d,
          f32x2(length(p - f32x3(-0.4f + cos(iTime) * 0.02f,
                                 0.1f + sin(iTime) * 0.01f,
                                 0.1f * sin(3.0f * iTime) * 0.3f)) -
                    0.1f,
                0.8f));
  d = opU(
      __ctx, d,
      f32x2(sdBox(__ctx, p - f32x3(0.0f, 0.0f, 0.5f), f32x3(2.1f, 3.1f, 0.1f)),
            0.7f));
  d = opU(
      __ctx, d,
      f32x2(sdBox(__ctx, p - f32x3(0.0f, -0.3f, 0.0f), f32x3(2.1f, 0.1f, 2.1f)),
            0.6f));
  return d;
}
float shadow(Context &__ctx, f32x3 p, f32x3 light) {
  f32x3 ro = p;
  f32x3 rd = normalize(light - p);
  float tmin = 0.21f;
  float t = tmin;
  int maxSteps = 700;
  float res = 1.0f;
  for (int i = 0; i < maxSteps; i++) {
    f32x3 p = ro + t * rd;
    f32x2 h = map(__ctx, p);
    if (h.data[0] < 0.0001f) {
      return 0.0f;
    }
    res = min(res, 32.0f * h.data[0] / t);
    t += h.data[0];
  }
  return res;
}
f32x3 calcNormal(Context &__ctx, f32x3 p) {
  f32x3 d = f32x3(0.0001f, 0.0f, 0.0f);
  float dx = (map(__ctx, p + d.swizzle<0, 1, 1>()) -
              map(__ctx, p - d.swizzle<0, 1, 1>()))
                 .data[0];
  float dy = (map(__ctx, p + d.swizzle<1, 0, 1>()) -
              map(__ctx, p - d.swizzle<1, 0, 1>()))
                 .data[0];
  float dz = (map(__ctx, p + d.swizzle<1, 1, 0>()) -
              map(__ctx, p - d.swizzle<1, 1, 0>()))
                 .data[0];
  return normalize(f32x3(dx, dy, dz));
}
f32x3x3 setUpCamera(Context &__ctx, f32x3 camera_pos, f32x3 camera_target,
                    float angle) {
  f32x3 up = f32x3(sin(angle), cos(angle), 0.0f);
  f32x3 cz = normalize(camera_target - camera_pos);
  f32x3 cx = normalize(cross(up, cz));
  f32x3 cy = normalize(cross(cz, cx));
  return f32x3x3(cx, cy, cz);
}
f32x2 rayCast(Context &__ctx, f32x3 ro, f32x3 rd) {
  float tmin = 0.1f;
  float tmax = 200.0f;
  float t = tmin;
  int maxSteps = 70;
  f32x2 res = f32x2(-1.0f, -7.0f);
  for (int i = 0; i < maxSteps && t < tmax; i++) {
    f32x3 p = ro + t * rd;
    f32x2 h = map(__ctx, p);
    if (h.data[0] < 0.0001f) {
      res = f32x2(t, h.data[1]);
      break;
    }
    t += h.data[0];
  }
  return res;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 ro = f32x3(0.0f, 0.1f, -1.0f);
  f32x3 ta = f32x3(0.0f, 0.1f, 0.0f);
  f32x3x3 camera = setUpCamera(__ctx, ro, ta, 0.0f);
  float fo = 2.5f;
  f32x2 uv =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 rd = camera * normalize(f32x3(uv, fo));
  f32x2 h = rayCast(__ctx, ro, rd);
  if (h.data[1] < 0.0f) {
    fragColor = f32x4(0.0f);
    return;
  }
  f32x3 light = f32x3(0.5f, 0.5f, -1.0f);
  f32x3 point = ro + rd * h.data[0];
  f32x3 ld = normalize(light - point);
  f32x3 normal = calcNormal(__ctx, point);
  f32x3 color = pallete(__ctx, h.data[1]) * max(0.0f, dot(normal, ld));
  f32x3 hf = normalize(normal + ld);
  color += f32x3(0.5f) * pow(dot(normal, hf), 1000.0f);
  color = shadow(__ctx, point, light) * color;
  color = pow(color, f32x3(0.4545f));
  fragColor = f32x4(color, 0.0f);
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
