// Port of https://www.shadertoy.com/view/cddBzf; author: Elyades.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cddBzf.json.
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

float PI = 3.14159265358979323846f;
float TWOPI = 6.28318530718f;
float PIONTWO = 1.57079632679f;
float atan2(Context &__ctx, float y, float x) {
  float angle = atan(y, x);
  if (angle < 0.0f) {
    angle += TWOPI;
  }
  return (angle);
}
float norm(Context &__ctx, f32x2 uv) {
  return (float(uv.data[0] * uv.data[0] + uv.data[1] * uv.data[1]));
}
f32x2 times(Context &__ctx, f32x2 z1, f32x2 z2) {
  return f32x2(z1.data[0] * z2.data[0] - z1.data[1] * z2.data[1],
               z1.data[0] * z2.data[1] + z1.data[1] * z2.data[0]);
}
f32x2 oneoverz(Context &__ctx, f32x2 uv) {
  float squaremodulus = norm(__ctx, uv);
  return (f32x2(uv.data[0] / squaremodulus, -uv.data[1] / squaremodulus));
}
f32x2 exp2d(Context &__ctx, f32x2 uv) {
  return f32x2(exp(uv.data[0]) * f32x2(cos(uv.data[1]), sin(uv.data[1])));
}
f32x2 cos2d(Context &__ctx, f32x2 uv) {
  return (f32x2(0.5f * (exp2d(__ctx, times(__ctx, uv, f32x2(0.0f, 1.0f))) +
                        exp2d(__ctx, times(__ctx, -uv, f32x2(0.0f, 1.0f))))));
}
f32x2 sin2d(Context &__ctx, f32x2 uv) {
  return (times(__ctx,
                0.5f * (exp2d(__ctx, times(__ctx, uv, f32x2(0.0f, 1.0f))) -
                        exp2d(__ctx, times(__ctx, -uv, f32x2(0.0f, 1.0f)))),
                f32x2(0.0f, 1.0f)));
}
f32x2 power(Context &__ctx, f32x2 uv, float n) {
  float modulus = length(uv);
  float angle = atan2(__ctx, uv.data[1], uv.data[0]);
  angle *= n;
  return (exp(n * log(modulus)) * f32x2(cos(angle), sin(angle)));
}
f32x3 palette1(Context &__ctx, float t) {
  f32x3 a = f32x3(0.5f, 0.5f, 0.5f);
  f32x3 b = f32x3(0.5f, 0.5f, 0.5f);
  f32x3 c = f32x3(1.0f, 1.0f, 2.0f);
  f32x3 d = f32x3(0.0f, 0.2f, 0.2f);
  return f32x3(a + b * cos(6.28318530718f * (c * (t) + d)));
}
f32x3 palette2(Context &__ctx, float t) {
  f32x3 a = f32x3(0.5f, 0.5f, 0.5f);
  f32x3 b = f32x3(0.5f, 0.5f, 0.5f);
  f32x3 c = f32x3(2.0f, 1.0f, 1.0f);
  f32x3 d = f32x3(0.0f, 0.33f, 0.66f);
  return f32x3(a + b * cos(6.28318530718f * (c * (t) + d)));
}
f32x3 palette3(Context &__ctx, float t) {
  f32x3 a = f32x3(0.5f, 0.5f, 0.5f);
  f32x3 b = f32x3(0.5f, 0.5f, 0.5f);
  f32x3 c = f32x3(2.0f, 1.5f, 1.0f);
  f32x3 d = f32x3(0.0f, 0.8f, 0.0f);
  return f32x3(a + b * cos(6.28318530718f * (c * (t) + d)));
}
f32x3 palette4(Context &__ctx, float t) {
  f32x3 a = f32x3(0.5f, 0.5f, 0.5f);
  f32x3 b = f32x3(0.5f, 0.5f, 0.5f);
  f32x3 c = f32x3(1.0f, 1.0f, 1.0f);
  f32x3 d = f32x3(0.0f, 0.1f, 0.2f);
  return f32x3(a + b * cos(6.28318530718f * (c * (t) + d)));
}
f32x3 palette5(Context &__ctx, float t) {
  float value = 0.61f;
  f32x3 a = f32x3(value);
  f32x3 b = f32x3(1.0f - value);
  f32x3 c = f32x3(2.0f, 1.0f, 1.0f);
  f32x3 d = f32x3(0.3f, 0.0f, 0.0f);
  return (a + b * cos(TWOPI * (c * t + d)));
}
f32x2 square(Context &__ctx, f32x2 uv) {
  return f32x2(uv.data[0] * uv.data[0] - uv.data[1] * uv.data[1],
               2.0f * uv.data[0] * uv.data[1]);
}
f32x2 cube(Context &__ctx, f32x2 uv) {
  return f32x2(uv.data[0] * uv.data[0] * uv.data[0] -
                   3.0f * uv.data[0] * uv.data[1] * uv.data[1],
               3.0f * uv.data[0] * uv.data[0] * uv.data[1] -
                   uv.data[1] * uv.data[1] * uv.data[1]);
}
f32x2 rotate(Context &__ctx, f32x2 uv, float angle) {
  float cosine = cos(angle);
  float sine = sin(angle);
  return (f32x2(cosine * uv.data[0] - sine * uv.data[1],
                sine * uv.data[0] + cosine * uv.data[1]));
}
float n = 3.0f;
f32x3 colorroot(Context &__ctx, float t, float k) {
  float r = mod(k, 3.0f);
  if (r == 0.0f) {
    return (palette1(__ctx, t));
  } else if (r == 1.0f) {
    return (palette2(__ctx, t));
  } else {
    return (palette3(__ctx, t));
  }
}
f32x3 colorunity(Context &__ctx, float t, f32x2 znext, float p) {
  for (int k = 0; k < int(p); k++) {
    if (norm(__ctx, znext - exp2d(__ctx, f32x2(0.0f, TWOPI * float(k) / p))) <
        0.1f) {
      return (colorroot(__ctx, t, float(k)));
    }
  }
}
f32x3 colorgeneral(Context &__ctx, float t, f32x2 znext) {
  f32x3 col;
  float iterationCount = t;
  float scale = 8.0f;
  if (znext.data[0] > 0.0f && znext.data[1] > 0.0f) {
    col = palette1(
        __ctx, sqrt(float(iterationCount) + 2.0f * norm(__ctx, znext)) / scale);
  } else if (znext.data[0] > 0.0f && znext.data[1] < 0.0f) {
    col = palette2(
        __ctx, sqrt(float(iterationCount) + 2.0f * norm(__ctx, znext)) / scale);
  } else if (znext.data[0] < 0.0f && znext.data[1] > 0.0f) {
    col = palette3(
        __ctx, sqrt(float(iterationCount) + 2.0f * norm(__ctx, znext)) / scale);
  } else if (znext.data[0] < 0.0f && znext.data[1] < 0.0f) {
    col = palette4(
        __ctx, sqrt(float(iterationCount) + 2.0f * norm(__ctx, znext)) / scale);
  }
  return (col);
}
f32x2 function(Context &__ctx, f32x2 uv, f32x2 c) {
  float remainder = 0.0f;
  return (remainder * sin2d(__ctx, uv) -
          (1.0f - remainder) * power(__ctx, uv, 4.0f) + c);
}
f32x2 dfunction(Context &__ctx, f32x2 uv, f32x2 c) {
  float remainder = 0.0f;
  return (remainder * cos2d(__ctx, uv) -
          4.0f * (1.0f - remainder) * power(__ctx, uv, 3.0f));
}
f32x2 newtonIteration(Context &__ctx, f32x2 uv, f32x2 c) {
  f32x2 a = f32x2(1.8f - min(0.8f, iTime / 10.0f));
  uv = uv - a * times(__ctx, function(__ctx, uv, c),
                      oneoverz(__ctx, dfunction(__ctx, uv, c)));
  return (uv);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord / iResolution.swizzle<0, 1>() - 0.5f) * 300.0f;
  uv *= pow(10.0f, (-1.5f + 0.4f * cos(iTime * 5.0f)) *
                       min(max(iTime - 9.0f, 0.0f), 1.0f));
  uv += f32x2(0.0f, 0.0f);
  uv = rotate(__ctx, uv, max(iTime - 8.0f, 0.0f) * 2.0f);
  f32x2 zcurrent = uv;
  float tolerance1 = pow(10.0f, -10.0f);
  float tolerance2 = pow(10.0f, -10.0f);
  f32x2 znext = zcurrent;
  float norm2 = 1.0f;
  int maxIteration = 1000;
  int iterationCount = 0;
  f32x2 c = oneoverz(__ctx, uv);
  while ((norm(__ctx, function(__ctx, zcurrent, c)) > tolerance1) &&
         (iterationCount <= maxIteration)) {
    zcurrent = znext;
    znext = newtonIteration(__ctx, zcurrent, c);
    norm2 = norm(__ctx, znext - zcurrent);
    iterationCount += 1;
  }
  f32x3 col;
  if (norm2 < tolerance2) {
    float scale = 20.0f;
    col = palette5(__ctx, float(iterationCount) / scale);
  } else {
    col = f32x3(0.0f);
  }
  fragColor = f32x4(col, 1.0f);
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
