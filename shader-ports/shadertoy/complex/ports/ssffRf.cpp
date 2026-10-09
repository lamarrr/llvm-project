// Port of https://www.shadertoy.com/view/ssffRf; author: codebender.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ssffRf.json.
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
float random(Context &__ctx, f32x2 st) {
  return fract(sin(dot(st.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
float random(Context &__ctx, f32x3 st) {
  return fract(
      sin(dot(st.swizzle<0, 1, 2>(), f32x3(12.9898f, 78.233f, 15.234f))) *
      43758.5453123f);
}
f32x4 permute(Context &__ctx, f32x4 x) {
  return mod(((x * 34.0f) + 1.0f) * x, 289.0f);
}
f32x4 taylorInvSqrt(Context &__ctx, f32x4 r) {
  return 1.79284291400159f - 0.85373472095314f * r;
}
f32x3 fade(Context &__ctx, f32x3 t) {
  return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}
float cnoise(Context &__ctx, f32x3 P) {
  f32x3 Pi0 = floor(P);
  f32x3 Pi1 = Pi0 + f32x3(1.0f);
  Pi0 = mod(Pi0, 289.0f);
  Pi1 = mod(Pi1, 289.0f);
  f32x3 Pf0 = fract(P);
  f32x3 Pf1 = Pf0 - f32x3(1.0f);
  f32x4 ix = f32x4(Pi0.data[0], Pi1.data[0], Pi0.data[0], Pi1.data[0]);
  f32x4 iy = f32x4(Pi0.swizzle<1, 1>(), Pi1.swizzle<1, 1>());
  f32x4 iz0 = Pi0.swizzle<2, 2, 2, 2>();
  f32x4 iz1 = Pi1.swizzle<2, 2, 2, 2>();
  f32x4 ixy = permute(__ctx, permute(__ctx, ix) + iy);
  f32x4 ixy0 = permute(__ctx, ixy + iz0);
  f32x4 ixy1 = permute(__ctx, ixy + iz1);
  f32x4 gx0 = ixy0 / 7.0f;
  f32x4 gy0 = fract(floor(gx0) / 7.0f) - 0.5f;
  gx0 = fract(gx0);
  f32x4 gz0 = f32x4(0.5f) - abs(gx0) - abs(gy0);
  f32x4 sz0 = step(gz0, f32x4(0.0f));
  gx0 -= sz0 * (step(0.0f, gx0) - 0.5f);
  gy0 -= sz0 * (step(0.0f, gy0) - 0.5f);
  f32x4 gx1 = ixy1 / 7.0f;
  f32x4 gy1 = fract(floor(gx1) / 7.0f) - 0.5f;
  gx1 = fract(gx1);
  f32x4 gz1 = f32x4(0.5f) - abs(gx1) - abs(gy1);
  f32x4 sz1 = step(gz1, f32x4(0.0f));
  gx1 -= sz1 * (step(0.0f, gx1) - 0.5f);
  gy1 -= sz1 * (step(0.0f, gy1) - 0.5f);
  f32x3 g000 = f32x3(gx0.data[0], gy0.data[0], gz0.data[0]);
  f32x3 g100 = f32x3(gx0.data[1], gy0.data[1], gz0.data[1]);
  f32x3 g010 = f32x3(gx0.data[2], gy0.data[2], gz0.data[2]);
  f32x3 g110 = f32x3(gx0.data[3], gy0.data[3], gz0.data[3]);
  f32x3 g001 = f32x3(gx1.data[0], gy1.data[0], gz1.data[0]);
  f32x3 g101 = f32x3(gx1.data[1], gy1.data[1], gz1.data[1]);
  f32x3 g011 = f32x3(gx1.data[2], gy1.data[2], gz1.data[2]);
  f32x3 g111 = f32x3(gx1.data[3], gy1.data[3], gz1.data[3]);
  f32x4 norm0 = taylorInvSqrt(__ctx, f32x4(dot(g000, g000), dot(g010, g010),
                                           dot(g100, g100), dot(g110, g110)));
  g000 *= norm0.data[0];
  g010 *= norm0.data[1];
  g100 *= norm0.data[2];
  g110 *= norm0.data[3];
  f32x4 norm1 = taylorInvSqrt(__ctx, f32x4(dot(g001, g001), dot(g011, g011),
                                           dot(g101, g101), dot(g111, g111)));
  g001 *= norm1.data[0];
  g011 *= norm1.data[1];
  g101 *= norm1.data[2];
  g111 *= norm1.data[3];
  float n000 = dot(g000, Pf0);
  float n100 = dot(g100, f32x3(Pf1.data[0], Pf0.swizzle<1, 2>()));
  float n010 = dot(g010, f32x3(Pf0.data[0], Pf1.data[1], Pf0.data[2]));
  float n110 = dot(g110, f32x3(Pf1.swizzle<0, 1>(), Pf0.data[2]));
  float n001 = dot(g001, f32x3(Pf0.swizzle<0, 1>(), Pf1.data[2]));
  float n101 = dot(g101, f32x3(Pf1.data[0], Pf0.data[1], Pf1.data[2]));
  float n011 = dot(g011, f32x3(Pf0.data[0], Pf1.swizzle<1, 2>()));
  float n111 = dot(g111, Pf1);
  f32x3 fade_xyz = fade(__ctx, Pf0);
  f32x4 n_z = mix(f32x4(n000, n100, n010, n110), f32x4(n001, n101, n011, n111),
                  fade_xyz.data[2]);
  f32x2 n_yz = mix(n_z.swizzle<0, 1>(), n_z.swizzle<2, 3>(), fade_xyz.data[1]);
  float n_xyz = mix(n_yz.data[0], n_yz.data[1], fade_xyz.data[0]);
  return 2.2f * n_xyz;
}
f32x3 rotateVector(Context &__ctx, f32x3 p, float q, float q2) {
  p = f32x3(p.data[0] * cos(q) - p.data[1] * sin(q),
            p.data[0] * sin(q) + p.data[1] * cos(q), p.data[2]);
  p = f32x3(p.data[0] * cos(q2) + p.data[2] * sin(q2), p.data[1],
            -p.data[0] * sin(q2) + p.data[2] * cos(q2));
  return p;
}
float sdSphere(Context &__ctx, f32x3 p, float s, f32x3 pos) {
  p -= pos;
  return length(p) - s;
}
float map(Context &__ctx, f32x3 p) {
  float d = sdSphere(__ctx, mod(p, 1.f), .15f,
                     f32x3(0.5f,
                           0.5f + sin(floor(p.data[2] / 1.f) +
                                      floor(p.data[0] / 1.f) + iTime * 10.f) *
                                      .05f,
                           0.5f));
  return d;
}
f32x3 calcNormal(Context &__ctx, f32x3 p) {
  float eps = 0.01f;
  return normalize(f32x3(
      map(__ctx, p + f32x3(eps, 0, 0)) - map(__ctx, p + f32x3(-eps, 0, 0)),
      map(__ctx, p + f32x3(0, eps, 0)) - map(__ctx, p + f32x3(0, -eps, 0)),
      map(__ctx, p + f32x3(0, 0, eps)) - map(__ctx, p + f32x3(0, 0, -eps))));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 point_light_pos = f32x3(1.f, 1.f, 1.f);
  f32x3 cam_pos = f32x3(iTime * 2.f, iTime, iTime * 5.f);
  f32x3 ray_dir = normalize(
      f32x3(fragCoord.data[0] / iResolution.data[1] -
                iResolution.data[0] / iResolution.data[1] / 2.f,
            fragCoord.data[1] / iResolution.data[1] - 1.f / 2.f, 1.f));
  ray_dir =
      rotateVector(__ctx, ray_dir, sin(iTime) * .5f, sin(iTime * .35f) * .5f);
  float closest_distance = 10000.f;
  bool hit = false;
  f32x3 hit_pos = cam_pos;
  float t = 0.f;
  for (int i = 0; i < 60; i += 1) {
    t = map(__ctx, hit_pos);
    hit_pos += ray_dir * t;
    if (t < .001f) {
      hit = true;
      break;
    }
    closest_distance = min(t, closest_distance);
  }
  float light_power =
      (dot(calcNormal(__ctx, hit_pos), normalize(point_light_pos - hit_pos)) +
       1.f) /
      2.f;
  float depth = distance(hit_pos, cam_pos);
  float vignette = (1.f - abs(fragCoord.data[0] - iResolution.data[0] / 2.f) /
                              iResolution.data[0]) *
                   (1.f - abs(fragCoord.data[1] - iResolution.data[1] / 2.f) /
                              iResolution.data[1]);
  if (hit) {
    fragColor =
        smoothstep(1.f, vignette / 40.f, depth / 10.f) * vignette *
        f32x4(random(__ctx, floor(hit_pos / 1.f)),
              random(__ctx, floor(hit_pos / 1.f) + f32x3(1.f, 1.f, 1.f)),
              random(__ctx, floor(hit_pos / 1.f) + f32x3(5.f, 5.f, 5.f)), 1.f);
  } else {
    fragColor = vignette / 40.f * f32x4(1.f);
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
