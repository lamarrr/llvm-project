// Port of https://www.shadertoy.com/view/ctVBRK; author: guvacode.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctVBRK.json.
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
const float MAX_SLICE_OFFSET = 0.4f;
const float T_MAX = 2.0f;
const float T_JUMP = 0.90f;
const float jump_speed = 5.0f;
const f32x3 blue_col = f32x3(0.3f, 0.3f, 0.6f);
const f32x3 white_col = f32x3(0.8f, 0.8f, 0.95f);
float sdLine(Context &__ctx, f32x2 p, f32x2 a, f32x2 b, float ring) {
  f32x2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h) - ring;
}
float rand(Context &__ctx, f32x2 co) {
  return fract(sin(dot(co.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 color = f32x3(0.0f);
  float time = mod(iTime, T_MAX);
  float t = time / T_MAX;
  f32x2 mo = (2.0f * iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
             min(iResolution.data[0], iResolution.data[1]);
  f32x2 p = (2.0f * fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
            min(iResolution.data[0], iResolution.data[1]);
  float p_len = length(p);
  p += f32x2(0, -0.2f);
  float ta = 6.28318f * mod(iTime, 8.0f) / 8.0f;
  float ay = 0.0f, ax = 0.0f, az = 0.0f;
  if (iMouse.data[2] > 0.0f) {
    ay = 3.0f * mo.data[0];
    ax = 3.0f * mo.data[1];
  }
  f32x3x3 mY = f32x3x3(cos(ay), 0.0f, sin(ay), 0.0f, 1.0f, 0.0f, -sin(ay), 0.0f,
                       cos(ay));
  f32x3x3 mX = f32x3x3(1.0f, 0.0f, 0.0f, 0.0f, cos(ax), sin(ax), 0.0f, -sin(ax),
                       cos(ax));
  f32x3x3 m = mX * mY;
  f32x3 v = f32x3(p, 1.0f);
  v = m * v;
  float trail_start, trail_end, trail_length = 1.0f, trail_x;
  float fade = mix(1.4f, 0.0f, smoothstep(0.65f, 0.95f, t));
  for (float i = 0.0f; i < 60.0f; i++) {
    f32x3 trail_color = f32x3(0.0f);
    float angle = atan(v.data[1], v.data[0]) / 3.141592f / 2.0f + 0.13f * i;
    float slice = floor(angle * 125.0f);
    float slice_fract = fract(angle * 125.0f);
    float slice_offset =
        MAX_SLICE_OFFSET * rand(__ctx, f32x2(slice, 4.0f + i * 25.0f)) -
        (MAX_SLICE_OFFSET / 2.0f);
    float dist = 10.0f * rand(__ctx, f32x2(slice, 1.0f + i * 2.0f)) - 5.0f;
    float z = dist * v.data[2] / length(v.swizzle<0, 1>());
    float f = sign(dist);
    if (f == 0.0f)
      f = 1.0f;
    float fspeed = f * (rand(__ctx, f32x2(slice, 1.0f + i * 0.1f)) + i * 0.01f);
    float fjump_speed = f * jump_speed;
    float ftrail_length = f * trail_length;
    trail_end = 10.0f * rand(__ctx, f32x2(slice, i + 10.0f)) - 5.0f;
    trail_end -= t * fspeed;
    trail_start = trail_end + ftrail_length;
    if (f >= 0.0f) {
      trail_start =
          max(trail_end, trail_start - (t * fspeed) -
                             mix(0.0f, fjump_speed, smoothstep(0.5f, 1.0f, t)));
    } else {
      trail_start =
          min(trail_end, trail_start - (t * fspeed) -
                             mix(0.0f, fjump_speed, smoothstep(0.5f, 1.0f, t)));
    }
    trail_x = smoothstep(trail_start, trail_end, z);
    trail_color = mix(blue_col, white_col, trail_x);
    float h = sdLine(__ctx, f32x2(slice_fract + slice_offset, z),
                     f32x2(0.5f, trail_start), f32x2(0.5f, trail_end),
                     mix(0.0f, 0.015f, z));
    float threshold = mix(0.12f, 0.0f, smoothstep(0.5f, 0.8f, t));
    h = (h < 0.01f) ? 1.0f : 0.75f * smoothstep(threshold, 0.0f, abs(h));
    trail_color *= fade * h;
    color = max(color, trail_color);
  }
  color += mix(1.0f, 0.0f, smoothstep(0.0f, 0.2f, t));
  fragColor = f32x4(color, 1.0f);
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
