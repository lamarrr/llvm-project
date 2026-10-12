// Port of https://www.shadertoy.com/view/ml2GR3; author: Hodisfut.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ml2GR3.json.
#include "../support.h"
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
f32x3 rotate(Context &__ctx, f32x3 v, f32x3 axis, float angle) {
  f32x3 res = v;
  res = res - dot(res, axis) * axis;
  res = res * cos(angle) + cross(axis, res) * sin(angle) +
        axis * dot(axis, res) * (1.0f - cos(angle));
  return res;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p = (fragCoord.swizzle<0, 1>() / iResolution.data[1]) * 2.0f - 1.0f;
  f32x3 r = f32x3(p, 0);
  f32x3 ro = f32x3(0, 0, 0);
  f32x3 ta = f32x3(0, 0, 0);
  float d = 0.0f;
  f32x3 c = f32x3(0);
  float an = iTime * 2.f;
  f32x2x2 m2 = f32x2x2(cos(an), -sin(an), sin(an), cos(an));
  f32x3x3 m3 = f32x3x3(cos(an), -sin(an), 0, sin(an), cos(an), 0, 0, 0, 1);
  float random_rotation = fract(sin(iTime) * 12.9898f);
  f32x3 random_axis =
      f32x3(fract(sin(iTime) * 43758.5453f), fract(cos(iTime) * 4376.5453f),
            fract(sin(iTime + 1.0f) * 43758.5453f));
  r = rotate(__ctx, r, normalize(random_axis),
             random_rotation * 3.14159265358979323846f * 2.f);
  for (int i = 0; i < 3; i++) {
    r = m3 * r;
    d = dot(r, r);
    if (d < 0.05f) {
      c = f32x3(0, 1, 0);
      break;
    }
  }
  f32x3 light = normalize(f32x3(0, 1, 1));
  float diffuse = dot(normalize(r), light);
  diffuse = pow(diffuse, 2.f);
  fragColor = f32x4(c * diffuse, 1);
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
