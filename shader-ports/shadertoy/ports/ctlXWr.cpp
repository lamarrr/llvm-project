// Port of https://www.shadertoy.com/view/ctlXWr; author: FifthStateOfMatter.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctlXWr.json.
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
f32x3 rotate2D(Context &__ctx, f32x3 p, f32x2 t) {
  float stx = sin(t.data[0]);
  float ctx = cos(t.data[0]);
  float sty = sin(t.data[1]);
  float cty = cos(t.data[1]);
  f32x3x3 xRotation;
  xRotation[0] = f32x3(1, 0, 0);
  xRotation[1] = f32x3(0, ctx, -stx);
  xRotation[2] = f32x3(0, stx, ctx);
  f32x3x3 yRotation;
  yRotation[0] = f32x3(cty, 0, -sty);
  yRotation[1] = f32x3(0, 1, 0);
  yRotation[2] = f32x3(sty, 0, cty);
  return p * xRotation * yRotation;
}
float random3(Context &__ctx, f32x3 p) {
  p = fract(p * 0.1031f);
  p += dot(p, p.swizzle<2, 1, 0>() + 31.32f);
  return (fract((p.data[0] + p.data[1]) * p.data[2]) * 2.0f) - 1.0f;
}
float random2(Context &__ctx, f32x2 p) {
  p = fract(p * 0.1031f);
  p += dot(p, p.swizzle<1, 0>() + 31.32f);
  return (fract(p.data[0] + p.data[1]) * 2.0f) - 1.0f;
}
float noise3(Context &__ctx, f32x3 p) {
  f32x3 fc = floor(p);
  f32x3 frc = fract(p);
  frc = frc * frc * (3.0f - 2.0f * frc);
  float tlf = random3(__ctx, fc + f32x3(0, 1, 0));
  float trf = random3(__ctx, fc + f32x3(1, 1, 0));
  float blf = random3(__ctx, fc + f32x3(0, 0, 0));
  float brf = random3(__ctx, fc + f32x3(1, 0, 0));
  float tlb = random3(__ctx, fc + f32x3(0, 1, 1));
  float trb = random3(__ctx, fc + f32x3(1, 1, 1));
  float blb = random3(__ctx, fc + f32x3(0, 0, 1));
  float brb = random3(__ctx, fc + f32x3(1, 0, 1));
  float lerpTopFront = mix(tlf, trf, frc.data[0]);
  float lerpBottomFront = mix(blf, brf, frc.data[0]);
  float lerpTopBack = mix(tlb, trb, frc.data[0]);
  float lerpBottomBack = mix(blb, brb, frc.data[0]);
  float lerpFront = mix(lerpBottomFront, lerpTopFront, frc.data[1]);
  float lerpBack = mix(lerpBottomBack, lerpTopBack, frc.data[1]);
  return mix(lerpFront, lerpBack, frc.data[2]);
}
float noise2(Context &__ctx, f32x2 p) {
  f32x2 fc = floor(p);
  f32x2 frc = fract(p);
  frc = frc * frc * (3.0f - 2.0f * frc);
  float tlf = random2(__ctx, fc + f32x2(0, 1));
  float trf = random2(__ctx, fc + f32x2(1, 1));
  float blf = random2(__ctx, fc + f32x2(0, 0));
  float brf = random2(__ctx, fc + f32x2(1, 0));
  float lerpTopFront = mix(tlf, trf, frc.data[0]);
  float lerpBottomFront = mix(blf, brf, frc.data[0]);
  return mix(lerpBottomFront, lerpTopFront, frc.data[1]);
}
float fbm(Context &__ctx, f32x3 p) {
  float f = 1.0f;
  float r = radians(57.0f);
  float h = 1.0f;
  float n = noise2(__ctx, p.swizzle<0, 2>() * f) * h;
  for (int i = 0; i < 8; i++) {
    f *= 2.0f;
    r += radians(57.0f);
    h /= 2.0f;
    n += noise2(__ctx, rotate2D(__ctx, p, f32x2(0, r)).swizzle<0, 2>() * f) * h;
  }
  return n / 1.0f;
}
float yPlane(Context &__ctx, f32x3 p) { return p.data[1]; }
float scene(Context &__ctx, f32x3 p) {
  return min(
      (yPlane(__ctx, p - f32x3(0, -1, 0)) - fbm(__ctx, p * 0.05f) * 5.0f) *
          0.65f,
      (yPlane(__ctx, p - f32x3(0, -2, 0)) -
       noise2(__ctx, p.swizzle<0, 2>() * 20.0f) * 0.002f));
}
float raymarch(Context &__ctx, f32x3 o, f32x3 d, int ms, float md, float eps,
               bool &hit) {
  float t = 0.0f;
  hit = false;
  for (int i = 0; i < ms && t < md && !hit; i++) {
    float s = scene(__ctx, o + d * t);
    t += s;
    if (abs(s) < eps * t * 0.1f) {
      hit = true;
      return t;
    }
  }
  hit = false;
  return t;
}
f32x3 getNormal(Context &__ctx, f32x3 h) {
  return normalize(
      f32x3(scene(__ctx, f32x3(h.data[0] + 0.01f, h.swizzle<1, 2>())) -
                scene(__ctx, f32x3(h.data[0] - 0.01f, h.swizzle<1, 2>())),
            scene(__ctx, f32x3(h.data[0], h.data[1] + 0.01f, h.data[2])) -
                scene(__ctx, f32x3(h.data[0], h.data[1] - 0.01f, h.data[2])),
            scene(__ctx, f32x3(h.swizzle<0, 1>(), h.data[2] + 0.01f)) -
                scene(__ctx, f32x3(h.swizzle<0, 1>(), h.data[2] - 0.01f))));
}
float getLighting(Context &__ctx, f32x3 h, f32x3 ldir, f32x3 n) {
  float c = dot(n, -ldir);
  float cr = c;
  float lt = 0.0f;
  bool hit = false;
  f32x3 so = h + n * 0.01f;
  for (int i = 0; i < 50 && lt <= 30.0f && !hit; i++) {
    float s = scene(__ctx, so - ldir * lt);
    lt += s;
    if (abs(s) <= lt * 0.001f && lt >= 0.0f) {
      hit = true;
      c -= 0.6f;
    } else {
      c = cr;
    }
  }
  return c;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 col = f32x3(0);
  f32x3 lDir = normalize(f32x3(-10, -10, -10));
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 o = f32x3(0, 4, iTime * 3.0f);
  f32x3 d = normalize(
      rotate2D(__ctx, f32x3(uv, 1), f32x2(radians(15.0f), iTime * 0.1f)));
  bool hit;
  float t = raymarch(__ctx, o, d, 150, 120.0f, 0.01f, hit);
  if (!hit)
    t = 500.0f;
  f32x3 n = getNormal(__ctx, o + d * t);
  float ny = n.data[1] - 0.1f;
  float rd = 1.0f - d.data[1];
  col = mix(f32x3(rd / 5.0f + 0.2f, rd / 2.0f + 0.2f, rd + 0.2f), f32x3(1),
            clamp(fbm(__ctx, (o + d * t).swizzle<0, 2, 1>() * 0.01f) + 0.2f,
                  0.0f, 1.0f));
  float c = clamp(getLighting(__ctx, o + d * t, lDir, n), 0.0f, 1.0f);
  f32x3 rfd = normalize(d - 2.0f * n * dot(n, d));
  float spec = pow(clamp(dot(-lDir, rfd), 0.0f, 1.0f), 9.0f);
  bool rhit;
  f32x3 rfo = (o + d * t) + n * 0.01f;
  float rft = raymarch(__ctx, rfo, rfd, 100, 100.0f, 0.01f, rhit);
  if (!rhit)
    rft = 500.0f;
  float rdy = 1.0f - rfd.data[1];
  f32x3 rfclouds =
      mix(f32x3(rdy / 5.0f + 0.2f, rdy / 2.0f + 0.2f, rdy + 0.2f), f32x3(1),
          clamp(fbm(__ctx, (rfo + rfd * rft).swizzle<0, 2, 1>() * 0.01f) + 0.2f,
                0.0f, 1.0f));
  f32x3 rn = getNormal(__ctx, rfo + rfd * rft);
  float rc = clamp(getLighting(__ctx, rfo + rfd * rft, lDir, rn), 0.0f, 1.0f);
  f32x3 matcol =
      (o.data[1] + d.data[1] * t) > -1.94f
          ? mix(f32x3(0.2f, 0.15f, 0.1f) + spec / 8.0f,
                f32x3(0.2f, 0.5f, 0) + spec / 8.0f,
                smoothstep(0.6f, 0.7f, ny)) *
                c
          : (rhit ? mix(f32x3(0.2f, 0.5f, 1) * c,
                        mix(f32x3(0.2f, 0.15f, 0.1f), f32x3(0.1f, 0.5f, 0),
                            smoothstep(0.6f, 0.7f, ny)) *
                            rc,
                        0.3f) *
                            c +
                        spec
                  : mix(f32x3(0.2f, 0.5f, 1), rfclouds, 0.3f) * c + spec);
  if (hit) {
    col = f32x3(mix(matcol.data[0], col.data[0], 1.0f - exp(-t * 0.01f)),
                mix(matcol.data[1], col.data[1], 1.0f - exp(-t * 0.01f)),
                mix(matcol.data[2], col.data[2], 1.0f - exp(-t * 0.012f)));
  }
  fragColor = f32x4(pow(col, f32x3(1.0f / 2.2f)), 1);
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
