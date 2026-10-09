// Port of https://www.shadertoy.com/view/DsjXRh; author: NinjaKoala.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DsjXRh.json.
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
f32x3 bg_col = f32x3(1);
f32x3 ellipse_col = f32x3(0, 0, 1);
f32x3 lag_curve_col = f32x3(0, 1, 0);
f32x3 dot_col = f32x3(0);
float line_width = .001f;
float dot_size = .01f;
float zoom = 2.f;
float ellipse_param = 3.f;
const int num_iterations = 16;
int aa = 2;
float pi = 3.1415925f;
float ellipse(Context &__ctx, f32x2 p, f32x2 &grad) {
  grad = f32x2(2, 2.f * ellipse_param) * p;
  return p.data[0] * p.data[0] + ellipse_param * p.data[1] * p.data[1] - 1.f;
}
float lagrange_curve(Context &__ctx, f32x2 p, f32x2 p2, f32x2 &grad) {
  grad = f32x2(-2.f * p2.data[1] + (2.f - 2.f * ellipse_param) * p.data[1],
               2.f * ellipse_param * p2.data[0] +
                   (2.f - 2.f * ellipse_param) * p.data[0]);
  return 2.f * ellipse_param * p2.data[0] * p.data[1] -
         2.f * p2.data[1] * p.data[0] +
         (2.f - 2.f * ellipse_param) * p.data[0] * p.data[1];
}
f32x2 iteration(Context &__ctx, f32x2 p, f32x2 p2) {
  float p_val1, p_val2;
  f32x2 p_grad1, p_grad2;
  p_val1 = ellipse(__ctx, p, p_grad1);
  p -= (p_grad1 * p_val1) / dot(p_grad1, p_grad1);
  p_val2 = lagrange_curve(__ctx, p, p2, p_grad2);
  p -= (p_grad2 * p_val2) / dot(p_grad2, p_grad2);
  return p;
}
float spectral(Context &__ctx, float x) {
  return clamp(abs(mod((x / pi + 1.f) * 4.f, 8.f) - 4.f) - 2.f, -.75f, .75f) *
             .5f / .75f +
         .5f;
}
f32x3 to_col(Context &__ctx, f32x2 z) {
  float direction = atan(z.data[0], z.data[1]);
  return f32x3(spectral(__ctx, direction),
               spectral(__ctx, direction + pi / 2.f),
               spectral(__ctx, direction - pi));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  uv -= .5f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x2 mouse = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  mouse -= .5f;
  mouse.data[0] *= iResolution.data[0] / iResolution.data[1];
  float border = 2.f / iResolution.data[1];
  float aspect_ratio = iResolution.data[0] / iResolution.data[1];
  uv *= zoom;
  mouse *= zoom;
  border *= zoom;
  const f32x2x2 aa_rot = f32x2x2(1.f, .5f, -.5f, 1.f);
  f32x3 col_sum = f32x3(0);
  f32x2 p1;
  for (int x = 0; x < aa; x++) {
    for (int y = 0; y < aa; y++) {
      f32x2 aa_offset = (f32x2(x, y) - .5f * (float(aa) - 1.f)) / float(aa);
      aa_offset *= zoom * f32x2(aspect_ratio, 1) / iResolution.swizzle<0, 1>();
      p1 = uv + aa_offset;
      for (int i = 0; i < num_iterations; i++) {
        p1 = iteration(__ctx, p1, mouse);
      }
      col_sum += pow(to_col(__ctx, p1), f32x3(2.2f));
    }
  }
  bg_col = pow(col_sum / (float(aa) * float(aa)), f32x3(1.0f / 2.2f));
  bg_col *= .75f;
  f32x3 color = bg_col;
  float dis = 1e38f;
  f32x2 ellipse_grad;
  float ellipse_val = ellipse(__ctx, uv, ellipse_grad);
  dis = min(dis, abs(ellipse_val / length(ellipse_grad)) - line_width);
  color = mix(ellipse_col, color, smoothstep(0.f, border, dis));
  dis = 1e38f;
  f32x2 lag_curve_grad;
  float lag_curve_val = lagrange_curve(__ctx, uv, mouse, lag_curve_grad);
  dis = min(dis, abs(lag_curve_val / length(lag_curve_grad)) - line_width);
  color = mix(lag_curve_col, color, smoothstep(0.f, border, dis));
  dis = 1e38f;
  dis = min(dis, distance(uv, p1) - dot_size);
  color = mix(dot_col, color, smoothstep(0.f, border, dis));
  fragColor = f32x4(color, 1);
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
