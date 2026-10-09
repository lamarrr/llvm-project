// Port of https://www.shadertoy.com/view/md2GDD; author: leon.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/md2GDD.json.
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
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float fft, material, rnd;
f32x2x2 rot(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, s, -s, c);
}
float gyroid(Context &__ctx, f32x3 p) {
  return dot(sin(p), cos(p.swizzle<1, 2, 0>()));
}
float random(Context &__ctx, f32x2 p) {
  return fract(sin(dot(p, f32x2(10.1324f, 5.654f))) * 46501.654f);
}
float fbm(Context &__ctx, f32x3 p) {
  float result = 0.f, a = .5f;
  for (float i = 0.f; i < 3.f; ++i) {
    result += abs(gyroid(__ctx, p / a) * a);
    a /= 2.f;
  }
  return result;
}
float box(Context &__ctx, f32x3 p, f32x3 s) {
  f32x3 b = abs(p) - s;
  return max(b.data[0], max(b.data[1], b.data[2]));
}
float map(Context &__ctx, f32x3 p) {
  float dist = 100.f;
  float t = iTime * 1.f + p.data[2] * .5f;
  t = pow(fract(t), 10.f) + floor(t);
  t += rnd;
  float tt = iTime + p.data[2];
  tt = pow(fract(tt), 10.f) + floor(tt);
  float r = .0f * fft + .2f + .1f * sin(length(p) * 3.f - tt + p.data[2] * 5.f);
  float a = 1.f;
  const float count = 12.f;
  for (float i = 0.f; i < count; ++i) {
    p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (rot(__ctx, t / a)));
    p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (rot(__ctx, sin(t) / a)));
    p.data[0] = abs(p.data[0]) - r * a;
    float shape = length(p) - .1f * a;
    material = shape < dist ? i : material;
    dist = min(dist, shape);
    a /= 1.2f;
  }
  float noise = fbm(__ctx, p * 60.f);
  dist -= noise * .002f;
  return dist * .3f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (2.f * fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
             iResolution.data[1];
  f32x3 pos = f32x3(0, 0, 2);
  f32x3 ray = normalize(f32x3(uv, -3));
  float rng = random(__ctx, uv);
  f32x2 jitter = f32x2(random(__ctx, uv + .196f), random(__ctx, uv + 4.1f));
  fft = texture(__ctx.channel0, f32x2(0.f)).data[0];
  fft = pow(fft, .8f);
  float aa = abs(atan(uv.data[1], uv.data[0])) / 10.f + fft * 3.f;
  float lod = 100.f;
  aa = floor(aa * lod) / lod;
  float fft2 = texture(__ctx.channel0, f32x2(fract(aa), 0)).data[0];
  float t = iTime * 2.f;
  float index = floor(t);
  float anim = fract(t);
  rnd =
      mix(random(__ctx, f32x2(index)), random(__ctx, f32x2(index + 1.f)), anim);
  float luv = length(uv);
  ray.set_swizzle<0, 1>(ray.swizzle<0, 1>() +
                        (jitter * smoothstep(.5f, 2.f, luv) * .1f));
  f32x2 llod = 10.f * f32x2(random(__ctx, f32x2(floor(iTime * 4.f + .5f))),
                            random(__ctx, f32x2(floor(iTime * 2.f))));
  float blur = random(__ctx, floor(uv * llod) + floor(iTime * 4.f));
  ray.set_swizzle<0, 1>(ray.swizzle<0, 1>() +
                        (jitter * step(.95f, blur) * .1f));
  const float count = 100.f;
  float shade = 0.f;
  float total = 0.f;
  for (float index = count; index > 0.f; --index) {
    float dist = map(__ctx, pos);
    if (dist < .0001f * total || total > 10.f) {
      shade = index / count;
      break;
    }
    ray.set_swizzle<0, 1>(ray.swizzle<0, 1>() + (jitter * total * .0005f));
    dist *= .9f + .1f * rng;
    total += dist;
    pos += ray * dist;
  }
  f32x3 color = f32x3(0);
  color += smoothstep(4.f, .5f, luv) * .5f;
  luv = length(uv);
  color += smoothstep(.01f, .0f, abs(abs(luv - fft)) - fft2 * .2f);
  if (total < 10.f && shade > .0f) {
    color = f32x3(0.2f);
    f32x2 unit = f32x2(.001f, 0);
    f32x3 normal =
        normalize(f32x3(map(__ctx, pos + unit.swizzle<0, 1, 1>()) -
                            map(__ctx, pos - unit.swizzle<0, 1, 1>()),
                        map(__ctx, pos + unit.swizzle<1, 0, 1>()) -
                            map(__ctx, pos - unit.swizzle<1, 0, 1>()),
                        map(__ctx, pos + unit.swizzle<1, 1, 0>()) -
                            map(__ctx, pos - unit.swizzle<1, 1, 0>())));
    f32x3 rf = reflect(ray, normal);
    color += .5f + .5f * cos(f32x3(1, 2, 3) * 5.f + pos.data[2] + blur);
    color *= mod(material, 2.f);
    color += pow(dot(ray, normal) * .5f + .5f, 1.f) * 2.f;
    color += pow(dot(rf, f32x3(0, 1, 0)) * .5f + .5f, 10.f);
    color *= shade;
  }
  fragColor = f32x4(color, 1);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
