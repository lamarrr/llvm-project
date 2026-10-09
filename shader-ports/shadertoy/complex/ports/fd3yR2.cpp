// Port of https://www.shadertoy.com/view/fd3yR2; author: vow.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fd3yR2.json.
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

float time;
float noise(Context &__ctx, f32x2 p) {
  return sin(p.data[0] * 10.f) * sin(p.data[1] * (3.f + sin(time / 11.f))) +
         .2f;
}
f32x2x2 rotate(Context &__ctx, float angle) {
  return f32x2x2(cos(angle), -sin(angle), sin(angle), cos(angle));
}
float fbm(Context &__ctx, f32x2 p) {
  p *= 1.1f;
  float f = 0.f;
  float amp = .5f;
  for (int i = 0; i < 3; i++) {
    f32x2x2 modify = rotate(__ctx, time / 50.f * float(i * i));
    f += amp * noise(__ctx, p);
    p = modify * p;
    p *= 2.f;
    amp /= 2.2f;
  }
  return f;
}
float pattern(Context &__ctx, f32x2 p, f32x2 &q, f32x2 &r) {
  q = f32x2(fbm(__ctx, p + f32x2(1.f)),
            fbm(__ctx, rotate(__ctx, .1f * time) * p + f32x2(1.f)));
  r = f32x2(fbm(__ctx, rotate(__ctx, .1f) * q + f32x2(0.f)),
            fbm(__ctx, q + f32x2(0.f)));
  return fbm(__ctx, p + 1.f * r);
}
float sampleFont(Context &__ctx, f32x2 p, float num) {
  float glyph[2];
  if (num < 1.f) {
    glyph[0] = 0.91333008f;
    glyph[1] = 0.89746094f;
  } else if (num < 2.f) {
    glyph[0] = 0.27368164f;
    glyph[1] = 0.06933594f;
  } else if (num < 3.f) {
    glyph[0] = 1.87768555f;
    glyph[1] = 1.26513672f;
  } else if (num < 4.f) {
    glyph[0] = 1.87719727f;
    glyph[1] = 1.03027344f;
  } else if (num < 5.f) {
    glyph[0] = 1.09643555f;
    glyph[1] = 1.51611328f;
  } else if (num < 6.f) {
    glyph[0] = 1.97045898f;
    glyph[1] = 1.03027344f;
  } else if (num < 7.f) {
    glyph[0] = 0.97045898f;
    glyph[1] = 1.27246094f;
  } else if (num < 8.f) {
    glyph[0] = 1.93945312f;
    glyph[1] = 1.03222656f;
  } else if (num < 9.f) {
    glyph[0] = 0.90893555f;
    glyph[1] = 1.27246094f;
  } else {
    glyph[0] = 0.90893555f;
    glyph[1] = 1.52246094f;
  }
  float pos = floor(p.data[0] + p.data[1] * 5.f);
  if (pos < 13.f) {
    return step(1.f, mod(pow(2.f, pos) * glyph[0], 2.f));
  } else {
    return step(1.f, mod(pow(2.f, pos - 13.f) * glyph[1], 2.f));
  }
}
float digit(Context &__ctx, f32x2 p) {
  p -= f32x2(0.5f, 0.5f);
  p *= (1.f + 0.15f * pow(length(p), 0.6f));
  p += f32x2(0.5f, 0.5f);
  p.data[0] += sin(iTime / 7.f) / 5.f;
  p.data[1] += sin(iTime / 13.f) / 5.f;
  f32x2 grid = f32x2(3.f, 1.f) * 15.f;
  f32x2 s = floor(p * grid) / grid;
  p = p * grid;
  f32x2 q;
  f32x2 r;
  float intensity = pattern(__ctx, s / 10.f, q, r) * 1.3f - 0.03f;
  p = fract(p);
  p *= f32x2(1.2f, 1.2f);
  float x = fract(p.data[0] * 5.f);
  float y = fract((1.f - p.data[1]) * 5.f);
  f32x2 fpos = f32x2(floor(p.data[0] * 5.f), floor((1.f - p.data[1]) * 5.f));
  float isOn = sampleFont(__ctx, fpos, floor(intensity * 10.f));
  return p.data[0] <= 1.f && p.data[1] <= 1.f
             ? isOn * (0.2f + y * 4.f / 5.f) * (0.75f + x / 4.f)
             : 0.f;
}
float hash(Context &__ctx, float x) {
  return fract(sin(x * 234.1f) * 324.19f +
               sin(sin(x * 3214.09f) * 34.132f * x) + x * 234.12f);
}
float onOff(Context &__ctx, float a, float b, float c) {
  return step(c, sin(iTime + a * cos(iTime * b)));
}
float displace(Context &__ctx, f32x2 look) {
  float y = (look.data[1] - mod(iTime / 4.f, 1.f));
  float window = 1.f / (1.f + 50.f * y * y);
  return sin(look.data[1] * 20.f + iTime) / 80.f * onOff(__ctx, 4.f, 2.f, .8f) *
         (1.f + cos(iTime * 60.f)) * window;
}
f32x3 getColor(Context &__ctx, f32x2 p) {
  float bar = mod(p.data[1] + time * 20.f, 1.f) < 0.2f ? 1.4f : 1.f;
  p.data[0] += displace(__ctx, p);
  float middle = digit(__ctx, p);
  float off = 0.002f;
  float sum = 0.f;
  for (float i = -1.f; i < 2.f; i += 1.f) {
    for (float j = -1.f; j < 2.f; j += 1.f) {
      sum += digit(__ctx, p + f32x2(off * i, off * j));
    }
  }
  return f32x3(0.9f) * middle + sum / 10.f * f32x3(0.f, 1.f, 0.f) * bar;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  time = iTime / 3.f;
  f32x2 p = fragCoord / iResolution.swizzle<0, 1>();
  float off = 0.0001f;
  f32x3 col = getColor(__ctx, p);
  fragColor = f32x4(col, 1);
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
