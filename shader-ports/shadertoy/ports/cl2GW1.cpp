// Port of https://www.shadertoy.com/view/cl2GW1; author: tianluo97.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cl2GW1.json.
#include "../support.h"
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

float random(Context &__ctx, f32x2 co) {
  float a = 12.9898f;
  float b = 78.233f;
  float c = 43758.5453f;
  float dt = dot(co.swizzle<0, 1>(), f32x2(a, b));
  float sn = mod(dt, 3.14f);
  return fract(sin(sn) * c);
}
float noise(Context &__ctx, f32x2 st) {
  f32x2 i = floor(st);
  f32x2 f = fract(st);
  float a = random(__ctx, i);
  float b = random(__ctx, i + f32x2(1.0f, 0.0f));
  float c = random(__ctx, i + f32x2(0.0f, 1.0f));
  float d = random(__ctx, i + f32x2(1.0f, 1.0f));
  f32x2 u = f * f * (3.0f - 2.0f * f);
  return mix(a, b, u.data[0]) + (c - a) * u.data[1] * (1.0f - u.data[0]) +
         (d - b) * u.data[0] * u.data[1];
}
float fbm(Context &__ctx, f32x2 _st) {
  float v = 0.0f;
  float a = 0.5f;
  f32x2 shift = f32x2(100.0f);
  f32x2x2 rot = f32x2x2(cos(0.5f), sin(0.5f), -sin(0.5f), cos(0.50f));
  for (int i = 0; i < 3; ++i) {
    v += a * noise(__ctx, _st);
    _st = rot * _st * 2.0f + shift;
    a *= 0.5f;
  }
  return v;
}
const f32x2 target_resolution = f32x2(938, 84);
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 texCoord = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 uv = texCoord;
  uv -= .5f;
  float r = (iResolution.data[0] / iResolution.data[1]) /
            (target_resolution.data[0] / target_resolution.data[1]);
  if (r >= 1.f)
    uv.data[0] *= r;
  else
    uv.data[1] /= r;
  uv += .5f;
  uv.data[1] -= 0.5f / r - 0.5f;
  f32x4 color1 = f32x4(0.71373f, 0.85882f, 0.92157f, 1.0f);
  f32x4 color2 = f32x4(0.60000f, 0.67843f, 1.0f, 1.0f);
  f32x4 color3 = f32x4(0.0f, 0.0f, 1.0f, 1.0f);
  float PI = 3.1415926f;
  float time = iTime * 0.3f;
  float scale = 3.0f;
  float colorStep1 = 0.33333f;
  float colorStep2 = 0.66666f;
  float ease[3];
  ease[0] = -(cos(PI * fract(time / scale)) - 1.0f) / 2.0f * scale;
  ease[1] = fract(time / scale) * scale;
  ease[2] = (1.0f - pow(1.0f - fract(time / scale), 3.0f)) * scale;
  float easeChoice = mod(time / scale, 4.0f);
  int i = int(easeChoice);
  float offset = ease[i] + floor(time / scale) * scale;
  offset = uv.data[1] - offset;
  float y = mod(offset, scale);
  float step1 = scale * (colorStep1);
  float step2 = scale * max(colorStep1 + 0.1f, (colorStep2));
  if (y < step1) {
    float dist1 = smoothstep(0.0f, step1, y);
    fragColor = mix(color3, color1, dist1);
  } else if (y > step1 && y < step2) {
    float dist2 = smoothstep(step1, step2, y);
    fragColor = f32x4(dist2, dist2, dist2, 1.0f);
    fragColor = mix(color1, color2, dist2);
  } else {
    float dist3 = smoothstep(step2, 1.0f * scale, y);
    fragColor = f32x4(dist3, dist3, dist3, 1.0f);
    fragColor = mix(color2, color3, dist3);
  }
  f32x2 uv2 = uv;
  uv2.data[0] *= 20.0f;
  uv2.data[1] *= 4.0f;
  uv2.data[0] += time * 0.4f;
  uv2.data[1] -= time;
  f32x3 col = mix(f32x3(0.0f, 0.0f, 0.f), f32x3(1.f),
                  fbm(__ctx, uv2 - f32x2(2.0f * iTime)));
  f32x3 color = f32x3(0.0f);
  f32x2 q = f32x2(0.f);
  q.data[0] = fbm(__ctx, uv2 - .4f * iTime);
  q.data[1] = fbm(__ctx, uv2 - f32x2(1.0f));
  f32x2 g = f32x2(0.f);
  g.data[0] = fbm(__ctx, uv2 - 1.0f * q + f32x2(1.7f, 9.2f) - .5f * iTime);
  g.data[1] = fbm(__ctx, uv2 - 1.0f * q + f32x2(8.3f, 2.8f) - .26f * iTime);
  float f = fbm(__ctx, uv2 + g);
  color = mix(f32x3(0.0f, 0.0f, 0.0f), f32x3(.2f, .2f, .2f),
              clamp((f * f) * 4.0f, 0.0f, 1.0f));
  color = mix(color, f32x3(0.7f, 0.7f, 0.7f), clamp(length(q), 0.0f, 1.0f));
  color =
      mix(color, f32x3(1.0f, 1.0f, 1.0f), clamp(length(g.data[0]), 0.0f, 1.0f));
  f32x4 fbmColor = f32x4((f * f * f + .6f * f * f + .5f * f) * f32x3(1.f), 1.f);
  fragColor =
      mix(fragColor, f32x4(1.0f, 1.0f, 1.0f, 1.0f), fbmColor.data[0] * 0.5f);
  if (uv.data[0] < 0.f || uv.data[0] > 1.f || uv.data[1] < 0.f ||
      uv.data[1] > 1.f)
    fragColor = f32x4(0);
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
