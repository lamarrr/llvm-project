// Port of https://www.shadertoy.com/view/Dls3D2; author: Shane.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Dls3D2-446c73334432.json.
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
  TextureCube channel0;
  Texture2D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
int objID;
f32x4 vID;
f32x2x2 rot2(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, -s, s, c);
}
float hash21(Context &__ctx, f32x2 p) {
  return fract(sin(dot(p, f32x2(27.609f, 57.583f))) * 43758.5453f);
}
f32x3 tex3D(Context &__ctx, sampler2D t, f32x3 p, f32x3 n) {
  n = max(abs(n) - .2f, .001f);
  n /= length(n);
  f32x3 tx = texture(t, p.swizzle<1, 2>()).swizzle<0, 1, 2>();
  f32x3 ty = texture(t, p.swizzle<2, 0>()).swizzle<0, 1, 2>();
  f32x3 tz = texture(t, p.swizzle<0, 1>()).swizzle<0, 1, 2>();
  return f32x3x3(tx * tx, ty * ty, tz * tz) * n;
}
f32x3 rotObj(Context &__ctx, f32x3 p) {
  if (iMouse.data[2] > 1.f) {
    p.set_swizzle<1, 2>(
        p.swizzle<1, 2>() *
        (rot2(__ctx, -(iMouse.data[1] - iResolution.data[1] * .5f) /
                         iResolution.data[1] * 3.1459f)));
    p.set_swizzle<0, 2>(
        p.swizzle<0, 2>() *
        (rot2(__ctx, -(iMouse.data[0] - iResolution.data[0] * .5f) /
                         iResolution.data[0] * 3.1459f)));
  }
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() *
                      (rot2(__ctx, iTime / 6.f / 4.f + 0.f)));
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (rot2(__ctx, iTime / 3.f / 2.f)));
  return p;
}
float sBoxS(Context &__ctx, f32x3 p, f32x3 b, float rf) {
  f32x3 d = abs(p) - b + rf;
  return min(max(max(d.data[0], d.data[1]), d.data[2]), 0.f) +
         length(max(d, 0.f)) - rf;
}
float sBoxS(Context &__ctx, f32x2 p, f32x2 b, float rf) {
  f32x2 d = abs(p) - b + rf;
  return min(max(d.data[0], d.data[1]), 0.f) + length(max(d, 0.f)) - rf;
}
float angle(Context &__ctx, f32x3 v1, f32x3 v2) {
  return acos(dot(v1, v2) / (length(v1) * length(v2)));
}
f32x3 sphericalToWorld(Context &__ctx, f32x3 sphCoord) {
  f32x4 cs =
      f32x4(cos(sphCoord.swizzle<0, 1>()), sin(sphCoord.swizzle<0, 1>()));
  return f32x3(cs.data[3] * cs.data[0], cs.data[1], cs.data[3] * cs.data[2]) *
         sphCoord.data[2];
}
const f32x3 triV0 = normalize(f32x3(-1, 1.6180339887498948482f, 0));
const f32x3 triV1 = normalize(f32x3(-1.6180339887498948482f, 0, 1));
const f32x3 triV2 = normalize(f32x3(0, 1, 1.6180339887498948482f));
const f32x3 mid = normalize(mix(triV1, triV2, .5f));
const f32x3 cntr = normalize(triV0 + triV1 + triV2);
const float ang = acos(dot(triV0, triV1));
const float mAng = acos(dot(triV0, mid));
const float cAng = acos(dot(triV0, cntr));
f32x3 pentCntr = f32x3(0);
int pID;
f32x3 getDodecPent(Context &__ctx, f32x3 &q, inout f32x3[5] gVertID,
                   const float rad) {
  const float scX = 5.f;
  f32x4 sph = mod(atan(q.data[2], q.data[0]) +
                      f32x4((6.283185307179586f * .5f) / 5.f,
                            (6.283185307179586f * .5f) / 5.f, 0, 0),
                  6.283185307179586f);
  sph = mod((floor(sph * scX / 6.283185307179586f) + f32x4(0, 0, .5f, .5f)) /
                scX * 6.283185307179586f,
            6.283185307179586f);
  f32x4 pLat = f32x4(0, ang, (6.283185307179586f * .5f) - ang,
                     (6.283185307179586f * .5f));
  float dist = 1e5f;
  int id;
  for (int i = 0; i < 4; i++) {
    f32x3 sc = f32x3(sph[i], pLat[i], rad);
    f32x3 wc = sphericalToWorld(__ctx, sc);
    float vDist = length(q - wc);
    if (vDist < dist) {
      dist = vDist;
      id = i;
      pentCntr = sc;
    }
  }
  float ax = sph[id];
  f32x3 vLat = f32x3(cAng, 2.f * mAng - cAng,
                     (6.283185307179586f * .5f) - (2.f * mAng - cAng));
  if (id == 0 || id == 3) {
    float xOff = (6.283185307179586f * .5f) / 5.f;
    if (id == 3) {
      vLat = (6.283185307179586f * .5f) - vLat;
      xOff = 0.f;
    }
    gVertID[0] = f32x3(xOff, vLat.data[0], rad);
    gVertID[1] =
        f32x3(2.f * (6.283185307179586f * .5f) / 5.f + xOff, vLat.data[0], rad);
    gVertID[2] =
        f32x3(4.f * (6.283185307179586f * .5f) / 5.f + xOff, vLat.data[0], rad);
    gVertID[3] =
        f32x3(6.f * (6.283185307179586f * .5f) / 5.f + xOff, vLat.data[0], rad);
    gVertID[4] =
        f32x3(8.f * (6.283185307179586f * .5f) / 5.f + xOff, vLat.data[0], rad);
  } else {
    if (id == 2) {
      vLat = (6.283185307179586f * .5f) - vLat;
    }
    f32x3 ax3 =
        mod(f32x3(ax + 6.283185307179586f - (6.283185307179586f * .5f) / 5.f,
                  ax + (6.283185307179586f * .5f) / 5.f, ax),
            6.283185307179586f);
    gVertID[0] = f32x3(ax3.data[0], vLat.data[1], rad);
    gVertID[1] = f32x3(ax3.data[0], vLat.data[0], rad);
    gVertID[2] = f32x3(ax3.data[1], vLat.data[0], rad);
    gVertID[3] = f32x3(ax3.data[1], vLat.data[1], rad);
    gVertID[4] = f32x3(ax3.data[2], vLat.data[2], rad);
  }
  if (id == 0 || id == 3)
    pentCntr.data[0] = 0.f;
  pID = id;
  return q;
}
float sphereLineDistCapBWave(Context &__ctx, f32x3 p, f32x3 a, f32x3 b,
                             float ang) {
  p = normalize(p);
  float ln = dot(p, cross(a, b)) / length(a - b);
  f32x3 perpB = normalize(b + cross((a - b), b)) * .5f;
  float endB = dot(p, cross(perpB, b)) / length(perpB - b);
  ln -= sin(6.2831f * 2.f * (endB) + 0.125f) * .06f;
  return sign(ln) * max(abs(ln), endB);
}
f32x3 sphPos = f32x3(0);
float map(Context &__ctx, f32x3 p) {
  f32x3 q = rotObj(__ctx, p - sphPos);
  p.data[2] -= dot(sin(p.swizzle<0, 1>() * 1.f - cos(p.swizzle<1, 0>() * 2.f)),
                   f32x2(.05f));
  float wall = -p.data[2] + 3.f;
  f32x3[5] vP, vPID;
  const float rad = .5f;
  f32x3 lq = getDodecPent(__ctx, q, vPID, rad);
  for (int i = 0; i < 5; i++) {
    vP[i] = sphericalToWorld(__ctx, vPID[i]);
  }
  f32x3 vPCntr = sphericalToWorld(__ctx, pentCntr);
  float crv = 1e5f, crv2 = 1e5f;
  for (int i = min(0, iFrame); i < 5; i++) {
    int ip1 = (i + 1) % 5;
    f32x3 vMid1 = normalize(mix(vP[i], vP[ip1], .5f)) * rad;
    float angR = angle(__ctx, lq, vPCntr);
    float line = sphereLineDistCapBWave(__ctx, lq, vMid1, vPCntr, angR);
    f32x2 lv = f32x2(line, (length(lq) - .5f));
    lv = rot2(__ctx,
              smoothstep(.25f, 1.f, angR / cAng * 1.15f) * 3.14159f / 4.f) *
         lv;
    float bx = sBoxS(__ctx, lv, f32x2(.04f), .0f);
    float sdBox = length(abs(lv) - .045f) - .0085f;
    sdBox += sin(angR * cAng * 256.f + 3.14159f) * .0005f;
    crv = min(crv, bx);
    crv2 = min(crv2, sdBox);
  }
  float sc = .45f;
  f32x3 qq = p - f32x3(0, 0, 3.f - .02f);
  qq.set_swizzle<0, 1>(qq.swizzle<0, 1>() * (rot2(__ctx, -3.14159f / 5.f)));
  qq.set_swizzle<0, 1>(
      qq.swizzle<0, 1>() +
      (sin(qq.swizzle<0, 1>() * 1.f - cos(qq.swizzle<1, 0>() * 2.f)) * .05f));
  qq.data[1] = mod(qq.data[1], sc) - sc / 2.f;
  float bgLn = length(qq.swizzle<1, 2>()) - .03f;
  bgLn += sin(6.2831f * qq.data[0] * 16.f) * .002f;
  crv2 = min(crv2, bgLn);
  float sph = length(lq - vPCntr * 1.115f) - .0325f;
  vID = f32x4(sph, wall, crv, crv2);
  return min(min(sph, wall), min(crv, crv2));
}
float trace(Context &__ctx, f32x3 ro, f32x3 rd) {
  float t = 0.f, d;
  for (int i = min(iFrame, 0); i < 96; i++) {
    d = map(__ctx, ro + rd * t);
    if (abs(d) < .001f || t > 20.f)
      break;
    t += d * .75f;
  }
  return min(t, 20.f);
}
f32x3 getNormal(Context &__ctx, f32x3 p, float t) {
  float sgn = 1.f;
  f32x3 e = f32x3(.001f, 0, 0), mp = e.swizzle<2, 2, 2>();
  for (int i = min(iFrame, 0); i < 6; i++) {
    mp.data[0] += map(__ctx, p + sgn * e) * sgn;
    sgn = -sgn;
    if ((i & 1) == 1) {
      mp = mp.swizzle<1, 2, 0>();
      e = e.swizzle<2, 0, 1>();
    }
  }
  return normalize(mp);
}
float softShadow(Context &__ctx, f32x3 ro, f32x3 lp, f32x3 n, float k) {
  const int maxIterationsShad = 24;
  ro += n * .0015f;
  f32x3 rd = lp - ro;
  float shade = 1.f;
  float t = 0.f;
  float end = max(length(rd), 0.0001f);
  rd /= end;
  for (int i = min(iFrame, 0); i < maxIterationsShad; i++) {
    float d = map(__ctx, ro + rd * t);
    shade = min(shade, k * d / t);
    t += clamp(d, .01f, .25f);
    if (d < 0.f || t > end)
      break;
  }
  return max(shade, 0.f);
}
float calcAO(Context &__ctx, f32x3 p, f32x3 n) {
  float sca = 2.f, occ = 0.f;
  for (int i = min(0, iFrame); i < 5; i++) {
    float hr = float(i + 1) * .15f / 5.f;
    float d = map(__ctx, p + n * hr);
    occ += (hr - d) * sca;
    sca *= .7f;
  }
  return clamp(1.f - occ, 0.f, 1.f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float fBlend = 0.f;
  f32x2 uv =
      (fragCoord - iResolution.swizzle<0, 1>() * .5f) / iResolution.data[1];
  f32x3 lk = f32x3(0, 0, 0);
  f32x3 ro = lk + f32x3(cos(iTime / 3.f) * .1f, .1f, -2);
  f32x3 lp = ro + f32x3(.25f, .75f, -1);
  float FOV = .75f;
  f32x3 fwd = normalize(lk - ro);
  f32x3 rgt = normalize(f32x3(fwd.data[2], 0.f, -fwd.data[0]));
  f32x3 up = cross(fwd, rgt);
  f32x3 rd = normalize(uv.data[0] * rgt + uv.data[1] * up + fwd / FOV);
  float t = trace(__ctx, ro, rd);
  objID = 0;
  float obD = vID[0];
  for (int i = 0; i < 4; i++) {
    if (vID[i] < obD) {
      obD = vID[i];
      objID = i;
    }
  }
  f32x3 col = f32x3(0);
  if (t < 20.f) {
    f32x3 sp = ro + rd * t;
    f32x3 sn = getNormal(__ctx, sp, t);
    f32x3 ld = lp - sp;
    float lDist = max(length(ld), .001f);
    ld /= lDist;
    float sh = softShadow(__ctx, sp, lp, sn, 8.f);
    float ao = calcAO(__ctx, sp, sn);
    float atten = 1.f / (1.f + lDist * .05f);
    float diff = max(dot(sn, ld), 0.f);
    float spec = pow(max(dot(reflect(ld, sn), rd), 0.f), 32.f);
    float fre = pow(clamp(1.f + dot(sn, rd), 0.f, 1.f), 2.f);
    float Schlick = pow(1.f - max(dot(rd, normalize(rd + ld)), 0.f), 5.f);
    float freS = mix(.2f, 1.f, Schlick);
    f32x3 texCol = f32x3(0);
    if (objID == 0) {
      texCol = mix(f32x3(1, .3f, .1f), f32x3(1, .1f, .2f),
                   abs(sn.data[1]) * .5f + .25f);
      texCol = max((sn.swizzle<0, 2, 1>()) * .35f + .65f, .1f) * texCol * 1.2f;
      f32x3 txP = sp - sphPos;
      f32x3 txN = sn;
      txP = rotObj(__ctx, txP);
      txN = rotObj(__ctx, txN);
      f32x3 tx = tex3D(__ctx, __ctx.channel1, txP, txN);
      texCol *= tx * 2.f;
    } else if (objID == 1) {
      texCol = mix(f32x3(1, .5f, .25f), f32x3(.175f), .9f);
      float sc = .45f;
      f32x3 qq = sp;
      qq.set_swizzle<0, 1>(qq.swizzle<0, 1>() * (rot2(__ctx, -3.14159f / 5.f)));
      qq.set_swizzle<0, 1>(
          qq.swizzle<0, 1>() +
          (sin(qq.swizzle<0, 1>() * 1.f - cos(qq.swizzle<1, 0>() * 2.f)) *
           .05f));
      float idy = floor(qq.data[1] / sc + .5f);
      qq.data[1] -= (idy + .5f) * sc;
      if (mod(idy, 2.f) < .5f)
        texCol = mix(f32x3(1, .5f, .25f), f32x3(.175f), .825f);
      f32x3 tx = texture(__ctx.channel1, sp.swizzle<0, 1>() / 3.f + .5f)
                     .swizzle<0, 1, 2>();
      tx *= tx;
      texCol *= (tx + .75f) * .9f;
    } else {
      texCol = mix(f32x3(1, .3f, .1f), f32x3(1, .1f, .2f),
                   abs(sn.data[1]) * .5f + .25f);
      if (objID == 3) {
        texCol =
            max((sn.swizzle<0, 2, 1>()) * .35f + .65f, .1f) * texCol * 1.2f;
        if (sp.data[2] < 2.5f) {
          diff *= sqrt(diff);
          texCol = mix(mix(texCol, f32x3(1, .5f, .25f) / 2.f, .8f),
                       f32x3(1) * dot(texCol, f32x3(.299f, .587f, .114f)), .2f);
        }
      }
      if (sp.data[2] > 2.5f) {
        f32x3 tx = tex3D(__ctx, __ctx.channel1, sp / 3.f + .5f, sn);
        texCol = mix(f32x3(1, .5f, .25f) / 2.f,
                     f32x3(1) * dot(texCol, f32x3(.299f, .587f, .114f)), .65f) *
                 .5f;
        texCol *= (tx + .75f) * .9f;
      } else {
        f32x3 txP = sp - sphPos;
        f32x3 txN = sn;
        txP = rotObj(__ctx, txP);
        txN = rotObj(__ctx, txN);
        f32x3 tx = tex3D(__ctx, __ctx.channel1, txP, txN);
        texCol *= tx * 2.f;
      }
    }
    float speR = pow(max(dot(normalize(ld - rd), sn), 0.f), 8.f);
    f32x3 rf = reflect(rd, sn);
    f32x3 rTx = texture(__ctx.channel0, rf).swizzle<0, 1, 2>();
    rTx *= rTx;
    float spF = objID == 2 ? 2.f : 3.f;
    if (objID != 1)
      texCol += (texCol * .9f + .1f) * speR * rTx * spF;
    col = texCol *
          (diff * sh + .15f + f32x3(1, .9f, .7f) * spec * freS * sh * 8.f +
           f32x3(.2f, .4f, 1) * fre * sh * 0.f);
    col *= ao * atten;
  }
  fragColor = f32x4(sqrt(max(col, 0.f)), 1);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_cube<f32x4> channel0,
               __binding(1, 0) sampler sampling0,
               __binding(0, 1) texture_2d<f32x4> channel1,
               __binding(1, 1) sampler sampling1) {
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
