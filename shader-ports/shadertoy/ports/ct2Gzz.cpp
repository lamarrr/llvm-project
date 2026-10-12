// Port of https://www.shadertoy.com/view/ct2Gzz; author: callumacrae.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ct2Gzz.json.
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
float drawLine(Context &__ctx, f32x2 fragCoord, float i) {
  float lineWidth = 4.0f;
  if (true) {
    float lineWidthNoise =
        snoise(__ctx, f32x2(iTime * 2.0f + rand(__ctx, 35), 0.0f));
    if (lineWidthNoise < -0.7f) {
      lineWidth *= 3.0f;
    }
  }
  float innerWidth = lineWidth / 2.0f;
  float outerWidth = lineWidth / 2.0f + 1.5f;
  float lineY = iResolution.data[1] / 20.0f * (i + 0.5f);
  float angleRoot =
      snoise(__ctx, f32x2(iTime * 0.1f + rand(__ctx, 1), 0.0f)) * 0.2f;
  float angleFactor =
      snoise(__ctx, f32x2(iTime * 0.08f + rand(__ctx, 2), 0.0f)) * 0.015f;
  float angle = angleRoot + (20.0f / 2.0f - i) * angleFactor;
  float x = fragCoord.data[0];
  bool shouldReflect =
      !true ||
      snoise(__ctx, f32x2(iTime * 1.2f + rand(__ctx, 15), 0.0f)) > -0.5f;
  float reflectAt =
      iResolution.data[0] * 0.33f +
      snoise(__ctx, f32x2(iTime * 0.1f + rand(__ctx, 3), 0.0f)) * 100.0f;
  float reflectedMultFact =
      1.5f + snoise(__ctx, f32x2(iTime * 0.12f + rand(__ctx, 4), 0.0f)) * 1.0f;
  x = shouldReflect && x < reflectAt
          ? reflectAt + reflectedMultFact * (reflectAt - x)
          : x;
  float lineYAtX = lineY - x * tan(angle);
  if (fragCoord.data[1] > lineYAtX - outerWidth &&
      fragCoord.data[1] < lineYAtX + outerWidth) {
    return smoothstep(outerWidth, innerWidth,
                      abs(fragCoord.data[1] - lineYAtX));
  }
  return 0.0f;
}
void generateRectangle(Context &__ctx, float &width, float &height,
                       f32x2 &topLeft, int randFrom, float minWidth,
                       float maxWidth, float widthVar, float minHeight,
                       float maxHeight, float heightVar) {
  float widthNoise =
      (snoise(__ctx, f32x2(iTime * widthVar + rand(__ctx, randFrom + 12))) +
       1.0f) /
      2.0f;
  width = iResolution.data[0] * (minWidth + widthNoise * (maxWidth - minWidth));
  float heightNoise =
      (snoise(__ctx, f32x2(iTime * heightVar + rand(__ctx, randFrom + 13))) +
       1.0f) /
      2.0f;
  height =
      iResolution.data[0] * (minHeight + heightNoise * (maxHeight - minHeight));
  topLeft =
      iResolution.swizzle<0, 1>() *
          (f32x2(snoise(__ctx,
                        f32x2(iTime * 0.05f + rand(__ctx, randFrom + 6), 0.0f)),
                 snoise(__ctx, f32x2(iTime * 0.1f + rand(__ctx, randFrom + 7),
                                     0.0f))) *
               0.5f +
           0.5f) -
      width / 2.0f;
}
bool isInRectangle(Context &__ctx, float width, float height, f32x2 topLeft,
                   f32x2 fragCoord) {
  return fragCoord.data[0] > topLeft.data[0] &&
         fragCoord.data[0] < topLeft.data[0] + width &&
         fragCoord.data[1] > topLeft.data[1] &&
         fragCoord.data[1] < topLeft.data[1] + height;
}
void swapRectangle(Context &__ctx, f32x2 &fragCoord, int randFrom,
                   int randFromB) {
  float width;
  float height;
  f32x2 topLeftA;
  generateRectangle(__ctx, width, height, topLeftA, randFrom, 0.2f, 0.5f, 0.01f,
                    0.2f, 0.5f, 0.01f);
  f32x2 topLeftB =
      iResolution.swizzle<0, 1>() *
          (f32x2(
               snoise(__ctx,
                      f32x2(iTime * 0.05f + rand(__ctx, randFromB + 10), 0.0f)),
               snoise(__ctx, f32x2(iTime * 0.05f + rand(__ctx, randFromB + 11),
                                   0.0f))) *
               0.5f +
           0.5f) -
      height / 2.0f;
  if (isInRectangle(__ctx, width, height, topLeftA, fragCoord)) {
    fragCoord.data[0] = fragCoord.data[0] - topLeftA.data[0] + topLeftB.data[0];
    fragCoord.data[1] = fragCoord.data[1] - topLeftA.data[1] + topLeftB.data[1];
  } else if (isInRectangle(__ctx, width, height, topLeftB, fragCoord)) {
    fragCoord.data[0] = fragCoord.data[0] - topLeftB.data[0] + topLeftA.data[0];
    fragCoord.data[1] = fragCoord.data[1] - topLeftB.data[1] + topLeftA.data[1];
  }
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 colorOut = f32x3(0.0f);
  if (true) {
    int randFrom = int(floor(
        snoise(__ctx, f32x2(iTime * 0.2f + rand(__ctx, 35), 0.0f)) * 2.0f));
    int randFromB = int(floor(
        snoise(__ctx, f32x2(iTime * 0.2f + rand(__ctx, 36), 0.0f)) * 2.0f));
    float rectSwapNoise =
        snoise(__ctx, f32x2(iTime * 2.0f + rand(__ctx, 14), 0.0f));
    if (rectSwapNoise < -0.6f)
      swapRectangle(__ctx, fragCoord, randFrom, randFromB);
    if (rectSwapNoise < -0.9f)
      swapRectangle(__ctx, fragCoord, randFrom + 1000, randFromB + 1000);
    if (rectSwapNoise < -0.05f && rectSwapNoise > -0.15f)
      swapRectangle(__ctx, fragCoord, randFrom + 2000, randFromB + 2000);
    if (rectSwapNoise > 0.6f)
      swapRectangle(__ctx, fragCoord, randFrom + 10000, randFromB + 10000);
    if (rectSwapNoise > 0.9f)
      swapRectangle(__ctx, fragCoord, randFrom + 11000, randFromB + 11000);
  }
  float bandShiftNoise =
      snoise(__ctx, f32x2(iTime * 1.3f + rand(__ctx, 23), 0.0f));
  if (true && bandShiftNoise < -0.3f) {
    float randAmount =
        rand(__ctx,
             floor(snoise(__ctx, f32x2(iTime * 0.2f + rand(__ctx, 37), 0.0f)) *
                   4.0f)) *
        80.0f;
    float bandHeight = 40.0f + randAmount;
    fragCoord.data[0] +=
        rand(__ctx, floor(fragCoord.data[1] / bandHeight)) * 300.0f;
  }
  if (true) {
    float flipNoise =
        snoise(__ctx, f32x2(iTime * 1.5f + rand(__ctx, 27), 0.0f));
    float flipDirectionNoise =
        snoise(__ctx, f32x2(iTime * 8.0f + rand(__ctx, 28), 0.0f));
    if (flipNoise < -0.85f) {
      if (flipDirectionNoise < 0.0f) {
        fragCoord.data[0] = iResolution.data[0] - fragCoord.data[0];
      } else {
        fragCoord.data[1] = iResolution.data[1] - fragCoord.data[1];
      }
    } else if (flipNoise < 0.5f && flipNoise > 0.2f) {
      int randFrom =
          9292 +
          int(floor(snoise(__ctx, f32x2(iTime * 3.0f + rand(__ctx, 29), 0.0f)) *
                    2.0f));
      float glitchWidth;
      float glitchHeight;
      f32x2 glitchTopLeft;
      generateRectangle(__ctx, glitchWidth, glitchHeight, glitchTopLeft,
                        randFrom, 0.2f, 0.7f, 0.15f, 0.3f, 0.7f, 0.15f);
      if (isInRectangle(__ctx, glitchWidth, glitchHeight, glitchTopLeft,
                        fragCoord)) {
        if (flipDirectionNoise < 0.0f) {
          fragCoord.data[0] =
              2.0f * glitchTopLeft.data[0] + glitchWidth - fragCoord.data[0];
        } else {
          fragCoord.data[1] =
              2.0f * glitchTopLeft.data[1] + glitchHeight - fragCoord.data[1];
        }
      }
    }
  }
  for (float i = -5.0f; i != 20.0f + 5.0f; ++i) {
    float color = drawLine(__ctx, fragCoord, i);
    if (color > 0.0f) {
      colorOut = f32x3(color);
    }
    if (true) {
      bool shouldGlitchColor1 =
          snoise(__ctx, f32x2(iTime * 3.0f + rand(__ctx, 18), 0.0f)) < -0.5f;
      if (shouldGlitchColor1) {
        int randFrom =
            2347 + int(floor(snoise(__ctx, f32x2(iTime * 4.0f + rand(__ctx, 30),
                                                 0.0f)) *
                             2.0f));
        float glitchWidth;
        float glitchHeight;
        f32x2 glitchTopLeft;
        generateRectangle(__ctx, glitchWidth, glitchHeight, glitchTopLeft,
                          randFrom, 0.1f, 1.3f, 0.2f, 0.1f, 1.3f, 0.2f);
        if (isInRectangle(__ctx, glitchWidth, glitchHeight, glitchTopLeft,
                          fragCoord)) {
          if (color > 0.0f) {
            colorOut.data[0] = 0.0f;
          }
          f32x2 colorSplitDist = f32x2(
              50.0f * rand(__ctx,
                           floor((snoise(__ctx,
                                         f32x2(iTime * 0.01f + rand(__ctx, 16),
                                               0.0f)) *
                                  100.0f))),
              7.0f * rand(__ctx, floor((snoise(__ctx, f32x2(iTime * 0.01f +
                                                                rand(__ctx, 17),
                                                            0.0f)) *
                                        40.0f))));
          float otherColor = drawLine(__ctx, fragCoord + colorSplitDist, i);
          if (otherColor > 0.0f) {
            colorOut.data[0] = otherColor;
          }
        }
      }
      bool shouldGlitchColor2 =
          snoise(__ctx, f32x2(iTime * 3.0f + rand(__ctx, 24), 0.0f)) < -0.5f;
      if (shouldGlitchColor2) {
        int randFrom =
            8374 + int(floor(snoise(__ctx, f32x2(iTime * 5.0f + rand(__ctx, 31),
                                                 0.0f)) *
                             2.0f));
        float glitchWidth;
        float glitchHeight;
        f32x2 glitchTopLeft;
        generateRectangle(__ctx, glitchWidth, glitchHeight, glitchTopLeft,
                          randFrom, 0.1f, 1.3f, 0.4f, 0.1f, 1.3f, 0.15f);
        if (isInRectangle(__ctx, glitchWidth, glitchHeight, glitchTopLeft,
                          fragCoord)) {
          if (color > 0.0f) {
            colorOut.data[0] = 0.0f;
          }
          f32x2 colorSplitDist = f32x2(
              60.0f * rand(__ctx,
                           floor((snoise(__ctx,
                                         f32x2(iTime * 0.01f + rand(__ctx, 25),
                                               0.0f)) *
                                  100.0f))),
              4.0f * rand(__ctx, floor((snoise(__ctx, f32x2(iTime * 0.01f +
                                                                rand(__ctx, 26),
                                                            0.0f)) *
                                        40.0f))));
          float otherColor = drawLine(__ctx, fragCoord + colorSplitDist, i);
          if (otherColor > 0.0f) {
            colorOut.data[0] = otherColor;
          }
        }
      }
    }
  }
  float blankNoise = snoise(__ctx, f32x2(iTime * 0.7f + rand(__ctx, 19), 0.0f));
  if (true) {
    if (blankNoise < -0.8f) {
      colorOut = f32x3(0.0f);
    } else if (blankNoise > 0.3f) {
      float blankNoise2 =
          snoise(__ctx, f32x2(iTime * 1.6f + rand(__ctx, 20), 0.0f));
      if (blankNoise2 < -0.1f) {
        int randFrom =
            2479 + int(floor(snoise(__ctx, f32x2(iTime * 2.0f + rand(__ctx, 32),
                                                 0.0f)) *
                             1.5f));
        float glitchWidth;
        float glitchHeight;
        f32x2 glitchTopLeft;
        generateRectangle(__ctx, glitchWidth, glitchHeight, glitchTopLeft,
                          randFrom, 0.1f, 0.7f, 0.06f, 0.2f, 0.6f, 0.05f);
        if (isInRectangle(__ctx, glitchWidth, glitchHeight, glitchTopLeft,
                          fragCoord)) {
          colorOut = f32x3(0.0f);
        }
      }
    }
  }
  float invertColorNoise =
      snoise(__ctx, f32x2(iTime * 0.6f + rand(__ctx, 5), 0.0f));
  if (true) {
    if (invertColorNoise < -0.85f) {
      colorOut = f32x3(1.0f) - colorOut;
    } else if (invertColorNoise > 0.4f) {
      float invertColorNoise2 =
          snoise(__ctx, f32x2(iTime * 3.2f + rand(__ctx, 18), 0.0f));
      if (invertColorNoise2 < -0.3f) {
        int randFrom =
            7254 + int(floor(snoise(__ctx, f32x2(iTime * 2.0f + rand(__ctx, 33),
                                                 0.0f)) *
                             2.5f));
        float glitchWidth;
        float glitchHeight;
        f32x2 glitchTopLeft;
        generateRectangle(__ctx, glitchWidth, glitchHeight, glitchTopLeft,
                          randFrom, 0.2f, 0.6f, 0.18f, 0.2f, 0.6f, 0.22f);
        if (isInRectangle(__ctx, glitchWidth, glitchHeight, glitchTopLeft,
                          fragCoord)) {
          colorOut = f32x3(1.0f) - colorOut;
        }
      }
    }
  }
  float swapColorNoise =
      snoise(__ctx, f32x2(iTime * 0.6f + rand(__ctx, 21), 0.0f));
  if (true && colorOut.data[0] + colorOut.data[1] + colorOut.data[2] > 2.5f) {
    float hue =
        snoise(__ctx, f32x2(iTime * 0.3f + rand(__ctx, 23), 0.0f)) / 2.0f +
        0.5f;
    f32x3 newColor = hsv2rgb(__ctx, f32x3(hue, 0.75f, 0.75f));
    if (swapColorNoise < -0.75f) {
      colorOut = newColor;
    } else if (swapColorNoise > 0.4f) {
      float swapColorNoise2 =
          snoise(__ctx, f32x2(iTime * 2.6f + rand(__ctx, 22), 0.0f));
      if (swapColorNoise2 < -0.3f) {
        int randFrom =
            29482 +
            int(floor(
                snoise(__ctx, f32x2(iTime * 0.5f + rand(__ctx, 34), 0.0f)) *
                5.0f));
        float glitchWidth;
        float glitchHeight;
        f32x2 glitchTopLeft;
        generateRectangle(__ctx, glitchWidth, glitchHeight, glitchTopLeft,
                          randFrom, 0.2f, 0.6f, 0.2f, 0.2f, 0.6f, 0.2f);
        if (isInRectangle(__ctx, glitchWidth, glitchHeight, glitchTopLeft,
                          fragCoord)) {
          colorOut = newColor;
        }
      }
    }
  }
  fragColor = f32x4(colorOut, 1.0f);
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
