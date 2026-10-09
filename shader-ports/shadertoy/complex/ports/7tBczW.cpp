// Port of https://www.shadertoy.com/view/7tBczW; author: fyisic123.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7tBczW.json.
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
f32x2 hash22(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(f32x3(p.swizzle<0, 1, 0>()) * f32x3(.1031f, .1030f, .0973f));
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.swizzle<0, 0>() + p3.swizzle<1, 2>()) * p3.swizzle<2, 1>());
}
float getBall(Context &__ctx, f32x2 p, float size, float Pulse) {
  float c = smoothstep(size * 0.2f, 0.f, length(p));
  c += smoothstep(size, 0.f, length(p)) * Pulse;
  c += pow(smoothstep(size, 0.f, length(p)), 3.f);
  c += smoothstep(0.1f * size, 0.0f * size, abs(length(p) - (size)));
  return c;
}
f32x2 getCosSin(Context &__ctx, float p) { return f32x2(cos(p), sin(p)); }
f32x2 getOrbitOffset(Context &__ctx, float orbSpeed, float orbPhase,
                     float dist) {
  float o = orbSpeed + orbPhase;
  return f32x2(cos(o), sin(o)) * dist;
}
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord.swizzle<0, 1>() - .5f * iResolution.swizzle<0, 1>()) /
             iResolution.data[1];
  float uvDist = length(uv);
  uv *= pow(length(uv), -0.2f);
  uv *= 4.f + (1.25f * sin(iTime * 0.5f));
  uv *= Rot(__ctx, iTime * 0.4f);
  uv += f32x2(iTime * 0.75f, 0.0f);
  f32x2 gv = fract(uv) - .5f;
  f32x2 ID = floor(uv);
  f32x3 col = f32x3(0);
  float nDist = 333.f;
  for (int x = -1; x <= 1; x++) {
    for (int y = -1; y <= 1; y++) {
      f32x2 offs = f32x2(x, y);
      f32x2 cID = hash22(__ctx, ID + offs);
      float nID = cID.data[0] + cID.data[1] * 0.5f;
      float nVariant = nID * 10000.f;
      float erratic = abs(sin(iTime * 0.25f + nVariant));
      float movSpeed = iTime + (3.14159265359f * erratic);
      f32x2 mov = getCosSin(__ctx, nVariant + movSpeed);
      f32x2 pos = gv - offs - mov * .5f;
      float orbitPhase = 100.f * (nID);
      float orbitSpeed = iTime * 3.0f;
      if (nID > 0.5f)
        orbitSpeed *= -1.f;
      f32x3 h =
          (.6f + .6f * cos(2.f * 3.14159265359f * (cID.data[0] + cID.data[1]) +
                           f32x3(0, -2.f * 3.14159265359f / 3.f,
                                 2.f * 3.14159265359f / 3.f)));
      float erraticScale = 0.1f + (0.9f * erratic);
      float orbitScale = 0.1f * erraticScale;
      float p = 0.f;
      float orbitDist = 0.2f * erraticScale;
      p += getBall(
          __ctx, pos + getOrbitOffset(__ctx, orbitSpeed, orbitPhase, orbitDist),
          orbitScale, 0.f);
      p += getBall(__ctx,
                   pos + getOrbitOffset(__ctx, orbitSpeed,
                                        orbitPhase + radians(180.f), orbitDist),
                   orbitScale, 0.f);
      p += getBall(__ctx,
                   pos + getOrbitOffset(__ctx, -orbitSpeed, orbitPhase,
                                        0.2f * erraticScale),
                   orbitScale, 0.f);
      p += getBall(__ctx,
                   pos + getOrbitOffset(__ctx, -orbitSpeed,
                                        orbitPhase + radians(180.f), orbitDist),
                   orbitScale, 0.f);
      p += getBall(__ctx,
                   pos + getOrbitOffset(__ctx, orbitSpeed * 2.f,
                                        orbitPhase + radians(0.f), orbitDist),
                   orbitScale * .75f, 0.f);
      p += getBall(__ctx,
                   pos + getOrbitOffset(__ctx, -orbitSpeed * 2.f,
                                        orbitPhase + radians(180.f), orbitDist),
                   orbitScale * .75f, 0.f);
      p += getBall(__ctx,
                   pos + getOrbitOffset(__ctx, orbitSpeed * 4.f,
                                        orbitPhase + radians(0.f), orbitDist),
                   orbitScale * .5f, 0.f);
      p += getBall(__ctx,
                   pos + getOrbitOffset(__ctx, -orbitSpeed * 4.f,
                                        orbitPhase + radians(180.f), orbitDist),
                   orbitScale * .5f, 0.f);
      float centerPulse = smoothstep(
          0.2f, 0.f, length(sin((iTime * (1.0f + (2.0f * nID))) + nVariant)));
      centerPulse += 0.3f * abs(sin(iTime * 4.f));
      p += getBall(__ctx, pos, 0.2f * erraticScale, centerPulse);
      col += h * p;
      col = mix(col, f32x3(1.f), min(max(0.f, p - 1.f), 1.0f)) *
            pow(erraticScale, 0.05f);
      float dist = pow(length(pos), erraticScale);
      if (dist < nDist) {
        nDist = dist;
      }
    }
  }
  col += (.6f +
          .6f * cos(2.f * 3.14159265359f * (pow(nDist, 3.f) + (iTime * 0.5f)) +
                    f32x3(0, -2.f * 3.14159265359f / 3.f,
                          2.f * 3.14159265359f / 3.f))) *
         pow(nDist, 8.f) * 0.25f;
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
