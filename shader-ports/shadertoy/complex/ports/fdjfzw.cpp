// Port of https://www.shadertoy.com/view/fdjfzw; author: ErinZ.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fdjfzw.json.
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
struct Context {
  TextureCube channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x4 someFunction(Context &__ctx, f32x4 a, float b) { return a + b; }
float logo(Context &__ctx, f32x2 uv);
float sdSphere(Context &__ctx, f32x3 p, f32x3 o, float r) {
  return length(p - o) - r;
}
float sdCapsule(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 ab = b - a;
  f32x3 ap = p - a;
  float t = dot(ab, ap) / dot(ab, ab);
  t = clamp(t, 0.f, 1.f);
  f32x3 c = a + t * ab;
  return length(p - c) - r;
}
float sdRoundCone(Context &__ctx, f32x3 p, float r1, float r2, float h) {
  float b = (r1 - r2) / h;
  float a = sqrt(1.0f - b * b);
  f32x2 q = f32x2(length(p.swizzle<0, 2>()), p.data[1]);
  float k = dot(q, f32x2(-b, a));
  if (k < 0.0f)
    return length(q) - r1;
  if (k > a * h)
    return length(q - f32x2(0.0f, h)) - r2;
  return dot(q, f32x2(a, b)) - r1;
}
f32x2x2 rotate(Context &__ctx, float a) {
  return f32x2x2(cos(a), sin(a), -sin(a), cos(a));
}
float smUni(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  return mix(d2, d1, h) - k * h * (1.0f - h);
}
float f(Context &__ctx, f32x3 p) {
  p.data[0] = abs(p.data[0]);
  float d;
  {
    f32x3 p0 = f32x3(p.data[0] * .8f, p.data[1] + .4f, p.data[2]);
    d = sdSphere(__ctx, p0, f32x3(0), 1.5f);
    p0 = f32x3(p.data[0] * .6f, p.data[1] + .6f, p.data[2] + .7f);
    float d0 = sdSphere(__ctx, p0, f32x3(0), 1.f);
    d = smUni(__ctx, d, d0, .3f);
  }
  float angle = sin(iTime * 5.f) * .1f;
  {
    f32x3 p0 = f32x3(p.data[0] - .85f, p.data[1] - .3f, p.data[2] * 1.2f + .3f);
    p0.set_swizzle<0, 1>(p0.swizzle<0, 1>() * (rotate(__ctx, -.3f)));
    p0.set_swizzle<1, 2>(p0.swizzle<1, 2>() * (rotate(__ctx, -.3f)));
    float d0 = sdRoundCone(__ctx, p0, .8f, .2f, 1.2f) / 1.2f;
    d = smUni(__ctx, d, d0, .3f);
  }
  {
    float d0 =
        sdCapsule(__ctx, p, f32x3(0, 0, -1.5f), f32x3(0, -.6f, -1.7f), .3f);
    d = smUni(__ctx, d, d0, .05f);
  }
  {
    float d0 = sdSphere(__ctx, p, f32x3(.3f, -.6f, -1.65f), .45f);
    d = smUni(__ctx, d, d0, .05f);
  }
  {
    f32x3 p0 = f32x3(p.data[0] - .6f, p.data[1] + .1f, p.data[2] + 1.45f);
    p0.set_swizzle<0, 1>(p0.swizzle<0, 1>() * (rotate(__ctx, -1.2f)));
    p0.set_swizzle<1, 2>(p0.swizzle<1, 2>() * (rotate(__ctx, -0.3f)));
    float d0 = sdRoundCone(__ctx, p0, .06f, .08f, .2f);
    d = smUni(__ctx, d, d0, .05f);
  }
  {
    f32x3 p0 = f32x3(p.data[0] - .9f, p.data[1] + .5f, p.data[2] + 1.3f);
    p0.set_swizzle<0, 1>(p0.swizzle<0, 1>() *
                         (rotate(__ctx, .2f + angle * .5f)));
    float d0 = sdCapsule(__ctx, p0, f32x3(0, 0, 0), f32x3(.8f, 0, 0), .1f);
    p0.set_swizzle<0, 1>(p0.swizzle<0, 1>() *
                         (rotate(__ctx, -.4f + angle * .5f)));
    d0 = smUni(__ctx, d0,
               sdCapsule(__ctx, p0, f32x3(0, 0, 0), f32x3(.8f, 0, 0), .1f),
               .05f);
    d = smUni(__ctx, d, d0, .05f);
  }
  return d;
}
float rayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float t = 0.01f;
  for (int i = 0; i < 128; i++) {
    f32x3 p = ro + t * rd;
    float d = f(__ctx, p);
    t += d;
    if (d < .001f || t > 200.f)
      break;
  }
  return t;
}
f32x3 calcNormal(Context &__ctx, f32x3 p) {
  const float h = 0.0001f;
  const f32x2 k = f32x2(1, -1);
  return normalize(
      k.swizzle<0, 1, 1>() * f(__ctx, p + k.swizzle<0, 1, 1>() * h) +
      k.swizzle<1, 1, 0>() * f(__ctx, p + k.swizzle<1, 1, 0>() * h) +
      k.swizzle<1, 0, 1>() * f(__ctx, p + k.swizzle<1, 0, 1>() * h) +
      k.swizzle<0, 0, 0>() * f(__ctx, p + k.swizzle<0, 0, 0>() * h));
}
f32x3x3 setCamera(Context &__ctx, f32x3 camtar, f32x3 campos, float camro) {
  f32x3 z = normalize(camtar - campos);
  f32x3 cp = f32x3(sin(camro), cos(camro), 0.f);
  f32x3 x = normalize(cross(cp, z));
  f32x3 y = cross(z, x);
  return f32x3x3(x, y, z);
}
f32x3 render(Context &__ctx, f32x2 uv) {
  f32x3 lightPos = f32x3(-5.f, 5.f, -5);
  f32x3 cam_tar = f32x3(0, -.5f, 0);
  float angle = pow(sin(iTime + .3f), 3.f) * .5f;
  f32x3 cam_pos = f32x3(sin(angle), 0, -cos(angle)) * 10.f;
  f32x3 rd = f32x3(uv, 4.f);
  if (iMouse.data[0] != 0.f || iMouse.data[1] != 0.f) {
    float angle_x =
        -(iMouse.data[0] / iResolution.data[0] - 0.5f) * 0.5f * 3.1415926f;
    float angle_y =
        (iMouse.data[1] / iResolution.data[1] - 0.5f) * 0.5f * 3.1415926f;
    rd.set_swizzle<1, 2>(rd.swizzle<1, 2>() * (rotate(__ctx, angle_y)));
    rd.set_swizzle<2, 0>(rd.swizzle<2, 0>() * (rotate(__ctx, angle_x)));
  }
  rd = normalize(setCamera(__ctx, cam_tar, cam_pos, 0.f) * rd);
  float t = rayMarch(__ctx, cam_pos, rd);
  f32x3 color = mix(f32x3(0, 0, 1), f32x3(0, 0, .6f), uv.data[1] * .5f + .5f);
  if (t > 200.f)
    return color;
  f32x3 p = cam_pos + t * rd;
  f32x3 n = calcNormal(__ctx, p);
  f32x3 l = normalize(lightPos - p);
  f32x3 h = normalize(l - rd);
  f32x3 diffusecol = f32x3(1.f);
  f32x3 specol = f32x3(1.f);
  f32x3 cmrfl =
      .5f * f32x3(1) *
      textureLod(__ctx.channel0, reflect(rd, n), 1.f).swizzle<0, 1, 2>();
  float dif = clamp(dot(l, n), 0.f, 1.f);
  float spe = pow(clamp(dot(h, n), 0.f, 1.f), 50.f);
  float fresnel = pow(clamp(1.f - dot(n, -rd), 0.f, 1.f), 5.f);
  color = cmrfl + .1f * dif * diffusecol + smoothstep(.6f, 1.f, spe) * specol;
  color = smoothstep(0.f, 1.f, color);
  color += fresnel * .4f;
  return color;
}
f32x2 getuv(Context &__ctx, f32x2 coord) {
  return (2.f * coord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (2.f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 color = f32x3(0.f);
  for (int m = 0; m < 3; m++) {
    for (int n = 0; n < 3; n++) {
      f32x2 offset = 2.f * (f32x2(float(m), float(n)) / float(3) - .5f);
      f32x2 uv = getuv(__ctx, fragCoord + offset);
      color += render(__ctx, uv);
    }
  }
  color /= float(3 * 3);
  color = mix(color, f32x3(1), logo(__ctx, uv));
  fragColor = f32x4(color, 1.0f);
}
float logo(Context &__ctx, f32x2 uv) {
  float n = 0.f;
  n += smoothstep(abs(length(uv - f32x2(-1.23f, 0)) - .12f),
                  abs(length(uv - f32x2(-1.23f, 0)) - .12f) +
                      3.f / iResolution.data[1],
                  .05f);
  n *= 1.f -
       smoothstep(uv.data[0], uv.data[0] + 3.f / iResolution.data[1], -1.25f);
  n += smoothstep(abs(uv.data[1]), abs(uv.data[1]) + 3.f / iResolution.data[1],
                  .17f) *
       smoothstep(abs(uv.data[0] + 1.29f),
                  abs(uv.data[0] + 1.29f) + 3.f / iResolution.data[1], .05f);
  n += smoothstep(abs(length(uv - f32x2(-1.5f, 0)) - .12f),
                  abs(length(uv - f32x2(-1.5f, 0)) - .12f) +
                      3.f / iResolution.data[1],
                  .05f);
  n += smoothstep(abs(length(uv - f32x2(-0.9f, 0)) - .12f),
                  abs(length(uv - f32x2(-0.9f, 0)) - .12f) +
                      3.f / iResolution.data[1],
                  .05f);
  n += (1.f +
        smoothstep(uv.data[0], uv.data[0] + 3.f / iResolution.data[1], -.6f) -
        smoothstep(uv.data[1], uv.data[1] + 3.f / iResolution.data[1], .05f)) *
       smoothstep(abs(length(uv - f32x2(-0.6f, .03f)) - .09f),
                  abs(length(uv - f32x2(-0.6f, .03f)) - .09f) +
                      3.f / iResolution.data[1],
                  .05f);
  n += (1.f -
        smoothstep(uv.data[0], uv.data[0] + 3.f / iResolution.data[1], -.6f) +
        smoothstep(uv.data[1], uv.data[1] + 3.f / iResolution.data[1], -.05f)) *
       smoothstep(abs(length(uv - f32x2(-0.6f, -.04f)) - .09f),
                  abs(length(uv - f32x2(-0.6f, -.04f)) - .09f) +
                      3.f / iResolution.data[1],
                  .05f);
  return n;
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_cube<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
  Context __ctx{{channel0, sampling0}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
