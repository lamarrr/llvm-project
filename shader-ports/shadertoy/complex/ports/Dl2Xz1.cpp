// Port of https://www.shadertoy.com/view/Dl2Xz1; author: NinjaKoala.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Dl2Xz1.json.
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
const float pi = 3.1415925f;
const int num_segments1 = 32;
const int num_segments2 = 128;
const float step_size = .07f;
const float zoom = 3.f;
const float random_offset_factor = .2f;
const float line_width = .004f;
const float dot_size = .02f;
const f32x3 bg_col = f32x3(1);
const f32x3 raster_col = f32x3(0);
const f32x3 traj_col = f32x3(0, 1, 0);
const f32x3 corr_col = f32x3(1, 0, 0);
const f32x3 conic_col = f32x3(0, 0, 1);
const f32x3 lagrange_col = f32x3(1, 1, 0);
const f32x3 dot_col = f32x3(0);
const float a_param = 5.f;
const float b_param = -5.f;
const float c_param = 0.f;
const float d_param = 0.f;
float conic_curve(Context &__ctx, f32x2 p, f32x2 &grad) {
  grad = f32x2(2.f * p.data[0] * a_param + c_param,
               2.f * p.data[1] * b_param + d_param);
  return p.data[0] * (p.data[0] * a_param + c_param) +
         p.data[1] * (p.data[1] * b_param + d_param) + 1.f;
}
float lagrange_curve(Context &__ctx, f32x2 p, f32x2 p2, f32x2 &grad) {
  grad =
      f32x2(2.f * (a_param * (p.data[1] - p2.data[1]) - p.data[1] * b_param) -
                d_param,
            2.f * (b_param * (p2.data[0] - p.data[0]) + p.data[0] * a_param) +
                c_param);
  return 2.f * (p.data[0] * a_param * (p.data[1] - p2.data[1]) -
                p.data[1] * b_param * (p.data[0] - p2.data[0])) +
         c_param * (p.data[1] - p2.data[1]) -
         d_param * (p.data[0] - p2.data[0]);
}
float length2(Context &__ctx, f32x2 v) { return dot(v, v); }
float segment_dis_sq(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length2(__ctx, pa - ba * h);
}
float trajectory_segments_dis_sq(Context &__ctx, f32x2 uv, f32x2 p, bool corr) {
  float d0 = 1e38f;
  f32x2 a = p;
  for (int i = 1; i < num_segments1; i++) {
    f32x2 b = a;
    f32x2 nor;
    float val = conic_curve(__ctx, a, nor);
    f32x2 tang = f32x2(nor.data[1], -nor.data[0]);
    if (!corr) {
      b += step_size * val * nor / dot(nor, nor);
    } else {
      b += step_size * val * nor / dot(nor, nor);
      f32x2 nor2;
      float val2 = lagrange_curve(__ctx, b, p, nor2);
      b += step_size * val2 * nor2 / dot(nor2, nor2);
    }
    d0 = min(d0, segment_dis_sq(__ctx, uv, a, b));
    a = b;
  }
  a = p;
  for (int i = 1; i < num_segments2; i++) {
    f32x2 b = a;
    f32x2 nor;
    float val = conic_curve(__ctx, a, nor);
    f32x2 tang = f32x2(nor.data[1], -nor.data[0]);
    if (!corr) {
      b -= step_size * val * nor / dot(nor, nor);
    } else {
      b -= step_size * val * nor / dot(nor, nor);
      f32x2 nor2;
      float val2 = lagrange_curve(__ctx, b, p, nor2);
      b -= step_size * val2 * nor2 / dot(nor2, nor2);
    }
    d0 = min(d0, segment_dis_sq(__ctx, uv, a, b));
    a = b;
  }
  return d0;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  uv -= .5f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x2 mouse = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  mouse -= .5f;
  mouse.data[0] *= iResolution.data[0] / iResolution.data[1];
  float border = 1.f / iResolution.data[1];
  float dis = 1e38f;
  uv *= zoom;
  border *= zoom;
  mouse *= zoom;
  f32x3 col = bg_col;
  f32x2 nor1;
  float val1 = conic_curve(__ctx, uv, nor1);
  col = f32x3(mix(clamp(pow(cos(abs(val1) * 3.f), 10.f), 0.f, 1.f), 1.f, .8f));
  dis = abs(val1) / length(nor1) - line_width;
  col = mix(conic_col, col, clamp(sqrt(dis / border), 0.f, 1.f));
  f32x2 nor2;
  float val2 = lagrange_curve(__ctx, uv, mouse, nor2);
  dis = abs(val2) / length(nor2) - line_width;
  col = mix(lagrange_col, col, clamp(sqrt(dis / border), 0.f, 1.f));
  dis = sqrt(trajectory_segments_dis_sq(__ctx, uv, mouse, true)) - line_width;
  col = mix(traj_col, col, clamp(sqrt(dis / border), 0.f, 1.f));
  dis = sqrt(trajectory_segments_dis_sq(__ctx, uv, mouse, false)) - line_width;
  col = mix(corr_col, col, clamp(sqrt(dis / border), 0.f, 1.f));
  dis = distance(uv, mouse) - dot_size;
  col = mix(dot_col, col, clamp(sqrt(dis / border), 0.f, 1.f));
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
