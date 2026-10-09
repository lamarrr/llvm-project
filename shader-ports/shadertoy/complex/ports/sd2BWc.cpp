// Port of https://www.shadertoy.com/view/sd2BWc; author: spqr.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/sd2BWc.json.
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
float box(Context &__ctx, f32x3 p, f32x3 s) {
  p = abs(p) - s;
  return max(p.data[0], max(p.data[1], p.data[2]));
}
f32x2x2 rot(Context &__ctx, float a) {
  float ca = cos(a);
  float sa = sin(a);
  return f32x2x2(ca, sa, -sa, ca);
}
float tick(Context &__ctx, float t) {
  float i = floor(t);
  float r = fract(t);
  r = sqrt(r);
  return r + i;
}
float grid(Context &__ctx, f32x3 p) {
  f32x3 p2 = p;
  p2.data[1] = (fract(p.data[1]) - 0.5f);
  float d = box(__ctx, p2, f32x3(10.0f, 0.2f, 0.05f));
  p.data[0] = (fract(p.data[0]) - 0.5f);
  d = min(d, box(__ctx, p, f32x3(0.2f, 5, 0.05f)));
  return d;
}
float hash(Context &__ctx, f32x3 p) {
  p = 17.0f * fract(p * 0.3183099f + f32x3(.11f, .17f, .13f));
  return fract(p.data[0] * p.data[1] * p.data[2] *
               (p.data[0] + p.data[1] + p.data[2]));
}
float sph(Context &__ctx, f32x3 i, f32x3 f, f32x3 c) {
  return length(f - c) - hash(__ctx, i + c) * .3f;
}
float spheres(Context &__ctx, f32x3 p) {
  f32x3 i = floor(p);
  f32x3 f = fract(p);
  return min(min(min(sph(__ctx, i, f, f32x3(0, 0, 0)),
                     sph(__ctx, i, f, f32x3(0, 0, 1))),
                 min(sph(__ctx, i, f, f32x3(0, 1, 0)),
                     sph(__ctx, i, f, f32x3(0, 1, 1)))),
             min(min(sph(__ctx, i, f, f32x3(1, 0, 0)),
                     sph(__ctx, i, f, f32x3(1, 0, 1))),
                 min(sph(__ctx, i, f, f32x3(1, 1, 0)),
                     sph(__ctx, i, f, f32x3(1, 1, 1)))));
}
float map(Context &__ctx, f32x3 p) {
  float d = spheres(__ctx, p);
  return d;
}
float rnd(Context &__ctx, f32x2 uv) {
  return fract(dot(sin(uv * 754.655f + uv.swizzle<1, 0>() * 942.742f),
                   f32x2(3847.554f)));
}
float rnd(Context &__ctx, float t) {
  return fract(sin(t * 472.355f) * 655.644f);
}
float curve(Context &__ctx, float t) {
  return mix(rnd(__ctx, floor(t)), rnd(__ctx, floor(t) + 1.0f),
             smoothstep(0.0f, 1.0f, fract(t)));
}
float shadow(Context &__ctx, f32x3 p, f32x3 l, float maxdist, int stepcount,
             float limit) {
  float shad = 1.0f;
  float dd = 0.0f;
  for (int i = 0; i < stepcount; ++i) {
    float d = min(map(__ctx, p), maxdist);
    if (d < limit) {
      shad = 0.0f;
      break;
    }
    if (dd >= maxdist) {
      break;
    }
    p += l * d;
    dd += d;
  }
  return shad;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = f32x2(fragCoord.data[0] / iResolution.data[0],
                   fragCoord.data[1] / iResolution.data[1]);
  uv -= 0.5f;
  uv /= f32x2(iResolution.data[1] / iResolution.data[0], 1);
  f32x3 s = f32x3(0, 0, 10);
  s.set_swizzle<0, 2>(s.swizzle<0, 2>() * (rot(__ctx, iTime * .1f)));
  s.set_swizzle<1, 2>(s.swizzle<1, 2>() * (rot(__ctx, iTime * .04f)));
  s.set_swizzle<1, 0>(s.swizzle<1, 0>() * (rot(__ctx, iTime * .061f)));
  f32x3 t = f32x3(0, 0, 0);
  f32x3 cz = normalize(t - s);
  f32x3 cx = normalize(cross(cz, f32x3(0, -1, 0)));
  f32x3 cy = normalize(cross(cz, cx));
  f32x3 r = normalize(cx * uv.data[0] + cy * uv.data[1] + cz * 0.7f);
  f32x3 p = s;
  float dd = 0.0f;
  const float maxdist = 100.0f;
  for (int i = 0; i < 100; ++i) {
    float d = map(__ctx, p);
    if (d < 0.001f)
      break;
    if (dd > maxdist) {
      dd = maxdist;
      break;
    }
    p += r * d;
    dd += d;
  }
  float rand = rnd(__ctx, uv);
  f32x3 lightpos = f32x3(3);
  f32x3 lightfocus = f32x3(3);
  float lightangle = .1f;
  float t1 = tick(__ctx, iTime);
  float t2 = tick(__ctx, iTime * .71f);
  float t3 = tick(__ctx, iTime * .55f);
  lightpos.set_swizzle<1, 2>(lightpos.swizzle<1, 2>() * (rot(__ctx, t1)));
  lightpos.set_swizzle<0, 2>(lightpos.swizzle<0, 2>() * (rot(__ctx, t2)));
  lightpos.set_swizzle<1, 0>(lightpos.swizzle<1, 0>() * (rot(__ctx, t3)));
  lightfocus.set_swizzle<1, 2>(lightfocus.swizzle<1, 2>() *
                               (rot(__ctx, t1 * .7f)));
  lightfocus.set_swizzle<0, 2>(lightfocus.swizzle<0, 2>() *
                               (rot(__ctx, t2 * .8f)));
  lightfocus.set_swizzle<1, 0>(lightfocus.swizzle<1, 0>() *
                               (rot(__ctx, t3 * .91f)));
  const int volsteps = 50;
  float voldist = 15.0f;
  float stepsize = voldist / float(volsteps);
  f32x3 lr = r * stepsize;
  f32x3 lp = s + lr * rand;
  float stepdist = rand * stepsize;
  f32x3 atcol = f32x3(0);
  for (int i = 0; i < volsteps; ++i) {
    if (stepdist > dd) {
      break;
    }
    f32x3 lv = lightpos - lp;
    float ldistvol = length(lv);
    lv = normalize(lv);
    float shadvol = shadow(__ctx, lp, lv, ldistvol, 10, 0.01f);
    if (dot(normalize(lv), normalize(lp)) > -1.f) {
      atcol += 5.f / (0.01f + (pow(ldistvol, 1.5f))) * shadvol;
    }
    lp += lr;
    stepdist += stepsize;
  }
  f32x2 off = f32x2(0.01f, 0);
  f32x3 n =
      normalize(map(__ctx, p) - f32x3(map(__ctx, p - off.swizzle<0, 1, 1>()),
                                      map(__ctx, p - off.swizzle<1, 0, 1>()),
                                      map(__ctx, p - off.swizzle<1, 1, 0>())));
  f32x3 l = normalize(lightpos - p);
  float ldist = length(lightpos - p);
  float shad = shadow(__ctx, p + n * 0.2f, l, ldist, 30, 0.01f);
  f32x3 col = f32x3(0);
  col += atcol * .02f;
  fragColor = f32x4(col, 1);
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
