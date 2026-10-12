// Port of https://www.shadertoy.com/view/mdtSD8; author: 42yeah.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mdtSD8.json.
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

f32x2 hash22(Context &__ctx, f32x2 uv) {
  return fract(sin(f32x2(dot(uv, f32x2(13.1323f, 57.81f)),
                         dot(uv, f32x2(9.8173f, 17.2552f)))) *
               10580.1f);
}
float hash(Context &__ctx, f32x2 uv) {
  return fract(sin(dot(uv, f32x2(13.1323f, 57.81f))) * 10580.1f);
}
float perlin(Context &__ctx, f32x2 uv) {
  f32x2 u = floor(uv);
  f32x2 f = fract(uv);
  f32x2 s = smoothstep(0.0f, 1.0f, f);
  f32x2 a = hash22(__ctx, u);
  f32x2 b = hash22(__ctx, u + f32x2(1.0f, 0.0f));
  f32x2 c = hash22(__ctx, u + f32x2(0.0f, 1.0f));
  f32x2 d = hash22(__ctx, u + f32x2(1.0f, 1.0f));
  return mix(mix(dot(a, -f), dot(b, f32x2(1.0f, 0.0f) - f), s.data[0]),
             mix(dot(c, f32x2(0.0f, 1.0f) - f), dot(d, f32x2(1.0f, 1.0f) - f),
                 s.data[0]),
             s.data[1]);
}
f32x2 voronoi(Context &__ctx, f32x2 uv) {
  f32x2 f = fract(uv);
  f32x2 u = floor(uv);
  float closest = 100.0f;
  float id = 0.0f;
  for (int y = -1; y <= 1; y++) {
    for (int x = -1; x <= 1; x++) {
      f32x2 d = f32x2(float(x), float(y));
      f32x2 nu = u + d;
      f32x2 p = hash22(__ctx, nu);
      float dist = distance(f, p + d);
      if (dist < closest) {
        closest = dist;
        id = hash(__ctx, nu);
      }
    }
  }
  return f32x2(1.0f - closest, id);
}
f32x3 voronoi_3d(Context &__ctx, f32x3 p) {
  float grassHeight = 1.0f;
  f32x2 uv = p.swizzle<0, 2>() * 8.0f;
  float id = voronoi(__ctx, uv).data[1];
  float windyness =
      max(0.0f, p.data[1] * p.data[1] * (0.5f + id * 0.5f) * 5.0f) * 0.5f;
  uv = f32x2(uv.data[0] + windyness * sin(0.66f * iTime),
             uv.data[1] + windyness * cos(0.5f * iTime));
  f32x2 vor = voronoi(__ctx, uv);
  float boundary = mix(0.0f, 1.44f, max(p.data[1] / grassHeight, 0.0f));
  return f32x3(boundary - vor.data[0], vor.data[0] - boundary, vor.data[1]);
}
float ball(Context &__ctx, f32x3 p, f32x3 &mat) {
  mat = f32x3(1.0f, 0.5f, 0.0f);
  return length(p) - 1.0f;
}
float perlin_height(Context &__ctx, f32x2 uv) {
  return perlin(__ctx, uv * 0.1f) * 5.0f + perlin(__ctx, uv * 0.01f) * 40.0f;
}
float ground(Context &__ctx, f32x3 p, f32x3 &mat) {
  mat = f32x3(0.2f, 0.4f, 0.1f);
  return p.data[1] - 1.0f;
}
float map(Context &__ctx, f32x3 p, f32x3 &mat) {
  f32x3 minMat = f32x3(1.0f, 0.0f, 1.0f);
  float minDist = ground(__ctx, p, minMat), dist;
  mat = minMat;
  return minDist;
}
f32x3 calcNormal(Context &__ctx, f32x3 p) {
  f32x3 trash;
  f32x2 eps = f32x2(0.001f, 0.0f);
  float t = map(__ctx, p, trash);
  return normalize(f32x3(t - map(__ctx, p - eps.swizzle<0, 1, 1>(), trash),
                         t - map(__ctx, p - eps.swizzle<1, 0, 1>(), trash),
                         t - map(__ctx, p - eps.swizzle<1, 1, 0>(), trash)));
}
bool march(Context &__ctx, f32x3 ro, f32x3 rd, float &t, f32x3 &mat) {
  t = 0.0f;
  mat = f32x3(0.0f);
  for (int i = 0; i < 200; i++) {
    f32x3 p = ro + t * rd;
    float dt = map(__ctx, p, mat);
    if (dt < 0.001f) {
      return true;
    }
    t += dt;
  }
  return false;
}
f32x4 grassBlades(Context &__ctx, f32x3 ro, f32x3 rd, float t) {
  f32x3 grassColor = f32x3(0.0f);
  f32x3 base = f32x3(0.0f, -0.4f, 0.0f);
  f32x4 color = f32x4(0.0f, 0.0f, 0.0f, 0.0f);
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + t * rd + base;
    f32x3 grass = voronoi_3d(__ctx, p);
    t += max(0.001f, t * 0.04f);
    if (grass.data[0] > 0.0f) {
      continue;
    }
    float den = clamp(min(grass.data[1], 1.0f - color.data[3]), 0.0f, 1.0f);
    float color_dense = 1.0f - den;
    color_dense = clamp(color_dense + p.data[1] - 0.64f, 0.0f, 1.0f);
    f32x3 bladeColor =
        mix(f32x3(0.0f, 0.0f, 0.0f), f32x3(0.3f, 0.9f, 0.2f), color_dense);
    bladeColor = mix(bladeColor, f32x3(1.0f, 1.0f, 1.0f),
                     clamp(p.data[1] - 0.45f, 0.0f, 1.0f));
    color.set_swizzle<0, 1, 2>(color.swizzle<0, 1, 2>() + bladeColor * den);
    color.data[3] += den;
    if (color.data[3] > 0.99f) {
      break;
    }
  }
  return color;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float aspect = iResolution.data[0] / iResolution.data[1];
  f32x2 uv =
      (fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>()) * 2.0f - 1.0f;
  uv.data[0] *= aspect;
  f32x2 camAngle = f32x2(sin(iTime * 0.1f) * 0.1f, cos(iTime * 0.08f) * 0.2f);
  f32x3 ro = f32x3(0.0f, 1.8f, iTime * 0.01f);
  f32x3 front = normalize(f32x3(
      cos(camAngle.data[0]) * sin(-camAngle.data[1]), sin(camAngle.data[0]),
      cos(camAngle.data[0]) * cos(-camAngle.data[1])));
  f32x3 right = normalize(cross(front, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 up = normalize(cross(right, front));
  f32x3x3 lookAt = f32x3x3(right, up, front);
  f32x3 rd = lookAt * normalize(f32x3(uv.data[0], uv.data[1], 2.0f));
  float t = 0.0f;
  f32x3 mat = f32x3(0.0f);
  bool intersect = march(__ctx, ro, rd, t, mat);
  f32x3 color = f32x3(0.0f);
  f32x3 skyColor =
      f32x3(mix(f32x3(0.5f, 0.9f, 0.9f), f32x3(0.1f, 1.0f, 1.0f), rd.data[1]));
  if (intersect) {
    f32x4 grassColor = grassBlades(__ctx, ro + t * rd, rd, t * 0.01f);
    color = mix(mat, grassColor.swizzle<0, 1, 2>(), grassColor.data[3]);
    color = pow(color, f32x3(0.4545f));
    color = mix(color, skyColor, min(1.0f, max(t - 100.0f, 0.0f)));
  } else {
    color = skyColor;
  }
  fragColor = f32x4(color, 1.0f);
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
