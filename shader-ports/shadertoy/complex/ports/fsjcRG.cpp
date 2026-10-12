// Port of https://www.shadertoy.com/view/fsjcRG; author: joeedh.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fsjcRG.json.
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
float tent(Context &__ctx, float f) {
  return 1.0f - abs(fract(f) - 0.5f) * 2.0f;
}
f32x2 rot2d(Context &__ctx, f32x2 p, float th) {
  return f32x2(cos(th) * p.data[0] + sin(th) * p.data[1],
               cos(th) * p.data[1] - sin(th) * p.data[0]);
}
f32x2 symmetry(Context &__ctx, f32x2 uv) {
  uv = abs(uv);
  float fsign = 1.0f;
  for (int i = 0; i < 5; i++) {
    uv = rot2d(__ctx, uv, fsign * 3.14159265354f * 0.15f);
    uv = abs(uv);
  }
  return uv;
}
float ctent(Context &__ctx, float f) {
  f = tent(__ctx, f);
  return f;
}
float fpattern(Context &__ctx, f32x2 uv, float seed, float fsign, float &w) {
  f32x2 p = uv;
  seed += iMouse.data[0] / iResolution.data[0] * 2.0f;
  seed -= iTime * 0.2f * fsign * 0.3f;
  float len = length(uv);
  uv = normalize(uv) * pow(len, 0.75f);
  uv = rot2d(__ctx, uv, -seed);
  uv = symmetry(__ctx, uv);
  float dx1 = ctent(__ctx, uv.data[0]);
  float dy1 = ctent(__ctx, uv.data[1]);
  uv = rot2d(__ctx, uv, seed);
  uv *= fsign * 0.95f;
  float dx2 = ctent(__ctx, uv.data[0]);
  float dy2 = ctent(__ctx, uv.data[1]);
  float f;
  f = (dx1 * dx1 + dy1 * dy1 + dx2 * dx2 + dy2 * dy2) / (4.0f);
  f *= f * 2.0f;
  if (f < 0.1f) {
    w = 0.4f;
    f = 0.0f;
  } else if (f < 0.2f) {
    w = 1.0f;
    f = 0.0f;
  } else if (f > 0.4f) {
    w = 0.7f;
    f = 0.7f;
  } else {
    w = 1.0f;
    f = 1.0f;
  }
  f /= pow(length(p) + 1.0f, 0.1f);
  return f;
}
f32x3 ray_plane_isect(Context &__ctx, f32x3 po, f32x3 pn, f32x3 ro, f32x3 rn,
                      float &t) {
  float div = (pn[1] * rn[1] + pn[2] * rn[2] + pn[0] * rn[0]);
  if (abs(div) < 0.000001f) {
    return po;
  }
  t = ((po[1] - ro[1]) * pn[1] + (po[2] - ro[2]) * pn[2] +
       (po[0] - ro[0]) * pn[0]) /
      div;
  return ro + rn * t;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  float time = iTime * 0.25f;
  const int steps = 50;
  uv = uv * 2.0f - 1.0f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  float zrange = 1.0f;
  float dz = zrange / float(steps - 1);
  float z = zrange;
  float z2 = z;
  float f = 0.2f;
  float th1 = time * 0.2f;
  f32x3 p = f32x3(cos(th1), 0.0f, sin(th1));
  f32x3 zn = f32x3(-sin(th1), 0.0f, cos(th1));
  f32x3 xn = normalize(cross(zn, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 yn = normalize(cross(zn, xn));
  float scale = 2.5f;
  f32x3 vn = xn * uv.data[0] * scale + yn * uv.data[1] * scale + zn * 3.0f;
  vn = normalize(vn);
  float tt = 10000.0f;
  float seed = 0.2f;
  for (int i = 0; i < steps; i++) {
    float fi = float(i);
    fi += th1 * float(steps - 1) / 3.14159265354f / 2.0f;
    fi = floor(fi);
    float th = fi / float(steps - 1);
    th *= 3.14159265354f * 2.0f;
    th = -th;
    f32x3 p2 = f32x3(cos(th), 0.0f, sin(th));
    f32x3 n = f32x3(-sin(th), 0.0f, cos(th));
    n.set_swizzle<0, 2>(rot2d(__ctx, n.swizzle<0, 2>(), 0.2f));
    if (dot(n, vn) < 0.0f) {
    }
    float t;
    f32x3 p3 = ray_plane_isect(__ctx, p2, n, p, vn, t);
    if (t < 0.0f || t > tt) {
      continue;
    }
    f32x3 tan = -normalize(p2);
    p3 -= p2;
    float u = dot(p3, tan);
    float v = p3[1];
    float len = length(f32x2(u, v));
    float lmax = 0.15f;
    float lmin = 0.012f;
    len *= 0.85f;
    if (len < lmax && len > lmin) {
      tt = t;
      float fac = max(1.0f - tt * 1.3f, 0.0f);
      fac = fac * fac * (3.0f - 2.0f * fac) * 1.15f;
      float fsign = mod(fi, 2.0f) * 2.0f - 1.0f;
      float w;
      f32x2 vuv = f32x2(u, v) * 25.0f;
      float f2 = fpattern(__ctx, vuv, fi * 0.2f + 0.2f, fsign, w);
      if (len < lmin * 1.3f || len > lmax * 0.95f) {
        f2 = float(len > lmin * 1.15f && len < lmax * 0.95f);
        w = 1.0f;
      }
      if (tt > 1.4f) {
        f2 = 0.0f;
        w = 0.2f;
      }
      f2 *= fac;
      f = mix(f, f2, w);
    }
  }
  fragColor = f32x4(f, f, f, 1.0f);
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
