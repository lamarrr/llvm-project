// Port of https://www.shadertoy.com/view/fsccRH; author: Fraktoler.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fsccRH.json.
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

f32x2 cis(Context &__ctx, float theta) { return f32x2(cos(theta), sin(theta)); }
f32x2 mulComplex(Context &__ctx, f32x2 z, f32x2 w) {
  return z.data[0] * w + z.data[1] * f32x2(-w.data[1], w.data[0]);
}
f32x3 rotate(Context &__ctx, f32x3 p, f32x3 rot) {
  f32x2 v = mulComplex(__ctx, p.swizzle<1, 2>(), cis(__ctx, rot.data[0]));
  f32x3 p_prime = f32x3(p.data[0], v.swizzle<0, 1>());
  v = mulComplex(__ctx, p_prime.swizzle<0, 2>(), cis(__ctx, rot.data[1]));
  p_prime = f32x3(v.data[0], p_prime.data[1], v.data[1]);
  v = mulComplex(__ctx, p_prime.swizzle<0, 1>(), cis(__ctx, rot.data[2]));
  p_prime = f32x3(v.swizzle<0, 1>(), p_prime.data[2]);
  return p_prime;
}
f32x4 rotateXY(Context &__ctx, f32x4 p, float rot) {
  f32x4x4 rotXY =
      f32x4x4(cos(rot), -sin(rot), 0.0f, 0.0f, sin(rot), cos(rot), 0.0f, 0.0f,
              0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
  return rotXY * p;
}
f32x4 rotateXZ(Context &__ctx, f32x4 p, float rot) {
  f32x4x4 rotXZ =
      f32x4x4(cos(rot), 0.0f, -sin(rot), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, sin(rot),
              0.0f, cos(rot), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
  return rotXZ * p;
}
f32x4 rotateZW(Context &__ctx, f32x4 p, float rot) {
  f32x4x4 rotZW =
      f32x4x4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
              cos(rot), -sin(rot), 0.0f, 0.0f, sin(rot), cos(rot));
  return rotZW * p;
}
f32x4 toSpherical(Context &__ctx, f32x4 z) {
  float r = length(z);
  return f32x4(r,
               acos(z.swizzle<3, 2>() / f32x2(r, length(z.swizzle<0, 1, 2>()))),
               atan(z.data[1], z.data[0]));
}
f32x4 toCartesian(Context &__ctx, f32x4 z) {
  return z.data[0] *
         f32x4(cos(z.data[1]),
               sin(z.data[1]) * f32x3(cos(z.data[2]),
                                      sin(z.data[2]) * cis(__ctx, z.data[3])));
}
float DE(Context &__ctx, f32x3 v) {
  f32x4 c = f32x4(v, sin(iTime * 0.25f));
  f32x4 z = toSpherical(__ctx, c);
  float power = 8.0f;
  float bailout = 2.0f;
  float dr = 1.0f;
  float r = 0.0f;
  for (int k = 0; k < 10; k++) {
    if (z.data[0] > bailout)
      break;
    dr = power * pow(z.data[0], power - 1.0f) * dr + 1.0f;
    z = toCartesian(
            __ctx, f32x4(pow(z.data[0], power), power * z.swizzle<1, 2, 3>())) +
        c;
    z = toSpherical(__ctx, z);
  }
  return 0.5f * log(z.data[0]) * z.data[0] / dr;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.0f;
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + rd * dO;
    float dS = DE(__ctx, p);
    dO += dS;
    if (dO > 8.f || dS < .0001f)
      break;
  }
  return dO;
}
f32x3 normalVector(Context &__ctx, f32x3 p) {
  float d = DE(__ctx, p);
  f32x2 e = f32x2(.0001f, 0);
  f32x3 n = d - f32x3(DE(__ctx, p - e.swizzle<0, 1, 1>()),
                      DE(__ctx, p - e.swizzle<1, 0, 1>()),
                      DE(__ctx, p - e.swizzle<1, 1, 0>()));
  return normalize(n);
}
float lighting(Context &__ctx, f32x3 p) {
  f32x3 lightPos = f32x3(0.0f, -5.0f, 5.0f);
  f32x3 l = normalize(lightPos - p);
  f32x3 n = normalVector(__ctx, p);
  float dif = clamp(dot(n, l), 0.0f, 1.0f);
  float d = RayMarch(__ctx, p + n * .0001f * 2.0f, l);
  return dif;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float zoom = 1.5f;
  f32x2 uv = zoom * (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) /
             iResolution.data[0];
  f32x3 ro = f32x3(0.0f, -3.0f, 0.0f);
  f32x3 rd = normalize(f32x3(uv.data[0], 1.0f, uv.data[1]));
  float d = RayMarch(__ctx, ro, rd);
  f32x3 p = ro + rd * d;
  float dif = lighting(__ctx, p);
  f32x3 col = f32x3(dif);
  col = pow(col, f32x3(0.45f));
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
