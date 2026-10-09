// Port of https://www.shadertoy.com/view/7tBfzz; author: zesterer.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7tBfzz.json.
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
struct Context {
  Texture2D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
const float PLANCK = 0.01f;
f32x4 sdf_union(Context &__ctx, f32x4 a, f32x4 b) {
  return a.data[3] < b.data[3] ? a : b;
}
float sdf(Context &__ctx, f32x3 pos) {
  float height =
      (texture(__ctx.channel1, pos.swizzle<0, 1>() * 0.002f).data[0] +
       texture(__ctx.channel1, pos.swizzle<0, 1>() * 0.02f).data[0] * 0.1f +
       texture(__ctx.channel1, pos.swizzle<0, 1>() * 0.1f).data[0] * 0.03f) *
      30.0f;
  float ao = min(
      pow(texture(__ctx.channel1, pos.swizzle<0, 1>() * 0.02f).data[0] * 2.0f,
          1.5f),
      1.0f);
  return pos.data[2] - height;
}
f32x3 sdf_col(Context &__ctx, f32x3 pos, f32x3 norm) {
  float ao = min(
      texture(__ctx.channel1, pos.swizzle<0, 1>() * 0.1f).data[0] * 2.0f, 1.0f);
  return ao * mix(f32x3(1.0f, 0.5f, 0.2f), f32x3(0, 1, 0),
                  clamp((-norm.data[2] - 0.5f) * 2.0f, 0.0f, 1.0f));
}
f32x3 sdf_norm(Context &__ctx, f32x3 pos) {
  const float NORM_PLANCK = PLANCK * 5.0f;
  float d000 = sdf(__ctx, pos);
  float d100 = sdf(__ctx, pos + f32x3(NORM_PLANCK, 0.0f, 0.0f));
  float d010 = sdf(__ctx, pos + f32x3(0.0f, NORM_PLANCK, 0.0f));
  float d001 = sdf(__ctx, pos + f32x3(0.0f, 0.0f, NORM_PLANCK));
  return normalize(d000 - f32x3(d100, d010, d001));
}
const int MAX_ITER = 256;
float march(Context &__ctx, f32x3 pos, f32x3 dir, bool &hit, int &i,
            float &min_d) {
  const float MIN_JUMP = PLANCK * 10.0f;
  const float MIN_JUMP_FACTOR = 0.003f;
  min_d = 100000.0f;
  float min_t = 0.0f;
  float t = 0.0f;
  for (i = 0; i < MAX_ITER; i++) {
    float d = sdf(__ctx, pos + dir * t);
    if (abs(d) < PLANCK * t * 0.1f) {
      hit = true;
      return t;
    } else if (d < min_d) {
      min_d = d;
      min_t = t;
    } else if (t > 2000.0f) {
      hit = false;
      return 10000.0f;
    }
    t += d * (0.4f + t * MIN_JUMP_FACTOR + MIN_JUMP);
  }
  hit = true;
  return min_t;
}
f32x3 ray_dir(Context &__ctx, f32x3 z_up, f32x3 dir, float fov, float aspect,
              f32x2 uv) {
  f32x3 right = normalize(cross(z_up, dir));
  f32x3 up = cross(dir, right);
  return normalize(dir + right * uv.data[0] * aspect * fov +
                   up * uv.data[1] * fov);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>() * 2.0f - 1.0f;
  f32x3 origin = f32x3(0, 0, 25);
  f32x2 angle = (iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>() * 2.0f -
                 f32x2(1.0f, 0.5f)) *
                    f32x2(-5, -2.5f) -
                f32x2(0, 5);
  f32x2 ldir = f32x2(sin(angle.data[0]), cos(angle.data[0]));
  f32x3 cpos =
      f32x3(ldir * max(cos(angle.data[1]), 0.01f), sin(angle.data[1])) * 50.0f +
      origin;
  if (!false) {
    cpos.set_swizzle<0, 1>(f32x2(sin(iTime), cos(iTime * 1.5f)) * 50.0f +
                           20.0f);
  }
  f32x3 cdir = normalize(origin - cpos);
  f32x3 dir = ray_dir(__ctx, f32x3(0, 0, 1), cdir, 0.8f, 1.5f, uv);
  float sun_angle = iTime * 0.2f * 0.0f + 3.3f;
  f32x3 sun_dir = normalize(f32x3(0, cos(sun_angle), sin(sun_angle)));
  f32x3 sky = f32x3(0.5f, 0.5f, 1.0f);
  bool hit;
  int n;
  float min_d;
  float t = march(__ctx, cpos, dir, hit, n, min_d);
  f32x3 pos = cpos + dir * t;
  f32x3 col = sky;
  if (hit) {
    f32x3 norm = sdf_norm(__ctx, pos);
    f32x3 surf = sdf_col(__ctx, pos, norm);
    float specular = pow(max(0.0f, dot(reflect(dir, norm), -sun_dir)), 5.0f);
    bool _hit;
    int _n;
    float _min_d;
    float shadow = clamp(
        (march(__ctx, pos - norm * 0.3f, -sun_dir, _hit, _n, _min_d) - 100.0f),
        0.0f, 1.0f);
    float occlusion = min(max(dot(sun_dir, norm), 0.0f), shadow);
    col = surf * ((1.0f + specular) * occlusion * 0.85f + 0.15f);
  }
  col = mix(sky, col, min(1.0f / exp(0.002f * t), 1.0f));
  if (false && iMouse.data[2] > 0.0f) {
    col = f32x3(float(n) / float(MAX_ITER), min_d, 0);
  }
  fragColor = f32x4(col, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 1) texture_2d<f32x4> channel1,
               __binding__(1, 1) sampler sampling1) {
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
  Context __ctx{{channel1, sampling1}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
