// Port of https://www.shadertoy.com/view/7tcBDl; author: NinjaKoala.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7tcBDl.json.
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
const f32x3 dot_col = f32x3(0, 0, 1);
const f32x3 line_col = f32x3(0, 0, 1);
const f32x3 bezier_col = f32x3(0, 1, 0);
const f32x3 arc_col = f32x3(1, 0, 0);
const f32x3 bg_col = f32x3(1);
const float dot_size = .004f;
const float line_width = .001f;
const float zoom = 1.f;
const float pi = 3.1415925f;
f32x2 parametric_quadratic_bezier(Context &__ctx, float t, f32x2 p0, f32x2 p1,
                                  f32x2 p2) {
  return mix(mix(p0, p1, t), mix(p1, p2, t), t);
}
float quadratic_bezier_normal_iteration(Context &__ctx, float t, f32x2 a0,
                                        f32x2 a1, f32x2 a2) {
  f32x2 a_1 = a1 + t * a2;
  f32x2 uv_to_p = a0 + t * a_1;
  f32x2 tang = a_1 + t * a2;
  float l_tang = dot(tang, tang);
  return t - dot(tang, uv_to_p) / l_tang;
}
float quadratic_bezier_dis_approx_sq(Context &__ctx, f32x2 uv, f32x2 p0,
                                     f32x2 p1, f32x2 p2) {
  f32x2 a2 = p0 - 2.f * p1 + p2;
  f32x2 a1 = -2.f * p0 + 2.f * p1;
  f32x2 a0 = p0 - uv;
  float d0 = 1e38f;
  f32x3 t0 = f32x3(0.f, .5f, 1.f);
  float t;
  for (int i = 0; i < 3; i++) {
    t = t0[i];
    for (int j = 0; j < 3; j++) {
      t = quadratic_bezier_normal_iteration(__ctx, t, a0, a1, a2);
    }
    t = clamp(t, 0.f, 1.f);
    f32x2 uv_to_p = (a2 * t + a1) * t + a0;
    d0 = min(d0, dot(uv_to_p, uv_to_p));
  }
  return d0;
}
float quadratic_bezier_dis_approx(Context &__ctx, f32x2 uv, f32x2 p0, f32x2 p1,
                                  f32x2 p2) {
  return sqrt(quadratic_bezier_dis_approx_sq(__ctx, uv, p0, p1, p2));
}
float segment_dis(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.f, 1.f);
  return length(pa - ba * h);
}
f32x2 incenter(Context &__ctx, f32x2 p0, f32x2 p1, f32x2 p2) {
  f32x2 v1 = normalize(p1 - p0);
  f32x2 v2 = normalize(p2 - p0);
  f32x2 v3 = normalize(p2 - p1);
  f32x2 v4 = normalize(p0 - p1);
  f32x3 line1, line2;
  line1.set_swizzle<0, 1>((v1 + v2) / 2.f);
  line1.set_swizzle<0, 1>(f32x2(-line1.data[1], line1.data[0]));
  line1.data[2] = -dot(line1.swizzle<0, 1>(), p0);
  line2.set_swizzle<0, 1>((v3 + v4) / 2.f);
  line2.set_swizzle<0, 1>(f32x2(-line2.data[1], line2.data[0]));
  line2.data[2] = -dot(line2.swizzle<0, 1>(), p1);
  f32x3 l12 = cross(line1, line2);
  return l12.swizzle<0, 1>() / l12.data[2];
}
f32x2 circumcenter(Context &__ctx, f32x2 p0, f32x2 p1, f32x2 p2) {
  f32x3 line1, line2;
  line1.set_swizzle<0, 1>(p1 - p0);
  line1.data[2] = -dot(line1.swizzle<0, 1>(), (p0 + p1) / 2.f);
  line2.set_swizzle<0, 1>(p2 - p1);
  line2.data[2] = -dot(line2.swizzle<0, 1>(), (p1 + p2) / 2.f);
  f32x3 m1 = cross(line1, line2);
  m1 /= m1.data[2];
  return m1.swizzle<0, 1>();
}
f32x2 compute_best_node(Context &__ctx, f32x2 p0, f32x2 p1, f32x2 p2) {
  f32x2 m1 = incenter(__ctx, p0, p1, p2);
  f32x2 m2 = circumcenter(__ctx, p0, m1, p2);
  f32x2 a2 = p0 - 2.f * p1 + p2;
  f32x2 a1 = 2.f * (p1 - p0);
  f32x2 a0 = p0 - m2;
  float b3 = dot(a2, a2);
  float b2 = 2.f * dot(a2, a1);
  float b1 = 2.f * dot(a2, a0) + dot(a1, a1);
  float c2 = b3;
  float c1 = b2 + b3;
  float c0 = b1 + b2 + b3;
  float discriminant = c1 * c1 - 4.f * c2 * c0;
  if (discriminant < 0.f) {
    return m1;
  }
  float disc_sq = sqrt(discriminant);
  float root1 = (-c1 - disc_sq) / (2.f * c2);
  float root2 = (-c1 + disc_sq) / (2.f * c2);
  if (root1 > 0.f && root1 < 1.f) {
    return parametric_quadratic_bezier(__ctx, root1, p0, p1, p2);
  } else if (root2 > 0.f && root2 < 1.f) {
    return parametric_quadratic_bezier(__ctx, root2, p0, p1, p2);
  } else {
    return m1;
  }
}
f32x2 compute_arc_midpoint(Context &__ctx, f32x2 p0, f32x2 p1, f32x2 p2) {
  f32x3 line1, line2;
  line1.set_swizzle<0, 1>(p1 - p0);
  line1.data[2] = -dot(line1.swizzle<0, 1>(), p0);
  line2.set_swizzle<0, 1>(p2 - p0);
  line2.data[2] = -dot(line2.swizzle<0, 1>(), (p0 + p2) / 2.f);
  f32x3 l12 = cross(line1, line2);
  return l12.swizzle<0, 1>() / l12.data[2];
}
float arc_dis(Context &__ctx, f32x2 uv, f32x2 p0, f32x2 p1, f32x2 m0,
              float r0) {
  f32x2 m1 = normalize((p0 + p1) / 2.f - m0);
  if (dot(m1, normalize(uv - m0)) > dot(m1, normalize(p0 - m0))) {
    return abs(distance(uv, m0) - r0);
  } else {
    return min(distance(uv, p0), distance(uv, p1));
  }
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float border = 2.f / iResolution.data[0];
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  uv -= .5f;
  uv.data[1] *= iResolution.data[1] / iResolution.data[0];
  f32x2 mouse = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  mouse -= .5f;
  mouse.data[1] *= iResolution.data[1] / iResolution.data[0];
  border *= zoom;
  uv *= zoom;
  mouse *= zoom;
  f32x2 p0 = f32x2(-.2f, -.2f);
  f32x2 p1 = mouse;
  f32x2 p2 = f32x2(.2f, -.2f);
  if (max(iMouse.data[0], iMouse.data[1]) < 20.f) {
    p1 = f32x2(-.26f, .1f) + f32x2(.2f * sin(iTime), .08f * cos(iTime));
  }
  f32x2 p3 = compute_best_node(__ctx, p0, p1, p2);
  f32x2 m1 = compute_arc_midpoint(__ctx, p0, p1, p3);
  float r1 = distance(m1, p0);
  f32x2 m2 = compute_arc_midpoint(__ctx, p2, p1, p3);
  float r2 = distance(m2, p2);
  f32x3 color = bg_col;
  float dis = 1e38f;
  dis = segment_dis(__ctx, uv, p0, p1) - line_width;
  dis = min(dis, segment_dis(__ctx, uv, p1, p2) - line_width);
  color = mix(line_col, color, smoothstep(-border / 2.f, border / 2.f, dis));
  dis = quadratic_bezier_dis_approx(__ctx, uv, p0, p1, p2) - line_width;
  color = mix(bezier_col, color, smoothstep(-border / 2.f, border / 2.f, dis));
  dis = arc_dis(__ctx, uv, p0, p3, m1, r1) - line_width;
  dis = min(dis, arc_dis(__ctx, uv, p2, p3, m2, r2) - line_width);
  color = mix(arc_col, color, smoothstep(-border / 2.f, border / 2.f, dis));
  dis = distance(uv, p0) - dot_size;
  dis = min(dis, distance(uv, p1) - dot_size);
  dis = min(dis, distance(uv, p2) - dot_size);
  color = mix(dot_col, color, smoothstep(-border / 2.f, border / 2.f, dis));
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
