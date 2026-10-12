// Port of https://www.shadertoy.com/view/DlB3WV; author: leon.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DlB3WV.json.
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
float material, total;
f32x2x2 rot(Context &__ctx, float a) {
  return f32x2x2(cos(a), -sin(a), sin(a), cos(a));
}
float gyroid(Context &__ctx, f32x3 p) {
  return dot(sin(p), cos(p.swizzle<1, 2, 0>()));
}
const unsigned k = 1103515245U;
f32x3 hash(Context &__ctx, u32x3 x) {
  x = ((x >> 8U) ^ x.swizzle<1, 2, 0>()) * k;
  x = ((x >> 8U) ^ x.swizzle<1, 2, 0>()) * k;
  x = ((x >> 8U) ^ x.swizzle<1, 2, 0>()) * k;
  return f32x3(x) * (1.0f / float(0xffffffffU));
}
float hash12(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(f32x3(p.swizzle<0, 1, 0>()) * .1031f);
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
float noise(Context &__ctx, f32x3 &p) {
  float result = 0.f, a = .5f;
  for (float i = 0.f; i < 3.f; ++i, a /= 2.f) {
    result += (gyroid(__ctx, p / a) * a);
  }
  return result;
}
float noise2(Context &__ctx, f32x3 p) {
  float result = 0.f, a = .5f;
  for (float i = 0.f; i < 6.f; ++i, a /= 2.f) {
    p.data[2] += result * .5f;
    result += abs(gyroid(__ctx, p / a) * a);
  }
  return result;
}
float noise3(Context &__ctx, f32x3 p) {
  float result = 0.f, a = .5f;
  for (float i = 0.f; i < 5.f; ++i, a /= 2.f) {
    p.data[1] += result * .5f + iTime * .05f;
    result += abs(gyroid(__ctx, p / a) * a);
  }
  return result;
}
float noise4(Context &__ctx, f32x3 p) {
  float result = 0.f, a = .5f;
  for (float i = 0.f; i < 3.f; ++i, a /= 2.f) {
    p.data[1] += result * .5f;
    result += abs(gyroid(__ctx, p / a) * a);
  }
  return result;
}
float map(Context &__ctx, f32x3 p) {
  float dist = 100.f;
  p.data[0] += .7f;
  p.data[2] -= iTime * .1f;
  f32x3 q = p;
  p.data[2] *= .5f;
  dist = noise(__ctx, p);
  float grid = .5f;
  float shape = length((mod(p, grid) - grid / 2.f)) - grid / 1.5f;
  shape = max(dist, abs(shape) - .1f);
  dist = max(dist, -abs(shape) * .5f);
  p = q * 5.f;
  p.data[1] *= .3f;
  dist += abs(noise(__ctx, p)) * .2f;
  p = q * 10.f;
  p.data[1] *= .2f;
  dist += pow(abs(noise(__ctx, p)), 4.f) * .1f;
  p = q;
  p.data[1] += cos(p.data[2] * 2.f) * .05f;
  p.set_swizzle<2, 0>(p.swizzle<2, 0>() * (.3f));
  dist -= pow(abs(noise4(__ctx, p * 10.f)), 4.f) * .03f;
  p = q * 10.f;
  p.data[2] *= 2.f;
  dist -= noise2(__ctx, p) * .05f;
  dist -= .1f;
  dist -= .1f * sin(q.data[2]);
  dist -= max(0.f, p.data[1]) * .02f;
  float water = q.data[1] + 1.f + noise3(__ctx, q * 2.f) * .01f;
  material = water < dist ? 1.f : 0.f;
  dist = min(water, dist);
  return dist;
}
f32x3 getNormal(Context &__ctx, f32x3 pos, float e) {
  f32x2 noff = f32x2(e, 0);
  return normalize(map(__ctx, pos) -
                   f32x3(map(__ctx, pos - noff.swizzle<0, 1, 1>()),
                         map(__ctx, pos - noff.swizzle<1, 0, 1>()),
                         map(__ctx, pos - noff.swizzle<1, 1, 0>())));
}
f32x3 getColor(Context &__ctx, f32x3 pos, f32x3 normal, f32x3 ray,
               float shade) {
  f32x3 color = .5f + .5f * cos(f32x3(1, 2, 3) * 5.9f + normal.data[1] -
                                normal.data[2] * .5f - .5f);
  color *= dot(normal, -normalize(pos)) * .5f + .5f;
  color *= shade * shade;
  return color;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 color = f32x3(0);
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 p =
      (2.f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 pos = f32x3(0, 0, 0);
  f32x3 ray = normalize(f32x3(p, -1.f));
  f32x3 rng = hash(__ctx, u32x3(fragCoord, 0.f));
  bool clicked = iMouse.data[0] > 0.f;
  bool clicking = iMouse.data[2] > 0.f;
  if (clicked) {
    f32x2 mouse = iMouse.swizzle<0, 1>() - abs(iMouse.swizzle<2, 3>()) +
                  iResolution.swizzle<0, 1>() / 2.f;
    f32x2 angle = f32x2((2.f * mouse - iResolution.swizzle<0, 1>()) /
                        iResolution.data[1]);
    ray.set_swizzle<1, 2>(ray.swizzle<1, 2>() * (rot(__ctx, angle.data[1])));
    ray.set_swizzle<0, 2>(ray.swizzle<0, 2>() * (rot(__ctx, angle.data[0])));
  }
  total = 0.f;
  float shade = 0.f;
  for (shade = 1.f; shade > 0.f; shade -= 1.f / 200.f) {
    float dist = map(__ctx, pos);
    if (dist < .001f * total || total > 20.f)
      break;
    dist *= 0.12f + 0.05f * rng.data[2];
    pos += ray * dist;
    total += dist;
  }
  if (shade > .01f) {
    float mat = material;
    f32x3 normal = getNormal(__ctx, pos, .003f * total);
    if (mat == 0.f) {
      color = getColor(__ctx, pos, normal, ray, shade);
      float spec = pow(dot(-ray, normal) * .5f + .5f, 100.f);
      color += .2f * spec * smoothstep(.5f, .0f, pos.data[1] + 1.f);
    } else {
      ray = reflect(ray, normal);
      pos += ray * .05f;
      total = 0.f;
      for (shade = 1.f; shade > 0.f; shade -= 1.f / 80.f) {
        float dist = map(__ctx, pos);
        if (dist < .05f * total || total > 20.f)
          break;
        dist *= 0.2f;
        pos += ray * dist;
        total += dist;
      }
      color = getColor(__ctx, pos, getNormal(__ctx, pos, .001f), ray, shade);
      color *= smoothstep(1.f, 0.f, pos.data[1] + 1.f);
      color *= smoothstep(0.f, 0.6f, (pos.data[1] + 1.2f));
    }
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
