// Port of https://www.shadertoy.com/view/dsS3Dh; author: kishimisu.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dsS3Dh.json.
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
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x2x2 rot(Context &__ctx, float a) {
  return f32x2x2(cos(a), -sin(a), sin(a), cos(a));
}
float noise(Context &__ctx, f32x3 p) {
  f32x3 ip = floor(p);
  f32x3 fp = fract(p);
  fp = fp * fp * (3.0f - 2.0f * fp);
  f32x2 tap = (ip.swizzle<0, 1>() + f32x2(37.0f, 17.0f) * ip.data[2]) +
              fp.swizzle<0, 1>();
  f32x2 rz =
      textureLod(__ctx.channel0, (tap + 0.5f) / 256.0f, 0.0f).swizzle<1, 0>();
  return mix(rz.data[0], rz.data[1], fp.data[2]);
}
float fbm(Context &__ctx, f32x3 x) {
  float rz = 0.f;
  float a = .35f;
  for (int i = 0; i < 4; i++) {
    rz += noise(__ctx, x) * a;
    a *= .35f;
    x *= 4.f;
  }
  return rz - .25f;
}
f32x2 boxIntersection(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x3 rad = f32x3(6.f, 6.f, 2.f);
  f32x3 m = 1.0f / rd;
  f32x3 n = m * ro;
  f32x3 k = abs(m) * rad;
  f32x3 t1 = -n - k;
  f32x3 t2 = -n + k;
  float tN = max(max(t1.data[0], t1.data[1]), t1.data[2]);
  float tF = min(min(t2.data[0], t2.data[1]), t2.data[2]);
  if (tN > tF || tF < 0.0f)
    return f32x2(-1.0f);
  return f32x2(tN, tF);
}
void marchVolume(Context &__ctx, f32x3 ro, f32x3 rd, float near, float far,
                 f32x3 &color) {
  f32x3 vColor = f32x3(0.f);
  float visibility = 1.f;
  float inside = far - near +
                 fract(sin((ro)*f32x3x3(127.1f, 311.7f, 74.7f, 269.5f, 183.3f,
                                        246.1f, 113.5f, 271.9f, 124.6f)) *
                       43758.5453123f)
                         .data[0] *
                     .01f;
  float stepSize = inside / 40.f;
  float i = 0.f;
  for (float t = near; t <= far; t += stepSize) {
    f32x3 p = ro + t * rd;
    float s = 1.5f * .01f;
    float dens = smoothstep(-s, s,
                            fbm(__ctx, p + f32x3(iTime * .055f, iTime * .065f,
                                                 1.f + iTime * .02f))) *
                 .1f;
    float prev = visibility;
    visibility *= exp(-stepSize * dens * 5.5f);
    float absorption = prev - visibility;
    float light = smoothstep(2.5f, 6.5f, p.data[2]);
    vColor += absorption * dens * light * 20.f;
    if (i++ > 40.f)
      break;
  }
  color = min(vColor, 1.f) + color * visibility;
}
void initCamera(Context &__ctx, f32x2 fragCoord, f32x3 &ro, f32x3 &rd) {
  f32x2 uv =
      (fragCoord - iResolution.swizzle<0, 1>() * .5f) / iResolution.data[1];
  f32x2 m = iMouse.data[2] == 0.f
                ? f32x2(.5f)
                : iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  ro = f32x3(m.data[0] - .5f, m.data[1] - .5f, .01f) * .8f;
  f32x3 f = normalize(f32x3(0.f, 0.f, 1.f) - ro * .05f),
        r = normalize(cross(f32x3(0, 1, 0), f)), u = cross(f, r), c = ro + f,
        i = c + uv.data[0] * r + uv.data[1] * u;
  rd = normalize(i - ro);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 ro, rd;
  initCamera(__ctx, fragCoord, ro, rd);
  float moon = dot(rd, f32x3(0.f, 0.f, 1.f));
  f32x3 color = (.7f - fbm(__ctx, rd * 10.f) * 3.f) *
                f32x3(smoothstep(.995f, .9955f, moon));
  color += (1.25f - color) * pow(moon + .1f, 6.f) * .25f;
  f32x3 h = fract(sin((f32x3(floor(rd.swizzle<0, 1>() * 80.f), 0.f)) *
                      f32x3x3(127.1f, 311.7f, 74.7f, 269.5f, 183.3f, 246.1f,
                              113.5f, 271.9f, 124.6f)) *
                  43758.5453123f);
  float stars =
      length(fract(rd.swizzle<0, 1>() * 80.f + h.swizzle<0, 1>() * .75f) - .5f);
  f32x2 hit = boxIntersection(__ctx, ro - f32x3(0.f, 0.f, 4.f), rd);
  marchVolume(__ctx, ro, rd, hit.data[0], hit.data[1], color);
  fragColor = f32x4(color, 1.f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
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
