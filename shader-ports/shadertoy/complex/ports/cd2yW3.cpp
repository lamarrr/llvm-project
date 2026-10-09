// Port of https://www.shadertoy.com/view/cd2yW3; author: 13rac1.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cd2yW3.json.
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
float random(Context &__ctx, f32x2 st) {
  return fract(sin(dot(st.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
f32x2 rotate(Context &__ctx, f32x2 uv, float th) {
  return f32x2x2(cos(th), sin(th), -sin(th), cos(th)) * uv;
}
float sdCircle(Context &__ctx, f32x2 p, float r) { return length(p) - r; }
f32x3 screen(Context &__ctx, f32x3 target, f32x3 blend) {
  return 1.0f - (1.0f - target) * (1.0f - blend);
}
f32x3 palette(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(2.0f * 3.1415926538f * (c * t + d));
}
f32x3 pSky(Context &__ctx, float t) {
  return palette(__ctx, t, f32x3(0.138f, 0.668f, 0.408f),
                 f32x3(0.000f, 0.500f, 0.500f), f32x3(0.000f, 0.178f, 0.178f),
                 f32x3(0.000f, 0.338f, 1.148f));
}
f32x3 pStars(Context &__ctx, float t) {
  return palette(
      __ctx, t, f32x3(0.718f, 0.718f, 1.388f), f32x3(-0.922f, -0.922f, 0.725f),
      f32x3(-0.060f, -0.060f, 1.028f), f32x3(0.738f, 0.738f, -0.072f));
}
f32x3 drawStarTrail(Context &__ctx, f32x2 randSeed, float starTrail,
                    float trailId, float dist, f32x2 cStarUv) {
  if (random(__ctx, sin(randSeed)) < 0.3f) {
    return f32x3(0);
  }
  float radStart = random(__ctx, cos(randSeed)) * 2.0f * 3.1415926538f;
  float radEnd = radStart + 2.0f * 3.1415926538f * 0.05f;
  f32x2 posStart = f32x2(dist * cos(radStart), dist * sin(radStart));
  f32x2 posEnd = f32x2(dist * cos(radEnd), dist * sin(radEnd));
  float radCurrent = 3.1415926538f + atan(cStarUv.data[1], cStarUv.data[0]);
  f32x2 posCurrent = f32x2(dist * cos(radCurrent), dist * sin(radCurrent));
  if (radCurrent < radStart) {
    radCurrent += 2.0f * 3.1415926538f;
  }
  if (radStart > radCurrent || radCurrent > radEnd) {
    return f32x3(0);
  }
  float edgeSize = sqrt(dist / 1000.0f);
  float edgeStart = distance(posStart, posCurrent);
  float edgeEnd = distance(posCurrent, posEnd);
  float edgeDistance = 1.0f;
  if (edgeStart < edgeSize) {
    edgeDistance = edgeStart / edgeSize;
  } else if (edgeEnd < edgeSize) {
    edgeDistance = edgeEnd / edgeSize;
  }
  float rand = random(__ctx, randSeed * trailId);
  return edgeDistance * pow(starTrail + 0.1f, 2.0f) * pStars(__ctx, rand);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float resolutionDiviser = min(iResolution.data[1], iResolution.data[0]);
  f32x2 uv = (fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>() * .5f) /
             resolutionDiviser;
  f32x3 col = pSky(__ctx, uv.data[1] + .5f);
  uv *= 13.0f;
  f32x2 cStarPos =
      -(iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>() * .5f) /
      resolutionDiviser * 13.0f;
  if (iMouse.data[0] < 0.00001f && iMouse.data[1] < 0.00001f) {
    cStarPos.data[0] =
        -(iResolution.data[0] / 2.0f - iResolution.data[0] * .5f) /
        resolutionDiviser * 13.0f;
    cStarPos.data[1] =
        -(iResolution.data[1] / 2.0f - iResolution.data[1] * .5f) /
        resolutionDiviser * 13.0f;
  }
  f32x2 cStarUv = uv + cStarPos;
  f32x2 scaledMouse = iMouse.swizzle<0, 1>() / resolutionDiviser;
  cStarUv = rotate(__ctx, cStarUv,
                   iTime / (10.f) + length(scaledMouse.swizzle<0, 1>()) * 5.f);
  float cStar = sdCircle(__ctx, cStarUv, 0.13f);
  col += f32x3(smoothstep(0.0f, 0.15f, -cStar));
  float dist = distance(-cStarPos, uv);
  float trailId = floor(dist * 60.0f / 3.1415926538f) + 1.0f;
  float starTrail = abs(sin(dist * 60.0f));
  f32x2 randSeed = f32x2(sin(trailId), cos(trailId));
  for (float i = 1.0f; i < 3.0f; i++) {
    randSeed = tan(randSeed);
    col = screen(
        __ctx, col,
        drawStarTrail(__ctx, randSeed, starTrail, trailId, dist, cStarUv));
  }
  for (float i = 1.0f; i < 18.0f; i++) {
    randSeed = tan(randSeed);
    col = screen(
        __ctx, col,
        i / 18.0f * 0.6f *
            drawStarTrail(__ctx, randSeed, starTrail, trailId, dist, cStarUv));
  }
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
