// Port of https://www.shadertoy.com/view/ctlSD8; author: FifthStateOfMatter.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctlSD8.json.
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

const f32x3 densityCoefficient = f32x3(0.02f, 0.021f, 0.023f);
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
  return (mix(lerpBottomFront, lerpTopFront, frc.data[1]) + 1.0f) / 2.0f;
}
float fbm(Context &__ctx, f32x3 p) {
  float f = 1.0f;
  float r = radians(57.0f);
  float h = 1.0f;
  float n = noise3(__ctx, p * f) * h;
  for (int i = 0; i < 2; i++) {
    f *= 2.0f;
    r += radians(57.0f);
    h /= 2.0f;
    n += noise3(__ctx, rotate2D(__ctx, p, f32x2(0, r)) * f) * h;
  }
  return n / 1.0f;
}
float fbm2(Context &__ctx, f32x3 p) {
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
float sphere(Context &__ctx, f32x3 p, float r) { return length(p) - r; }
float plane(Context &__ctx, f32x3 p, float h, f32x3 n) {
  return length((p - h) * n);
}
float spherePattern(Context &__ctx, f32x3 p, f32x3 c) {
  f32x3 q = mod(p + 0.5f * c, c) - 0.5f * c;
  return sphere(__ctx, q, 1.0f);
}
float scene(Context &__ctx, f32x3 p) {
  return (plane(__ctx, p, -1.0f, f32x3(0, 1, 0)) -
          fbm2(__ctx, p * 0.07f) * 5.0f) *
         0.9f;
}
float raymarch(Context &__ctx, f32x3 o, f32x3 d, bool &hit, int ms, float md) {
  float t = 0.0f;
  hit = false;
  for (int i = 0; i < ms && t < md && !hit; i++) {
    float s = scene(__ctx, o + d * t);
    t += s;
    if (abs(s) <= 0.001f * t && t >= 0.0f) {
      hit = true;
    }
  }
  return t;
}
f32x3 getNormal(Context &__ctx, f32x3 h) {
  f32x2 eps = f32x2(0.01f, 0);
  return normalize(f32x3(scene(__ctx, h + eps.swizzle<0, 1, 1>()) -
                             scene(__ctx, h - eps.swizzle<0, 1, 1>()),
                         scene(__ctx, h + eps.swizzle<1, 0, 1>()) -
                             scene(__ctx, h - eps.swizzle<1, 0, 1>()),
                         scene(__ctx, h + eps.swizzle<1, 1, 0>()) -
                             scene(__ctx, h - eps.swizzle<1, 1, 0>())));
}
float G_SCATTERING = 2.0f;
float ComputeScattering(Context &__ctx, float lightDotView) {
  float result = 1.0f - G_SCATTERING * G_SCATTERING;
  result /= (4.0f * 3.1415926535f *
             pow(1.0f + G_SCATTERING * G_SCATTERING -
                     (2.0f * G_SCATTERING) * lightDotView,
                 1.5f));
  return result;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float seed = iTime;
  float hardness = 1.0f;
  f32x3 outCol = f32x3(0);
  f32x3 lPos = f32x3(10, 10, -10.0f + iTime * 3.0f);
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 o = f32x3(0, 8, iTime * 3.0f);
  f32x3 d = normalize(
      rotate2D(__ctx, f32x3(uv, 1), f32x2(radians(15.0f), iTime * 0.5f)));
  bool hit;
  float density;
  float t = raymarch(__ctx, o, d, hit, 200, 200.0f);
  f32x3 h = o + d * t;
  if (!hit)
    t = 500.0f;
  for (int j = 0; j <= 0; j++) {
    seed += float(j);
    f32x3 lRay = normalize(lPos - h);
    f32x3 l = -normalize(f32x3(-10, -10, 10));
    f32x3 skycol =
        mix(f32x3(0.9f), f32x3(1, 1, 0.9f),
            smoothstep(0.0f, 1.0f,
                       clamp(pow(dot(d, -lRay), 200000.0f), 0.0f, 1.0f)));
    f32x3 col = skycol;
    f32x3 n = getNormal(__ctx, h);
    float ny = n.data[1] - 0.25f;
    f32x3 mat = mix(f32x3(0.2f, 0.15f, 0.1f), f32x3(0.3f, 0.5f, 0.2f),
                    smoothstep(0.6f, 0.7f, ny));
    float c = dot(n, l);
    bool lhit;
    float lt = raymarch(__ctx, h + n * 0.01f, l, lhit, 30, 30.0f);
    lhit ? c *= 0.4f : c = dot(n, l);
    f32x3 testPos = o;
    float cRayRes = 50.0f;
    float stepLength = t / cRayRes;
    f32x3 Step = d * stepLength;
    density = 0.0f;
    for (int i = 0; i < int(cRayRes); i++) {
      testPos += Step;
      density += stepLength +
                 fbm(__ctx, (testPos + f32x3(0, -iTime * 0.0f, 0)) * 1.0f) *
                     2.0f * stepLength;
    }
    if (hit) {
      col = mix(mat * clamp(c, 0.0f, 1.0f), f32x3(0.9f),
                f32x3(1.0f - exp(-density * densityCoefficient)));
    } else {
      col = skycol;
    }
    outCol = col;
  }
  fragColor = f32x4(pow(outCol, f32x3(1.0f / 2.2f)), 1);
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
