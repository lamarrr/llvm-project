// Port of https://www.shadertoy.com/view/Ds2Xzw; author: Etidou.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Ds2Xzw.json.
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

f32x2 rotate(Context &__ctx, f32x2 v, float a) {
  float sinA = sin(a);
  float cosA = cos(a);
  return f32x2(v.data[0] * cosA - v.data[1] * sinA,
               v.data[1] * cosA + v.data[0] * sinA);
}
float poly(Context &__ctx, f32x2 uv, float d) {
  float a = atan(uv.data[0], uv.data[1]) + 3.1415926535897932384626433832795f;
  float r = length(uv);
  float v = 6.2831853071795864769252867665590f / float(8);
  float final = cos(floor(.5f + a / v) * v - a) * r;
  return max(abs(final), abs(final)) - d;
}
float smootheststep(Context &__ctx, float edge0, float edge1, float x) {
  x = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f) * 3.14159265f;
  return 0.5f - (cos(x) * 0.5f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  uv = uv * 2.0f - 1.0f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  uv *= 1.5f;
  float blurAmount = -0.005f * 1080.0f / iResolution.data[1];
  float period = 25.0f;
  float time = iTime / period;
  time = mod(time, 1.0f);
  time = smootheststep(__ctx, 0.0f, 1.0f, time);
  fragColor = f32x4(0.0f, 0.0f, 0.0f, 1.0f);
  for (int i = 0; i < 25; i++) {
    float n = float(i);
    float size = 1.0f - n / 25.0f;
    float rotateAmount =
        (n * 0.5f + 0.25f) * 3.1415926535897932384626433832795f * 2.0f;
    float animTime = time;
    if (n < 1.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.467f, 0.051f, 1.f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 2.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.184f, 0.047f, 0.91f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 3.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.f, 0.11f, 1.f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 4.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.f, 0.345f, 0.91f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 5.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.0f, 0.6f, 0.941f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 6.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.051f, 0.663f, 1.f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 7.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.0047f, 0.835f, 0.91f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 8.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.f, 1.f, 0.808f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 9.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.f, 0.91f, 0.471f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 10.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.063f, 0.91f, 0.047f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 11.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.369f, 1.0f, 0.f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 12.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.651f, 0.91f, 0.f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 13.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(1.f, 0.98f, 0.051f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 14.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.91f, 0.808f, 0.047f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 15.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(1.f, 0.78f, 0.f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 16.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.91f, 0.62f, 0.f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 17.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(1.f, 0.6f, 0.051f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 18.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.91f, 0.424f, 0.047f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 19.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(1.f, 0.29f, 0.f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 20.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.91f, 0.125f, 0.f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 21.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(1.f, 0.051f, 0.078f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 22.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.91f, 0.047f, 0.506f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 23.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.98f, 0.f, 1.f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 24.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.647f, 0.f, 0.91f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    } else if (n < 25.f) {
      fragColor.set_swizzle<0, 1, 2>(mix(
          fragColor.swizzle<0, 1, 2>(), f32x3(0.467f, 0.051f, 1.f),
          smoothstep(
              0.0f, blurAmount,
              poly(__ctx, rotate(__ctx, uv, -rotateAmount * animTime), size))));
    }
    float blackOffset = mix(1.0f / 4.0f, 1.0f / 2.0f, n / 25.f) / 25.f;
    fragColor.set_swizzle<0, 1, 2>(mix(
        fragColor.swizzle<0, 1, 2>(), f32x3(0.0f, 0.0f, 0.0f),
        smoothstep(0.0f, blurAmount,
                   poly(__ctx,
                        rotate(__ctx, uv,
                               -(rotateAmount +
                                 3.1415926535897932384626433832795f / 2.0f) *
                                   time),
                        size - blackOffset))));
  }
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
