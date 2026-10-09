// Port of https://www.shadertoy.com/view/dlXSWn; author: dr2.
// Dataset license metadata: cc-by-nc-sa-3.0 AND proprietary-license; original
// retained in upstream/dlXSWn.json.
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
struct Context {};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float Minv2(Context &__ctx, f32x2 p);
float Maxv2(Context &__ctx, f32x2 p);
float Maxv3(Context &__ctx, f32x3 p);
float PrBoxDf(Context &__ctx, f32x3 p, f32x3 b);
float PrBox2Df(Context &__ctx, f32x2 p, f32x2 b);
float PrRoundBoxDf(Context &__ctx, f32x3 p, f32x3 b, float r);
float PrSphDf(Context &__ctx, f32x3 p, float r);
float SmoothMin(Context &__ctx, float a, float b, float r);
float SmoothMax(Context &__ctx, float a, float b, float r);
float SmoothBump(Context &__ctx, float lo, float hi, float w, float x);
f32x3x3 StdVuMat(Context &__ctx, f32x2 vAzEl);
f32x2 Rot2D(Context &__ctx, f32x2 q, float a);
f32x2 Rot2Cs(Context &__ctx, f32x2 q, f32x2 cs);
f32x3 RotVF(Context &__ctx, f32x3 v, f32x4 cs);
f32x3 RotVB(Context &__ctx, f32x3 v, f32x4 cs);
f32x3 HsvToRgb(Context &__ctx, f32x3 c);
float Fbm1(Context &__ctx, float p);
float Fbm2(Context &__ctx, f32x2 p);
f32x3 VaryNf(Context &__ctx, f32x3 p, f32x3 n, float f);
f32x3 sunDir, qHit, doorSz, cutVec, bmOrg, bmCol;
f32x2 bmAzEl, drAngCs, nSeg;
float tCur, dstFar, domeRd, domeBs, oculRd, wallRd, wallThk, wallHt, bmRd;
int idObj, dirCut;
bool disco;
const int idDomeI = 1, idDomeM = 2, idDomeO = 3, idDomeOx = 4, idWallin = 5,
          idWallex = 6, idCeil = 7, idFlor = 8, idPort = 9, idPortRf = 10,
          idGrt = 11, idPil = 12, idPlin = 13, idBall = 14, idEnt = 15,
          idDoor = 16, idBase = 17, idBmSup = 18, idBmSrc = 19;
