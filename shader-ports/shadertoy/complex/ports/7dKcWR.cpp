// Port of https://www.shadertoy.com/view/7dKcWR; author: Sergeindamix.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7dKcWR.json.
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
f32x2 rot(Context &__ctx, f32x2 uv, float t) {
  float c = cos(t), s = sin(t);
  return f32x2x2(c, -s, s, c) * uv;
}
f32x3 RotateY(Context &__ctx, f32x3 v, float degree) {
  float rad = 0.0174532925f * degree;
  float cosY = cos(rad);
  float sinY = sin(rad);
  return f32x3(cosY * v.data[0] - sinY * v.data[2], v.data[1],
               sinY * v.data[0] + cosY * v.data[2]);
}
float sdCircle(Context &__ctx, f32x2 p, float radius) {
  return length(p) - radius;
}
float sdStar5(Context &__ctx, f32x2 p, float r, float rf) {
  const f32x2 k1 = f32x2(0.809016994375f, -0.587785252292f);
  const f32x2 k2 = f32x2(-k1.data[0], k1.data[1]);
  p.data[0] = abs(p.data[0]);
  p -= 2.0f * max(dot(k1, p), 0.0f) * k1;
  p -= 2.0f * max(dot(k2, p), 0.0f) * k2;
  p.data[0] = abs(p.data[0]);
  p.data[1] -= r;
  f32x2 ba = rf * f32x2(-k1.data[1], k1.data[0]) - f32x2(0, 1);
  float h = clamp(dot(p, ba) / dot(ba, ba), 0.0f, r);
  return length(p - ba * h) *
         sign(p.data[1] * ba.data[0] - p.data[0] * ba.data[1]);
}
float triangleDist(Context &__ctx, f32x2 p, float radius) {
  return max(abs(p).data[0] * 0.866025f + p.data[1] * 0.5f, -p.data[1]) -
         radius * 0.5f;
}
float substract(Context &__ctx, float d1, float d2) { return max(-d1, d2); }
float pie(Context &__ctx, f32x2 p, float angle) {
  angle = radians(angle) / 2.0f;
  f32x2 n = f32x2(cos(angle), sin(angle));
  return abs(p).data[0] * n.data[0] + p.data[1] * n.data[1];
}
float circleDist(Context &__ctx, f32x2 p, float radius) {
  return length(p) - radius;
}
float semiCircleDist(Context &__ctx, f32x2 p, float radius, float angle,
                     float width) {
  width /= 2.0f;
  radius -= width;
  return substract(__ctx, pie(__ctx, p, angle),
                   abs(circleDist(__ctx, p, radius)) - width);
}
float boxDist(Context &__ctx, f32x2 p, f32x2 size, float radius) {
  size -= f32x2(radius);
  f32x2 d = abs(p) - size;
  return min(max(d.data[0], d.data[1]), 0.0f) + length(max(d, 0.0f)) - radius;
}
float lineDist(Context &__ctx, f32x2 p, f32x2 start, f32x2 end, float width) {
  f32x2 dir = start - end;
  float lngth = length(dir);
  dir /= lngth;
  f32x2 proj = max(0.0f, min(lngth, dot((start - p), dir))) * dir;
  return length((start - p) - proj) - (width / 2.0f);
}
float radius = .5f;
float angle = .1f;
float width = .5f;
f32x2 size = f32x2(0.4f, .24f);
f32x2 start = f32x2(0, 0);
f32x2 end = f32x2(1, 0);
float map(Context &__ctx, f32x2 p) {
  p += f32x2(sin(iTime), cos(iTime) * .5f);
  float a1 = 5.f, a2 = a1 + 5.f, a3 = a2 + 5.f, a4 = a3 + 5.f, a5 = a4 + 5.f,
        a6 = a5 + 5.f, a7 = a6 + 5.f, a8 = a7 + 5.f;
  float gTime = mod(iTime, a8 + 1.f);
  float Circ = sdCircle(__ctx, p, 0.3f);
  float tria = triangleDist(__ctx, p, radius);
  float box = boxDist(__ctx, p, size, radius - .45f);
  float star = sdStar5(__ctx, p, 0.4f, 0.5f);
  float scir = semiCircleDist(__ctx, p, radius, angle, width);
  float lin = lineDist(__ctx, p, start, end, width - .45f);
  for (int i = 0; i < 4; i++) {
    f32x3 p3;
    p3 = RotateY(__ctx, p3, -22.5f * float(i)) +
         f32x3(float(i) * sin(iTime) * .75f, float(i) * cos(iTime) * .75f,
               float(i) * 3.1415926535898f);
    float circAdd = sdCircle(__ctx, p - p3.swizzle<0, 1>(), 0.3f);
    float triaAdd = triangleDist(__ctx, p - p3.swizzle<0, 1>(), radius);
    Circ = min(Circ, circAdd);
    tria = min(tria, triaAdd);
    float boxAdd = boxDist(__ctx, p - p3.swizzle<0, 1>(), size, radius - .45f);
    box = min(box, boxAdd);
    float starAdd = sdStar5(__ctx, p - p3.swizzle<0, 1>(), 0.4f, 0.5f);
    star = min(star, starAdd);
    float scirAdd =
        semiCircleDist(__ctx, p - p3.swizzle<0, 1>(), radius, angle, width);
    scir = min(scir, scirAdd);
    float linAdd =
        lineDist(__ctx, p - p3.swizzle<0, 1>(), start, end, width - .45f);
    lin = min(lin, linAdd);
  }
  if (gTime < a1) {
    float t0 = gTime - 2.f;
    return Circ;
  } else if (gTime < a2) {
    float t0 = gTime - a1;
    return tria;
  } else if (gTime < a3) {
    float t0 = gTime - a2;
    return box;
  } else if (gTime < a4) {
    float t0 = gTime - a3;
    return star;
  } else if (gTime < a5) {
    return Circ;
  } else if (gTime < a5) {
    return tria;
  } else if (gTime < a6) {
    return scir;
  } else if (gTime < a7) {
    return box;
  } else if (gTime < a8) {
    return lin;
  }
}
f32x2 getNormal(Context &__ctx, f32x2 p) {
  f32x2 d = f32x2(1, 0) * 0.01f;
  return normalize(f32x2(
      map(__ctx, p + d.swizzle<0, 1>()) - map(__ctx, p - d.swizzle<0, 1>()),
      map(__ctx, p + d.swizzle<1, 0>()) - map(__ctx, p - d.swizzle<1, 0>())));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 p = uv * 2.f - 1.f;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 col;
  float a = floor((uv.data[0] - uv.data[1] * 0.5f - iTime * .08f) * 8.f) -
            iTime * 2.f;
  col = f32x3(sin(a), sin(a + 2.f), sin(a + 3.f)) * 0.2f + 0.7f;
  float angle = radians(-50.0f);
  f32x2 dir = -f32x2(cos(angle), sin(angle));
  float d;
  float t = 0.f;
  f32x2 p2;
  for (int i = 0; i < 30; i++) {
    p2 = p + dir * t;
    d = map(__ctx, p2);
    if (d < 0.01f) {
      f32x2 n = getNormal(__ctx, p2);
      float shade = dot(n, normalize(f32x2(1, 1)));
      col = f32x3(1, 0, 0);
      ;
      col *= 1.f + shade * 0.3f;
    }
    t += d;
  }
  d = map(__ctx, p);
  float inside = step(d, 0.01f);
  if (d < 0.01f) {
    col = f32x3(step(d, -0.01f));
    uv /= f32x2(.75f);
    uv += f32x2(.75f);
    uv += f32x2(sin(iTime), cos(iTime));
    col = min(col, texture(__ctx.channel0, uv).swizzle<0, 1, 2>());
  }
  col -= smoothstep(0.5f, 3.0f, length(p));
  fragColor = f32x4(col, 1.0f);
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
