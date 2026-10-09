// Port of https://www.shadertoy.com/view/dt2GRd; author: raymarchingenthusiast.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dt2GRd.json.
#include "../support.h"
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
  Texture3D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
struct Light {
  float ai;
  f32x3 pos;
  f32x3 col;
  float str;
};
struct Mat {
  int lt;
  f32x3 col;
  float di;
  float si;
  float sa;
};
f32x2 rot(Context &__ctx, f32x2 p, float a) {
  return f32x2(p.data[0] * cos(a) + p.data[1] * sin(a),
               p.data[0] * sin(a) - p.data[1] * cos(a));
}
f32x3 crd(Context &__ctx, f32x2 fragCoord, f32x3 iResolution, bool sideways) {
  f32x2 mouse =
      3.14159265f *
      ((f32x2(iResolution.data[0] * 3.f, .4f * iResolution.data[1] * 1.1f) /
        iResolution.swizzle<0, 1>()) -
       .5f);
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  if (.5f * iResolution.swizzle<0, 1>() == f32x2(0.f))
    mouse.set_swizzle<0, 1>(f32x2(3.14159265f / 2.f, 0.f));
  f32x3 rd = normalize(
      f32x3(uv.swizzle<0, 1>(), 1.f / tan(60.f * 3.14159265f / 360.f)));
  f32x2 rotAmt = f32x2(sideways ? -1.5f * 3.14159265f - mouse.data[0]
                                : 3.14159265f - mouse.data[0],
                       mouse.data[1] - 3.14159265f);
  rd = f32x3(rd.data[0], rot(__ctx, rd.swizzle<1, 2>(), rotAmt.data[1]));
  rd.data[1] = -rd.data[1];
  f32x2 rotRd = rot(__ctx, rd.swizzle<2, 0>(), rotAmt.data[0]);
  rd = f32x3(-rotRd.data[1], rd.data[1], rotRd.data[0]);
  return rd;
}
float maxabs3(Context &__ctx, f32x3 p) {
  float o = abs(p.data[0]) > abs(p.data[1]) ? p.data[0] : p.data[1];
  return abs(o) > abs(p.data[2]) ? o : p.data[2];
}
f32x3 v(Context &__ctx, float f) { return f32x3(f); }
float seed = 0.f;
float rand1(Context &__ctx, void) {
  seed += f32x3(48973.f, 3942067.f, 539.f).data[2];
  return mod(f32x3(48973.f, 3942067.f, 539.f).data[1] *
                     (seed - f32x3(48973.f, 3942067.f, 539.f).data[2]) +
                 f32x3(48973.f, 3942067.f, 539.f).data[2],
             f32x3(48973.f, 3942067.f, 539.f).data[0]) /
         f32x3(48973.f, 3942067.f, 539.f).data[0];
}
f32x2 rand2(Context &__ctx, void) { return f32x2(rand1(__ctx), rand1(__ctx)); }
void initRand(Context &__ctx, float time, f32x2 c) {
  seed = time + sin(time - c.data[0]) * c.data[0] -
         iResolution.data[1] * (cos(c.data[1] - time) * .5f + .5f);
}
f32x3 skybox(Context &__ctx, f32x3 rd, f32x3 cam, float time) {
  f32x3 skybox = f32x3(.5f, .8f, .95f);
  float sun = pow(dot(normalize(Light(.0f, f32x3(0.f, 120.f, -50.f),
                                      f32x3(1.f, .95f, .85f), 2.f)
                                    .pos),
                      rd) *
                          .5f +
                      .5f,
                  100.f) *
              2.f;
  f32x2 cloudMove = time / 15.f * f32x2(.1f, .3f);
  f32x3 cloud =
      texture(__ctx.channel1, f32x3(rd.swizzle<0, 2>() / (rd.data[1] * .5f) -
                                        cloudMove + cam.swizzle<0, 2>() / 25.f,
                                    time / 96.f))
          .swizzle<0, 0, 0>();
  cloud +=
      texture(__ctx.channel1, f32x3(rd.swizzle<0, 2>() / (rd.data[1] * 1.5f) -
                                        cloudMove + cam.swizzle<0, 2>() / 75.f,
                                    time / 96.f))
          .swizzle<0, 0, 0>() *
      3.f;
  cloud +=
      texture(__ctx.channel1, f32x3(rd.swizzle<0, 2>() / (rd.data[1] * 4.5f) -
                                        cloudMove + cam.swizzle<0, 2>() / 225.f,
                                    time / 96.f))
          .swizzle<0, 0, 0>() *
      9.f;
  cloud +=
      texture(__ctx.channel1, f32x3(rd.swizzle<0, 2>() / (rd.data[1] * 13.5f) -
                                        cloudMove + cam.swizzle<0, 2>() / 675.f,
                                    time / 96.f))
          .swizzle<0, 0, 0>() *
      27.f;
  cloud /= 40.f;
  cloud = 3.f * pow(cloud, f32x3(3.f));
  if (rd.data[1] > .03f) {
    skybox = mix(skybox, f32x3(1.f, .8f, .4f), clamp(sun, 0.f, 1.f));
    skybox = mix(skybox, f32x3(1.f), cloud);
  } else {
  }
  return skybox;
}
float sdSphere(Context &__ctx, f32x3 p, f32x4 sphere) {
  return length(p - sphere.swizzle<0, 1, 2>()) - sphere.data[3];
}
float sdCapsule(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.f, 1.f);
  return length(pa - ba * h) - r;
}
float sdBox(Context &__ctx, f32x3 p, f32x3 a, f32x4 b) {
  f32x3 q = abs(p - a) - b.swizzle<0, 1, 2>();
  return length(max(q, 0.f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.f) - b.data[3];
}
float sdBox(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 q = abs(p) - b;
  return length(max(q, 0.f)) + min(max(q.data[0], q.data[1]), 0.f);
}
float sdCross(Context &__ctx, f32x3 p) {
  float da =
      max(abs(p.swizzle<0, 1>()).data[0], abs(p.swizzle<0, 1>()).data[1]);
  float db =
      max(abs(p.swizzle<1, 2>()).data[0], abs(p.swizzle<1, 2>()).data[1]);
  float dc =
      max(abs(p.swizzle<2, 0>()).data[0], abs(p.swizzle<2, 0>()).data[1]);
  return min(da, min(db, dc)) - 1.f;
}
f32x2 map(Context &__ctx, f32x3 p, float ctime) {
  f32x3 q = (p - f32x3(0.f, .5f, 15.f)) / 9.f;
  float d0 = sdBox(__ctx, q, v(__ctx, 0.f), f32x4(v(__ctx, 1.f), 0.f)) * 9.f;
  float c = sdCross(__ctx, q * 3.f) / 3.f * 9.f;
  float s = 1.f;
  f32x3 l = p;
  for (float m = 0.f; m < 3.f; m++) {
    l += f32x3(.05f, .1f, -.15f) * pow(m, 3.f);
    f32x3 a = mod(l * s, 1.f) - .5f;
    s *= 3.f;
    f32x3 r = 1.f - 3.f * abs(a);
    float c = sdCross(__ctx, r * 2.f) / s / 2.f;
    d0 = max(d0, c);
  }
  float d1 = sdSphere(__ctx, p, f32x4(0.f, 0.f, 5.f, 2.f));
  float d2 = sdSphere(__ctx, p, f32x4(-1.5f, 3.f, 5.f, 1.5f));
  float d3 = sdSphere(__ctx, p, f32x4(1.5f, 3.f, 5.f, 1.5f));
  float d = min(d0, min(d1, min(d2, d3)));
  float mat;
  mat = 0.f;
  if (d == d1)
    mat = 1.f;
  if (d == d2 || d == d3)
    mat = 2.f;
  return f32x2(d, mat);
}
f32x3 getNormal(Context &__ctx, f32x3 p, float ctime) {
  float d0 = map(__ctx, p, ctime).data[0];
  f32x2 e = f32x2(.01f, 0.f);
  f32x3 n = d0 - f32x3(map(__ctx, p - e.swizzle<0, 1, 1>(), ctime).data[0],
                       map(__ctx, p - e.swizzle<1, 0, 1>(), ctime).data[0],
                       map(__ctx, p - e.swizzle<1, 1, 0>(), ctime).data[0]);
  return normalize(n);
}
f32x3 trimap(Context &__ctx, f32x3 p, f32x3 n, sampler2D s, float k, float rk) {
  n = pow(n, f32x3(k));
  f32x3 tx = texture(s, p.swizzle<1, 2>() / rk).swizzle<0, 1, 2>();
  f32x3 ty = texture(s, p.swizzle<0, 2>() / rk).swizzle<0, 1, 2>();
  f32x3 tz = texture(s, p.swizzle<0, 1>() / rk).swizzle<0, 1, 2>();
  return (tx * n.data[0] + ty * n.data[1] + tz * n.data[2]) /
         (n.data[0] + n.data[1] + n.data[2]);
}
f32x2 raymarch(Context &__ctx, f32x3 rd, f32x3 ro, bool &hit, float ctime) {
  float h = 0.f;
  hit = false;
  float m = -1.f;
  for (int i = 0; i < 1000; i++) {
    f32x3 p = ro + rd * h;
    f32x2 q = map(__ctx, p, ctime);
    float d = q.data[0];
    if (d < .01f) {
      hit = true;
      m = q.data[1];
      break;
    }
    if (h > 1000.f)
      break;
    h += d;
  }
  return f32x2(h, m);
}
float shadow(Context &__ctx, f32x3 p, f32x3 rd, float len, float ctime) {
  float ss = 1.f;
  float h = 0.f;
  for (int t = 0; t < 1000 && h < len; t++) {
    f32x3 cp = p + rd * h;
    float d = map(__ctx, cp, ctime).data[0];
    if (d < .01f)
      return 0.f;
    h += d;
  }
  return 1.f;
}
float diffuse(Context &__ctx, f32x3 p, f32x3 lp0, f32x3 n, float ctime) {
  f32x3 l = normalize(lp0);
  float ss = shadow(__ctx, p + n * .01f * 1.05f, l, length(lp0), ctime);
  float dif = dot(l, n);
  return max(dif, 0.f) * ss;
}
f32x3 randCone(Context &__ctx, void) {
  float phi = rand1(__ctx) * 6.283185f;
  float x = rand1(__ctx) * 2.f - 1.f;
  float z = rand1(__ctx);
  return pow(z, 1.f / 3.f) *
         f32x3(sqrt(1.f - x * x) * f32x2(sin(phi), cos(phi)), x);
}
f32x3 cosDir(Context &__ctx, f32x3 nor) {
  f32x2 uv = rand2(__ctx);
  float a = 6.283185f * uv.data[1];
  float b = 2.f * uv.data[0] - 1.f;
  f32x3 dir = f32x3(sqrt(1.f - b * b) * f32x2(cos(a), sin(a)), b);
  return normalize(nor + dir);
}
void setMat(Context &__ctx, f32x3 color, float fp, float g, float drr,
            f32x3 &scol, f32x3 &sdat) {
  scol = color;
  sdat = f32x3(fp, g, drr);
}
void worldGetMat(Context &__ctx, float id, f32x3 n, f32x3 &col, f32x3 &dat) {
  if (id == 0.f)
    setMat(__ctx, v(__ctx, .2f), 4.f, .9f, .7f, col, dat);
  if (id == 1.f)
    setMat(__ctx, f32x3(.2f, .15f, .05f), 2.f, .6f, .8f, col, dat);
  if (id == 2.f)
    setMat(__ctx, v(__ctx, .03f), 7.f, 1.f, .9f, col, dat);
}
f32x3 applyFog(Context &__ctx, f32x3 col, float dist, f32x3 rd, f32x3 ro,
               f32x3 ld) {
  float fa = (.01f / .01f) * exp(-ro.data[1] * .01f) *
             (1.f - exp(-dist * rd.data[1] * .01f)) / rd.data[1];
  float sa = max(dot(rd, ld), 0.f);
  f32x3 fc = mix(f32x3(.5f, .6f, .7f), f32x3(1.f, .9f, .7f), pow(sa, 8.f));
  return mix(col, fc, fa);
}
void getBRDFRay(Context &__ctx, f32x3 &ro, f32x3 &rd, f32x3 p, f32x3 n,
                f32x3 m) {
  ro = p + n * .01f * 1.05f;
  if (rand1(__ctx) < m.data[2]) {
    rd = cosDir(__ctx, n);
  } else {
    f32x3 rc = randCone(__ctx);
    float f = m.data[1] * clamp(pow(1.f - dot(-rd, n), m.data[0]), 0.f, 1.f);
    rd = normalize(reflect(rd, n) + rc * f);
  }
}
void getCam(Context &__ctx, float time, f32x3 &ro, f32x3 &uu, f32x3 &vv,
            f32x3 &ww) {
  time += 1.f;
  ro = f32x3(cos(time * 2.f) * 4.f, sin(time) * 2.f + 2.f,
             sin(time * 2.5f) * 4.f);
  uu = f32x3(1.f, 0.f, 0.f);
  vv = f32x3(0.f, 1.f, 0.f);
  ww = f32x3(0.f, 0.f, 1.f);
}
f32x3 rendererCalculateCol(Context &__ctx, f32x3 ro, f32x3 rd, float ctime,
                           f32x3 uu, f32x3 vv, f32x3 ww) {
  f32x3 tcol = v(__ctx, 0.f);
  f32x3 fcol = v(__ctx, 1.f);
  float td = 0.f;
  for (float i = 0.f; i < 2.f; i++) {
    bool hit = false;
    f32x2 q = raymarch(__ctx, rd, ro, hit, ctime);
    float d = q.data[0];
    f32x3 p = ro + rd * d;
    float mat;
    mat = q.data[1];
    f32x3 dcol;
    f32x3 scol;
    td += d;
    if (hit == true) {
      float tdReq = 30.f;
      tdReq += tdReq / i;
      if (td > tdReq)
        break;
      f32x3 n = getNormal(__ctx, p, ctime);
      f32x3 ld =
          Light(.0f, f32x3(0.f, 120.f, -50.f), f32x3(1.f, .95f, .85f), 2.f)
              .pos -
          p;
      float dif = diffuse(__ctx, p, ld, n, ctime);
      ld = normalize(ld);
      f32x3 sdat;
      worldGetMat(__ctx, mat, n, scol, sdat);
      f32x3 diffuse =
          (dif * .5f + .5f) *
          Light(.0f, f32x3(0.f, 120.f, -50.f), f32x3(1.f, .95f, .85f), 2.f)
              .str *
          Light(.0f, f32x3(0.f, 120.f, -50.f), f32x3(1.f, .95f, .85f), 2.f).col;
      dcol = diffuse;
      fcol *= scol;
      tcol += applyFog(__ctx, fcol * dcol, d, rd, ro, ld);
      getBRDFRay(__ctx, ro, rd, p, n, sdat);
    } else {
      f32x3 sky = skybox(__ctx, rd, ro, ctime);
      scol = sky;
      tcol += scol * fcol;
      break;
    }
  }
  return tcol;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  initRand(__ctx, float(iFrame), fragCoord);
  float spp = 1.f;
  float shutAp = .4f;
  float blurAmt = .0075f;
  float fcsDist = .65f;
  bool perfect = true;
  if (perfect) {
    shutAp = .0f;
    blurAmt = .0f;
    fcsDist = 100.f;
  }
  f32x3 qcol = v(__ctx, 0.f);
  for (float i = 0.f; i < spp; i++) {
    f32x2 of = -.5f + fract(rand2(__ctx));
    if (perfect)
      of = f32x2(0.f);
    f32x2 p = (2.0f * (fragCoord + of) - iResolution.swizzle<0, 1>()) /
              iResolution.data[1];
    f32x3 ro;
    f32x3 uu;
    f32x3 vv;
    f32x3 ww;
    float frameTime = iTime;
    float ctime = (iTime * 1.f) - 3.f + shutAp * (1.f / 24.f) * rand1(__ctx);
    getCam(__ctx, ctime, ro, uu, vv, ww);
    f32x3 er = normalize(f32x3(p, 1.f / tan(60.f * 3.14159265f / 360.f)));
    f32x3 rd = uu * er.data[0] + vv * er.data[1] + ww * er.data[2];
    f32x3 go = blurAmt * f32x3(2.f * rand2(__ctx) - 1.f, 0.f);
    f32x3 gd = normalize(er * fcsDist - go);
    ro += go.data[0] * uu + go.data[1] * vv;
    rd += gd.data[0] * uu + gd.data[1] * vv;
    f32x3 col =
        clamp(rendererCalculateCol(__ctx, ro, normalize(rd), ctime, uu, vv, ww),
              0.f, 1.f);
    qcol += col;
  }
  qcol /= spp;
  qcol = pow(qcol, v(__ctx, 1.f / 2.2f));
  fragColor = f32x4(qcol, 1.f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 1) texture_3d<f32x4> channel1,
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
  Context __ctx{{channel1, sampling1}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
