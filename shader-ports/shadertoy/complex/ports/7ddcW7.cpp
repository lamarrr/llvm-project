// Port of https://www.shadertoy.com/view/7ddcW7; author: tqs.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7ddcW7.json.
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
  Texture2D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
const float freqA = .15f / 3.75f;
const float freqB = .25f / 2.75f;
const float ampA = 20.f;
const float ampB = 2.f;
f32x2x2 rot2(Context &__ctx, float th) {
  f32x2 a = sin(f32x2(1.5707963f, 0) + th);
  return f32x2x2(a, -a.data[1], a.data[0]);
}
float hash(Context &__ctx, float n) { return fract(cos(n) * 45758.5453f); }
float hash(Context &__ctx, f32x3 p) {
  return fract(sin(dot(p, f32x3(7, 157, 113))) * 45758.5453f);
}
float getGrey(Context &__ctx, f32x3 p) {
  return dot(p, f32x3(0.299f, 0.587f, 0.114f));
}
float sminP(Context &__ctx, float a, float b, float s) {
  float h = clamp(.5f + .5f * (b - a) / s, 0.f, 1.f);
  return mix(b, a, h) - h * (1.0f - h) * s;
}
float smaxP(Context &__ctx, float a, float b, float s) {
  float h = clamp(.5f + .5f * (a - b) / s, 0.f, 1.f);
  return mix(b, a, h) + h * (1.f - h) * s;
}
f32x2 path(Context &__ctx, float z) {
  return f32x2(ampA * sin(z * freqA),
               ampB * cos(z * freqB) + 5.f * (sin(z * 0.025f) - 5.f));
}
float map(Context &__ctx, f32x3 p) {
  float tx =
      (-textureLod(__ctx.channel0,
                   p.swizzle<0, 2>() / 22.f + p.swizzle<0, 1>() / 80.f, 0.f)
            .data[0] -
       0.4f) *
      0.95f;
  f32x3 q =
      sin(p.swizzle<2, 1, 0>() * .2f - (0.5f + tx) * 0.5f + iTime * 0.13f) *
      sminP(__ctx, 5.0f, pow((0.5f - tx) * 5.5f, 0.22f), 3.f);
  float h = q.data[0] * q.data[1] * q.data[2];
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() - (path(__ctx, p.data[2])));
  float tnl = 2.0f - length(p.swizzle<0, 1>() * f32x2(.33f, .66f)) * 0.5f +
              h * 0.4f + (1.f - tx) * .25f;
  return tnl - tx * .5f + tnl * 0.3f;
}
float getprot(Context &__ctx, f32x3 p) {
  float tx =
      (-textureLod(__ctx.channel0,
                   p.swizzle<0, 2>() / 22.f + p.swizzle<0, 1>() / 80.f, 0.f)
            .data[0] -
       0.4f) *
      0.95f;
  f32x3 q =
      sin(p.swizzle<2, 1, 0>() * .2f - (0.5f + tx) * 0.5f + iTime * 0.13f) *
      sminP(__ctx, 5.0f, pow((0.5f - tx) * 5.5f, 0.22f), 3.f);
  return q.data[0] * q.data[1] * q.data[2];
}
float logBisectTrace(Context &__ctx, f32x3 ro, f32x3 rd) {
  float t = 0.f, told = 0.f, mid, dn;
  float d = map(__ctx, rd * t + ro);
  float sgn = sign(d);
  for (int i = 0; i < 128; i++) {
    if (sign(d) != sgn || d < 0.001f || t > 125.f)
      break;
    told = t;
    t += step(d, 1.f) * (log(abs(d) + 1.1f) - d) + d;
    d = map(__ctx, rd * t + ro);
  }
  if (sign(d) != sgn) {
    dn = sign(map(__ctx, rd * told + ro));
    f32x2 iv = f32x2(told, t);
    for (int ii = 0; ii < 8; ii++) {
      mid = dot(iv, f32x2(.5f));
      float d = map(__ctx, rd * mid + ro);
      if (abs(d) < 0.001f)
        break;
      iv = mix(f32x2(iv.data[0], mid), f32x2(mid, iv.data[1]),
               step(0.0f, d * dn));
    }
    t = mid;
  }
  return min(t, 125.f);
}
f32x3 normal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(-1, 1) * .001f;
  return normalize(e.swizzle<1, 0, 0>() * map(__ctx, p + e.swizzle<1, 0, 0>()) +
                   e.swizzle<0, 0, 1>() * map(__ctx, p + e.swizzle<0, 0, 1>()) +
                   e.swizzle<0, 1, 0>() * map(__ctx, p + e.swizzle<0, 1, 0>()) +
                   e.swizzle<1, 1, 1>() * map(__ctx, p + e.swizzle<1, 1, 1>()));
}
f32x3 tex3D(Context &__ctx, sampler2D tex, f32x3 p, f32x3 n) {
  n = max(n * n, .001f);
  n /= (n.data[0] + n.data[1] + n.data[2]);
  return (texture(tex, p.swizzle<1, 2>()) * n.data[0] +
          texture(tex, p.swizzle<2, 0>()) * n.data[1] +
          texture(tex, p.swizzle<0, 1>()) * n.data[2])
      .swizzle<0, 1, 2>();
}
f32x3 doBumpMap(Context &__ctx, sampler2D tex, f32x3 p, f32x3 nor,
                float bumpfactor) {
  const float eps = .001f;
  f32x3 grad = f32x3(
      getGrey(__ctx, tex3D(__ctx, tex,
                           f32x3(p.data[0] - eps, p.data[1], p.data[2]), nor)),
      getGrey(__ctx, tex3D(__ctx, tex,
                           f32x3(p.data[0], p.data[1] - eps, p.data[2]), nor)),
      getGrey(__ctx, tex3D(__ctx, tex,
                           f32x3(p.data[0], p.data[1], p.data[2] - eps), nor)));
  grad = (grad - getGrey(__ctx, tex3D(__ctx, tex, p, nor))) / eps;
  grad -= nor * dot(nor, grad);
  return normalize(nor + grad * bumpfactor);
}
float calculateAO(Context &__ctx, f32x3 p, f32x3 n, float maxDist) {
  float ao = 0.f, l;
  const float nbIte = 6.f;
  for (float i = 1.f; i < nbIte + .5f; i++) {
    l = (i + hash(__ctx, i)) * .5f / nbIte * maxDist;
    ao += (l - map(__ctx, p + n * l)) / (1.f + l);
  }
  return clamp(1.f - ao / nbIte, 0.f, 1.f);
}
float curve(Context &__ctx, f32x3 p) {
  const float eps = .05f, amp = 4.f, ampInit = .5f;
  f32x2 e = f32x2(-1, 1) * eps;
  float t1 = map(__ctx, p + e.swizzle<1, 0, 0>()),
        t2 = map(__ctx, p + e.swizzle<0, 0, 1>());
  float t3 = map(__ctx, p + e.swizzle<0, 1, 0>()),
        t4 = map(__ctx, p + e.swizzle<1, 1, 1>());
  return clamp((t1 + t2 + t3 + t4 - 4.f * map(__ctx, p)) * amp + ampInit, 0.f,
               1.f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 u =
      (fragCoord - iResolution.swizzle<0, 1>() * .5f) / iResolution.data[1];
  f32x3 lookAt = f32x3(0, 0, 18.f + iTime * 1.9f);
  f32x3 ro = lookAt + f32x3(0, .0f, -.25f);
  lookAt.set_swizzle<0, 1>(lookAt.swizzle<0, 1>() +
                           (path(__ctx, lookAt.data[2])));
  ro.set_swizzle<0, 1>(ro.swizzle<0, 1>() + (path(__ctx, ro.data[2])));
  float FOV = 3.14159f / 1.5f;
  f32x3 forward = normalize(lookAt - ro);
  f32x3 right = normalize(f32x3(forward.data[2], 0, -forward.data[0]));
  f32x3 up = cross(forward, right);
  f32x3 rd =
      normalize(forward + FOV * u.data[0] * right + FOV * u.data[1] * up);
  rd.set_swizzle<0, 1>(rot2(__ctx, path(__ctx, lookAt.data[2]).data[0] / 64.f) *
                       rd.swizzle<0, 1>());
  f32x3 lp = f32x3(125.f * .5f, 125.f, 125.f) + f32x3(0, 0, ro.data[2]);
  float t = logBisectTrace(__ctx, ro, rd);
  f32x3 fog = f32x3(1, .9f, .7f);
  f32x3 col = fog;
  if (t < 125.f) {
    f32x3 sp = ro + t * rd;
    f32x3 sn = normal(__ctx, sp);
    f32x3 ld = lp - sp;
    ld /= max(length(ld), 0.001f);
    const float tSize1 = 1.f / 6.f;
    sn = doBumpMap(__ctx, __ctx.channel1, sp * tSize1, sn,
                   .007f / (1.f + t / 125.f));
    float shd = 0.9f;
    float curv = curve(__ctx, sp) * .7f + .1f;
    float ao = calculateAO(__ctx, sp, sn, 10.f);
    float dif = max(dot(ld, sn), 0.f);
    float spe = pow(max(dot(reflect(-ld, sn), -rd), 0.f), 5.f) * 2.0f;
    float fre = clamp(1.0f + dot(rd, sn), 0.f, 1.f);
    float Schlick = pow(1.f - max(dot(rd, normalize(rd + ld)), 0.f), 1.5f);
    float fre2 = mix(.2f, 1.f, Schlick);
    float amb = fre * fre2 + .76f * ao;
    col = clamp(mix(f32x3(.5f, .4f, .3f), f32x3(.25f, .45f, .125f),
                    (sp.data[1] + 1.f) * .15f),
                f32x3(.25f, .45f, .125f), f32x3(1));
    curv = smoothstep(0.f, .7f, curv);
    col *= f32x3(curv * 0.9f, curv * .45f, curv * .25f) * 1.0f;
    float prot = smoothstep(-0.0f, -3.0f, getprot(__ctx, sp));
    col = mix(col, f32x3(0.9f, 0.6f, 0.4f), prot);
    col = (col * (dif + .1f) + fre2 * spe) * shd * ao + amb * col;
  }
  col = mix(col, fog, sqrt(smoothstep(125.f - 85.f, 125.f, t)));
  col = pow(max(col, 0.f), f32x3(.85f));
  u = fragCoord / iResolution.swizzle<0, 1>();
  col *=
      pow(16.f * u.data[0] * u.data[1] * (1.f - u.data[0]) * (1.f - u.data[1]),
          .0625f);
  fragColor = f32x4(clamp(col, 0.f, 1.f), 1);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0,
               __binding__(0, 1) texture_2d<f32x4> channel1,
               __binding__(1, 1) sampler sampling1) {
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
  Context __ctx{{channel0, sampling0}, {channel1, sampling1}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
