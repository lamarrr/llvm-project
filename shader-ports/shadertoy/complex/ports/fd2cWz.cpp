// Port of https://www.shadertoy.com/view/fd2cWz; author: pancakespeople.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fd2cWz.json.
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

f32x2 random2(Context &__ctx, f32x2 st) {
  st = f32x2(dot(st, f32x2(127.1f, 311.7f)), dot(st, f32x2(269.5f, 183.3f)));
  return -1.0f + 2.0f * fract(sin(st) * 43758.5453123f * 0.7897f);
}
float noise(Context &__ctx, f32x2 st) {
  f32x2 i = floor(st);
  f32x2 f = fract(st);
  f32x2 u = f * f * (3.0f - 2.0f * f);
  return mix(
      mix(dot(random2(__ctx, i + f32x2(0.0f, 0.0f)), f - f32x2(0.0f, 0.0f)),
          dot(random2(__ctx, i + f32x2(1.0f, 0.0f)), f - f32x2(1.0f, 0.0f)),
          u.data[0]),
      mix(dot(random2(__ctx, i + f32x2(0.0f, 1.0f)), f - f32x2(0.0f, 1.0f)),
          dot(random2(__ctx, i + f32x2(1.0f, 1.0f)), f - f32x2(1.0f, 1.0f)),
          u.data[0]),
      u.data[1]);
}
float fbm(Context &__ctx, f32x2 x) {
  float v = 0.0f;
  float a = 0.5f;
  f32x2 shift = f32x2(100);
  f32x2x2 rot = f32x2x2(cos(0.5f), sin(0.5f), -sin(0.5f), cos(0.50f));
  for (int i = 0; i < 5; ++i) {
    v += a * noise(__ctx, x);
    x = rot * x * 2.0f + shift;
    a *= 0.5f;
  }
  return v;
}
float sdf(Context &__ctx, f32x3 point) {
  float plane = length(max(point.data[1], 0.0f)) +
                fbm(__ctx, point.swizzle<0, 2>() / 10.0f) * 10.0f - 0.5f;
  return plane;
}
float rayMarch(Context &__ctx, f32x3 rayOrigin, f32x3 rayDir) {
  float distMarched = 0.0f;
  for (int i = 0; i < 100; i++) {
    float dist = sdf(__ctx, rayOrigin);
    rayOrigin += rayDir * dist;
    distMarched += dist;
    if (distMarched < 0.01f || dist > 100.0f) {
      break;
    }
  }
  return distMarched;
}
f32x3 getNormal(Context &__ctx, f32x3 point) {
  float dist = sdf(__ctx, point);
  f32x3 norm = dist - f32x3(sdf(__ctx, point - f32x3(0.01f, 0.0f, 0.0f)),
                            sdf(__ctx, point - f32x3(0.0f, 0.01f, 0.0f)),
                            sdf(__ctx, point - f32x3(0.0f, 0.0f, 0.01f)));
  return normalize(norm);
}
f32x3 getColor(Context &__ctx, f32x3 point) {
  float n = fbm(__ctx, point.swizzle<0, 2>() * 10.0f) + 0.5f;
  if (point.data[1] > 0.6f) {
    return f32x3(1.0f) * n;
  } else if (point.data[1] > 0.4f) {
    return f32x3(0.0f, 1.0f, 0.0f) * n;
  } else {
    return f32x3(0.0f, 0.0f, 1.0f) * n;
  }
}
float getSpecular(Context &__ctx, f32x3 point, f32x3 lightDir,
                  f32x3 cameraPos) {
  f32x3 viewDir = normalize(cameraPos - point);
  f32x3 reflectDir = reflect(-lightDir, getNormal(__ctx, point));
  return pow(max(dot(viewDir, reflectDir), 0.0f), 32.0f);
}
float getSpecularity(Context &__ctx, f32x3 point) {
  if (point.data[1] < 0.4f) {
    return 2.0f;
  }
  return 1.0f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[0];
  f32x3 cameraPos = f32x3(0.0f, 3.5f + sin(iTime / 5.0f), iTime);
  f32x3 rayDir =
      f32x3(uv.data[0] + sin(iTime / 5.0f), uv.data[1] - 0.25f, 1.0f);
  f32x3 lightDir = f32x3(0.5f, 0.5f, 0.7f);
  float dist = rayMarch(__ctx, cameraPos, rayDir);
  f32x3 point = rayDir * dist + cameraPos;
  f32x3 normal = getNormal(__ctx, point);
  float light = dot(normal, lightDir);
  float lightDist = rayMarch(__ctx, point + normal * 0.01f, lightDir);
  if (lightDist < 90.0f) {
    light *= 0.1f;
  }
  f32x3 col = f32x3(light * f32x3(0.7f, 0.7f, 1.0f));
  col = mix(col, f32x3(0.7f, 0.7f, 1.0f), dist / 100.0f);
  col = mix(col, f32x3(0.0f), point.data[1] / 10.0f);
  col *=
      getColor(__ctx, point) + getSpecular(__ctx, point, lightDir, cameraPos) *
                                   getSpecularity(__ctx, point);
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
