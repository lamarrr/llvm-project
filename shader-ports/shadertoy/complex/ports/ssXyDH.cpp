// Port of https://www.shadertoy.com/view/ssXyDH; author: bgold.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ssXyDH.json.
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

const float LIGHTSPEED = 4.0f;
const f32x3 CAMPOS = f32x3(0);
const float WALLPOS = 10.0f;
const float FLOORPOS = -0.5f;
const f32x3 LIGHTPOS = f32x3(2.0f, -5.0f, 1.0f);
const f32x3 OBJCEN = f32x3(0, 5.0f, 0);
const float OBJRAD = 0.3f;
const float EPS = 1e-2f;
const f32x3 OBJCOLOR = f32x3(0.929f, 0.831f, 0.000f);
const f32x3 FLOORCOLOR = f32x3(0.9f);
const f32x3 WALLCOLOR = f32x3(0.447f, 1, 1);
f32x3 objpos(Context &__ctx, float t) {
  return OBJCEN + 1.3f * f32x3(cos(t), sin(t), 0);
}
const float R1 = 2.1f;
const float R2 = 0.5f;
f32x3 rotxy(Context &__ctx, f32x3 x, float a) {
  return f32x3x3(cos(a), -sin(a), 0.f, sin(a), cos(a), 0.f, 0.f, 0.f, 1.f) * x;
}
f32x3 rotxz(Context &__ctx, f32x3 x, float a) {
  return f32x3x3(cos(a), 0.f, -sin(a), 0.f, 1.f, 0.f, sin(a), 0.f, cos(a)) * x;
}
float sdf(Context &__ctx, f32x3 r, float t) {
  f32x3 dr = rotxy(__ctx, r - objpos(__ctx, t), R1 * t);
  dr = rotxz(__ctx, dr, R2 * t);
  dr = abs(dr) - OBJRAD;
  float d = max(dr.data[0], dr.data[1]);
  d = max(d, dr.data[2]);
  return d;
}
f32x3 grad(Context &__ctx, f32x3 r, float t) {
  f32x3 n;
  f32x3 dr = rotxy(__ctx, r - objpos(__ctx, t), R1 * t);
  dr = rotxz(__ctx, dr, R2 * t);
  if (abs(dr.data[0]) > abs(dr.data[1]) && abs(dr.data[0]) > abs(dr.data[2])) {
    n = f32x3(sign(dr.data[0]), 0.0f, 0.0f);
  } else if (abs(dr.data[1]) > abs(dr.data[0]) &&
             abs(dr.data[1]) > abs(dr.data[2])) {
    n = f32x3(0.0f, sign(dr.data[1]), 0.0f);
  } else {
    n = f32x3(0.0f, 0.0f, sign(dr.data[2]));
  }
  n = rotxz(__ctx, n, -R2 * t);
  n = rotxy(__ctx, n, -R1 * t);
  return n;
}
float objhit(Context &__ctx, f32x3 p, f32x3 v, float t0) {
  float d;
  float t = iTime - t0;
  float dt;
  for (int i = 0; i < 100; i++) {
    d = 0.7f * sdf(__ctx, p, t);
    d = min(d, WALLPOS - p.data[1]);
    d = min(d, (p.data[2] - FLOORPOS) / max(abs(normalize(v).data[2]), EPS));
    if (d < 0.0f)
      break;
    dt = d / length(v);
    p += v * dt;
    t -= dt;
    if (p.data[1] < -1.0f)
      return 0.0f;
  }
  return (iTime - t0) - t;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord - iResolution.swizzle<0, 1>() * 0.5f) /
             iResolution.data[1] * 2.0f;
  f32x3 rayvel = LIGHTSPEED * normalize(f32x3(uv.data[0], 0.0f, uv.data[1]) -
                                        f32x3(0, -5.0f, 0));
  f32x3 raypos = f32x3(0);
  float cht = objhit(__ctx, raypos, rayvel, 0.0f);
  f32x3 bouncepos = raypos + cht * rayvel;
  f32x3 bouncevel;
  f32x3 col;
  float intensity = clamp(
      pow(distance(LIGHTPOS, bouncepos) / WALLPOS * 1.2f, -2.0f), 0.0f, 1.0f);
  if (bouncepos.data[1] > WALLPOS - EPS || bouncepos.data[2] < FLOORPOS + EPS) {
    col = FLOORCOLOR;
    if (bouncepos.data[1] > WALLPOS - EPS)
      col = WALLCOLOR;
    bouncepos = f32x3(bouncepos.data[0], min(WALLPOS - EPS, bouncepos.data[1]),
                      max(FLOORPOS + EPS, bouncepos.data[2]));
    bouncevel = LIGHTSPEED * normalize(LIGHTPOS - bouncepos);
    float sht = objhit(__ctx, bouncepos, bouncevel, cht);
    if (sht > 0.0f) {
      col *= 0.2f;
    }
  } else {
    float dd = abs(dot(grad(__ctx, bouncepos, iTime - cht),
                       normalize(LIGHTPOS - bouncepos)));
    col = OBJCOLOR * clamp(dd, 0.2f, 1.0f);
  }
  fragColor = f32x4(col * intensity, 1.0f);
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
