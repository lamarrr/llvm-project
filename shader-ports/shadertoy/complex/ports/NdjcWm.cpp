// Port of https://www.shadertoy.com/view/NdjcWm; author: ersh.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NdjcWm.json.
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
f32x3 col(Context &__ctx, float x, float side) {
  return f32x3(clamp(x / 2.f + 0.5f, 0.f, 1.f), side,
               clamp(0.5f - x / 2.f, 0.f, 1.f));
}
float h(Context &__ctx, f32x2 p, float t) {
  return sin(iTime * 10.f +
             tanh(iTime / 10.f) * 10.f / pow(length(p.swizzle<0, 1>()), 0.5f)) /
         length(p.swizzle<0, 1>());
}
f32x2x2 rot(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, s, -s, c);
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 I) {
  f32x3 D = normalize(f32x3(I + I, -4.f * iResolution.data[1]) -
                      f32x3(iResolution.swizzle<0, 1>(), 0.f));
  f32x3 p = f32x3(0, 0, 7);
  f32x2x2 rot_yz, rot_xy;
  if (iMouse.data[0] < 0.5f) {
    rot_yz =
        rot(__ctx, -3.1415926f * (0.35f - (sin(iTime / 6.f) + 1.f) * 0.06f));
    rot_xy = rot(__ctx, 3.1415926f / 3.f + iTime / 10.f);
  } else {
    f32x2 M = (iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>() *
                   f32x2(1.f, -0.5f) +
               f32x2(0.5f, 0.f)) *
              2.f * 3.1415926f;
    rot_yz = rot(__ctx, M.data[1]);
    rot_xy = rot(__ctx, M.data[0]);
  }
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (rot_yz));
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (rot_xy));
  D.set_swizzle<1, 2>(D.swizzle<1, 2>() * (rot_yz));
  D.set_swizzle<0, 1>(D.swizzle<0, 1>() * (rot_xy));
  float height, diff;
  float heightprev = h(__ctx, (p.swizzle<0, 1>()) * 1.f, iTime) / 5.f,
        diffprev = 0.f;
  f32x3 pprev = p;
  float dist, stp = 0.1f;
  for (dist = 0.f; dist < 10.f; dist += stp, p += stp * D) {
    height = h(__ctx, (p.swizzle<0, 1>()) * 1.f, iTime) / 5.f;
    diff = p.data[2] - height;
    if (diff * diffprev < 0.f) {
      break;
    }
    heightprev = height;
    pprev = p;
    diffprev = diff;
  }
  if (dist < 10.f) {
    float interp = (0.f - diffprev) / (diff - diffprev);
    f32x3 pexact = mix(pprev, p, interp);
    float heightexact = h(__ctx, (pexact.swizzle<0, 1>()) * 1.f, iTime) / 5.f;
    float sgn = sign(diffprev);
    f32x3 c = col(__ctx, heightexact * 2.f, (-sgn + 1.f) / 4.f) *
              (1.f - (1.f - sgn) / 3.f);
    float mesh_thick = 0.015f;
    float mesh_density = 3.f;
    f32x2 mesh =
        max(f32x2(0),
            abs(fract(pexact.swizzle<0, 1>() * mesh_density) - f32x2(0.5f)) -
                f32x2(0.5f * (1.f - 2.f * mesh_thick))) /
        mesh_thick;
    c = mix(c, f32x3(1), max(mesh.data[0], mesh.data[1]));
    f32x2 shade = f32x2(1, 0) * rot_xy;
    float surface_angle = atan(
        (heightexact -
         h(__ctx, ((pexact.swizzle<0, 1>() + shade * 0.001f)) * 1.f, iTime) /
             5.f) /
        0.001f);
    c += sgn * (surface_angle / 3.f);
    O = f32x4(c * (10.f - dist) / 10.f * 2.f, 0.f);
  } else {
    O = f32x4(0);
  }
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
