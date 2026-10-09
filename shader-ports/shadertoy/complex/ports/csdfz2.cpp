// Port of https://www.shadertoy.com/view/csdfz2; author: jarble.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/csdfz2.json.
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
float hash11(Context &__ctx, float p) {
  p = fract(p * .1031f);
  p *= p + 33.33f;
  p *= p + p;
  return fract(p);
}
float func1(Context &__ctx, float seed, float param1, float &arity) {
  int func_id = int(hash11(__ctx, seed * 2.f) * 8.f);
  if (func_id == 0)
    return sin(param1);
  else if (func_id == 1) {
    return cos(param1);
    arity = 1.f;
  } else if (func_id == 2) {
    return param1 * param1;
    arity = 1.f;
  } else if (func_id == 3) {
    return param1 + (hash11(__ctx, (seed + 1.f) * 5.f - 10.f));
    arity = 2.f;
  } else if (func_id == 4) {
    return abs(param1);
    arity = 1.f;
  } else if (func_id == 5) {
    return log(param1);
    arity = 1.f;
  } else if (func_id == 6) {
    return log(param1);
    arity = 1.f;
  } else if (func_id == 7) {
    return param1 * (hash11(__ctx, (seed + 1.f) * 5.f - 10.f));
    arity = 2.f;
  } else
    return param1;
}
float random_function_generator(Context &__ctx, float param1, float seed) {
  float arity = 1.f, h1 = hash11(__ctx, seed),
        f1 = func1(__ctx, (seed + h1) * 2.f, param1, arity);
  while (h1 > 0.25f) {
    seed += arity;
    h1 = hash11(__ctx, seed);
    f1 = func1(__ctx, (seed + h1) * 2.f, f1, arity);
  }
  return f1;
}
float function(Context &__ctx, float x) {
  return random_function_generator(__ctx, x, floor(iTime * 4.f));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 color = f32x3(0.0f);
  f32x2 coord = fragCoord;
  for (int y = -4; y < 4; y++) {
    for (int x = -4; x < 4; x++) {
      coord = (fragCoord + (f32x2(x, y) / float(4)));
      bool pointa = round(function(__ctx, (coord.data[0] - 1.0f) * 4.0f /
                                              iResolution.data[0]) *
                          iResolution.data[1]) >=
                    2.5f * (coord.data[1] - 0.5f * iResolution.data[1]);
      bool pointb =
          round(function(__ctx, (coord.data[0]) * 4.0f / iResolution.data[0]) *
                iResolution.data[1]) <=
          2.5f * (coord.data[1] - 0.5f * iResolution.data[1]);
      bool pointc =
          round(function(__ctx, (coord.data[0]) * 4.0f / iResolution.data[0]) *
                iResolution.data[1]) >=
          2.5f * (coord.data[1] - 0.5f * iResolution.data[1]);
      bool pointd = round(function(__ctx, (coord.data[0] - 1.0f) * 4.0f /
                                              iResolution.data[0]) *
                          iResolution.data[1]) <=
                    2.5f * (coord.data[1] - 0.5f * iResolution.data[1]);
      bool pointe =
          abs(round(
                  function(__ctx, coord.data[0] * 4.0f / iResolution.data[0]) *
                  iResolution.data[1]) -
              2.5f * (coord.data[1] - 0.5f * iResolution.data[1])) <= 1.0f;
      if ((pointa && pointb) || (pointc && pointd) || pointe) {
        color += f32x3(1.0f);
      }
    }
  }
  fragColor = f32x4(color / float(4 * 4 * 4), 1.0f);
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
