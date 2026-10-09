// Port of https://www.shadertoy.com/view/dtXyDj; author: MacSlow.
// Dataset license metadata: gpl-3.0; original retained in upstream/dtXyDj.json.
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
const int MAX_ITER = 64;
const float EPSILON = .0001f;
const float STEP_BIAS = .95f;
float sdPlane(Context &__ctx, f32x3 p, float d) { return p.data[1] - d; }
float sdSphere(Context &__ctx, f32x3 p, float r) { return length(p) - r; }
float map(Context &__ctx, f32x3 p) {
  float ground = sdPlane(__ctx, p, -.2f);
  float r = .01f * sin(120.f * p.data[1] * p.data[0] + 5.f * iTime);
  float ball1 =
      sdSphere(__ctx, p - f32x3(2.f * cos(iTime), .2f, sin(iTime)), .1f);
  float ball2 =
      sdSphere(__ctx, p - f32x3(.3f, .4f + .2f * cos(3.f * iTime), .0f), .2f);
  float ball3 = sdSphere(__ctx, p - f32x3(-.2f, .3f, -.2f), .2f);
  float ball4 = sdSphere(__ctx, p - f32x3(-.4f, .2f, .4f), .3f + r);
  float balls = min(ball1, min(ball2, min(ball3, ball4)));
  return min(ground, balls);
}
f32x3 normal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(.001f, .0f);
  float d = map(__ctx, p);
  f32x3 n = f32x3(map(__ctx, p + e.swizzle<0, 1, 1>()) - d,
                  map(__ctx, p + e.swizzle<1, 0, 1>()) - d,
                  map(__ctx, p + e.swizzle<1, 1, 0>()) - d);
  return normalize(n);
}
float shadow(Context &__ctx, f32x3 ro, f32x3 rd) {
  float value = 1.f;
  const float steps = 10.f;
  const float s = 1.f / steps;
  float t = .25f;
  float d = .0f;
  for (float i = .0f; i < steps; ++i) {
    d = map(__ctx, ro + t * rd);
    value = clamp(min(value, 8.f * d / t), .0f, 1.f);
    if (abs(d) < EPSILON * (1.f + .125f * d))
      break;
    t += d * STEP_BIAS;
  }
  return value;
}
const f32x3 lpos1 = f32x3(-1.f);
const f32x3 lcol1 = f32x3(.3f, .2f, .2f);
float rnd(Context &__ctx, f32x3 value) {
  float v = dot(value, f32x3(12.324f, 34.439f, 78.1858f));
  return fract(sin(v) * 783.94f + 82.2453f);
}
float raymarch(Context &__ctx, f32x3 ro, f32x3 rd, float &atmophere,
               float &attenuation) {
  float d = .0f;
  float t = .0f;
  atmophere = .0f;
  attenuation = 1.f;
  for (int i = 0; i < MAX_ITER; i++) {
    f32x3 p = ro + d * rd;
    float ldist = distance(p, lpos1 + .15f * rnd(__ctx, rd));
    float illum = 1.f / (ldist * ldist);
    f32x3 ldir = normalize(p - lpos1);
    f32x3 n = normal(__ctx, p);
    float sha = shadow(__ctx, p, normalize(p - lpos1));
    t = map(__ctx, p);
    atmophere += illum * .25f * attenuation * sha;
    if (abs(t) < EPSILON * (1.f + .125f * t)) {
      break;
    }
    attenuation *= .95f;
    d += t * STEP_BIAS;
  }
  return d;
}
f32x3 light(Context &__ctx, f32x3 light_pos, f32x3 light_col, f32x3 surf_pos,
            f32x3 surf_norm, f32x3 rd) {
  f32x3 light_dir = normalize(surf_pos - light_pos);
  float light_dist = distance(surf_pos, light_pos);
  float diffuse = max(.0f, dot(surf_norm, light_dir));
  float sha = shadow(__ctx, surf_pos - 0.01f * surf_norm, light_dir);
  f32x3 h = normalize(light_dir - rd);
  float spec = pow(max(dot(h, surf_norm), .0f), 4.f);
  return sha * diffuse * light_col * spec;
}
f32x3 shade(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 p, f32x3 n) {
  float attenuation = 1.0f;
  float atmosphere = 0.0f;
  f32x3 lpos2 = f32x3(.0f, -2.f, .0f);
  f32x3 lcol2 = f32x3(.6f);
  f32x3 lpos3 = f32x3(2.f, -1.5f, .0f);
  f32x3 lcol3 = f32x3(.1f, .4f, .8f);
  f32x3 lamb = f32x3(.1f);
  return lamb + light(__ctx, lpos1, lcol1, p, n, rd) +
         light(__ctx, lpos2, lcol2, p, n, rd) +
         light(__ctx, lpos3, lcol3, p, n, rd);
}
f32x3 camera(Context &__ctx, f32x2 uv, f32x3 ro, f32x3 aim, float zoom) {
  f32x3 camForward = normalize(aim - ro);
  f32x3 worldUp = f32x3(.0f, 1.0f, .0f);
  f32x3 camRight = normalize(cross(camForward, worldUp));
  f32x3 camUp = normalize(cross(camRight, camForward));
  f32x3 camCenter = normalize(ro + camForward * zoom);
  return normalize(camCenter + uv.data[0] * camRight + uv.data[1] * camUp - ro);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  uv = uv * 2.f - 1.f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  float camdist = 4.f;
  f32x3 ro = f32x3(camdist * cos(iTime), .8f + .4f * cos(2.f * iTime),
                   camdist * sin(iTime));
  f32x3 aim = f32x3(.0f, .2f, .0f);
  float zoom = 1.25f;
  f32x3 rd = camera(__ctx, uv, ro, aim, zoom);
  float atmophere = .0f;
  float attenuation = .0f;
  float d = raymarch(__ctx, ro, rd, atmophere, attenuation);
  float fog = 1.f / (1.f + d * d * .1f);
  f32x3 p = ro + d * rd;
  f32x3 n = normal(__ctx, p);
  f32x3 color = fog * shade(__ctx, ro, rd, p, n);
  color += atmophere * (lcol1);
  float alpha = 1.0f;
  color = color / (1.f + color);
  color += .025f * rnd(__ctx, rd);
  color *= f32x3(.975f, .9f, .825f);
  color = .25f * color + .75f * sqrt(color);
  color = smoothstep(f32x3(.0125f), f32x3(.9875f), color);
  fragColor = f32x4(color, alpha);
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
