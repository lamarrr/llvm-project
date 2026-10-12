// Port of https://www.shadertoy.com/view/ssGcDR; author: fl0a1e.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/ssGcDR.json.
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
float sdPlane(Context &__ctx, f32x3 p) { return p.data[1] + 0.3f; }
float sdSphere(Context &__ctx, f32x3 p, float r, f32x3 offset) {
  return length(p - offset) - r;
}
float sdTorus(Context &__ctx, f32x3 p, f32x2 t) {
  f32x2 q = f32x2(length(p.swizzle<0, 2>()) - t.data[0], p.data[1]);
  return length(q) - t.data[1];
}
float sdBoxFrame(Context &__ctx, f32x3 p, f32x3 b, float e) {
  p = abs(p) - b;
  f32x3 q = abs(p + e) - e;
  return min(min(length(max(f32x3(p.data[0], q.data[1], q.data[2]), 0.0f)) +
                     min(max(p.data[0], max(q.data[1], q.data[2])), 0.0f),
                 length(max(f32x3(q.data[0], p.data[1], q.data[2]), 0.0f)) +
                     min(max(q.data[0], max(p.data[1], q.data[2])), 0.0f)),
             length(max(f32x3(q.data[0], q.data[1], p.data[2]), 0.0f)) +
                 min(max(q.data[0], max(q.data[1], p.data[2])), 0.0f));
}
float opU(Context &__ctx, float a, float b) { return (a < b) ? a : b; }
float sdSmoothUnion(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  return mix(d2, d1, h) - k * h * (1.0f - h);
}
float map(Context &__ctx, f32x3 p) {
  float d = sdSphere(__ctx, p, .3f, f32x3(2.f * sin(iTime), 0.f, 0.f));
  d = sdSmoothUnion(__ctx, d,
                    sdTorus(__ctx, p + f32x3(0.f, .1f, 0.f), f32x2(0.8f, 0.2f)),
                    0.2f);
  d = sdSmoothUnion(__ctx, d,
                    sdSphere(__ctx, p, .3f, f32x3(0.f, 0.f, 2.f * cos(iTime))),
                    0.2f);
  d = sdSmoothUnion(__ctx, d,
                    sdBoxFrame(__ctx, p - f32x3(2.f, 0.f, 0.f),
                               f32x3(0.3f, 0.3f, 0.3f), 0.03f),
                    .1f);
  d = sdSmoothUnion(__ctx, d,
                    sdBoxFrame(__ctx, p - f32x3(0.f, 0.f, -2.f),
                               f32x3(0.3f, 0.3f, 0.3f), 0.03f),
                    .1f);
  d = sdSmoothUnion(__ctx, d,
                    sdBoxFrame(__ctx, p - f32x3(-2.f, 0.f, 0.f),
                               f32x3(0.3f, 0.3f, 0.3f), 0.03f),
                    .1f);
  d = sdSmoothUnion(__ctx, d,
                    sdBoxFrame(__ctx, p - f32x3(0.f, 0.f, 2.f),
                               f32x3(0.3f, 0.3f, 0.3f), 0.03f),
                    .1f);
  d = opU(__ctx, d, sdPlane(__ctx, p));
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
float raymarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dist = 0.0f;
  for (int i = 0; i < 255; i++) {
    f32x3 p = ro + rd * dist;
    float d = map(__ctx, p);
    if (d < 0.001f || d > 100.0f)
      break;
    dist += d;
  }
  return dist;
}
float calcAO(Context &__ctx, f32x3 pos, f32x3 nor) {
  float occ = 0.0f;
  float sca = 1.0f;
  for (int i = 0; i < 5; i++) {
    float h = .01f + 0.015f * float(i);
    float d = map(__ctx, pos + nor * h);
    occ = (h - d) * sca;
    sca *= .95f;
    if (occ > 0.35f)
      break;
  }
  return clamp(1.0f - 5.0f * occ, 0.1f, 1.0f);
}
float calcSoftshadow(Context &__ctx, f32x3 ro, f32x3 rd, float k) {
  float res = 1.0f;
  float t = .02f;
  for (int i = 0; i < 24; i++) {
    float h = map(__ctx, ro + rd * t);
    float s = smoothstep(0.0f, 1.0f, clamp(k * h / t, 0.0f, 1.0f));
    res = min(res, s);
    t += clamp(h, 0.02f, 0.2f);
    if (res < 0.004f || t > 2.5f)
      break;
  }
  return clamp(res, 0.2f, 1.0f);
}
f32x3x3 camera(Context &__ctx, f32x3 campos, f32x3 camtar) {
  float roll = 0.0f;
  f32x3 cw = normalize(camtar - campos);
  f32x3 cr = normalize(cross(cw, f32x3(sin(roll), cos(roll), 0.0f)));
  f32x3 cu = normalize(cross(cr, cw));
  return f32x3x3(cr, cu, -cw);
}
f32x3 render(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x3 col;
  float d = raymarch(__ctx, ro, rd);
  if (d > 100.0f) {
    col = f32x3(.4f, .5f, .9f);
  } else {
    f32x3 pos = ro + rd * d;
    f32x3 nor = calcNormal(__ctx, pos);
    float ao = calcAO(__ctx, pos, nor);
    {
      f32x3 sunPos = f32x3(13.f, 7.f, 5.f);
      f32x3 ld = normalize(sunPos - pos);
      float dif = clamp(dot(nor, ld), 0.0f, 1.0f);
      pos += nor * 0.001f * 2.0f;
      dif *= calcSoftshadow(__ctx, pos, ld, 8.0f);
      float spe = pow(clamp(dot(nor, ld), 0.0f, 1.0f), 16.0f);
      spe *= dif;
      spe *=
          0.04f + 0.96f * pow(clamp(1.0f - dot(ld, sunPos), 0.0f, 1.0f), 5.0f);
      col += dif * f32x3(.6f, .6f, 1.f);
    }
    col *= ao;
  }
  col = mix(col, f32x3(.4f, .5f, .9f), 1.0f - exp(-0.0001f * d * d * d));
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[0];
  float time = iTime * .2f;
  f32x3 ro = f32x3(0.0f, 2.0f, 10.0f);
  f32x3 camtar = f32x3(0.0f, 0.0f, 0.0f);
  float cameraRadius = 10.f;
  ro.data[0] = cameraRadius * cos(time) + camtar.data[0];
  ro.data[2] = cameraRadius * sin(time) + camtar.data[2];
  f32x3x3 cam = camera(__ctx, ro, camtar);
  f32x3 rd = normalize(cam * f32x3(uv, -1.5f));
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
