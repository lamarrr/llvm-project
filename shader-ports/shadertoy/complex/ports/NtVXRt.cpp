// Port of https://www.shadertoy.com/view/NtVXRt; author: Cleric.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NtVXRt.json.
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
float nextFloat(Context &__ctx, int &seed) {
  seed ^= seed << 13;
  seed ^= seed >> 17;
  seed ^= seed << 5;
  return -1.f + 2.f * abs(fract(float(seed) / 3141.592653f));
}
f32x2x2 rotate2D(Context &__ctx, float rad) {
  float cs = cos(rad);
  float sn = sin(rad);
  return f32x2x2(cs, sn, -sn, cs);
}
f32x2 mandelpent_iter(Context &__ctx, f32x2 z, f32x2 c, float t) {
  float scale = 1.82f;
  float rad = -t / 4.0f;
  f32x2x2 M = rotate2D(__ctx, rad);
  z = M * z;
  float ang = atan(z.data[1], z.data[0]);
  float p = floor(mod((ang - 3.14159265359f / 2.0f +
                       ((360.0f * 3.14159265359f / 180.0f / 5.0f) / 2.0f)) /
                          (360.0f * 3.14159265359f / 180.0f / 5.0f),
                      5.0f));
  float rot = -(360.0f * 3.14159265359f / 180.0f / 5.0f) * p;
  float d = .76f + .1f * sin(3.0f * t);
  float radius =
      .51f *
          smoothstep((((((5.0f + 10.0f) + 5.0f) - 5.0f) + 15.0f) + 10.f) - 2.0f,
                     (((((5.0f + 10.0f) + 5.0f) - 5.0f) + 15.0f) + 10.f), t) +
      .1f * sin(t);
  scale = scale + .1f * sin(3.0f * t);
  M = rotate2D(__ctx, rot);
  z = M * z;
  if (z.data[1] > d)
    z.data[1] = d - (z.data[1] - d);
  M[0][1] = -M[0][1];
  M[1][0] = -M[1][0];
  z = M * z;
  float rr = dot(z, z);
  float m = scale;
  if (rr < radius * radius) {
    m = scale / (radius * radius);
  } else if (rr < 1.0f) {
    m = scale / rr;
  }
  return z * m + c;
}
f32x2 ifsgeneric_iter(Context &__ctx, f32x2 z, f32x2 c_, float t) {
  float d = 1e10f;
  f32x2 np;
  for (int i = 0; i < 2; i++) {
    float deg = float(i) * 360.0f / float(2) / 180.0f * 3.14159265359f;
    f32x2 p = f32x2(cos(deg + t), sin(deg + t));
    float td = dot(z - p, z - p);
    if (td < d) {
      np = p;
      d = td;
    }
  }
  z = np + (z - np) * 2.f;
  float rad = t;
  f32x2x2 M = rotate2D(__ctx, rad);
  z = M * z;
  return z;
}
f32x3 mandelpent(Context &__ctx, f32x2 c, float t) {
  f32x2 z = c;
  int n;
  for (int i = 0; i <= 15; i++) {
    n = i;
    z = mix(ifsgeneric_iter(__ctx, z, c, t), mandelpent_iter(__ctx, z, c, t),
            smoothstep((((((5.0f + 10.0f) + 5.0f) - 5.0f) + 15.0f) - 5.f),
                       (((((5.0f + 10.0f) + 5.0f) - 5.0f) + 15.0f) + 10.f), t));
    float mz = dot(z, z);
    if (mz > 124.0f)
      break;
  }
  float comp = float(n) / float(15);
  return f32x3(1.f, .9f - comp * .5f, .0f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float t =
      mod(iTime, ((((((5.0f + 10.0f) + 5.0f) - 5.0f) + 15.0f) + 10.f) + 15.0f));
  f32x2 uv = (fragCoord - .5f * iResolution.swizzle<0, 1>()) /
             min(iResolution.data[0], iResolution.data[1]);
  float step = floor(t / 0.3f);
  int randSeed = int(step) * 12345678;
  f32x2 curr_xy = f32x2(nextFloat(__ctx, randSeed), nextFloat(__ctx, randSeed));
  randSeed = (int(step) + 1) * 12345678;
  f32x2 next_xy = f32x2(nextFloat(__ctx, randSeed), nextFloat(__ctx, randSeed));
  float next_trans = step * 0.3f + 0.3f;
  float trans_weight = smoothstep(next_trans - .25f, next_trans, t);
  f32x2 blob_xy = mix(curr_xy, next_xy, trans_weight);
  f32x2 spiral_xy =
      f32x2(cos(t * 2.f * 3.14159265359f), sin(t * 2.f * 3.14159265359f));
  spiral_xy *= 1.f - smoothstep(5.0f, ((5.0f + 10.0f) + 5.0f), t);
  float spiral_weight = smoothstep(5.0f, (5.0f + 10.0f), t);
  blob_xy = mix(blob_xy, spiral_xy, spiral_weight);
  blob_xy *= .5f;
  float ang = atan(uv.data[0], uv.data[1]);
  float r = length(uv);
  float veil_mask =
      (.5f + cos(.5f * 3.14159265359f * t + 12.f * ang) / 2.0f) *
      (.5f + cos(-2.f * 3.14159265359f * t + 12.f * ang - r * 50.f) / 2.f);
  f32x2 blob_xy_0 = uv - blob_xy;
  float thought_mask =
      1.f - smoothstep((((5.0f + 10.0f) + 5.0f) - 5.0f),
                       ((((5.0f + 10.0f) + 5.0f) - 5.0f) + 15.0f), t) *
                .96f;
  f32x3 fract_color = mandelpent(__ctx, uv, t);
  veil_mask =
      mix(.5f + veil_mask / 5.f, 1.f,
          smoothstep((((((5.0f + 10.0f) + 5.0f) - 5.0f) + 15.0f) - 5.f),
                     (((((5.0f + 10.0f) + 5.0f) - 5.0f) + 15.0f) + 10.f), t));
  veil_mask *= clamp(1.0f - length(blob_xy_0) * 15.f * thought_mask, 0.f, 1.f);
  fragColor = f32x4(fract_color * veil_mask, 1.0f);
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
