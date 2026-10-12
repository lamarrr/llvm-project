// Port of https://www.shadertoy.com/view/fdycD1; author: fl0a1e.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/fdycD1.json.
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
float hash(Context &__ctx, float n) { return fract(sin(n) * 43758.5453f); }
float noise(Context &__ctx, f32x3 x) {
  f32x3 p = floor(x);
  f32x3 f = fract(x);
  f = f * f * (3.0f - 2.0f * f);
  float n = p.data[0] + p.data[1] * 157.0f + 113.0f * p.data[2];
  return mix(
      mix(mix(hash(__ctx, n + 0.0f), hash(__ctx, n + 1.0f), f.data[0]),
          mix(hash(__ctx, n + 157.0f), hash(__ctx, n + 158.0f), f.data[0]),
          f.data[1]),
      mix(mix(hash(__ctx, n + 113.0f), hash(__ctx, n + 114.0f), f.data[0]),
          mix(hash(__ctx, n + 270.0f), hash(__ctx, n + 271.0f), f.data[0]),
          f.data[1]),
      f.data[2]);
}
float fbm(Context &__ctx, f32x3 p) {
  float f = 0.0f;
  float freq = 4.0f;
  for (int i = 0; i < 5; i++) {
    float n = 4.f * noise(__ctx, p * freq) / freq;
    f += n;
    freq *= 1.5f;
  }
  return f;
}
float sdSphere(Context &__ctx, f32x3 p, float r) {
  return length(p) - (r + fbm(__ctx, p));
}
float map(Context &__ctx, f32x3 p) {
  float d = sdSphere(__ctx, p, 1.0f);
  return d;
}
f32x3 calcNormal(Context &__ctx, f32x3 pos) {
  f32x2 e = f32x2(1.0f, -1.0f) * 0.5773f * 0.0005f;
  return normalize(
      e.swizzle<0, 1, 1>() * map(__ctx, pos + e.swizzle<0, 1, 1>()) +
      e.swizzle<1, 1, 0>() * map(__ctx, pos + e.swizzle<1, 1, 0>()) +
      e.swizzle<1, 0, 1>() * map(__ctx, pos + e.swizzle<1, 0, 1>()) +
      e.swizzle<0, 0, 0>() * map(__ctx, pos + e.swizzle<0, 0, 0>()));
}
float calcAO(Context &__ctx, f32x3 pos, f32x3 nor) {
  float occ = 0.0f;
  float sca = 1.0f;
  for (int i = 0; i < 4; i++) {
    float h = .001f + 0.015f * float(i);
    float d = map(__ctx, pos + nor * h);
    occ = (h - d) * sca;
    sca *= .95f;
    if (occ > 0.35f)
      break;
  }
  return clamp(1.0f - 1.0f * occ, 0.0f, 1.0f);
}
float raymarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dist = 0.0f;
  for (int i = 0; i < 255; i++) {
    f32x3 p = ro + rd * dist;
    float d = map(__ctx, p);
    if (abs(d) < 0.001f || d > 100.0f)
      break;
    dist += d * 0.75f;
  }
  return dist;
}
f32x3 render(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x3 col;
  f32x3 background = f32x3(.01f, .01f, .01f);
  float d = raymarch(__ctx, ro, rd);
  if (d > 100.0f) {
  } else {
    f32x3 pos = ro + rd * d;
    f32x3 nor = calcNormal(__ctx, pos);
    pos += nor * 0.001f * 2.0f;
    float ao = calcAO(__ctx, pos, nor);
    {
      f32x3 sunPos =
          f32x3(13.f * sin(iTime * .5f), 7.f, 5.f * cos(iTime * .5f));
      f32x3 ld = normalize(sunPos - pos);
      float dif = clamp(dot(nor, ld), 0.0f, 1.0f);
      float spe = pow(clamp(dot(nor, ld), 0.0f, 1.0f), 16.0f);
      spe *= dif;
      spe *=
          0.04f + 0.96f * pow(clamp(1.0f - dot(ld, sunPos), 0.0f, 1.0f), 5.0f);
      col += 1.2f * dif * f32x3(.6f, .6f, 1.f);
    }
    col *= ao;
  }
  col = mix(col, background, 1.0f - exp(-0.0001f * d * d * d));
  return col;
}
f32x3x3 camera(Context &__ctx, f32x3 campos, f32x3 camtar) {
  float roll = 0.0f;
  f32x3 cw = normalize(camtar - campos);
  f32x3 cr = normalize(cross(cw, f32x3(sin(roll), cos(roll), 0.0f)));
  f32x3 cu = normalize(cross(cr, cw));
  return f32x3x3(cr, cu, -cw);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[0];
  float time = iTime * .2f;
  f32x2 mouse = 3.14159265359f * (-1.0f + 2.0f * iMouse.swizzle<0, 1>() /
                                              iResolution.swizzle<0, 1>());
  f32x3 ro = f32x3(0.0f + sin(mouse.data[0]) * 15.f, 2.0f,
                   15.0f - 15.f * (cos(mouse.data[0]) + 1.f));
  f32x3 camtar = f32x3(0.0f, 0.0f, 0.0f);
  f32x3x3 cam = camera(__ctx, ro, camtar);
  f32x3 rd = normalize(cam * f32x3(uv, -1.3f));
  f32x3 col = render(__ctx, ro, rd);
  col = pow(col, f32x3(1.0f / 2.2f));
  fragColor = f32x4(col, 1.0f);
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
