// Port of https://www.shadertoy.com/view/NtGyDh; author: ruojake.
// Dataset license metadata: cc-by-sa-4.0; original retained in
// upstream/NtGyDh.json.
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
f32x2x2 rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float cheb(Context &__ctx, f32x3 p, float k) {
  return pow(pow(abs(p.data[0]), k) + pow(abs(p.data[1]), k) +
                 pow(abs(p.data[2]), k),
             1.f / k);
}
float cheb(Context &__ctx, f32x2 p, float k) {
  return pow(pow(abs(p.data[0]), k) + pow(abs(p.data[1]), k), 1.f / k);
}
float time;
float scene(Context &__ctx, f32x3 p) {
  float pt = time * 3.141593f;
  f32x2x2 r = rot(__ctx, pt / 3.f);
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (r));
  float tr0 = abs(cheb(__ctx, p.swizzle<0, 1>(), 2.5f + sin(pt)) - 1.f) - .5f;
  p.set_swizzle<2, 1>(p.swizzle<2, 1>() * (r));
  float tr1 =
      abs(cheb(__ctx, p.swizzle<1, 2>(), 3.f - sin(pt * 2.f) * 1.5f) - 1.f) -
      .5f;
  p.set_swizzle<1, 0>(p.swizzle<1, 0>() * (r));
  float tr2 =
      abs(cheb(__ctx, p.swizzle<2, 0>(), 2.1f + sin(pt * -3.f) * .5f) - 1.f) -
      .5f;
  return cheb(__ctx, f32x3(tr0, tr1, tr2), 2.25f + .75f * sin(pt * .5f)) - .5f;
}
f32x3 normal(Context &__ctx, f32x3 p, float d) {
  const f32x2 e = f32x2(.004f, 0);
  return normalize(d - f32x3(scene(__ctx, p - e.swizzle<0, 1, 1>()),
                             scene(__ctx, p - e.swizzle<1, 0, 1>()),
                             scene(__ctx, p - e.swizzle<1, 1, 0>())));
}
float shadow(Context &__ctx, f32x3 ro, f32x3 rd) {
  float res = 1.f;
  float t = 0.f;
  for (int i = 0; i < 80 && res > 0.f; ++i) {
    f32x3 p = ro + rd * t;
    float d = scene(__ctx, p);
    if (d < .001f)
      return 0.f;
    res = min(res, d / t);
    t += d;
  }
  return res;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - iResolution.swizzle<0, 1>() * .5f) / iResolution.data[1];
  f32x3 ro = f32x3(0, 0, -6), rd = normalize(f32x3(uv, 1.75f)), p;
  time = iTime;
  float zoomPos = uv.data[1] * .5f;
  float zoomWidth = .5f;
  zoomPos += fract(time * .17f) * 1.5f - fract(time * .3f);
  zoomWidth *= 2.f + sin(time * .5f) * fract(time * .2f + .3f);
  bool isZoomed = abs(uv.data[0] - zoomPos) < zoomWidth * .5f;
  if (isZoomed) {
    rd = normalize(f32x3(uv, 3.f));
    ro.data[1] += floor(cos(time * .7f) * 1.5f) * .5f;
  };
  float hft = length(fract(uv * (isZoomed ? 15.f : 40.f)) - .5f);
  time += step(.5f, hft) * .1f * (sin(iTime + length(uv) * 8.f) * .5f + .5f);
  float t = 2.f, d = 0.f, i = 0.f;
  for (; i < 1.f && t < 12.f; i += 1.f / 256.f) {
    p = ro + rd * t;
    d = scene(__ctx, p) * .7f;
    if (abs(d) < .0001f)
      break;
    t += d;
  }
  f32x3 col = f32x3(.75f + uv.data[1] * .25f);
  bool line = abs(abs(uv.data[0] - zoomPos) - zoomWidth * .5f) < .05f;
  bool render = t < 12.f && !line;
  if (render) {
    f32x3 n = normal(__ctx, p, d);
    float frn = pow(1.f - dot(rd, -n), 8.f);
    f32x3 rfd = reflect(rd, n);
    f32x3 ld = normalize(f32x3(1, 1, -1));
    float l = dot(ld, n) * .5f + .5f;
    l *= l;
    float s = shadow(__ctx, p + n * .1f, ld);
    float rf = frn * pow(dot(f32x3(1), cos(rfd * 16.f + 3.f)), 4.f) *
               (n.data[1] * .5f + .5f) * (1.f - i);
    float spec = pow(dot(rfd, ld), 16.f);
    float lighting = min(max(rf, s * max(spec, l)), 1.f);
    col = mix(f32x3(.05f, .1f, .5f) * max(sqrt(lighting), .1f),
              mix(f32x3(1.f, .5f, .3f), f32x3(1.f, .1f, .6f), lighting),
              lighting);
    col = mix(col, f32x3(lighting), lighting * lighting * lighting);
    if (isZoomed)
      col = max(f32x3(.85f, 1.f, 0), f32x3(floor(lighting * 6.f) / 6.f)) *
            step(.125f, lighting);
  }
  col *=
      1.f - pow(length((fragCoord / iResolution.swizzle<0, 1>()) - .5f), 5.f);
  float dth =
      fract(dot(fragCoord + float(iFrame & 7) * iResolution.swizzle<0, 1>(),
                f32x2(.11592f, .60233f)));
  if (line)
    col = f32x3(1);
  fragColor = f32x4(col + dth / 255.f, 1.0f);
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
