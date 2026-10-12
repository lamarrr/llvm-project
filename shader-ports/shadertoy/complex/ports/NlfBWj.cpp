// Port of https://www.shadertoy.com/view/NlfBWj; author: Spi3lot.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NlfBWj.json.
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
float sdSphere(Context &__ctx, f32x3 p, float r) { return length(p) - r; }
float sdClosestCube(Context &__ctx, f32x3 p) {
  f32x3 distances = abs(p - round(p));
  return min(min(distances.data[0], distances.data[1]), distances.data[2]);
}
f32x3 GetNormal(Context &__ctx, f32x3 dpdx, f32x3 dpdy) {
  return normalize(cross(dpdx, dpdy));
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.0f;
  bool touchedSphere = false;
  for (int i = 0; i < 250; i++) {
    f32x3 p = ro + rd * dO;
    float dS;
    if (touchedSphere) {
      dS = -sdClosestCube(__ctx, p);
    } else {
      dS = sdSphere(__ctx, trunc(p), iTime);
      if (dS <= 0.001f) {
        touchedSphere = true;
        dS = -sdClosestCube(__ctx, p);
      }
    }
    dO += dS;
    if (abs(dS) <= 0.001f || dO > 2.0f * (2.0f + iTime * 2.0f))
      break;
  }
  return dO;
}
f32x4 intersection(Context &__ctx, f32x2 fragCoord) {
  f32x2 uv = (fragCoord - iResolution.swizzle<0, 1>() * 0.5f) /
             min(iResolution.data[0], iResolution.data[1]);
  f32x3 ro = f32x3((2.0f + iTime * 2.0f));
  f32x3 lookAt = f32x3(0, 0, 0), f = normalize(lookAt - ro),
        r = normalize(cross(f, f32x3(0, 1, 0))), u = cross(r, f),
        c = ro + f * 1.25f, i = c + uv.data[0] * r + uv.data[1] * u,
        rd = normalize(i - ro);
  float d = RayMarch(__ctx, ro, rd);
  f32x3 p = ro + rd * d;
  return f32x4(p, d);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x4 p = intersection(__ctx, fragCoord);
  f32x4 px = intersection(__ctx, fragCoord + f32x2(1, 0));
  f32x4 py = intersection(__ctx, fragCoord + f32x2(0, 1));
  fragColor = f32x4(0.5f, 0.5f, 0.5f, 1.0f);
  if (p.data[3] <= 2.0f * (2.0f + iTime * 2.0f) &&
      px.data[3] <= 2.0f * (2.0f + iTime * 2.0f) &&
      py.data[3] <= 2.0f * (2.0f + iTime * 2.0f)) {
    f32x3 dpdx = px.swizzle<0, 1, 2>() - p.swizzle<0, 1, 2>();
    f32x3 dpdy = py.swizzle<0, 1, 2>() - p.swizzle<0, 1, 2>();
    f32x3 n = GetNormal(__ctx, dpdx, dpdy);
    fragColor = f32x4(abs(n), 1.0f);
  }
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
