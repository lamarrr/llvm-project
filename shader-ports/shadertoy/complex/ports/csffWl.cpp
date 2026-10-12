// Port of https://www.shadertoy.com/view/csffWl; author: timmaffett.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/csffWl.json.
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
f32x2 mouse;
float id;
f32x2 path(Context &__ctx, float z) {
  float a = sin(z * .07f);
  float b = cos(z * .09f);
  return 3.f * f32x2(a + b, a - b);
}
float smoothMin(Context &__ctx, float a, float b, float c, float t) {
  return -log(exp(t * -a) + exp(t * -b) + exp(t * -c)) / t;
}
float smoothMin(Context &__ctx, float a, float b, float t) {
  return -log(exp(t * -a) + exp(t * -b)) / t;
}
f32x3 tri(Context &__ctx, f32x3 p) { return abs(fract(p) - .5f); }
float surf(Context &__ctx, f32x3 p) {
  return dot(tri(__ctx, p * .5f), f32x3(.5f)) - p.data[1] * .6f;
}
float map(Context &__ctx, f32x3 p) {
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() - (path(__ctx, p.data[2])));
  f32x3 q = p * atan(.0f, -1.f) / 2.f;
  float gyr = dot(cos(q), sin(q.swizzle<1, 2, 0>()));
  float osso = gyr + 1.3f;
  p.set_swizzle<0, 1>(
      p.swizzle<0, 1>() *
      (f32x2x2(cos((iTime / 2.0f) * .8f), sin((iTime / 2.0f) * .8f),
               -sin((iTime / 2.0f) * .8f), cos((iTime / 2.0f) * .8f))));
  float cyl = length(p.swizzle<0, 1>()) - 1.8f;
  float carne = -smoothMin(__ctx, cyl, gyr, 2.f) + surf(__ctx, p);
  id = step(carne, osso);
  return osso;
  return min(carne, osso);
}
float rayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float step, t = 0.f;
  for (int i = 0; i < 30; i++) {
    step = map(__ctx, ro + t * rd);
    t += step * .3f;
    if (t > 200.0f || step < 0.01f)
      break;
  }
  return t;
}
f32x3 normal(Context &__ctx, f32x3 p) {
  float d = map(__ctx, p);
  f32x2 e = f32x2(0.01f * .5f, 0);
  f32x3 n = f32x3(d) - f32x3(map(__ctx, p - e.swizzle<0, 1, 1>()),
                             map(__ctx, p - e.swizzle<1, 0, 1>()),
                             map(__ctx, p - e.swizzle<1, 1, 0>()));
  return n;
}
float shadow(Context &__ctx, f32x3 p, f32x3 lightPos) {
  return dot(normalize(normal(__ctx, p)), normalize(lightPos - p));
}
f32x3 hsb2rgb(Context &__ctx, f32x3 c) {
  f32x3 rgb = clamp(
      abs(mod(c.data[0] * 6.0f + f32x3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f,
      0.0f, 1.0f);
  rgb = rgb * rgb * (3.0f - 2.0f * rgb);
  return (c.data[2] * mix(f32x3(1.0f), rgb, c.data[1]));
}
f32x3 color(Context &__ctx, f32x3 ro, float t, f32x3 rd, f32x3 lightPos) {
  f32x3 col2 =
      hsb2rgb(__ctx, f32x3(0.02f * ro.data[1] * 3.1514f, 0.9f, 0.9f)) * 0.5f;
  f32x3 col3 =
      hsb2rgb(__ctx, f32x3(0.1f * (ro.data[2] + t * rd.data[2]) * 3.1514f, 0.9f,
                           0.9f)) *
      0.5f;
  f32x3 col =
      id < 0.5f
          ? col3 * f32x3(1.f)
          : col2 +
                (col3 * f32x3(clamp(mouse.data[1] / mouse.data[0], 0.1f, 1.0f) *
                              0.8f));
  col += mouse.data[1] / mouse.data[0] * shadow(__ctx, ro + t * rd, lightPos);
  col = mix(col2, col, 1.f / (t / 200.0f + .9f));
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  mouse = (iMouse.swizzle<0, 1>() == f32x2(0.f))
              ? f32x2(0.3f, 0.1f)
              : iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 uv =
      (fragCoord - iResolution.swizzle<0, 1>() * 0.5f) / iResolution.data[1];
  f32x3 lookAt = f32x3(0, 1, (iTime / 2.0f) * 4.f);
  f32x3 ro = lookAt + f32x3(0, 0, -0.1f);
  f32x3 lightPos = ro + f32x3(0, 0, 4);
  lookAt.set_swizzle<0, 1>(lookAt.swizzle<0, 1>() +
                           (path(__ctx, lookAt.data[2])));
  ro.set_swizzle<0, 1>(ro.swizzle<0, 1>() + (path(__ctx, ro.data[2])));
  lightPos.set_swizzle<0, 1>(lightPos.swizzle<0, 1>() +
                             (path(__ctx, lightPos.data[2])));
  f32x3 forward = normalize(lookAt - ro);
  f32x3 right = normalize(f32x3(forward.data[2], 0.f, -forward.data[0]));
  f32x3 up = cross(forward, right);
  f32x3 rd = normalize(forward + uv.data[0] * right + uv.data[1] * up);
  float t = rayMarch(__ctx, ro, rd);
  f32x3 col = color(__ctx, ro, t, rd, lightPos);
  fragColor = f32x4(col * 2.f, 1.0f);
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
