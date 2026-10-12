// Port of https://www.shadertoy.com/view/ctBSWR; author: NinjaKoala.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctBSWR.json.
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
const int num_iterations = 6;
const int num_iterations2 = 10;
float a_param = 30.f;
float b_param = -30.f;
float c_param = 0.f;
float d_param = 0.f;
f32x3 bg_col = f32x3(1);
f32x3 conic_col = f32x3(0, 0, 1);
f32x3 lag_curve_col = f32x3(0, 1, 0);
f32x3 asym_col = f32x3(1, 0, 0);
f32x3 dot_col = f32x3(0);
float line_width = .001f;
float dot_size = .01f;
float zoom = 1.f;
const float pi = 3.1415925f;
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
f32x2 iteration(Context &__ctx, f32x2 p, f32x2 p2) {
  float p_val1, p_val2;
  f32x2 p_grad1, p_grad2;
  p_val2 = lagrange_curve(__ctx, p, p2, p_grad2);
  p -= (p_grad2 * p_val2) / dot(p_grad2, p_grad2);
  p_val1 = conic_curve(__ctx, p, p_grad1);
  p -= (p_grad1 * p_val1) / dot(p_grad1, p_grad1);
  return p;
}
f32x2 iteration2(Context &__ctx, f32x2 p, f32x2 p2) {
  float p_val1, p_val2;
  f32x2 p_grad1, p_grad2;
  p_val1 = conic_curve(__ctx, p, p_grad1);
  f32x2 tang = f32x2(p_grad1.data[1], -p_grad1.data[0]);
  p -= dot(p - p2, tang) / dot(tang, tang) * tang;
  p_val1 = conic_curve(__ctx, p, p_grad1);
  p -= (p_grad1 * p_val1) / dot(p_grad1, p_grad1);
  return p;
}
f32x2 nearest_point_to_conic(Context &__ctx, f32x2 uv, float &sgn) {
  float asym_x =
      -.5f * (2.f * b_param * uv.data[0] + c_param) / (a_param - b_param);
  float asym_y =
      .5f * (2.f * a_param * uv.data[1] + d_param) / (a_param - b_param);
  float x_discriminant =
      c_param * c_param -
      4.f * a_param * ((b_param * asym_y + d_param) * asym_y + 1.f);
  float y_discriminant =
      d_param * d_param -
      4.f * b_param * ((a_param * asym_x + c_param) * asym_x + 1.f);
  f32x2 nearest_point = uv;
  float nearest_dist = 1e38f;
  if (x_discriminant > 0.f) {
    float x_intersection1 = (-c_param + sqrt(x_discriminant)) / (2.f * a_param);
    f32x2 cur_point1 = f32x2(x_intersection1, asym_y);
    float cur_dis1 = distance(cur_point1, uv);
    if (cur_dis1 < nearest_dist) {
      nearest_dist = cur_dis1;
      nearest_point = cur_point1;
    }
    float x_intersection2 = (-c_param - sqrt(x_discriminant)) / (2.f * a_param);
    f32x2 cur_point2 = f32x2(x_intersection2, asym_y);
    float cur_dis2 = distance(cur_point2, uv);
    if (cur_dis2 < nearest_dist) {
      nearest_dist = cur_dis2;
      nearest_point = cur_point2;
    }
  }
  if (y_discriminant > 0.f) {
    float y_intersection1 = (-d_param + sqrt(y_discriminant)) / (2.f * b_param);
    f32x2 cur_point1 = f32x2(asym_x, y_intersection1);
    float cur_dis1 = distance(cur_point1, uv);
    if (cur_dis1 < nearest_dist) {
      nearest_dist = cur_dis1;
      nearest_point = cur_point1;
    }
    float y_intersection2 = (-d_param - sqrt(y_discriminant)) / (2.f * b_param);
    f32x2 cur_point2 = f32x2(asym_x, y_intersection2);
    float cur_dis2 = distance(cur_point2, uv);
    if (cur_dis2 < nearest_dist) {
      nearest_dist = cur_dis2;
      nearest_point = cur_point2;
    }
  }
  f32x2 nor1;
  float val1 = conic_curve(__ctx, uv, nor1);
  sgn = sign(val1);
  if (distance(uv, iteration(__ctx, uv, uv)) <
      distance(nearest_point, iteration(__ctx, nearest_point, uv))) {
    nearest_point = uv;
  }
  if (sign(a_param) != sign(b_param) || sgn < 0.f) {
    for (int i = 0; i < num_iterations; i++) {
      nearest_point = iteration(__ctx, nearest_point, uv);
    }
  } else {
    for (int i = 0; i < num_iterations2; i++) {
      nearest_point = iteration2(__ctx, nearest_point, uv);
    }
  }
  return nearest_point;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  a_param = mix(30.f, -5.f, sin(iTime));
  b_param = mix(-30.f, -10.f, sin(iTime));
  c_param = 0.f;
  d_param = 0.f;
  f32x2 uv = gl_FragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
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
  f32x3 color = bg_col;
  float dis = 1e38f;
  f32x2 conic_grad;
  float conic_val = conic_curve(__ctx, uv, conic_grad);
  dis = min(dis, abs(conic_val / length(conic_grad)) - line_width);
  color = mix(conic_col, color, smoothstep(0.f, border, dis));
  dis = 1e38f;
  float asym_x =
      -.5f * (2.f * b_param * mouse.data[0] + c_param) / (a_param - b_param);
  float asym_y =
      .5f * (2.f * a_param * mouse.data[1] + d_param) / (a_param - b_param);
  dis = min(dis, abs(uv.data[0] - asym_x));
  dis = min(dis, abs(uv.data[1] - asym_y));
  color = mix(asym_col, color, smoothstep(0.f, border, dis));
  dis = 1e38f;
  f32x2 lag_curve_grad;
  float lag_curve_val = lagrange_curve(__ctx, uv, mouse, lag_curve_grad);
  dis = min(dis, abs(lag_curve_val / length(lag_curve_grad)) - line_width);
  color = mix(lag_curve_col, color, smoothstep(0.f, border, dis));
  dis = 1e38f;
  float sgn;
  f32x2 nearest_point = nearest_point_to_conic(__ctx, mouse, sgn);
  dis = min(dis, distance(uv, nearest_point) - dot_size);
  color = mix(dot_col, color, smoothstep(0.f, border, dis));
  nearest_point = nearest_point_to_conic(__ctx, uv, sgn);
  dis = distance(uv, nearest_point);
  color = f32x3(1.0f) - sgn * f32x3(0.1f, 0.4f, 0.7f);
  color *= 1.0f - exp(-8.0f * dis);
  color *= 0.8f + 0.2f * cos(480.0f * dis * sgn);
  color = mix(color, f32x3(1.0f), 1.0f - smoothstep(0.0f, 0.005f, dis));
  fragColor = f32x4(color, 1);
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
