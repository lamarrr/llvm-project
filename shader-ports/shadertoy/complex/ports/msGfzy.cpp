// Port of https://www.shadertoy.com/view/msGfzy; author: willwombell.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/msGfzy.json.
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
const f32x4 count_mixer = f32x4(1, 2, 3, 5);
const f32x3 coset_div = f32x3(2.f, 3.f, 5.f);
f32x2 cinv(Context &__ctx, f32x2 z) {
  return f32x2(z.data[0], -z.data[1]) / dot(z, z);
}
f32x2 cmul(Context &__ctx, f32x2 a, f32x2 b) {
  return f32x2(a.data[0] * b.data[0] - a.data[1] * b.data[1],
               a.data[0] * b.data[1] + a.data[1] * b.data[0]);
}
f32x2 cdiv(Context &__ctx, f32x2 a, f32x2 b) {
  return cmul(__ctx, a, cinv(__ctx, b));
}
f32x3 eflip(Context &__ctx, f32x2 p, f32x2 ms) {
  int c = 0;
  int d = 0;
  int e = 0;
  int f = 0;
  bool esc;
  for (int i = 0; i < 5000; i++) {
    if (i > int((ms.data[0])))
      break;
    esc = true;
    if (length(p) < 1.f) {
      p = cinv(__ctx, p);
      c++;
      esc = false;
    }
    if ((p.data[0] > 0.f) && true) {
      p.data[0] *= -1.f;
      d++;
      esc = false;
    }
    if (p.data[0] < -.5f) {
      p.data[0] = -1.f - p.data[0];
      e++;
      esc = false;
    }
    if ((p.data[0] > .5f) && true) {
      p.data[0] = 1.f - p.data[0];
      f++;
      esc = false;
    }
    if (esc)
      break;
  }
  float t = dot(f32x4(c, d, e, f), count_mixer);
  float coset = ms.data[1] * 500.f;
  f32x3 coset_vec = ms.data[1] * coset_div;
  switch (0) {
  case 0:
    return (0.5f + 0.5f * cos(3.14159265f * f32x3(float(t)) / coset_vec));
  case 1:
    return f32x3(0.5f + 0.5f * cos(3.14159265f * float(t) / coset));
  case 2:
    return f32x3(mod(float(t), coset) / float(coset));
  case 3:
    return f32x3(0.5f / float((c + d) % 2), 0.4f / float((d + e * f) % 2),
                 0.6f / float((c * f + f - e) % 3));
  case 4:
    return f32x3(0.5f * cos(ms.data[1]) / float((c + d) % 2),
                 0.4f * cos(3.f * ms.data[1] - 0.2f) / float((d + e * f) % 2),
                 0.6f * cos(2.f * ms.data[1] + 0.3f) /
                     float((c * f + f - e) % 3));
  }
}
void mainImage(Context &__ctx, f32x4 &col, f32x2 coord) {
  f32x2 z =
      5.4f * (coord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 ms = (iMouse.swizzle<0, 1>() - 0.5f * iResolution.swizzle<0, 1>()) /
             iResolution.data[1];
  float t = 0.1f * iTime;
  f32x2x2 rot = f32x2x2(1, 0, 0, 1);
  if (true) {
    rot = f32x2x2(cos(t), sin(t), -sin(t), cos(t));
  }
  if (true) {
    z = z * rot;
    z = cdiv(__ctx, f32x2(0.f, 1.f) - f32x2(z.data[1], -z.data[0]),
             z - f32x2(1.f, 0.f));
  }
  if (true) {
    z.data[0] += cos(iTime);
    z.data[1] += pow(0.4f * sin(iTime * 1.61803398874989f), 2.f);
  }
  if (true) {
    z.data[0] += pow(iTime, 1.f);
  }
  if (true) {
    z.data[1] /= pow(iTime, 2.f);
    z.data[0] /= pow(iTime, 2.f);
  }
  col = f32x4(
      eflip(__ctx, z,
            f32x2(exp(7.f * (-ms.data[0] + 0.5f)), exp(7.f * (ms.data[1])))),
      0.f);
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