const float pi = 3.1415927f;
void Cut(Context &__ctx, f32x3 p, float &d) {
  if (dirCut > 0)
    d = max(d, dot(p, cutVec));
}
float ObjDf(Context &__ctx, f32x3 p) {
  f32x3 q;
  f32x2 a, b1, b2, dmx[3], u;
  float dMin, d, dd, r, rh, t;
  dMin = dstFar;
  rh = length(p.swizzle<0, 2>());
  r = length(p - f32x3(0.f, domeBs, 0.f));
  q = p;
  q.data[1] -= domeBs;
  a = 2.f * pi *
      (floor(nSeg * atan(q.swizzle<2, 1>(), -f32x2(q.data[0], rh)) /
             (2.f * pi)) +
       0.5f) /
      nSeg;
  b1 = Rot2Cs(__ctx, q.swizzle<0, 2>(), sin(a.data[0] + f32x2(0.5f * pi, 0.f)));
  b1.data[1] = abs(b1.data[1]);
  b2 = Rot2Cs(__ctx, f32x2(rh, q.data[1]),
              sin(a.data[1] + f32x2(0.5f * pi, 0.f)));
  b2.data[1] = abs(b2.data[1]);
  for (int k = 0; k < 3; k++) {
    u = sin(0.1f * float(k + 1) * 2.f * pi / nSeg + f32x2(0.f, 0.5f * pi));
    dmx[k] = f32x2(dot(b1, u), dot(b2, u));
  }
  t = domeRd - r;
  d = max(min(max(abs(t - 0.06f) - 0.01f, Minv2(__ctx, dmx[0])),
              max(abs(t - 0.04f) - 0.04f, rh - 0.43f)),
          max(oculRd - rh, -0.01f - q.data[1]));
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idDomeI;
  };
  d = max(min(max(abs(t - 0.045f) - 0.01f, Minv2(__ctx, dmx[1])),
              max(abs(t - 0.03f) - 0.01f, Minv2(__ctx, dmx[2]))),
          max(0.42f - rh, -q.data[1]));
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idDomeM;
  };
  dd = domeRd + 0.45f - length(f32x3(q.data[0], q.data[1] + 0.42f, q.data[2]));
  d = max(-min(dd, q.data[1]), t - 0.02f);
  d = max(max(d, oculRd - rh), min(1.6f - rh, 0.2f - abs(q.data[1] + 0.2f)));
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idDomeO;
  };
  d = max(min(max(abs(dd) - 0.02f, max(oculRd - rh, dmx[0].data[0] - 0.005f)),
              max(abs(dd) - 0.025f, abs(oculRd + 0.05f - rh) - 0.05f)),
          -q.data[1]);
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idDomeOx;
  };
  d = max(min(min(min(max(abs(0.15f + dd) - 0.05f, q.data[1] - 0.44f),
                      max(abs(0.1f + dd) - 0.05f, q.data[1] - 0.48f)),
                  max(abs(0.05f + dd) - 0.05f, q.data[1] - 0.52f)),
              max(abs(dd) - 0.05f, q.data[1] - 0.56f)),
          -q.data[1] + 0.3f);
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idDomeOx;
  };
  q = p;
  d = max(max(rh - wallRd + wallThk, domeRd - rh - 0.025f),
          abs(q.data[1] - domeBs + 0.13f) - 0.27f);
  dd = length(f32x2(Minv2(__ctx, abs(q.swizzle<0, 2>())),
                    q.data[1] - domeBs + 0.52f)) -
       0.2f;
  d = max(d, -max(rh - wallRd + wallThk + 0.03f, dd));
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idCeil;
  };
  d = max(max(max(abs(domeRd - rh - 0.02f) - 0.04f,
                  abs(q.data[1] - domeBs + 0.2f) - 0.2f),
              min(dmx[1].data[0], 0.09f - abs(q.data[1] - domeBs + 0.15f))),
          -dd);
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idWallin;
  };
  d = max(abs(wallRd - wallThk - rh) - 0.03f,
          abs(q.data[1] - domeBs + 0.86f) - 0.54f);
  if (q.data[2] > 0.f && abs(q.data[0]) < 0.23f) {
    t = min(0.22f - abs(q.data[0]), -q.data[1] + domeBs - 0.54f);
  } else {
    d = max(
        d, min(dmx[1].data[0] - 0.03f, 0.25f - abs(q.data[1] - domeBs + 0.8f)));
    u = (abs(q.data[0]) > abs(q.data[2])) ? q.swizzle<0, 2>()
                                          : q.swizzle<2, 0>();
    t = 0.15f - length(f32x3(abs(u.data[0]) - wallRd + wallThk + 0.05f,
                             max(0.f, q.data[1] - domeBs + 0.59f), u.data[1]));
  }
  d = max(d, t);
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idWallin;
  };
  d = max(
      max(abs(wallRd - rh) - wallThk, abs(q.data[1] - wallHt - 0.03f) - wallHt),
      t);
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idWallex;
  };
  d = max(length(f32x2(rh - wallRd - wallThk,
                       abs(abs(q.data[1] - wallHt - 0.03f) - 0.6f) - 0.27f)) -
              0.02f,
          min(q.data[2], 1.f - abs(q.data[0])));
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idWallex;
  };
  q = p;
  q.data[1] -= 0.51f;
  d = max(max(abs(wallRd - rh - 0.01f) - wallThk - 0.07f,
              length(f32x2(q.data[0], max(q.data[1] - 0.28f, 0.f))) - 0.24f),
          -0.45f - q.data[1]);
  d = max(max(d, -max(abs(q.data[0]) - 2.f * doorSz.data[0] - 0.01f,
                      abs(q.data[1]) - doorSz.data[1])),
          -q.data[2]);
  dd = d;
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idEnt;
  };
  t = 0.97f / 12.f;
  q = p;
  q.data[1] -= 0.09f;
  d = max(rh - wallRd - wallThk, abs(q.data[1]) - 0.05f);
  d = max(d, min(t - rh, q.data[1] + 0.01f));
  d = max(d, -dd);
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idFlor;
  };
  q = p;
  d = max(max(rh - t, abs(q.data[1] - 0.13f) - 0.01f),
          min(0.3f * t - abs(0.6f * t - rh), dmx[2].data[0]));
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idGrt;
  };
  if (p.data[2] < 0.3f) {
    q = p;
    q.set_swizzle<0, 2>((abs(q.data[0]) > abs(q.data[2])) ? q.swizzle<0, 2>()
                                                          : q.swizzle<2, 0>());
    q.data[1] -= 0.28f;
    q.data[0] = abs(q.data[0]) - wallRd + wallThk + 0.04f;
    d = min(
        max(length(q.swizzle<0, 2>()) - 0.02f, abs(q.data[1]) - 0.13f),
        PrRoundBoxDf(
            __ctx,
            f32x3(q.swizzle<0, 2>(), abs(q.data[1]) - 0.13f).swizzle<0, 2, 1>(),
            f32x3(0.06f, 0.005f, 0.06f), 0.005f));
    Cut(__ctx, p, d);
    if (d < dMin) {
      dMin = d;
      idObj = idPlin;
      qHit = q;
    };
    d = PrSphDf(__ctx,
                f32x3(q.swizzle<0, 2>(), q.data[1] - 0.21f).swizzle<0, 2, 1>(),
                0.07f);
    Cut(__ctx, p, d);
    if (d < dMin) {
      dMin = d;
      idObj = idBall;
    };
  }
  q = p;
  q.data[2] -= wallRd + wallThk;
  d = max(
      PrBoxDf(__ctx,
              f32x3(q.data[0], q.data[1] - wallHt - 0.03f, q.data[2] + 0.1f),
              f32x3(1.05f, wallHt, 0.21f)),
      -PrBox2Df(__ctx, f32x2(q.data[0], q.data[1] - 0.6f), f32x2(0.7f, 0.46f)));
  d = min(d, PrBoxDf(__ctx,
                     f32x3(q.data[0], abs(q.data[1] - 0.58f) - 0.49f,
                           q.data[2] - 0.47f),
                     f32x3(1.05f, 0.05f, 0.47f)));
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idPort;
  };
  d = max(
      PrRoundBoxDf(__ctx,
                   f32x3(q.data[0], q.data[1] - 2.f * wallHt, q.data[2] + 0.1f),
                   f32x3(1.05f, 0.f, 0.21f), 0.02f),
      wallRd + 0.03f - rh);
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idWallex;
  };
  q = p;
  q.set_swizzle<1, 2>(q.swizzle<1, 2>() -
                      (f32x2(1.56f, wallRd + wallThk + 0.31f)));
  d = PrBox2Df(__ctx, q.swizzle<0, 2>(), f32x2(1.09f, 0.67f));
  q.data[0] = abs(q.data[0]);
  u = Rot2Cs(__ctx, q.swizzle<0, 1>(),
             (sin((0.125f * pi) + f32x2(0.5f * pi, 0.f))));
  d = max(d, max(max(u.data[1] - 0.01f, -0.45f - q.data[1]),
                 -max(max(u.data[1] + 0.04f, -0.4f - q.data[1]),
                      0.63f - q.data[2])));
  d = max(
      d,
      -max(
          min(max(PrBox2Df(__ctx,
                           f32x2((fract(16.f * q.data[0] + 0.5f) - 0.5f) / 16.f,
                                 q.data[1] + 0.45f),
                           f32x2(1.2f / 64.f, 0.02f)),
                  q.data[0] - 0.98f),
              max(PrBox2Df(
                      __ctx,
                      f32x2((fract(16.f * u.data[0] + 0.75f) - 0.5f) / 16.f,
                            u.data[1] + 0.04f),
                      f32x2(1.2f / 64.f, 0.02f)),
                  u.data[0] - 0.9f)),
          0.63f - q.data[2]));
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idPortRf;
  };
  q = p;
  q.data[0] = abs(q.data[0]);
  q -= f32x3(2.f * doorSz.data[0], 0.51f, wallRd);
  q.set_swizzle<0, 2>(Rot2Cs(__ctx, q.swizzle<0, 2>(), drAngCs) +
                      doorSz.swizzle<0, 2>() * f32x2(1.f, -1.f));
  d = PrBoxDf(__ctx, q, doorSz);
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idDoor;
    qHit = q;
  };
  q = p;
  q.data[1] -= 0.59f;
  d = 0.15f - Minv2(__ctx, abs(q.swizzle<0, 2>()));
  q.set_swizzle<0, 2>(
      Rot2Cs(__ctx, q.swizzle<0, 2>(), sin(a.data[0] + f32x2(0.5f * pi, 0.f))));
  q.data[2] = abs(q.data[2]);
  q.set_swizzle<0, 2>(q.swizzle<0, 2>() -
                      (f32x2(-wallRd + wallThk + 0.15f, 0.05f)));
  u = f32x2(length(q.swizzle<0, 2>()) - 0.025f, abs(q.data[1]) - 0.45f);
  d = max(d, max(Maxv2(__ctx, u),
                 0.022f - length(f32x2(u.data[0] - 0.01f,
                                       -max(u.data[1] + 0.04f, 0.f)))));
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idPil;
    qHit = q;
  };
  q = p;
  q.set_swizzle<1, 2>(q.swizzle<1, 2>() -
                      (f32x2(0.585f, wallRd + wallThk + 0.4f)));
  d = max(max(abs(q.data[0]) - 1.1f, abs(q.data[2] - 0.1f) - 0.4f),
          -max(abs(abs(abs(q.data[0]) - 0.42f) - 0.27f) - 0.11f,
               q.data[2] - 0.23f));
  q.set_swizzle<0, 2>((1.f / 3.7f) *
                      (abs(fract(3.7f * q.swizzle<0, 2>()) - 0.5f)));
  u = f32x2(length(q.swizzle<0, 2>()) - 0.08f, abs(q.data[1]) - 0.44f);
  d = max(d, max(Maxv2(__ctx, u),
                 0.06f - length(f32x2(u.data[0] - 0.02f,
                                      -max(u.data[1] + 0.09f, 0.f)))));
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idPil;
    qHit = q;
  };
  q = p;
  d = max(rh - wallRd - wallThk - 1.4f, abs(q.data[1] - 0.02f) - 0.02f);
  Cut(__ctx, p, d);
  if (d < dMin) {
    dMin = d;
    idObj = idBase;
    qHit = q;
  };
  if (disco) {
    q = p;
    d = max(length(q.swizzle<0, 2>()) - 0.05f, abs(q.data[1] - 0.14f) - 0.005f);
    q.data[1] -= bmOrg.data[1];
    q.set_swizzle<0, 2>(
        Rot2Cs(__ctx, q.swizzle<0, 2>(),
               (sin((bmAzEl.data[0]) + f32x2(0.5f * pi, 0.f)))));
    t = length(q.swizzle<0, 1>()) - bmRd;
    d = min(d, max(max(max(t - 0.017f, abs(q.data[2]) - 0.004f), 0.012f - t),
                   q.data[1] - 0.005f));
    Cut(__ctx, p, d);
    if (d < dMin) {
      dMin = d;
      idObj = idBmSup;
      qHit = q;
    };
    q = p;
    q -= bmOrg;
    q = RotVB(__ctx, q,
              f32x4((sin((bmAzEl.data[0]) + f32x2(0.5f * pi, 0.f))),
                    (sin((bmAzEl.data[1]) + f32x2(0.5f * pi, 0.f)))));
    d = max(length(q.swizzle<0, 2>()) - bmRd - 0.01f, abs(q.data[1]) - 0.004f);
    Cut(__ctx, p, d);
    if (d < dMin) {
      dMin = d;
      idObj = idBmSrc;
      qHit = q;
    };
  }
  return dMin;
}
float ObjRay(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dHit, d;
  dHit = 0.f;
  for (int j = 0; j < 120; j++) {
    d = ObjDf(__ctx, ro + dHit * rd);
    if (d < 0.0002f || dHit > dstFar)
      break;
    dHit += d;
  }
  return dHit;
}
f32x3 ObjNf(Context &__ctx, f32x3 p) {
  f32x4 v;
  f32x2 e;
  e = f32x2(0.0001f, -0.0001f);
  for (int j = 0; j < 4; j++) {
    v[j] = ObjDf(
        __ctx,
        p + ((j < 2)
                 ? ((j == 0) ? e.swizzle<0, 0, 0>() : e.swizzle<0, 1, 1>())
                 : ((j == 2) ? e.swizzle<1, 0, 1>() : e.swizzle<1, 1, 0>())));
  }
  v.data[0] = -v.data[0];
  return normalize(2.f * v.swizzle<1, 2, 3>() - dot(v, f32x4(1.f)));
}
float ObjSShadow(Context &__ctx, f32x3 ro, f32x3 rd) {
  float sh, d, h;
  sh = 1.f;
  d = 0.02f;
  for (int j = 0; j < 30; j++) {
    h = ObjDf(__ctx, ro + d * rd);
    sh = min(sh, smoothstep(0.f, 0.05f * d, h));
    d += max(h, 0.01f);
    if (sh < 0.05f || d > dstFar)
      break;
  }
  return 0.5f + 0.5f * sh;
}
f32x4 FloorCol(Context &__ctx, f32x2 p) {
  f32x4 col4;
  f32x2 wd, w;
  p = 6.f * p + 0.5f;
  wd = abs(fract(p + 0.5f) - 0.5f);
  if (disco && length(wd) < 0.03f) {
    col4 = f32x4(1.2f, 1.2f, 0.7f, -1.f);
  } else if (Minv2(__ctx, wd) < 0.01f) {
    col4 = f32x4(0.6f, 0.6f, 0.6f, 0.f);
  } else {
    w = mod(floor(p), 2.f);
    col4 = mix(
        f32x4(0.f, 0.f, 0.8f, 0.1f), f32x4(0.8f, 0.8f, 0.8f, 0.1f),
        smoothstep(
            0.15f, 0.2f,
            fract(16.f *
                  Fbm2(__ctx, Rot2D(__ctx, f32x2(2.f, 4.f / 3.f) * p,
                                    0.25f * pi *
                                        (0.5f + w.data[0] + 2.f * w.data[1])) +
                                  20.f))));
    if (w.data[0] == w.data[1])
      col4 = mix(((dot(mod(floor(p), 4.f), f32x2(1.f, -1.f)) == 0.f)
                      ? f32x4(0.4f, 0.f, 0.f, 0.1f)
                      : f32x4(0.6f, 0.6f, 0.55f, 0.1f)),
                 mix(f32x4(0.7f, 0.5f, 0.f, 0.1f), col4,
                     0.1f + 0.9f * step(Minv2(__ctx, wd - 0.13f), 0.f)),
                 step(0.28f, length(wd - 0.5f)));
    else
      col4 = mix(f32x4(0.4f, 0.f, 0.f, 0.1f), col4,
                 0.1f + 0.9f * step(0.07f, abs(Minv2(__ctx, wd - 0.2f))));
    if (disco)
      col4 *= 1.2f;
  }
  return col4;
}
f32x4 ObjCol(Context &__ctx, f32x3 ro, f32x2 &vf) {
  f32x4 col4, mCol4, dwCol4, cwCol4, hiCol4;
  f32x2 u;
  float rh, a, aa, x, wtm, wtp;
  mCol4 = f32x4(0.8f, 0.8f, 0.75f, 0.f);
  dwCol4 = f32x4(0.3f, 0.3f, 0.7f, 0.f);
  hiCol4 = f32x4(HsvToRgb(__ctx, f32x3(fract(0.1f * tCur), 1.f, 0.7f)), -1.f);
  rh = length(ro.swizzle<0, 2>());
  aa = atan(ro.data[2], -ro.data[0]) / (2.f * pi) + 0.5f;
  vf = f32x2(0.f);
  wtm = wallRd - wallThk;
  wtp = wallRd + wallThk;
  if (idObj <= idFlor) {
    if (disco) {
      a = 0.f;
      if (idObj == idDomeO && length(ro - f32x3(0.f, domeBs, 0.f)) < domeRd ||
          idObj == idCeil && rh < domeRd && ro.data[1] > 1.4f)
        a = 2.f *
            (floor(nSeg.data[1] *
                       (atan(ro.data[1] + domeBs, -rh) / (2.f * pi) + 0.5f) +
                   floor(nSeg.data[1] * 0.1f * tCur) + 0.5f) -
             0.5f) /
            nSeg.data[1];
      else if (idObj == idWallex || idObj == idCeil && rh < domeRd &&
                                        ro.data[1] > 1.2f && ro.data[1] < 1.4f)
        a = (floor(nSeg.data[0] * (aa * ((idObj == idWallex) ? 1.f : -1.f)) +
                   floor(nSeg.data[0] * 0.1f * tCur) + 0.5f) -
             0.5f) /
            nSeg.data[0];
      cwCol4 = f32x4(HsvToRgb(__ctx, f32x3(a, 1.f, 0.7f)), -1.f);
    }
    if (idObj == idDomeI) {
      if (rh < oculRd + 0.002f)
        col4 = mCol4;
      else if (rh > oculRd + 0.03f && rh < 0.43f)
        col4 = disco ? mix(dwCol4, 0.5f * hiCol4, step(0.5f, fract(8.f * tCur)))
                     : dwCol4;
      else
        col4 = 1.1f * mCol4;
    } else if (idObj == idDomeM) {
      col4 = 1.05f * mCol4;
    } else if (idObj == idDomeO) {
      if (length(ro - f32x3(0.f, domeBs, 0.f)) < domeRd) {
        col4 = disco ? cwCol4 : dwCol4;
      } else {
        col4 =
            mCol4 *
            (0.95f +
             0.05f * smoothstep(
                         0.05f, 0.1f,
                         fract(32.f * atan((ro.data[1] - 1.12f) / rh) / pi)));
        vf = f32x2(128.f, 0.8f);
      }
    } else if (idObj == idDomeOx) {
      col4 = f32x4(1.f, 1.f, 1.1f, 1.f) * mCol4;
      vf = f32x2(128.f, 0.8f);
    } else if (idObj == idWallin) {
      if (rh > wtm - 0.02f && ro.data[1] < 1.f && ro.data[1] > 0.12f &&
          (ro.data[2] < 0.f || abs(ro.data[0]) > 0.22f)) {
        col4 = dwCol4;
      } else if (rh < wtm - 0.05f && ro.data[1] < 1.15f) {
        if (PrBox2Df(__ctx,
                     f32x2(fract(224.f * aa + 0.5f) - 0.5f, ro.data[1] - 1.14f),
                     f32x2(0.3f, 0.004f)) > 0.01f)
          col4 = mCol4 * f32x4(1.1f, 1.1f, 0.9f, 1.f);
        else
          col4 = disco ? hiCol4 : 1.3f * mCol4 * f32x4(1.1f, 1.1f, 0.9f, 1.f);
      } else {
        a = fract(nSeg.data[0] * aa + 0.5f) - 0.5f;
        col4 =
            mCol4 *
            (1.2f -
             0.2f *
                 step(0.01f,
                      min(PrBox2Df(__ctx, f32x2(a, ro.data[1] - domeBs + 0.15f),
                                   f32x2(0.35f, 0.09f)),
                          PrBox2Df(__ctx, f32x2(a, ro.data[1] - domeBs + 0.8f),
                                   f32x2(0.24f, 0.25f)))));
        if (ro.data[1] > 0.92f &&
            length(f32x2(a, 3.3f * (ro.data[1] - 0.92f))) < 0.2f)
          col4 = disco ? hiCol4 : 1.2f * col4;
        vf = f32x2(128.f, 0.8f);
      }
    } else if (idObj == idWallex) {
      if (ro.data[1] < 1.f && rh < wtm + 0.03f && abs(ro.data[0]) > 0.22f) {
        col4 = disco ? cwCol4 : dwCol4;
      } else if (ro.data[1] < 1.f && rh < wtm + 0.11f && ro.data[1] > 0.12f &&
                 (ro.data[2] < 0.f || abs(ro.data[0]) > 0.22f)) {
        col4 = dwCol4;
      } else {
        col4 = mCol4 * (0.95f + 0.05f * smoothstep(0.05f, 0.1f,
                                                   fract(8.f * ro.data[1])));
        if (rh > wtp - 0.02f) {
          u = Rot2Cs(__ctx, ro.swizzle<0, 2>(),
                     (sin((pi / 14.f) + f32x2(0.5f * pi, 0.f))));
          x = 0.5f * wtp *
              dot(u.swizzle<1, 0>(),
                  (sin(
                      (2.f * pi *
                       (floor(14.f * atan(u.data[1], -u.data[0]) / (2.f * pi)) +
                        0.5f) /
                       14.f) +
                      f32x2(0.5f * pi, 0.f))));
          if (ro.data[1] > 1.44f &&
                  abs(length(f32x2(x, ro.data[1] - 1.12f)) - 0.55f) < 0.05f ||
              abs(ro.data[1] - 1.36f) < 0.08f && abs(x) > 0.37f)
            col4 = 1.03f * mCol4;
          if (PrBox2Df(__ctx, f32x2(x, ro.data[1] - 1.34f),
                       f32x2(0.04f, 0.06f)) < 0.f)
            col4 = disco ? f32x4(0.3f, 0.3f, 0.1f, -1.f)
                         : f32x4(0.3f, 0.3f, 0.3f, 0.f);
        }
        vf = f32x2(128.f, 0.8f);
      }
    } else if (idObj == idCeil) {
      if (ro.data[1] < 1.2f && rh < wtm - 0.01f) {
        col4 = mCol4 * f32x4(1.1f, 1.1f, 0.9f, 1.f);
      } else if (rh < domeRd) {
        col4 = disco ? cwCol4 : dwCol4;
      } else {
        col4 = mCol4 * (0.95f + 0.05f * smoothstep(0.05f, 0.1f,
                                                   fract(8.f * ro.data[1])));
        vf = f32x2(128.f, 0.8f);
      }
    } else if (idObj == idFlor) {
      if (ro.data[1] > 0.12f && rh < wallRd) {
        col4 = FloorCol(__ctx, ro.swizzle<0, 2>());
      } else if (rh < 0.85f * oculRd && ro.data[1] > 0.05f) {
        col4 = f32x4(0.f, 0.3f, 0.f, 0.f);
      } else {
        col4 = mCol4;
        vf = f32x2(128.f, 0.8f);
      }
    }
  } else {
    if (idObj == idPil || idObj == idPlin)
      a = abs(
          fract(16.f * atan(qHit.data[2], qHit.data[0]) / (2.f * pi) + 0.5f) -
          0.5f);
    if (idObj == idPort) {
      col4 = mCol4 *
             (0.95f + 0.05f * smoothstep(0.05f, 0.1f, fract(8.f * ro.data[1])));
      if (abs(ro.data[0]) < 1.04f &&
          abs(dot(f32x2(abs(ro.data[0]), ro.data[1]),
                  sin(0.125f * pi + f32x2(0.f, 0.5f * pi))) -
              1.54f) < 0.02f)
        col4 = 1.03f * mCol4;
      if (abs(ro.data[0]) > 1.f &&
          PrBox2Df(__ctx,
                   f32x2(abs(ro.data[1] - 0.5f * wallHt - 0.5f) - 0.5f,
                         ro.data[2] - wtp + 0.1f),
                   f32x2(0.08f, 0.04f)) < 0.f)
        col4 = disco ? f32x4(0.3f, 0.3f, 0.1f, -1.f)
                     : f32x4(0.3f, 0.3f, 0.3f, 0.f);
      vf = f32x2(128.f, 0.8f);
    } else if (idObj == idPortRf) {
      col4 = mCol4 * f32x4(1.f, 0.95f, 0.95f, 1.f) *
             (0.9f +
              0.1f * step(0.05f, abs(fract(16.f * ro.data[2] + 0.8f) - 0.5f)));
      vf = f32x2(128.f, 0.8f);
    } else if (idObj == idGrt) {
      col4 = f32x4(1.f, 0.8f, 0.f, 0.2f);
    } else if (idObj == idPil) {
      if (rh < wallRd) {
        col4 = f32x4(0.65f, 0.6f, 0.6f, 0.f);
        if (disco && abs(qHit.data[1]) < 0.41f)
          col4 =
              1.3f *
              mix(col4, hiCol4,
                  step(0.8f, fract(8.f * ro.data[1] +
                                   sin(4.f * pi * (aa + 0.2f * tCur)) - tCur)));
      } else
        col4 = 0.95f * mCol4;
      if (abs(qHit.data[1]) < 0.41f)
        col4 *= 0.9f + 0.1f * smoothstep(0.1f, 0.15f, a);
    } else if (idObj == idPlin) {
      col4 = f32x4(0.8f, 0.7f, 0.6f, 0.f);
      if (abs(qHit.data[1]) < 0.11f)
        col4 *= 0.9f + 0.1f * smoothstep(0.1f, 0.15f, a);
    } else if (idObj == idBall) {
      col4 =
          f32x4((disco ? bmCol.swizzle<0, 1, 2>() : f32x3(0.9f, 0.8f, 0.5f)) *
                    (1.f - 0.2f * Fbm1(__ctx, 16.f * tCur)),
                -1.f);
    } else if (idObj == idEnt) {
      col4 = (disco && ro.data[2] < wtm - 0.07f && ro.data[1] > 0.92f)
                 ? hiCol4
                 : mCol4 * f32x4(1.1f, 1.1f, 0.9f, 1.f);
    } else if (idObj == idDoor) {
      col4 =
          f32x4(0.3f, 0.3f, 0.f, 0.1f) *
          (0.8f + 0.2f * step(0.f, PrBox2Df(__ctx, qHit.swizzle<0, 1>(),
                                            doorSz.swizzle<0, 1>() - 0.03f)));
    } else if (idObj == idBase) {
      u = qHit.swizzle<0, 2>() * f32x2(1.f / 6.f, 1.f) * 16.f;
      x = u.data[0] + 1.f / 3.f +
          ((fract(0.5f * u.data[1]) < 0.5f) ? 0.f : 0.5f);
      col4 =
          f32x4(0.5f, 0.5f, 0.5f, 0.f) *
          (0.9f +
           0.1f * smoothstep(0.1f, 0.15f,
                             min(fract(u.data[1]),
                                 6.f * min(fract(x), fract(x + 2.f / 3.f)))));
    } else if (idObj == idBmSup) {
      col4 = f32x4(1.f, 1.f, 1.f, 0.1f);
    } else if (idObj == idBmSrc) {
      col4 = (qHit.data[1] < 0.f && length(qHit.swizzle<0, 2>()) < bmRd)
                 ? f32x4(bmCol.swizzle<0, 1, 2>(), -1.f)
                 : f32x4(1.f, 1.f, 1.f, 0.1f);
    }
  }
  return col4;
}
f32x4 CylHit(Context &__ctx, f32x3 ro, f32x3 rd, float rad, f32x3 &rHit) {
  f32x3 vn;
  float dMin, d, a, b, w;
  dMin = 1e6f;
  a = dot(rd.swizzle<0, 2>(), rd.swizzle<0, 2>());
  b = dot(rd.swizzle<0, 2>(), ro.swizzle<0, 2>());
  w = b * b - a * (dot(ro.swizzle<0, 2>(), ro.swizzle<0, 2>()) - rad * rad);
  if (w > 0.f && a > 0.f) {
    d = -b - sqrt(w);
    if (d > 0.f) {
      d /= a;
      rHit = ro + d * rd;
      dMin = d;
      vn = f32x3(rHit.swizzle<0, 2>() / rad, 0.f).swizzle<0, 2, 1>();
    }
  }
  return f32x4(dMin, vn);
}
float BmVal(Context &__ctx, f32x3 ro, f32x3 rd, float dstLim, float bmRad,
            f32x3 &bmHit, f32x3 &bmAx) {
  f32x4 dc4, csBm;
  float bmVal;
  csBm = f32x4((sin((bmAzEl.data[0]) + f32x2(0.5f * pi, 0.f))),
               (sin((bmAzEl.data[1]) + f32x2(0.5f * pi, 0.f))));
  bmAx = RotVF(__ctx, f32x3(0.f, -1.f, 0.f), csBm);
  bmVal = 0.f;
  dc4 = CylHit(__ctx, RotVB(__ctx, ro, csBm), RotVB(__ctx, rd, csBm), bmRad,
               bmHit);
  if (dc4.data[0] < dstLim) {
    bmVal = max(-dot(RotVF(__ctx, dc4.swizzle<1, 2, 3>(), csBm), rd), 0.f);
    bmHit = RotVF(__ctx, bmHit, csBm);
  }
  return bmVal;
}
f32x3 StarPat(Context &__ctx, f32x3 rd, float scl) {
  f32x3 tm, qn, u;
  f32x2 q;
  float f;
  tm = -1.f / max(abs(rd), 0.0001f);
  qn = -sign(rd) * step(tm.swizzle<2, 0, 1>(), tm) *
       step(tm.swizzle<1, 2, 0>(), tm);
  u = Maxv3(__ctx, tm) * rd;
  q = atan(f32x2(dot(u.swizzle<2, 0, 1>(), qn), dot(u.swizzle<1, 2, 0>(), qn)),
           f32x2(1.f)) /
      pi;
  f = 0.57f * (Fbm2(__ctx, 11.f * dot(0.5f * (qn + 1.f), f32x3(1.f, 2.f, 4.f)) +
                               131.13f * scl * q) +
               Fbm2(__ctx, 13.f * dot(0.5f * (qn + 1.f), f32x3(1.f, 2.f, 4.f)) +
                               171.13f * scl * q.swizzle<1, 0>()));
  return 6.f * f32x3(1.f, 1.f, 0.8f) * pow(f, 16.f);
}
f32x3 SkyBgCol(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x3 col, clCol, skCol;
  f32x2 q;
  float f, fd, ff, sd, a;
  if (rd.data[1] > -0.02f &&
      rd.data[1] < 0.03f * Fbm1(__ctx, 16.f * atan(rd.data[2], -rd.data[0]))) {
    col = f32x3(0.3f, 0.4f, 0.5f);
    if (disco)
      col *= 0.25f;
  } else {
    q = 0.02f * (ro.swizzle<0, 2>() + 2.f * tCur +
                 ((100.f - ro.data[1]) / rd.data[1]) * rd.swizzle<0, 2>());
    ff = Fbm2(__ctx, q);
    f = smoothstep(0.2f, 0.8f, ff);
    fd = smoothstep(0.2f, 0.8f,
                    Fbm2(__ctx, q + 0.01f * sunDir.swizzle<0, 2>())) -
         f;
    clCol = (0.7f + 0.5f * ff) *
            (f32x3(0.7f) - 0.7f * f32x3(0.3f, 0.3f, 0.2f) * sign(fd) *
                               smoothstep(0.f, 0.05f, abs(fd)));
    sd = max(dot(rd, sunDir), 0.f);
    skCol = f32x3(0.3f, 0.4f, 0.8f) +
            step(0.1f, sd) * f32x3(1.f, 1.f, 0.9f) *
                min(0.3f * pow(sd, 64.f) + 0.5f * pow(sd, 2048.f), 1.f);
    col = mix(skCol, clCol,
              0.1f + 0.9f * f * smoothstep(0.01f, 0.1f, rd.data[1]));
    if (disco)
      col = f32x3(0.1f) * col.data[0] + StarPat(__ctx, rd, 16.f);
  }
  return col;
}
f32x3 ShowScene(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x4 col4;
  f32x3 ltDir, col, vn, roo, bmHit, bmAx, oculPos;
  f32x2 vf;
  float dstObj, dstGrnd, sh, bmVal;
  bool inBldg, isBg;
  domeRd = 1.2f;
  domeBs = domeRd + 0.24f;
  wallRd = 1.7f;
  wallHt = 0.9f;
  wallThk = 0.26f;
  oculRd = 0.16f;
  oculPos = f32x3(0.f, domeBs + domeRd, 0.f);
  doorSz = f32x3(0.09f, 0.35f, 0.013f);
  drAngCs = (sin(
      (-0.5f * pi *
       clamp(-0.3f + 1.6f * abs(2.f * fract(0.25f * tCur) - 1.f), 0.f, 1.f)) +
      f32x2(0.5f * pi, 0.f)));
  bmCol = disco ? HsvToRgb(__ctx, f32x3(fract(0.1f * tCur), 0.7f, 1.f))
                : f32x3(0.8f, 1.f, 0.8f);
  nSeg = f32x2(28.f, 27.f);
  roo = ro;
  isBg = false;
  inBldg = false;
  dstGrnd = dstFar;
  sh = 1.f;
  dstObj = ObjRay(__ctx, ro, rd);
  if (dstObj < dstFar) {
    ro += dstObj * rd;
    vn = ObjNf(__ctx, ro);
    inBldg =
        ((dirCut == 0 &&
          ((length(ro.swizzle<0, 2>()) < wallRd && ro.data[1] < 2.f * wallHt ||
            length(ro - f32x3(0.f, domeBs, 0.f)) < domeRd &&
                ro.data[1] > domeBs) ||
           idObj == idEnt && ro.data[2] < wallRd || idObj == idDoor)) ||
         idObj == idBall);
    col4 = ObjCol(__ctx, ro, vf);
    if (vf.data[1] > 0.f)
      vn = VaryNf(__ctx, vf.data[0] * ro, vn, vf.data[1]);
    if (col4.data[3] >= 0.f) {
      if (inBldg) {
        ltDir = normalize(oculPos - ro);
        col =
            col4.swizzle<0, 1, 2>() * (0.1f + 0.5f * max(dot(vn, ltDir), 0.f)) +
            col4.data[3] * pow(max(dot(reflect(ltDir, vn), rd), 0.f), 32.f);
      }
    } else {
      col = col4.swizzle<0, 1, 2>() * (0.4f + 0.6f * max(-dot(rd, vn), 0.f));
    }
  } else if (rd.data[1] < 0.f && dirCut == 0) {
    dstGrnd = -ro.data[1] / rd.data[1];
    ro += dstGrnd * rd;
    vn = VaryNf(__ctx, 16.f * ro, f32x3(0.f, 1.f, 0.f),
                1.f - smoothstep(0.5f, 0.8f, dstGrnd / dstFar));
    col4 = f32x4(
        0.5f * mix(f32x3(0.8f, 1.f, 0.5f), f32x3(0.7f, 0.9f, 0.5f),
                   0.2f + 0.8f * smoothstep(
                                     0.3f, 0.7f,
                                     Fbm2(__ctx, 4.f * ro.swizzle<0, 2>()))),
        0.05f);
  } else {
    isBg = true;
    col = (dirCut == 0) ? SkyBgCol(__ctx, ro, rd)
                        : f32x3(0.3f, 0.4f, 0.5f) * (0.7f + 0.3f * rd.data[1]);
  }
  if (!isBg && !inBldg) {
    if (disco) {
      if (col4.data[3] >= 0.f) {
        if (abs(ro.data[0]) < 1.f && ro.data[1] < 1.f && ro.data[2] > wallRd &&
            ro.data[2] < wallRd + wallThk + 0.9f) {
          ltDir = normalize(f32x3(0.f, 1.6f, wallRd + wallThk + 0.6f) - ro);
          col = 0.6f * col4.swizzle<0, 1, 2>() *
                (0.2f + 0.8f * max(dot(vn, ltDir), 0.f));
        } else {
          col = 0.3f * col4.swizzle<0, 1, 2>() *
                (0.2f + 0.8f * max(dot(vn, sunDir), 0.f));
        }
      } else {
        col = col4.swizzle<0, 1, 2>() * (0.4f + 0.6f * max(-dot(rd, vn), 0.f));
      }
      if (dstObj >= dstFar)
        col = mix(col, 0.25f * f32x3(0.3f, 0.4f, 0.5f),
                  pow(1.f + rd.data[1], 16.f));
    } else {
      sh = ObjSShadow(__ctx, ro + 0.01f * vn, sunDir);
      col =
          col4.swizzle<0, 1, 2>() * (0.2f + 0.2f * max(-dot(vn, sunDir), 0.f) +
                                     0.8f * sh * max(dot(vn, sunDir), 0.f)) +
          col4.data[3] * step(0.95f, sh) *
              pow(max(dot(reflect(sunDir, vn), rd), 0.f), 32.f);
      if (dstObj >= dstFar)
        col = mix(col, f32x3(0.3f, 0.4f, 0.5f), pow(1.f + rd.data[1], 16.f));
    }
  }
  if (inBldg) {
    if (disco) {
      bmVal =
          BmVal(__ctx, roo - bmOrg, rd, min(dstObj, dstFar), bmRd, bmHit, bmAx);
      if (bmVal > 0.f && length(bmHit.swizzle<0, 2>()) < wallRd - 0.12f &&
          RotVB(__ctx, bmHit,
                f32x4((sin((bmAzEl.data[0]) + f32x2(0.5f * pi, 0.f))),
                      (sin((bmAzEl.data[1]) + f32x2(0.5f * pi, 0.f)))))
                  .data[1] < 0.f) {
        col += 0.3f * mix(col4.swizzle<0, 1, 2>(), bmCol, 0.5f) *
               max(0.f, -dot(vn, bmAx)) *
               (1.f -
                smoothstep(-0.1f, 0.1f,
                           length(ro - bmOrg - dot(ro - bmOrg, bmAx) * bmAx) -
                               bmRd));
        col += 0.3f * bmVal * bmCol;
      }
    } else {
      bmVal = BmVal(__ctx, roo - oculPos, rd, min(dstObj, dstFar), oculRd,
                    bmHit, bmAx);
      bmHit += oculPos;
      if (bmVal > 0.f && length(bmHit.swizzle<0, 2>()) < wallRd - 0.12f &&
          bmHit.data[1] > 0.12f) {
        col +=
            0.2f * col4.swizzle<0, 1, 2>() * max(0.f, -dot(vn, bmAx)) *
            (1.f -
             smoothstep(-0.1f, 0.1f,
                        length(ro - oculPos - dot(ro - oculPos, bmAx) * bmAx) -
                            oculRd));
        if (bmHit.data[1] < oculPos.data[1] + 0.02f)
          col += 0.2f * f32x3(1.f, 1.f, 0.9f) * bmVal;
      }
    }
  }
  return clamp(col, 0.f, 1.f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3x3 vuMat;
  f32x4 mPtr;
  f32x3 ro, rd, col, vd;
  f32x2 canvas, uv, uvv, mMid[2], ut[2], mSize, msw, mAzEl, vAzEl;
  float zmFac, asp, sr, t, tt;
  int vuId, regId;
  bool isPano;
  canvas = iResolution.swizzle<0, 1>();
  uv = 2.f * fragCoord.swizzle<0, 1>() / canvas - 1.f;
  uv.data[0] *= canvas.data[0] / canvas.data[1];
  tCur = iTime;
  mPtr = iMouse;
  mPtr.set_swizzle<0, 1>(mPtr.swizzle<0, 1>() / canvas - 0.5f);
  asp = canvas.data[0] / canvas.data[1];
  mSize = (1.f / 6.f) * f32x2(asp, 1.f);
  mMid[0] = (1.f - mSize.data[1]) * f32x2(asp, 1.f);
  mMid[1] = (1.f - mSize.data[1]) * f32x2(-asp, 1.f);
  disco = true;
  for (int k = 0; k < 2; k++)
    ut[k] = abs(uv - mMid[k]) - mSize;
  regId = -1;
  if (mPtr.data[2] > 0.f) {
    regId = 0;
    for (int k = 0; k < 2; k++) {
      msw = 2.f * mPtr.swizzle<0, 1>() - mMid[k] / f32x2(asp, 1.f);
      if (Maxv2(__ctx, abs(msw)) < mSize.data[1]) {
        regId = k + 1;
        msw /= 2.f * mSize.data[1];
        break;
      }
    }
    if (regId == 0)
      msw = mPtr.swizzle<0, 1>();
  }
  vuId = 0;
  for (int k = 0; k < 2; k++) {
    if (Maxv2(__ctx, ut[k]) < 0.f) {
      uv = (uv - mMid[k]) / mSize.data[1];
      vuId = k + 1;
      break;
    }
  }
  if (regId > 0 && (vuId == 0 || vuId == regId))
    vuId = regId - vuId;
  mAzEl = (mPtr.data[2] > 0.f && vuId == regId) ? f32x2(2.f, 1.f) * pi * msw
                                                : f32x2(0.f);
  if (mPtr.data[2] <= 0.f) {
    t = mod(0.04f * tCur, 2.f);
    t = (abs((floor(8.f * t) + smoothstep(0.8f, 1.f, fract(8.f * t))) / 8.f -
             1.f) -
         0.5f);
  } else
    t = 0.f;
  dirCut = 0;
  isPano = false;
  if (vuId == 0) {
    ro = f32x3(0.f, 0.7f, -1.f);
    vAzEl = f32x2(0.25f * pi + mAzEl.data[0] + 0.2f * pi * t,
                  clamp(0.05f * pi + mAzEl.data[1], -0.37f * pi, 0.37f * pi));
    zmFac = 1.5f;
  } else if (vuId == 1) {
    ro = f32x3(0.f, 1.f, -15.f);
    vAzEl = f32x2(pi + 1.1f * mAzEl.data[0] + 0.4f * pi * t,
                  clamp(-0.1f * pi + mAzEl.data[1], -0.4f * pi, 0.f));
    zmFac = 6.f;
  } else if (vuId == 2) {
    ro = f32x3(0.f, 1.2f, -0.8f);
    vAzEl = f32x2(1.1f * mAzEl.data[0],
                  clamp(0.5f * mAzEl.data[1], -0.1f * pi, 0.1f * pi));
    zmFac = 0.3f;
    isPano = true;
  } else if (vuId == 3) {
    ro = f32x3(0.f, 1.2f, -10.f);
    vAzEl = f32x2(1.1f * mAzEl.data[0], 0.f);
    zmFac = 5.f;
    dirCut = 2;
  } else if (vuId == 4) {
    ro = f32x3(0.f, 1.3f, -10.f);
    vAzEl = f32x2(-0.5f * pi + 1.1f * mAzEl.data[0],
                  clamp(-0.1f * pi + mAzEl.data[1], -0.4f * pi, 0.4f * pi));
    zmFac = 5.f;
    dirCut = 1;
  }
  vuMat = StdVuMat(__ctx, vAzEl);
  ro = vuMat * ro;
  if (vuId == 0)
    ro.data[1] = max(ro.data[1], 0.2f);
  if (dirCut == 1) {
    cutVec = f32x3(1.f, 0.f, 0.f);
  } else if (dirCut == 2) {
    cutVec = f32x3(0.f, 0.f, -1.f);
    cutVec.set_swizzle<0, 2>(
        Rot2D(__ctx, cutVec.swizzle<0, 2>(), -vAzEl.data[0]));
  }
  tt = 1.f - fract(0.05f * tCur);
  t = SmoothBump(__ctx, 0.25f, 0.75f, 0.23f, tt);
  bmOrg = f32x3(0.f, 0.23f, 0.f);
  bmRd = 1.f * 0.97f / 12.f;
  if (disco)
    bmAzEl = pi * f32x2(0.2f * tCur + 2.f * Fbm1(__ctx, 0.2f * tCur),
                        0.8f + 0.14f * cos(2.f * pi * t));
  else
    bmAzEl = pi * ((tt < 0.5f) ? f32x2(0.5f + t, 0.16f)
                               : f32x2(0.5f, 0.16f * (1.f - 2.f * t)));
  sunDir = vuMat * normalize(f32x3(1.f, 1.f, -1.f));
  dstFar = 60.f;
  const float naa = 3.f;
  col = f32x3(0.f);
  sr = 2.f * mod(dot(mod(floor(0.5f * (uv + 1.f) * canvas), 2.f), f32x2(1.f)),
                 2.f) -
       1.f;
  for (float a = float(0); a < naa; a++) {
    uvv =
        (uv + step(1.5f, naa) * Rot2D(__ctx, f32x2(0.5f / canvas.data[1], 0.f),
                                      sr * (0.667f * a + 0.5f) * pi)) /
        zmFac;
    rd =
        vuMat *
        normalize(
            isPano ? f32x3(2.f * sin(uvv.data[0] / asp + f32x2(0.f, 0.5f * pi)),
                           uvv.data[1])
                         .swizzle<0, 2, 1>()
                   : f32x3(2.f * tan(0.5f * atan(uvv / f32x2(asp, 1.f))) *
                               f32x2(asp, 1.f),
                           1.f));
    col += (1.f / naa) * ShowScene(__ctx, ro, rd);
  }
  for (int k = 0; k < 2; k++) {
    if (Maxv2(__ctx, ut[k]) < 0.f &&
        Minv2(__ctx, abs(ut[k])) * canvas.data[1] < 3.f)
      col = f32x3(0.7f, 0.3f, 0.3f);
  }
  fragColor = f32x4(col, 1.f);
}
float Minv2(Context &__ctx, f32x2 p) { return min(p.data[0], p.data[1]); }
float Maxv2(Context &__ctx, f32x2 p) { return max(p.data[0], p.data[1]); }
float Maxv3(Context &__ctx, f32x3 p) {
  return max(p.data[0], max(p.data[1], p.data[2]));
}
float PrBoxDf(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 d;
  d = abs(p) - b;
  return min(Maxv3(__ctx, d), 0.f) + length(max(d, 0.f));
}
float PrBox2Df(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d;
  d = abs(p) - b;
  return min(Maxv2(__ctx, d), 0.f) + length(max(d, 0.f));
}
float PrRoundBoxDf(Context &__ctx, f32x3 p, f32x3 b, float r) {
  return length(max(abs(p) - b, 0.f)) - r;
}
float PrSphDf(Context &__ctx, f32x3 p, float r) { return length(p) - r; }
float SmoothMin(Context &__ctx, float a, float b, float r) {
  float h;
  h = clamp(0.5f + 0.5f * (b - a) / r, 0.f, 1.f);
  return mix(b - h * r, a, h);
}
float SmoothMax(Context &__ctx, float a, float b, float r) {
  return -SmoothMin(__ctx, -a, -b, r);
}
float SmoothBump(Context &__ctx, float lo, float hi, float w, float x) {
  return (1.f - smoothstep(hi - w, hi + w, x)) * smoothstep(lo - w, lo + w, x);
}
f32x3x3 StdVuMat(Context &__ctx, f32x2 vAzEl) {
  f32x2 ca, sa;
  ca = cos(vAzEl);
  sa = sin(vAzEl);
  return f32x3x3(ca.data[0], 0.f, -sa.data[0], 0.f, 1.f, 0.f, sa.data[0], 0.f,
                 ca.data[0]) *
         f32x3x3(1.f, 0.f, 0.f, 0.f, ca.data[1], -sa.data[1], 0.f, sa.data[1],
                 ca.data[1]);
}
f32x2 Rot2D(Context &__ctx, f32x2 q, float a) {
  f32x2 cs;
  cs = sin(a + f32x2(0.5f * pi, 0.f));
  return f32x2(dot(q, f32x2(cs.data[0], -cs.data[1])),
               dot(q.swizzle<1, 0>(), cs));
}
f32x2 Rot2Cs(Context &__ctx, f32x2 q, f32x2 cs) {
  return f32x2(dot(q, cs * f32x2(1.f, -1.f)), dot(q.swizzle<1, 0>(), cs));
}
f32x3 RotVF(Context &__ctx, f32x3 v, f32x4 cs) {
  v.set_swizzle<1, 2>(
      Rot2Cs(__ctx, v.swizzle<1, 2>(), cs.swizzle<2, 3>() * f32x2(1.f, -1.f)));
  v.set_swizzle<0, 2>(
      Rot2Cs(__ctx, v.swizzle<0, 2>(), cs.swizzle<0, 1>() * f32x2(1.f, -1.f)));
  return v;
}
f32x3 RotVB(Context &__ctx, f32x3 v, f32x4 cs) {
  v.set_swizzle<0, 2>(Rot2Cs(__ctx, v.swizzle<0, 2>(), cs.swizzle<0, 1>()));
  v.set_swizzle<1, 2>(Rot2Cs(__ctx, v.swizzle<1, 2>(), cs.swizzle<2, 3>()));
  return v;
}
f32x3 HsvToRgb(Context &__ctx, f32x3 c) {
  return c.data[2] * mix(f32x3(1.f),
                         clamp(abs(fract(c.swizzle<0, 0, 0>() +
                                         f32x3(1.f, 2.f / 3.f, 1.f / 3.f)) *
                                       6.f -
                                   3.f) -
                                   1.f,
                               0.f, 1.f),
                         c.data[1]);
}
const float cHashM = 43758.54f;
float Hashfv2(Context &__ctx, f32x2 p) {
  return fract(sin(dot(p, f32x2(37.f, 39.f))) * cHashM);
}
f32x2 Hashv2f(Context &__ctx, float p) {
  return fract(sin(p + f32x2(0.f, 1.f)) * cHashM);
}
f32x2 Hashv2v2(Context &__ctx, f32x2 p) {
  f32x2 cHashVA2 = f32x2(37.f, 39.f);
  return fract(sin(dot(p, cHashVA2) + f32x2(0.f, cHashVA2.data[0])) * cHashM);
}
float Noiseff(Context &__ctx, float p) {
  f32x2 t;
  float ip, fp;
  ip = floor(p);
  fp = fract(p);
  fp = fp * fp * (3.f - 2.f * fp);
  t = Hashv2f(__ctx, ip);
  return mix(t.data[0], t.data[1], fp);
}
float Noisefv2(Context &__ctx, f32x2 p) {
  f32x2 t, ip, fp;
  ip = floor(p);
  fp = fract(p);
  fp = fp * fp * (3.f - 2.f * fp);
  t = mix(Hashv2v2(__ctx, ip), Hashv2v2(__ctx, ip + f32x2(0.f, 1.f)),
          fp.data[1]);
  return mix(t.data[0], t.data[1], fp.data[0]);
}
float Fbm1(Context &__ctx, float p) {
  float f, a;
  f = 0.f;
  a = 1.f;
  for (int j = 0; j < 5; j++) {
    f += a * Noiseff(__ctx, p);
    a *= 0.5f;
    p *= 2.f;
  }
  return f * (1.f / 1.9375f);
}
float Fbm2(Context &__ctx, f32x2 p) {
  float f, a;
  f = 0.f;
  a = 1.f;
  for (int j = 0; j < 5; j++) {
    f += a * Noisefv2(__ctx, p);
    a *= 0.5f;
    p *= 2.f;
  }
  return f * (1.f / 1.9375f);
}
float Fbmn(Context &__ctx, f32x3 p, f32x3 n) {
  f32x3 s;
  float a;
  s = f32x3(0.f);
  a = 1.f;
  for (int j = 0; j < 5; j++) {
    s += a * f32x3(Noisefv2(__ctx, p.swizzle<1, 2>()),
                   Noisefv2(__ctx, p.swizzle<2, 0>()),
                   Noisefv2(__ctx, p.swizzle<0, 1>()));
    a *= 0.5f;
    p *= 2.f;
  }
  return dot(s, abs(n));
}
f32x3 VaryNf(Context &__ctx, f32x3 p, f32x3 n, float f) {
  f32x4 v;
  f32x3 g;
  f32x2 e = f32x2(0.1f, 0.f);
  for (int j = 0; j < 4; j++)
    v[j] = Fbmn(
        __ctx,
        p + ((j < 2)
                 ? ((j == 0) ? e.swizzle<0, 1, 1>() : e.swizzle<1, 0, 1>())
                 : ((j == 2) ? e.swizzle<1, 1, 0>() : e.swizzle<1, 1, 1>())),
        n);
  g = v.swizzle<0, 1, 2>() - v.data[3];
  return normalize(n + f * (g - n * dot(n, g)));
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
