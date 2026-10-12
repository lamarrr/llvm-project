// Port of https://www.shadertoy.com/view/mldSzj; author: CodeJUN.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mldSzj.json.
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
f32x2 fixUV(Context &__ctx, f32x2 c) {
  return (2.f * c - iResolution.swizzle<0, 1>()) /
         min(iResolution.data[0], iResolution.data[1]);
}
float sdfSphere(Context &__ctx, f32x3 p) { return length(p) - .8f; }
float sdfRect(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 d = abs(p) - b;
  return length(max(d, -.2f)) +
         min(max(d.data[0], max(d.data[1], d.data[2])), 0.f);
}
float sdfPlane(Context &__ctx, f32x3 p) { return p.data[1]; }
float smin(Context &__ctx, float a, float b, float k) {
  float h = clamp(.5f + .5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
float map(Context &__ctx, f32x3 p) {
  float d = sdfSphere(__ctx, p);
  d = smin(__ctx, d, sdfPlane(__ctx, p), .25f + 0.5f * sin(iTime));
  return d;
}
float rayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float t = 0.1f;
  for (int i = 0; i < 128 && t < 150.f; i++) {
    f32x3 p = ro + t * rd;
    float d = map(__ctx, p);
    if (d < .001f) {
      break;
    }
    t += d;
  }
  return t;
}
float calcAO(Context &__ctx, f32x3 p, f32x3 n) {
  float occ = 0.0f;
  float sca = 1.0f;
  for (int i = 0; i < 5; i++) {
    float h = .01f + .03f * float(i);
    float d = map(__ctx, p + n * h);
    occ += (h - d) * sca;
    if (occ > .35f)
      break;
  }
  return clamp(1.f - 3.f * occ, 0.f, 1.f);
}
float calcAO2(Context &__ctx, f32x3 n) { return 0.5f + 0.5f * n.data[1]; }
f32x3 calcNormal(Context &__ctx, f32x3 p) {
  const float h = 0.0001f;
  ;
  const f32x2 k = f32x2(1, -1);
  return normalize(
      k.swizzle<0, 1, 1>() * map(__ctx, p + k.swizzle<0, 1, 1>() * h) +
      k.swizzle<1, 1, 0>() * map(__ctx, p + k.swizzle<1, 1, 0>() * h) +
      k.swizzle<1, 0, 1>() * map(__ctx, p + k.swizzle<1, 0, 1>() * h) +
      k.swizzle<0, 0, 0>() * map(__ctx, p + k.swizzle<0, 0, 0>() * h));
}
f32x3x3 setCamera(Context &__ctx, f32x3 ta, f32x3 ro, float cr) {
  f32x3 z = normalize(ta - ro);
  f32x3 cp = f32x3(sin(cr), cos(cr), 0.f);
  f32x3 x = normalize(cross(z, cp));
  f32x3 y = cross(x, z);
  return f32x3x3(x, y, z);
}
float softShadow(Context &__ctx, f32x3 ro, f32x3 rd, float k) {
  float res = 1.0f;
  float ph = 1e20f;
  for (float t = 0.1f; t < 150.f;) {
    float h = map(__ctx, ro + rd * t);
    if (h < .01f)
      return 0.0f;
    float y = h * h / (2.0f * ph);
    float d = sqrt(h * h - y * y);
    res = min(res, k * d / max(0.0f, t - y));
    ph = h;
    t += h;
  }
  return res;
}
f32x3 render(Context &__ctx, f32x2 uv) {
  f32x3 color = f32x3(.05f);
  f32x3 ro = f32x3(2.f * cos(iTime), 1.f, 2.f * sin(iTime));
  if (iMouse.data[2] > 0.001f) {
    float theta = iMouse.data[0] / iResolution.data[0] * 2.f * 3.14159265f;
    ro = f32x3(2.f * cos(theta), 1.f + 0.2f * cos(iTime), 2.f * sin(theta));
  }
  f32x3 ta = f32x3(0.f);
  f32x3x3 cam = setCamera(__ctx, ta, ro, .1f * sin(iTime));
  f32x3 rd = normalize(cam * f32x3(uv, 1.f));
  float t = rayMarch(__ctx, ro, rd);
  if (t < 150.f) {
    f32x3 p = ro + t * rd;
    f32x3 n = calcNormal(__ctx, p);
    f32x3 light = f32x3(2.f * cos(iTime), 2.f, 2.f * sin(iTime + 2.f));
    float dif = clamp(dot(normalize(light - p), n), 0.f, 1.f);
    p += .001f * n;
    dif *= softShadow(__ctx, p, normalize(light - p), 2.f);
    float amd = 0.5f + 0.5f * dot(n, f32x3(0.f, 1.f, 0));
    color = amd * f32x3(0.05f) + dif * f32x3(.6f);
    float ao = calcAO(__ctx, p, n);
    ao *= calcAO2(__ctx, n);
    color *= ao;
  }
  return sqrt(color);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 color = f32x3(0.f);
  for (int m = 0; m < 3; m++) {
    for (int n = 0; n < 3; n++) {
      f32x2 offset = 2.f * (f32x2(float(m), float(n)) / float(3) - 0.5f);
      f32x2 uv = fixUV(__ctx, fragCoord + offset);
      color += render(__ctx, uv);
    }
  }
  fragColor = f32x4(color / float(3 * 3), 1.f);
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
