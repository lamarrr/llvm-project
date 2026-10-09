// Port of https://www.shadertoy.com/view/ddlGzN; author: the0neboy.
// Dataset license metadata: mit; original retained in upstream/ddlGzN.json.
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
f32x3 camPos = f32x3(0, 0, 0);
struct MarchData {
  float mindistance;
  bool didCollide;
  bool doGlow;
  float distanceTravelled;
  int steps;
  f32x3 color;
  float minDist;
};
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float clmp(Context &__ctx, float a, float b, float h) {
  if (h <= a)
    return a;
  if (h >= b)
    return b;
  return h;
}
float fogAmount(Context &__ctx, float d) {
  return clmp(__ctx, 0.f, 20.f - 10.f, d - 10.f) / (20.f - 10.f);
}
f32x4 sdSphere(Context &__ctx, f32x3 p) {
  f32x3 col = f32x3(0, 255, 0);
  float d = length(p) - float(1.f);
  return f32x4(col, d);
}
f32x4 sdRect(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 col = f32x3(255, 0, 0);
  f32x3 q = abs(p) - b;
  float d = length(max(q, 0.0f)) +
            min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
  return f32x4(f32x3(col), d);
}
f32x4 sdFractal(Context &__ctx, f32x3 p) {
  f32x3 col = f32x3(0, 255, 0);
  f32x3 np = f32x3(mod(p.data[0], 3.f) - 1.5f, mod(p.data[1], 3.f) - 1.5f,
                   mod(p.data[2], 3.f) - 1.5f);
  float d = length(np) - float(1.f);
  return f32x4(col, d);
}
f32x4 sdPlane(Context &__ctx, f32x3 p) {
  f32x3 col = f32x3(0, 220, 100);
  float dist = p.data[1] + 1.3f;
  return f32x4(col, dist);
}
f32x4 sdTorus(Context &__ctx, f32x3 p) {
  f32x2 t = f32x2(1.f, .3f);
  float dist = length(f32x2(length(f32x2(p.data[0], p.data[2])) - t.data[0],
                            p.data[1])) -
               t.data[1];
  f32x3 col = f32x3(255, 255, 0);
  return f32x4(col, dist);
}
f32x4 sierpinskiTetrahedron(Context &__ctx, f32x3 p) {
  f32x3 col = f32x3(127, 127, 127);
  f32x3 z = p;
  float Scale = 2.f;
  f32x3 a1 = f32x3(1, 1, 1);
  f32x3 a2 = f32x3(-1, -1, 1);
  f32x3 a3 = f32x3(1, -1, -1);
  f32x3 a4 = f32x3(-1, 1, -1);
  f32x3 c;
  int n = 0;
  float dist, d;
  while (n < 20) {
    c = a1;
    dist = length(z - a1);
    d = length(z - a2);
    if (d < dist) {
      c = a2;
      dist = d;
    }
    d = length(z - a3);
    if (d < dist) {
      c = a3;
      dist = d;
    }
    d = length(z - a4);
    if (d < dist) {
      c = a4;
      dist = d;
    }
    z = Scale * z - c * (Scale - 1.0f);
    n++;
  }
  float DE = length(z) * pow(Scale, float(-n));
  return f32x4(col, DE);
}
f32x4 smin(Context &__ctx, f32x4 a, f32x4 b, float k) {
  float h = clamp(0.5f + 0.5f * (a.data[3] - b.data[3]) / k, 0.0f, 1.0f);
  f32x3 color =
      f32x3(mix(a.data[0], b.data[0], h), mix(a.data[1], b.data[1], h),
            mix(a.data[2], b.data[2], h));
  float dist = mix(a.data[3], b.data[3], h) - k * h * (1.0f - h);
  return f32x4(color, dist);
}
f32x4 smax(Context &__ctx, f32x4 a, f32x4 b, float k) {
  return -smin(__ctx, -a, -b, k);
}
f32x4 GetDist(Context &__ctx, f32x3 p) {
  float s = .3f;
  f32x4 d1 = smin(
      __ctx, sdSphere(__ctx, f32x3(p.data[0] + 1.8f, p.data[1], p.data[2])),
      sdRect(__ctx, f32x3(p.data[0] - 1.3f, p.data[1], p.data[2] - .7f),
             f32x3(1, 1, 2)),
      s);
  return smin(__ctx, sdTorus(__ctx, p), d1, s);
}
MarchData RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  MarchData _dat = MarchData(0.f, false, false, 0.f, 0, f32x3(127), .1f);
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + rd * _dat.distanceTravelled;
    float dS = GetDist(__ctx, p).data[3];
    _dat.distanceTravelled += dS;
    _dat.steps += 1;
    _dat.color = f32x3(GetDist(__ctx, p));
    if (dS < _dat.minDist)
      _dat.minDist = dS;
    if (_dat.distanceTravelled < 20.f && abs(dS) < .001f) {
      _dat.didCollide = true;
      return _dat;
    }
  }
  if (_dat.minDist < .1f && !_dat.didCollide)
    _dat.doGlow = true;
  return _dat;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p).data[3];
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()).data[3],
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()).data[3],
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()).data[3]);
  return normalize(n);
}
f32x3 GetRayDir(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i);
  return d;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 ro = f32x3(0, 4, -3) + camPos;
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() *
                       (Rot(__ctx, -m.data[1] * 3.14f + 1.f)));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (Rot(__ctx, -m.data[0] * 6.2831f)));
  f32x3 rd = GetRayDir(__ctx, uv, ro, f32x3(0, 0.f, 0), 1.f) + camPos;
  f32x3 col = f32x3(135, 206, 235) / 255.f;
  MarchData _dat = RayMarch(__ctx, ro, rd);
  if (_dat.didCollide) {
    f32x3 p = ro + rd * _dat.distanceTravelled;
    f32x3 n = GetNormal(__ctx, p);
    f32x3 r = reflect(rd, n);
    float dif = dot(n, normalize(f32x3(1, 2, 3))) * .5f + .5f;
    col = f32x3(dif) * (_dat.color / 255.f);
  }
  if (_dat.doGlow) {
    col = f32x3(255, 255, 255);
  }
  col = pow(col, f32x3(.4545f));
  fragColor = f32x4(col, 1.0f);
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
