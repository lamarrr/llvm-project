// Port of https://www.shadertoy.com/view/7l2yWR; author: Fraktoler.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7l2yWR.json.
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
f32x4 conjQuat(Context &__ctx, f32x4 q) {
  return f32x4(q.data[0], -q.swizzle<1, 2, 3>());
}
f32x4 mulQuat(Context &__ctx, f32x4 p, f32x4 q) {
  return f32x4(
      p.data[0] * q.data[0] - dot(p.swizzle<1, 2, 3>(), q.swizzle<1, 2, 3>()),
      p.data[0] * q.swizzle<1, 2, 3>() + q.data[0] * p.swizzle<1, 2, 3>() +
          cross(p.swizzle<1, 2, 3>(), q.swizzle<1, 2, 3>()));
}
f32x4 sqrQuat(Context &__ctx, f32x4 q) {
  return f32x4(q.data[0] * q.data[0] -
                   dot(q.swizzle<1, 2, 3>(), q.swizzle<1, 2, 3>()),
               (q.data[0] + q.data[0]) * q.swizzle<1, 2, 3>());
}
f32x4 cubeQuat(Context &__ctx, f32x4 q) {
  float r2 = q.data[0] * q.data[0];
  float v2 = dot(q.swizzle<1, 2, 3>(), q.swizzle<1, 2, 3>());
  return q * f32x4(r2 - 3.f * v2, f32x3(3.f * r2 - v2));
}
f32x4 recipQuat(Context &__ctx, f32x4 q) {
  return f32x4(q.data[0], -q.swizzle<1, 2, 3>()) / dot(q, q);
}
f32x4 sqrtQuat(Context &__ctx, f32x4 q) {
  return f32x4(sqrt(0.5f * (length(q) + q.data[0])),
               normalize(q.swizzle<1, 2, 3>()) *
                   sqrt(0.5f * (length(q) - q.data[0])));
}
f32x4 expQuat(Context &__ctx, f32x4 q) {
  float r = length(q.swizzle<1, 2, 3>());
  return exp(q.data[0]) *
         f32x4(cos(r), normalize(q.swizzle<1, 2, 3>()) * sin(r));
}
f32x4 logQuat(Context &__ctx, f32x4 q) {
  return f32x4(0.5f * log(dot(q, q)),
               normalize(q.swizzle<1, 2, 3>()) * acos(q.data[0] / length(q)));
}
f32x4 powQuat(Context &__ctx, f32x4 q, float x) {
  float r = length(q);
  float phi = acos(q.data[0] / r);
  return pow(r, x) *
         f32x4(cos(x * phi), normalize(q.swizzle<1, 2, 3>()) * sin(x * phi));
}
f32x4 powerQuat(Context &__ctx, f32x4 p, f32x4 q) {
  return expQuat(__ctx, mulQuat(__ctx, logQuat(__ctx, p), q));
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
float DE(Context &__ctx, f32x3 v) {
  f32x4 c = f32x4(v, sin(iTime * 0.2f) * 1.25f);
  f32x4 q = c;
  float power = 2.0f;
  float bailout = 2.0f;
  float dr = 1.0f;
  float r = 0.0f;
  for (int k = 0; k < 10; k++) {
    r = length(q);
    if (r > bailout)
      break;
    dr = power * pow(r, power - 1.0f) * dr + 1.0f;
    q = powQuat(__ctx, q, power) + c;
  }
  return 0.5f * log(r) * r / dr;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.0f;
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + rd * dO;
    float dS = DE(__ctx, p);
    dO += dS;
    if (dO > 8.f || dS < .001f)
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
  f32x3 lightPos =
      rotate(__ctx, f32x3(0.0f, -5.0f, 5.0f), f32x3(-0.5f, 0.f, -iTime / 5.f));
  f32x3 l = normalize(lightPos - p);
  f32x3 n = normalVector(__ctx, p);
  float dif = clamp(dot(n, l), 0.0f, 1.0f);
  float d = RayMarch(__ctx, p + n * .001f * 2.0f, l);
  if (d < length(lightPos - p))
    dif *= .1f;
  return dif;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float zoom = 2.0f;
  f32x2 uv = zoom * (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) /
             iResolution.data[0];
  f32x3 rot = f32x3(-0.5f, 0.0f, -iTime / 5.f);
  f32x3 ro = rotate(__ctx, f32x3(0.0f, -3.0f, 0.0f), rot);
  f32x3 rd = normalize(rotate(__ctx, f32x3(uv.data[0], 1.0f, uv.data[1]), rot));
  float d = RayMarch(__ctx, ro, rd);
  f32x3 p = ro + rd * d;
  float dif = lighting(__ctx, p);
  f32x3 col = f32x3(dif);
  col = pow(col, f32x3(0.45f));
  fragColor = f32x4(col, 1.0f);
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
