// Port of https://www.shadertoy.com/view/Dll3RB; author: szuhanlu_karen.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Dll3RB.json.
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
float sdSphere(Context &__ctx, f32x3 p) {
  f32x4 sphere1 = f32x4(50.f, -70.f, 1.f, 28.f);
  f32x4 sphere2 = f32x4(8.f, 20.f, 2.f, 15.f);
  f32x4 sphere3 = f32x4(70.f, 20.f, 10.f, 10.f);
  float dSphere1 = distance(p, sphere1.swizzle<0, 1, 2>()) - sphere1.data[3];
  float dSphere2 = distance(p, sphere2.swizzle<0, 1, 2>()) - sphere2.data[3];
  float dSphere3 = distance(p, sphere3.swizzle<0, 1, 2>()) - sphere3.data[3];
  return min(min(dSphere1, dSphere2), min(min(dSphere1, dSphere2), dSphere3));
}
float plasma(Context &__ctx, f32x3 r) {
  float mx = r.data[0] + iTime / 2.f;
  mx += 60.0f * sin((r.data[1] + mx) / 50.0f + iTime);
  float my = r.data[1] + iTime / 5.f;
  my += 50.0f * cos(r.data[0] / 20.0f + iTime);
  return r.data[2] - (sin(mx / 15.0f) + sin(my / 5.0f) + 5.f);
}
float smin(Context &__ctx, float a, float b, float k) {
  float h = max(k - abs(a - b), 0.0f) / k;
  return min(a, b) - h * h * k * (1.0f / 4.0f);
}
float hash11(Context &__ctx, float p) {
  return fract(sin(p * 727.1f) * 43758.5453123f);
}
float hash12(Context &__ctx, f32x2 p) {
  float h = dot(p, f32x2(127.1f, 311.7f));
  return fract(sin(h) * 43758.5453123f);
}
f32x3 hash31(Context &__ctx, float p) {
  f32x3 h = f32x3(1275.231f, 4461.7f, 7182.423f) * p;
  return fract(sin(h) * 43758.543123f);
}
float noise_3(Context &__ctx, f32x3 p) {
  f32x3 i = floor(p);
  f32x3 f = fract(p);
  f32x3 u = f * f * (3.0f - 2.0f * f);
  f32x2 ii = i.swizzle<0, 1>() + i.data[2] * f32x2(5.0f);
  float a = hash12(__ctx, ii + f32x2(0.0f, 0.0f));
  float b = hash12(__ctx, ii + f32x2(1.0f, 0.0f));
  float c = hash12(__ctx, ii + f32x2(0.0f, 1.0f));
  float d = hash12(__ctx, ii + f32x2(1.0f, 1.0f));
  float v1 = mix(mix(a, b, u.data[0]), mix(c, d, u.data[0]), u.data[1]);
  ii += f32x2(5.0f);
  a = hash12(__ctx, ii + f32x2(0.0f, 0.0f));
  b = hash12(__ctx, ii + f32x2(1.0f, 0.0f));
  c = hash12(__ctx, ii + f32x2(0.0f, 1.0f));
  d = hash12(__ctx, ii + f32x2(1.0f, 1.0f));
  float v2 = mix(mix(a, b, u.data[0]), mix(c, d, u.data[0]), u.data[1]);
  return max(mix(v1, v2, u.data[2]), 0.0f);
}
f32x3x3 fromEuler(Context &__ctx, f32x3 ang) {
  f32x2 a1 = f32x2(sin(ang.data[0]), cos(ang.data[0]));
  f32x2 a2 = f32x2(sin(ang.data[1]), cos(ang.data[1]));
  f32x2 a3 = f32x2(sin(ang.data[2]), cos(ang.data[2]));
  f32x3 m0 =
      f32x3(a1.data[1] * a3.data[1] + a1.data[0] * a2.data[0] * a3.data[0],
            a1.data[1] * a2.data[0] * a3.data[0] + a3.data[1] * a1.data[0],
            -a2.data[1] * a3.data[0]);
  f32x3 m1 =
      f32x3(-a2.data[1] * a1.data[0], a1.data[1] * a2.data[1], a2.data[0]);
  f32x3 m2 =
      f32x3(a3.data[1] * a1.data[0] * a2.data[0] + a1.data[1] * a3.data[0],
            a1.data[0] * a3.data[0] - a1.data[1] * a3.data[1] * a2.data[0],
            a2.data[1] * a3.data[1]);
  return f32x3x3(m0, m1, m2);
}
float map(Context &__ctx, f32x3 p) {
  f32x3x3 rot = fromEuler(__ctx, f32x3(0.0f, 0.0f, iTime));
  f32x4 sphere1 = f32x4(0.0f, 0.0f, 1.2f, .2f);
  f32x4 sphere2 = f32x4(1.2f, 0.0f, 0.0f, .4f);
  f32x4 sphere3 = f32x4(-1.2f, 0.0f, 1.0f, .4f);
  float dSphere1 = distance(p, sphere1.swizzle<0, 1, 2>()) - sphere1.data[3];
  float dSphere2 = distance(p, sphere2.swizzle<0, 1, 2>()) - sphere2.data[3];
  float dSphere3 = distance(p, sphere3.swizzle<0, 1, 2>()) - sphere3.data[3];
  float bump = noise_3(__ctx, p) * .8f;
  f32x3 p1 = p + bump;
  bump = noise_3(__ctx, p * .2f) * 5.f;
  f32x3 p2 = p + bump;
  return smin(__ctx, plasma(__ctx, p1), sdSphere(__ctx, p2), 6.f);
}
f32x3 gradient(Context &__ctx, f32x3 p) {
  const float d = 0.001f;
  f32x3 grad =
      f32x3(map(__ctx, p + f32x3(d, 0, 0)) - map(__ctx, p - f32x3(d, 0, 0)),
            map(__ctx, p + f32x3(0, d, 0)) - map(__ctx, p - f32x3(0, d, 0)),
            map(__ctx, p + f32x3(0, 0, d)) - map(__ctx, p - f32x3(0, 0, d)));
  return grad;
}
float RayMarching(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.f;
  for (int i = 0; i < 64; i++) {
    f32x3 p = ro + rd * dO;
    float ds = map(__ctx, p);
    dO += ds;
    if (ds < 0.01f || dO > 100.f)
      break;
  }
  return dO;
}
f32x3x3 setCamera(Context &__ctx, f32x3 ro, f32x3 ta, float cr) {
  f32x3 cw = normalize(ta - ro);
  f32x3 cp = f32x3(sin(cr), cos(cr), 0.0f);
  f32x3 cu = normalize(cross(cw, cp));
  f32x3 cv = normalize(cross(cu, cw));
  return f32x3x3(cu, cv, cw);
}
f32x4 render(Context &__ctx, f32x3 ro, f32x3 rd) {
  float c, s;
  f32x3 cam = ro + f32x3(0.0f, 0.0f, 50.0f);
  f32x2 fragCoord = gl_FragCoord.swizzle<0, 1>();
  f32x2 uv = (fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>()) - 0.5f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  uv.data[1] *= -1.0f;
  f32x3 dir = f32x3(0.0f, 0.0f, -1.0f);
  float xrot = length(uv) * .8f;
  c = cos(xrot);
  s = sin(xrot);
  dir = f32x3x3(1.0f, 0.0f, 0.0f, 0.0f, c, -s, 0.0f, s, c) * dir;
  c = normalize(uv).data[0];
  s = normalize(uv).data[1];
  dir = f32x3x3(c, -s, 0.0f, s, c, 0.0f, 0.0f, 0.0f, 1.0f) * dir;
  c = cos(0.8f);
  s = sin(0.8f);
  dir = f32x3x3(c, 0.0f, s, 0.0f, 1.0f, 0.0f, -s, 0.0f, c) * dir;
  float dist = RayMarching(__ctx, cam, dir);
  f32x3 pos = cam + dist * dir;
  f32x4 fragColor = f32x4(1.f);
  fragColor.set_swizzle<0, 1, 2>(
      mix(f32x3(55, 60, 56) / 255.f,
          mix(f32x3(0.0f, 0.0f, 0.0f), f32x3(1.0f, 1.0f, 1.0f),
              pos.data[2] / 15.0f),
          1.0f / (dist / 25.0f)));
  return fragColor;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p = (-iResolution.swizzle<0, 1>() + 2.0f * fragCoord.swizzle<0, 1>()) /
            iResolution.data[1];
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 ro = 10.0f * f32x3(-6.28f * m.data[1], -6.f * m.data[0], m.data[1]);
  f32x3 ta = f32x3(0.0f, -1.0f, 0.0f);
  f32x3x3 ca = setCamera(__ctx, ro, ta, 0.0f);
  f32x3 rd = ca * normalize(f32x3(p.swizzle<0, 1>(), 1.5f));
  fragColor = render(__ctx, ro, rd);
}
void mainVR(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord, f32x3 fragRayOri,
            f32x3 fragRayDir) {
  fragColor = render(__ctx, fragRayOri, fragRayDir);
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
