// Port of https://www.shadertoy.com/view/ltc3Dr; author: jherico.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ltc3Dr.json.
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
const f32x3 DARKGREY = f32x3(0.1f);
const f32x3 LIGHTGREY = f32x3(0.2f);
const f32x3 WHITE = f32x3(1);
const f32x3 AMBER = f32x3(1, 0.49f, 0);
const f32x3 TRON_BLUE = f32x3(0.1f, 0.5f, 0.8f);
const f32x3 HEX_BORDER = LIGHTGREY;
const f32x3 HEX_BODY = DARKGREY;
const f32x3 HAZE = LIGHTGREY;
const float HEX_SIZE = 18.0f;
i32x3 cube_round(Context &__ctx, f32x3 h) {
  f32x3 r = floor(h + 0.5f);
  f32x3 diff = abs(r - h);
  if (diff.data[0] > diff.data[1] && diff.data[0] > diff.data[2]) {
    r.data[0] = -r.data[1] - r.data[2];
  } else if (diff.data[1] > diff.data[2]) {
    r.data[1] = -r.data[0] - r.data[2];
  } else {
    r.data[2] = -r.data[0] - r.data[1];
  }
  return i32x3(int(r.data[0]), int(r.data[1]), int(r.data[2]));
}
float cube_distance(Context &__ctx, i32x3 a, i32x3 b) {
  f32x3 ab = abs(f32x3(a) - f32x3(b));
  return max(max(ab.data[0], ab.data[1]), ab.data[2]);
}
i32x2 cube_to_hex(Context &__ctx, i32x3 c) { return c.swizzle<0, 1>(); }
i32x3 hex_to_cube(Context &__ctx, i32x2 h) {
  return i32x3(h.data[0], h.data[1], -h.data[0] - h.data[1]);
}
f32x3 hex_to_cube(Context &__ctx, f32x2 h) {
  return f32x3(h.data[0], h.data[1], -h.data[0] - h.data[1]);
}
i32x2 hex_round(Context &__ctx, f32x2 h) {
  return cube_to_hex(__ctx, cube_round(__ctx, hex_to_cube(__ctx, h)));
}
i32x2 pixel_to_hex(Context &__ctx, f32x2 pixel) {
  const f32x2 PIXEL_TO_HEX_CONST = f32x2(sqrt(3.0f), 2.0f) / 3.0f;
  f32x2 result = PIXEL_TO_HEX_CONST * pixel;
  result.data[0] -= pixel.data[1] / 3.0f;
  return hex_round(__ctx, result);
}
f32x2 hex_to_pixel(Context &__ctx, i32x2 hex2) {
  f32x2 hex = f32x2(hex2);
  return f32x2(sqrt(3.0f) * (hex.data[0] + hex.data[1] / 2.0f),
               1.5f * hex.data[1]);
}
f32x2 hex_center(Context &__ctx, f32x2 pixel) {
  return hex_to_pixel(__ctx, pixel_to_hex(__ctx, pixel));
}
float point_line_distance(Context &__ctx, f32x2 slope, f32x2 p) {
  f32x2 normal = f32x2(-slope.data[1], slope.data[0]);
  return abs(dot(normalize(normal), p));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 col = DARKGREY;
  f32x2 xy =
      1.0f - 2.0f * fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  xy *= HEX_SIZE;
  xy.data[1] /= iResolution.data[0] / iResolution.data[1];
  f32x2 mousexy =
      1.0f - 2.0f * iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  mousexy.data[1] /= iResolution.data[0] / iResolution.data[1];
  mousexy *= HEX_SIZE;
  float lineDist = point_line_distance(__ctx, mousexy, xy);
  if (lineDist < 0.03f) {
    fragColor = f32x4(1.0f, 0.0f, 0.0f, 1.0f);
    return;
  }
  col = mix(col, f32x3(1.0f, 0.0f, 0.0f), smoothstep(0.05f, 0.03f, lineDist));
  i32x2 hex = pixel_to_hex(__ctx, xy);
  f32x2 hexCenter = hex_to_pixel(__ctx, hex);
  i32x2 origin = i32x2(0);
  if (hex == origin) {
    fragColor = f32x4(0.0f, 1.0f, 1.0f, 1.0f);
    return;
  }
  i32x2 mousehex = pixel_to_hex(__ctx, mousexy);
  i32x3 cur = hex_to_cube(__ctx, hex);
  i32x3 a = hex_to_cube(__ctx, origin);
  i32x3 b = hex_to_cube(__ctx, mousehex);
  if (cur == b) {
    fragColor = f32x4(0.0f, 0.0f, 1.0f, 1.0f);
    return;
  }
  float cubedist = floor(cube_distance(__ctx, a, b));
  for (float i = 0.0f; i < 10000.0f; i += 0.01f) {
    if (i > cubedist) {
      break;
    }
    float mixAmount = i / cubedist;
    i32x3 testCube = cube_round(__ctx, mix(f32x3(a), f32x3(b), mixAmount));
    i32x2 testHex = cube_to_hex(__ctx, testCube);
    if (hex == testHex) {
      fragColor = f32x4(1.0f, 1.0f, 1.0f, 1.0f);
      return;
    }
  }
  f32x2 hexPos = hex_to_pixel(__ctx, hex.swizzle<0, 1>());
  float dist = length(xy - hexPos);
  if (dist < 0.3f) {
    col = f32x3(1.0f, 0.0f, 1.0f);
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
