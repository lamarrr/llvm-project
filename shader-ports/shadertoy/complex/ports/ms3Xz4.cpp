// Port of https://www.shadertoy.com/view/ms3Xz4; author: ford442.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ms3Xz4.json.
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
const float err = 0.001f;
const float sta = 0.0f;
const float end = 7000.0f;
const int mxmrch = 255;
float spheresdf(Context &__ctx, f32x3 p, float r) { return length(p) - r; }
float rep(Context &__ctx, f32x3 p, f32x3 c) {
  f32x3 q = mod(p, c) - 0.5f * c;
  return spheresdf(__ctx, q, max(50.0f * sin(iTime * 5.0f), 30.0f));
}
float scenesdf(Context &__ctx, f32x3 p) {
  f32x3 damn = f32x3(200.0f, 200.0f, 200.0f);
  return rep(__ctx, p, damn);
}
f32x3 raydir(Context &__ctx, float fov, f32x2 res, f32x2 pos) {
  f32x2 dir = pos - res / 2.0f;
  float depth = (res.data[1] / 2.0f) / tan(radians(fov / 2.0f));
  return normalize(f32x3(dir, -depth));
}
f32x4x4 viewmat(Context &__ctx, f32x3 pos, f32x3 centerdir, f32x3 roll) {
  f32x3 f = normalize(centerdir - pos);
  f32x3 s = normalize(cross(f, roll));
  f32x3 u = cross(s, f);
  return f32x4x4(f32x4(s, 0.0f), f32x4(u, 0.0f), f32x4(-f, 0.0f),
                 f32x4(0.0f, 0.0f, 0.0f, 1));
}
f32x3 normal(Context &__ctx, f32x3 p) {
  return normalize(f32x3(
      scenesdf(__ctx, f32x3(p.data[0] + 1.5f + sin(iTime) * 0.1f, p.data[1],
                            p.data[2])) -
          scenesdf(__ctx, f32x3(p.data[0] - 4.1f, p.data[1], p.data[2])),
      scenesdf(__ctx, f32x3(p.data[0], p.data[1] + 4.1f, p.data[2])) -
          scenesdf(__ctx, f32x3(p.data[0], p.data[1] - 1.7f + sin(iTime) * 0.1f,
                                p.data[2])),
      scenesdf(__ctx, f32x3(p.data[0], p.data[1],
                            p.data[2] + 9.7f + sin(iTime) * 0.1f)) -
          scenesdf(__ctx, f32x3(p.data[0], p.data[1], p.data[2] - 8.1f))));
}
float dirlength(Context &__ctx, f32x3 p, f32x3 raydir) {
  float depth = 2.0f;
  for (int i = 0; i < mxmrch; i++) {
    float dist = scenesdf(__ctx, p + raydir * depth);
    if (dist < err) {
      return depth;
    }
    depth += dist;
    if (depth >= end) {
      return end;
    }
  }
  return end;
}
float Minresetnor(Context &__ctx, f32x3 lightpos, f32x3 normalo, f32x3 eye,
                  f32x3 rgb, float ext, float strongness) {
  f32x3 dir = normalo;
  f32x3 point = eye;
  f32x3 altp = dir * dirlength(__ctx, eye, normalo);
  float altl = dirlength(__ctx, eye, normalo);
  float altl2 = altl;
  for (int i = 0; i < 2; i++) {
    altp = dir * altl;
    dir = reflect(dir, normal(__ctx, altp));
    altl = dirlength(__ctx, altp, dir);
    altl2 = altl2 + altl;
  }
  return altl2;
}
f32x3 Minresetnorr(Context &__ctx, f32x3 lightpos, f32x3 normalo, f32x3 eye,
                   f32x3 rgb, float ext, float strongness) {
  f32x3 dir = normalo;
  f32x3 point = eye;
  f32x3 altp = dir * dirlength(__ctx, eye, normalo);
  float altl = dirlength(__ctx, eye, normalo);
  float altl2 = altl;
  for (int i = 0; i < 2; i++) {
    altp = dir * altl;
    dir = reflect(dir, normal(__ctx, altp));
    altl = dirlength(__ctx, altp, dir);
    altl2 = altl2 + altl;
  }
  return altp;
}
f32x3 PCFL(Context &__ctx, f32x3 k_d, f32x3 k_s, float alpha, f32x3 p,
           f32x3 eye, f32x3 lightPos, f32x3 lightITs, f32x3 ffff) {
  f32x3 N = ffff;
  f32x3 L = normalize(lightPos - p);
  f32x3 V = normalize(eye - p);
  f32x3 R = normalize(reflect(-L, N));
  float dotLN = dot(L, N);
  float dotRV = dot(R, V);
  if (dotLN < 0.0f) {
    return f32x3(0.0f, 0.0f, 0.0f);
  }
  if (dotRV < 0.0f) {
    return lightITs * (k_d * dotLN);
  }
  return lightITs * (k_d * dotLN + k_s * pow(dotRV, alpha));
}
f32x3 POI(Context &__ctx, f32x3 k_a, f32x3 k_d, f32x3 k_s, float alpha, f32x3 p,
          f32x3 eye, f32x3 ffff) {
  const f32x3 ambientLight = 0.5f * f32x3(1.0f, 1.0f, 1.0f);
  f32x3 color = ambientLight * k_a;
  f32x3 light1Pos = f32x3(4.0f * sin(iTime), 2.0f, 4.0f * cos(iTime));
  f32x3 light1ITs = f32x3(0.4f, 0.4f, 0.4f);
  color += PCFL(__ctx, k_d, k_s, alpha, p, eye, light1Pos, light1ITs, ffff);
  f32x3 light2Pos =
      f32x3(2.0f * sin(0.37f * iTime), 2.0f * cos(0.37f * iTime), 2.0f);
  f32x3 light2ITs = f32x3(0.4f, 0.4f, 0.4f);
  color += PCFL(__ctx, k_d, k_s, alpha, p, eye, light2Pos, light2ITs, ffff);
  return color;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x4 background = f32x4(0.5f, 0.5f, 0.5f, 1.0f);
  f32x3 viewDir = raydir(__ctx, 90.0f, iResolution.swizzle<0, 1>(),
                         fragCoord.swizzle<0, 1>());
  f32x3 pointt = f32x3(700.0f * sin(iTime), 400.0f * cos(iTime),
                       mod(iTime * 400.0f, sin(iTime * 400.0f)));
  f32x4x4 viewToWorld =
      viewmat(__ctx, pointt, f32x3(0.0f),
              normalize(f32x3(sin(mod(iTime * 5.0f, 5.0f)), cos(iTime * 0.5f),
                              cos(iTime))));
  f32x3 worldDir = (viewToWorld * f32x4(viewDir, 0.0f)).swizzle<0, 1, 2>();
  float dist = dirlength(__ctx, pointt, worldDir);
  f32x3 eye = (pointt + dist * worldDir);
  f32x3 ambient = f32x3(1.0f, 1.0f, 1.0f);
  f32x3 hellr = Minresetnorr(__ctx, f32x3(0.0f), worldDir, pointt,
                             f32x3(0.0f, 0.0f, 1.0f), 10.0f, 0.5f);
  f32x3 K_a = f32x3(0.2f, 0.2f, 0.2f);
  f32x3 K_d = f32x3(0.7f, 0.2f, 0.2f);
  f32x3 K_s = f32x3(1.0f, 1.0f, 1.0f);
  float shininess = 10.0f;
  f32x3 p = pointt + dist * worldDir;
  f32x3 normcolor = normal(__ctx, hellr);
  f32x3 color = POI(__ctx, K_a, K_d, K_s, shininess, p, pointt, normcolor);
  float hell = Minresetnor(__ctx, f32x3(0.0f), worldDir, eye,
                           f32x3(0.0f, 0.0f, 1.0f), 10.0f, 0.5f);
  float minphase = clamp(
      abs(1.1f * sin(iTime * 10.0f) * sin(gl_FragCoord.data[1])), 0.0f, 0.3f);
  fragColor = f32x4(1.0f) - f32x4((hell) / (end)) + f32x4(f32x3(color), 0.0f) -
              f32x4(dist / (end - 40.0f * abs(sin(iTime)))) - f32x4(minphase) +
              f32x4(f32x3(0.2f, 0.2f, 0.5f), 0.0f);
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
