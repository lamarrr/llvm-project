// Port of https://www.shadertoy.com/view/ctdfzs; author: isaacchurchill.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctdfzs.json.
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

const int MAX_MARCHES = 300;
const float RAYMARCH_THRESHOLD = 0.001f;
const float NORMAL_EPSILON = 0.001f;
f32x2x2 rot_xy(Context &__ctx, float theta) {
  return f32x2x2(cos(theta), sin(theta), -sin(theta), cos(theta));
}
f32x3 palette(Context &__ctx, float iter) {
  return 0.5f + 0.5f * sin(iter + f32x3(0, 1, 2));
}
f32x4 union_de(Context &__ctx, f32x4 a, f32x4 b) {
  if (b.data[3] < a.data[3]) {
    return b;
  } else {
    return a;
  }
}
float box3d_de(Context &__ctx, f32x3 p, f32x3 sizes) {
  f32x3 temp = abs(p) - sizes;
  return length(max(temp, 0.0f)) +
         min(max(max(temp.data[0], temp.data[1]), temp.data[2]), 0.0f);
}
float fractal_de(Context &__ctx, f32x3 p, float &iter) {
  float scale = 2.0f;
  float max_i = floor(mod(iTime + 5.0f, 5.99f)) + 1.0f;
  float s = 1.0f;
  float d = 1e6f;
  const float width = 1.0f;
  const float thickness = 0.05f;
  const float r = thickness / 2.0f;
  const float a = (width - thickness) / 2.0f;
  for (float i = 0.0f; i < max_i; i++) {
    f32x3 b = abs(p);
    p.set_swizzle<0, 2>(b.swizzle<0, 2>());
    if (b.data[0] < b.data[1]) {
      b.set_swizzle<0, 1>(b.swizzle<1, 0>());
    }
    if (b.data[1] < b.data[2]) {
      b.set_swizzle<1, 2>(b.swizzle<2, 1>());
    }
    if (b.data[0] < b.data[2]) {
      b.set_swizzle<0, 2>(b.swizzle<2, 0>());
    }
    float d1 = box3d_de(__ctx, b - f32x3(a, a, width / 4.0f),
                        f32x3(r, r, width / 4.0f)) *
               s;
    if (d1 < d) {
      iter = i;
    }
    d = min(d, d1);
    p -= f32x3(0.5f, -0.3f, 0.5f);
    p *= scale;
    s /= scale;
  }
  return d;
}
float dist_estim(Context &__ctx, f32x3 p, f32x3 &col) {
  float iter = 0.0f;
  float scale = 0.4f;
  float fractal = fractal_de(__ctx, p / scale, iter) * scale;
  f32x3 plane_col = f32x3(0.5f);
  if ((mod(p.data[0], 1.0f) > 0.5f) != (mod(p.data[2], 1.0f) > 0.5f)) {
    plane_col = f32x3(1.0f);
  }
  f32x4 de = f32x4(palette(__ctx, iter), fractal);
  de = union_de(__ctx, de, f32x4(plane_col, abs(p.data[1] + 0.25f)));
  col = de.swizzle<0, 1, 2>();
  return de.data[3];
}
f32x3 calc_normal(Context &__ctx, f32x3 p) {
  const f32x2 eps = f32x2(0.0f, NORMAL_EPSILON);
  f32x3 temp = f32x3(0.0f);
  return normalize(f32x3(dist_estim(__ctx, p + eps.swizzle<1, 0, 0>(), temp),
                         dist_estim(__ctx, p + eps.swizzle<0, 1, 0>(), temp),
                         dist_estim(__ctx, p + eps.swizzle<0, 0, 1>(), temp)) -
                   dist_estim(__ctx, p, temp));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  uv = uv * 2.0f - 1.0f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 col = f32x3(0.0f);
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  if (m.data[0] < 0.01f && m.data[1] < 0.01f) {
    m = f32x2(0.4f, 0.4f);
  }
  f32x2 rot = (m - 0.5f) * 4.0f;
  f32x3 ray_dir = normalize(f32x3(uv, 2.0f));
  f32x2x2 yz_mat = rot_xy(__ctx, rot.data[1]);
  f32x2x2 xz_mat = rot_xy(__ctx, rot.data[0]);
  ray_dir.set_swizzle<1, 2>(ray_dir.swizzle<1, 2>() * (yz_mat));
  ray_dir.set_swizzle<0, 2>(ray_dir.swizzle<0, 2>() * (xz_mat));
  f32x3 ray_endpoint = f32x3(0.0f, 0.0f, -1.0f);
  ray_endpoint.set_swizzle<1, 2>(ray_endpoint.swizzle<1, 2>() * (yz_mat));
  ray_endpoint.set_swizzle<0, 2>(ray_endpoint.swizzle<0, 2>() * (xz_mat));
  f32x3 light_dir = normalize(f32x3(0.3f, 1.0f, -0.5f));
  for (int march = 0; march < MAX_MARCHES; march++) {
    f32x3 surface_col = f32x3(0.0f);
    float dist = dist_estim(__ctx, ray_endpoint, surface_col);
    if (dist < RAYMARCH_THRESHOLD) {
      f32x3 norm = calc_normal(__ctx, ray_endpoint);
      float intensity = max(dot(norm, light_dir), 0.0f);
      f32x3 light_col = f32x3(0.9f);
      f32x3 ambient = f32x3(0.1f);
      col = surface_col * (light_col * intensity + ambient);
      break;
    }
    ray_endpoint += ray_dir * dist * 0.5f;
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
