// Port of https://www.shadertoy.com/view/dtXSWr; author: callumacrae.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtXSWr.json.
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
float rand(Context &__ctx, float n) { return fract(sin(n) * 43758.5453123f); }
float rand(Context &__ctx, int n) { return rand(__ctx, float(n)) * 1000.0f; }
f32x3 permute(Context &__ctx, f32x3 x) {
  return mod(((x * 34.0f) + 1.0f) * x, 289.0f);
}
float snoise(Context &__ctx, f32x2 v) {
  const f32x4 C = f32x4(0.211324865405187f, 0.366025403784439f,
                        -0.577350269189626f, 0.024390243902439f);
  f32x2 i = floor(v + dot(v, C.swizzle<1, 1>()));
  f32x2 x0 = v - i + dot(i, C.swizzle<0, 0>());
  f32x2 i1;
  i1 = (x0.data[0] > x0.data[1]) ? f32x2(1.0f, 0.0f) : f32x2(0.0f, 1.0f);
  f32x4 x12 = x0.swizzle<0, 1, 0, 1>() + C.swizzle<0, 0, 2, 2>();
  x12.set_swizzle<0, 1>(x12.swizzle<0, 1>() - (i1));
  i = mod(i, 289.0f);
  f32x3 p =
      permute(__ctx, permute(__ctx, i.data[1] + f32x3(0.0f, i1.data[1], 1.0f)) +
                         i.data[0] + f32x3(0.0f, i1.data[0], 1.0f));
  f32x3 m = max(0.5f - f32x3(dot(x0, x0),
                             dot(x12.swizzle<0, 1>(), x12.swizzle<0, 1>()),
                             dot(x12.swizzle<2, 3>(), x12.swizzle<2, 3>())),
                0.0f);
  m = m * m;
  m = m * m;
  f32x3 x = 2.0f * fract(p * C.swizzle<3, 3, 3>()) - 1.0f;
  f32x3 h = abs(x) - 0.5f;
  f32x3 ox = floor(x + 0.5f);
  f32x3 a0 = x - ox;
  m *= 1.79284291400159f - 0.85373472095314f * (a0 * a0 + h * h);
  f32x3 g;
  g.data[0] = a0.data[0] * x0.data[0] + h.data[0] * x0.data[1];
  g.set_swizzle<1, 2>(a0.swizzle<1, 2>() * x12.swizzle<0, 2>() +
                      h.swizzle<1, 2>() * x12.swizzle<1, 3>());
  return 130.0f * dot(m, g);
}
f32x3 hsv2rgb(Context &__ctx, f32x3 c) {
  f32x4 K = f32x4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
  f32x3 p = abs(fract(c.swizzle<0, 0, 0>() + K.swizzle<0, 1, 2>()) * 6.0f -
                K.swizzle<3, 3, 3>());
  return c.data[2] * mix(K.swizzle<0, 0, 0>(),
                         clamp(p - K.swizzle<0, 0, 0>(), 0.0f, 1.0f),
                         c.data[1]);
}
f32x3x3 translationMatrix(Context &__ctx, float x, float y) {
  return f32x3x3(1, 0, 0, 0, 1, 0, x, y, 1);
}
f32x3x3 rotationMatrix(Context &__ctx, float angle) {
  float c = cos(angle);
  float s = sin(angle);
  return f32x3x3(c, s, 0.0f, -s, c, 0.0f, 0.0f, 0.0f, 1.0f);
}
f32x3x3 scaleMatrix(Context &__ctx, float x, float y) {
  return f32x3x3(x, 0, 0, 0, y, 0, 0, 0, 1);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  if (iResolution.data[0] > iResolution.data[1]) {
    uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  } else {
    uv.data[1] *= iResolution.data[1] / iResolution.data[0];
  }
  float triangleSize = 0.1f;
  float triangleHeight =
      triangleSize / 2.0f / tan(3.1415926535897932384626433832795f / 6.0f);
  f32x2 triangleFrom;
  if (iMouse.data[2] > 0.0f) {
    triangleFrom = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  } else {
    triangleFrom =
        f32x2(
            snoise(__ctx, f32x2(iTime * 2.0f / 100.0f + rand(__ctx, 3), 0.0f)),
            snoise(__ctx,
                   f32x2(iTime * 2.0f / 100.0f + rand(__ctx, 4), 0.0f))) *
            0.2f +
        0.5f;
  }
  float triangleAngle =
      snoise(__ctx, f32x2(iTime * 2.5f / 100.0f + rand(__ctx, 8), 0.0f)) *
      3.1415926535897932384626433832795f * 2.0f * 0.25f;
  f32x2 uvTS =
      (rotationMatrix(__ctx, triangleAngle) * f32x3(uv - triangleFrom, 1.0f))
          .swizzle<0, 1>();
  if (!false) {
    uvTS.data[1] = mod(uvTS.data[1], triangleHeight * 2.0f);
    if (uvTS.data[1] > triangleHeight) {
      uvTS.data[1] = 2.0f * triangleHeight - uvTS.data[1];
    }
    float modX = mod((uvTS.data[0] + triangleSize / 2.0f), triangleSize);
    float maxWidthAtX = uvTS.data[1] / triangleHeight * triangleSize;
    bool isInverted = abs(modX - triangleSize / 2.0f) > maxWidthAtX / 2.0f;
    float offsetX =
        floor((uvTS.data[0] + (isInverted ? 0.0f : triangleSize / 2.0f)) /
              triangleSize);
    uvTS.data[0] -= offsetX * triangleSize;
    if (isInverted) {
      f32x3x3 reflect = rotationMatrix(__ctx, radians(-60.0f)) *
                        scaleMatrix(__ctx, -1.0f, 1.0f);
      uvTS = (reflect * f32x3(uvTS, 1.0f)).swizzle<0, 1>();
    }
    float originY = triangleSize * sqrt(3.0f) / 3.0f;
    f32x3x3 rotation = translationMatrix(__ctx, 0.0f, originY) *
                       rotationMatrix(__ctx, radians(-120.0f) * offsetX) *
                       translationMatrix(__ctx, 0.0f, -originY);
    uvTS = (rotation * f32x3(uvTS, 1.0f)).swizzle<0, 1>();
  }
  float size = 0.01f * max(iResolution.data[0], iResolution.data[1]);
  uv = (rotationMatrix(__ctx, -triangleAngle) * f32x3(uvTS, 1.0f))
           .swizzle<0, 1>() +
       triangleFrom;
  f32x2 xy = uv * iResolution.swizzle<0, 1>() / size;
  xy.data[0] = floor(xy.data[0]);
  xy.data[1] = floor(xy.data[1]);
  float randIndex = xy.data[0] + xy.data[1] * (1.0f / 0.01f);
  if (rand(__ctx, randIndex - 1.0f) > (1.0f - 0.3f / 2.0f)) {
    randIndex -= 2.0f;
  } else if (rand(__ctx, randIndex) > (1.0f - 0.3f)) {
    randIndex -= 1.0f;
  }
  float hRand = rand(__ctx, randIndex);
  float hNoise = snoise(__ctx, xy * 3.8f / 100.0f) * 0.5f + 0.5f;
  float hBlend = hRand < 0.5f ? 2.0f * hRand * hNoise
                              : 1.0f - 2.0f * (1.0f - hRand) * (1.0f - hNoise);
  float h;
  if (0.4f > 0.5f) {
    h = mix(hBlend, hRand, 0.4f * 2.0f - 1.0f);
  } else {
    h = mix(hNoise, hBlend, 0.4f * 2.0f);
  }
  fragColor = f32x4(hsv2rgb(__ctx, f32x3(h, 0.5f, 0.75f)), 1.0f);
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
