// Port of https://www.shadertoy.com/view/cdlBDl; author: tristanantonsen.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cdlBDl.json.
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
float sdPlane(Context &__ctx, f32x3 p, f32x3 n, float h) {
  return dot(p, normalize(n)) + h;
}
float sdSphere(Context &__ctx, f32x3 p, f32x3 c, float r) {
  return length(p - c) - r;
}
float opUnion(Context &__ctx, float d1, float d2) { return min(d1, d2); }
float opSubtraction(Context &__ctx, float d1, float d2) { return max(-d2, d1); }
float opIntersection(Context &__ctx, float d1, float d2) { return max(d1, d2); }
float opSmoothUnion(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  return mix(d2, d1, h) - k * h * (1.0f - h);
}
float opSmoothSubtraction(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f - 0.5f * (d2 + d1) / k, 0.0f, 1.0f);
  return mix(d1, -d2, h) + k * h * (1.0f - h);
}
float opSmoothIntersection(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f - 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  return mix(d2, d1, h) + k * h * (1.0f - h);
}
f32x3 rotY(Context &__ctx, f32x3 p, float a) {
  float s = sin(a);
  float c = cos(a);
  f32x3x3 m = f32x3x3(c, 0.f, s, 0.f, 1.f, 0.f, -s, 0.f, c);
  return m * p;
}
f32x2 map(Context &__ctx, f32x3 p) {
  f32x2 m = (2.f * iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
            iResolution.data[1];
  float y = 0.1f * sin(iTime);
  float x = 0.2f * cos(iTime);
  f32x2 res;
  float s1 = sdSphere(__ctx, p, f32x3(0.5f, y, 0.f), 0.6f);
  res = f32x2(s1, 1.0f);
  float s2 = sdSphere(__ctx, p, f32x3(x, -y, 0.f), 0.7f);
  if (s2 < res.data[0])
    res.data[1] = 2.0f;
  res.data[0] = opSmoothUnion(__ctx, s1, s2, 0.1f);
  float s3 = sdSphere(__ctx, p, f32x3(m.data[0], m.data[1], -1.f), 0.8f);
  res.data[0] = opSmoothSubtraction(__ctx, res.data[0], s3, 0.05f);
  if (s3 < res.data[0] + 0.0001f)
    res.data[1] = 3.0f;
  float s4 = sdSphere(__ctx, p, f32x3(m.data[0], m.data[1], -1.f), 0.2f);
  if (s4 < res.data[0])
    res.data[1] = 4.0f;
  res.data[0] = opUnion(__ctx, res.data[0], s4);
  return res;
}
f32x2 rayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float d = 0.0f;
  float mat = 1.0f;
  for (int i = 0; i < 100; i++) {
    if (i >= 100)
      break;
    f32x3 p = ro + rd * d;
    f32x2 ds = map(__ctx, p);
    d += ds.data[0];
    mat = ds.data[1];
    if (d >= 100.0f || ds.data[0] < 0.0001f)
      break;
    i++;
  }
  return f32x2(d, mat);
}
f32x3 gradient(Context &__ctx, f32x3 p) {
  float epsilon = 0.0001f;
  f32x3 dx = f32x3(epsilon, 0.f, 0.0f);
  f32x3 dy = f32x3(0.f, epsilon, 0.0f);
  f32x3 dz = f32x3(0.f, 0.0f, epsilon);
  float ddx = map(__ctx, p + dx).data[0] - map(__ctx, p - dx).data[0];
  float ddy = map(__ctx, p + dy).data[0] - map(__ctx, p - dy).data[0];
  float ddz = map(__ctx, p + dz).data[0] - map(__ctx, p - dz).data[0];
  return normalize(f32x3(ddx, ddy, ddz));
}
f32x3 rayDirection(Context &__ctx, f32x2 p, f32x3 ro, f32x3 rt) {
  f32x3 vup = f32x3(0.f, 1.0f, 0.0f);
  float aspectRatio = iResolution.data[0] / iResolution.data[1];
  f32x3 vw = normalize(ro - rt);
  f32x3 vu = normalize(cross(vup, vw));
  f32x3 vv = cross(vw, vu);
  float theta = radians(30.f);
  float viewport_height = 2.f * tan(theta);
  float viewport_width = aspectRatio * viewport_height;
  f32x3 horizontal = -viewport_width * vu;
  f32x3 vertical = viewport_height * vv;
  float focus_dist = length(ro - rt);
  f32x3 center = ro - vw * focus_dist;
  f32x3 rd = center + p.data[0] * horizontal + p.data[1] * vertical - ro;
  return normalize(rd);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord / iResolution.swizzle<0, 1>() - 0.5f) * 2.0f;
  f32x3 rt = f32x3(0.f, 0.f, 0.f);
  f32x3 ro = f32x3(0.f, 0.f, -10.02f);
  f32x3 rd = rayDirection(__ctx, uv, ro, rt);
  f32x2 d = rayMarch(__ctx, ro, rd);
  float v = length(uv) * .75f;
  fragColor = f32x4(mix(0.1f, 0.2f, smoothstep(0.0f, 1.0f, uv.data[1])));
  if (d.data[0] <= 100.0f) {
    f32x3 p = ro + rd * d.data[0];
    f32x3 N = gradient(__ctx, p);
    float light1 = dot(N, normalize(f32x3(1, 1, -1))) * .5f + .5f;
    float light2 = dot(N, normalize(f32x3(-1, 1, -1))) * .5f + .5f;
    f32x3 color = 0.5f * f32x3(light1) + 0.5f * f32x3(light2);
    fragColor = f32x4(color, 1.0f);
    if (d.data[1] == 1.f)
      fragColor.set_swizzle<1, 2>(fragColor.swizzle<1, 2>() * (0.5f));
    if (d.data[1] == 2.f)
      fragColor.set_swizzle<0, 1, 2>(fragColor.swizzle<0, 1, 2>() *
                                     (f32x3(0.5f, 0.5f, 0.9f)));
    if (d.data[1] == 3.f)
      fragColor.set_swizzle<0, 1, 2>(fragColor.swizzle<0, 1, 2>() *
                                     (f32x3(0.5f, 0.9f, 0.5f)));
    if (d.data[1] == 4.f)
      fragColor.set_swizzle<0, 1, 2>(fragColor.swizzle<0, 1, 2>() *
                                     (f32x3(0.9f, 0.5f, 0.9f)));
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
