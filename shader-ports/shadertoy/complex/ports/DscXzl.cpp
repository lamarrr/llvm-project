// Port of https://www.shadertoy.com/view/DscXzl; author: sh1boot.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DscXzl.json.
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

const float density = 12.0f;
const int w = 2, h = 2;
const bool polar = false;
const bool accel = true;
const float wobble = 0.25f;
float nrand(Context &__ctx, f32x2 n) {
  return fract(sin(dot(n.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453f);
}
f32x2 nrand2(Context &__ctx, f32x2 n) {
  return f32x2(nrand(__ctx, n * f32x2(-3.2145f, 1.2345f)),
               nrand(__ctx, n * f32x2(-5.4321f, 3.4521f)));
}
float nrand(Context &__ctx, i32x2 n) {
  return fract(sin(dot(f32x2(n.swizzle<0, 1>()), f32x2(12.9898f, 78.233f))) *
               43758.5453f);
}
f32x2 nrand2(Context &__ctx, i32x2 n) {
  return f32x2(nrand(__ctx, f32x2(n) * f32x2(-3.2145f, 1.2345f)),
               nrand(__ctx, f32x2(n) * f32x2(-5.4321f, 3.4521f)));
}
f32x4 point(Context &__ctx, i32x3 c, float t) {
  const int period = 256;
  float speed = 0.2f + 1.0f * nrand(__ctx, c.swizzle<0, 2>());
  float st = mod(speed * t, float(period));
  c.data[1] -= int(floor(st));
  if (c.data[1] < 0)
    c.data[1] += period;
  f32x4 p = f32x4(nrand2(__ctx, c.swizzle<0, 1>()), speed,
                  nrand(__ctx, c.swizzle<0, 1>() + c.swizzle<1, 2>()));
  p.data[1] += fract(st) - 0.5f;
  return p;
}
f32x3 _pal(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(6.28318f * (c * t + d));
}
f32x4 pal(Context &__ctx, float i) {
  return f32x4(_pal(__ctx, i, f32x3(0.5f, 0.5f, 0.5f), f32x3(0.5f, 0.5f, 0.5f),
                    f32x3(1.0f, 1.0f, 1.0f), f32x3(0.0f, 0.33f, 0.67f)),
               1.0f);
}
f32x2 xfrm(Context &__ctx, f32x2 uv) {
  if (polar) {
    uv -= 0.5f;
    uv = f32x2(atan(uv.data[1], uv.data[0]), distance(uv, f32x2(0.0f)));
    uv.data[0] /= 3.141592653589f;
  }
  if (accel) {
    uv.data[1] = pow(uv.data[1] + 0.03f, 0.25f);
  }
  return uv;
}
f32x2 ixfrm(Context &__ctx, f32x2 uv) {
  if (accel) {
    uv.data[1] = pow(uv.data[1], 1.0f / 0.25f) - 0.03f;
  }
  if (polar) {
    uv.data[0] *= 3.141592653589f;
    uv = f32x2(cos(uv.data[0]), sin(uv.data[0])) * uv.data[1];
    uv += 0.5f;
  }
  return uv;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.data[1];
  fragColor = f32x4(0);
  f32x4 nearest = f32x4(1.0f);
  float nearest_d = 9999.0f;
  i32x2 iuv = i32x2(floor(xfrm(__ctx, uv) * density));
  for (int y = -h; y <= h; ++y) {
    if (iuv.data[1] + y < 0)
      continue;
    for (int x = -w; x <= w; ++x) {
      for (int z = 0; z < 2; ++z) {
        i32x3 ic = i32x3(iuv + i32x2(x, y), z);
        f32x4 p = point(__ctx, ic, iTime);
        p.set_swizzle<0, 1>(p.swizzle<0, 1>() + (f32x2(ic)));
        p.data[0] +=
            sin((p.data[1] * 0.5f + p.data[3]) * 2.0f * 3.141592653589f) *
            wobble;
        p.set_swizzle<0, 1>(ixfrm(__ctx, p.swizzle<0, 1>() / density));
        p.data[2] = mix(p.data[2], 1.0f, 0.25f);
        float d = distance(uv, p.swizzle<0, 1>());
        if (nearest_d * p.data[2] >= d * nearest.data[2]) {
          nearest = p;
          nearest_d = d;
        }
      }
    }
  }
  if (iMouse.data[2] > 0.0f) {
    fragColor = pal(__ctx, nearest.data[3]);
  } else {
    fragColor =
        pal(__ctx, nearest.data[3]) * nearest.data[2] / (nearest_d * 150.0f);
    fragColor.set_swizzle<0, 1, 2>(
        clamp(fragColor.swizzle<0, 1, 2>() - 0.1f, 0.0f, 1.0f));
  }
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
