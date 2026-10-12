// Port of https://www.shadertoy.com/view/DtB3zV; author: dr2.
// Dataset license metadata: cc-by-nc-sa-4.0; original retained in
// upstream/DtB3zV.json.
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
float PrBoxDf(Context &__ctx, f32x3 p, f32x3 b);
float PrBox2Df(Context &__ctx, f32x2 p, f32x2 b);
float PrRoundBoxDf(Context &__ctx, f32x3 p, f32x3 b, float r);
float PrRoundBox2Df(Context &__ctx, f32x2 p, f32x2 b, float r);
float PrCylDf(Context &__ctx, f32x3 p, float r, float h);
float PrRoundCylDf(Context &__ctx, f32x3 p, float r, float rt, float h);
float PrCapsDf(Context &__ctx, f32x3 p, float r, float h);
float PrTorusDf(Context &__ctx, f32x3 p, float ri, float rc);
float Minv2(Context &__ctx, f32x2 p);
float Maxv2(Context &__ctx, f32x2 p);
float Minv3(Context &__ctx, f32x3 p);
float Maxv3(Context &__ctx, f32x3 p);
float SmoothMin(Context &__ctx, float a, float b, float r);
float SmoothBump(Context &__ctx, float lo, float hi, float w, float x);
float ShowIntPZ(Context &__ctx, f32x2 q, f32x2 cBox, float mxChar, float val);
f32x3 HsvToRgb(Context &__ctx, f32x3 c);
f32x2 PixToHex(Context &__ctx, f32x2 p);
f32x2 HexToPix(Context &__ctx, f32x2 h);
float HexEdgeDist(Context &__ctx, f32x2 p);
void HexVorInit(Context &__ctx);
f32x4 HexVor(Context &__ctx, f32x2 p);
f32x3x3 StdVuMat(Context &__ctx, float el, float az);
f32x2 Rot2D(Context &__ctx, f32x2 q, float a);
f32x2 Rot2Cs(Context &__ctx, f32x2 q, f32x2 cs);
float Hashfv2(Context &__ctx, f32x2 p);
f32x2 Hashv2v2(Context &__ctx, f32x2 p);
float Noisefv2(Context &__ctx, f32x2 p);
float Fbm1(Context &__ctx, float p);
float Fbm2(Context &__ctx, f32x2 p);
f32x3 VaryNf(Context &__ctx, f32x3 p, f32x3 n, float f);
f32x3 ltDir, qHit, objOrg, cabSize;
float tCur, dstFar, tLoop, tCyc, abSide, nRec, selRec, cRec, nLoop, carslAng,
    bCylRad, bCylHt;
int idObj;
bool haveRec, showPick, showVol;
const int idCarsl = 1, idRec = 2, idGrab = 3, idGrabS = 4, idPlat = 5,
          idArm = 6, idNeed = 7, idSup = 8, idLeg = 9, idBase = 10, idPick = 11,
          idVol = 12, idArc = 13, idCab = 14, idSpkr = 15, idGril = 16;
const float pi = 3.1415927f, sqrt3 = 1.7320508f;
struct CS {
  f32x2 carslAng, armAng, armAngV, platAng, platAngS, recAng, recTwAng, grabAng,
      grabTwAng;
} cs;
void SetConf(Context &__ctx) {
  f32x3 armLim;
  f32x2 tpLim, taLim;
  float armAng, armAngV, grabAng, recAng, platAng, ti, tg, platSpd;
  nRec = 64.f;
  selRec = mod(nLoop * (floor(nRec / 2.f) - 5.f), nRec);
  abSide = 1.f - 2.f * mod(floor(nLoop * 2.3f), 2.f);
  ti = tLoop / tCyc;
  tg = 0.05f;
  platSpd = 7.f;
  tpLim = 0.05f + (selRec / nRec) * 0.2f + f32x2(0.f, 0.7f);
  if (ti < 0.05f || ti > 1.f - 0.05f)
    carslAng = 0.f;
  else if (ti < tpLim.data[0])
    carslAng = 5.f * (ti - 0.05f);
  else if (ti < tpLim.data[1])
    carslAng = selRec / nRec;
  else
    carslAng = 5.f * (ti - 0.05f) - 5.f * 0.7f;
  armLim = f32x3(0.f, 0.14f, 0.195f) * pi;
  taLim = tpLim + f32x2(1.f, -1.f) * 4.f * tg;
  if (ti < taLim.data[0] || ti >= taLim.data[1])
    armAng = armLim.data[0];
  else if (ti < taLim.data[0] + tg)
    armAng = mix(armLim.data[0], armLim.data[1], (ti - taLim.data[0]) / tg);
  else if (ti < taLim.data[1] - tg)
    armAng = mix(armLim.data[1], armLim.data[2],
                 (ti - (taLim.data[0] + tg)) /
                     (tpLim.data[1] - tpLim.data[0] - 10.f * tg));
  else if (ti < taLim.data[1])
    armAng =
        mix(armLim.data[2], armLim.data[0], (ti - (taLim.data[1] - tg)) / tg);
  armAngV = -0.02f * pi *
            (1.f - smoothstep(-0.2f * tg, 0.f, ti - (taLim.data[0] + tg)) +
             smoothstep(0.f, 0.2f * tg, ti - (taLim.data[1] - tg)));
  platAng = (ti > tpLim.data[0] + 4.f * tg && ti < tpLim.data[1] - 4.f * tg)
                ? platSpd * 2.f * pi * (ti - (tpLim.data[0] + 4.f * tg)) /
                      (tpLim.data[1] - tpLim.data[0] - 8.f * tg)
                : 0.f;
  recAng = pi * (smoothstep(tpLim.data[0] + tg, tpLim.data[0] + 3.f * tg, ti) -
                 smoothstep(tpLim.data[1] - 3.f * tg, tpLim.data[1] - tg, ti));
  grabAng =
      recAng +
      0.5f * pi *
          (1.f - smoothstep(tpLim.data[0], tpLim.data[0] + tg, ti) -
           smoothstep(tpLim.data[0] + 3.f * tg, tpLim.data[0] + 4.f * tg, ti) +
           smoothstep(tpLim.data[1] - 4.f * tg, tpLim.data[1] - 3.f * tg, ti) +
           smoothstep(tpLim.data[1] - tg, tpLim.data[1], ti));
  cs.carslAng = (sin((2.f * pi * carslAng) + f32x2(0.5f * pi, 0.f)));
  cs.recAng = (sin((recAng) + f32x2(0.5f * pi, 0.f)));
  cs.recTwAng =
      (sin((-0.5f * pi * abSide * smoothstep(0.5f * pi, 0.8f * pi, recAng)) +
           f32x2(0.5f * pi, 0.f)));
  cs.platAng = (sin((platAng) + f32x2(0.5f * pi, 0.f)));
  cs.platAngS = (sin((-abSide * platAng) + f32x2(0.5f * pi, 0.f)));
  cs.grabAng = (sin((grabAng) + f32x2(0.5f * pi, 0.f)));
  cs.grabTwAng =
      (sin((-0.5f * pi * abSide * smoothstep(0.5f * pi, 0.8f * pi, grabAng)) +
           f32x2(0.5f * pi, 0.f)));
  cs.armAng = (sin((armAng) + f32x2(0.5f * pi, 0.f)));
  cs.armAngV = (sin((armAngV) + f32x2(0.5f * pi, 0.f)));
  haveRec = (ti > tpLim.data[0] + tg && ti < tpLim.data[1] - tg);
  showPick = (ti > 0.05f && ti < 1.f - 0.05f);
  showVol = (armAngV == 0.f);
}
float ObjDf(Context &__ctx, f32x3 p) {
  f32x3 q, qq;
  float dMin, d, dd, dr, r, a, s, rRec, tRec, rHole, cRecT;
  rRec = 0.49f;
  tRec = 0.008f;
  rHole = 0.05f;
  dMin = dstFar;
  p -= objOrg;
  q = p;
  q.data[0] -= -1.2f;
  q.set_swizzle<0, 2>(Rot2Cs(__ctx, q.swizzle<0, 2>(), cs.carslAng));
  d = min(
      PrCylDf(__ctx,
              f32x3(((abs(q.data[0]) > abs(q.data[2])) ? q.swizzle<0, 2>()
                                                       : q.swizzle<2, 0>()),
                    q.data[1] + 0.07f)
                  .swizzle<1, 2, 0>(),
              0.02f, 0.5f),
      PrCylDf(__ctx, f32x3(q.swizzle<0, 2>(), q.data[1] + 0.3f), 0.1f, 0.3f));
  d = min(d, PrCylDf(__ctx,
                     f32x3(q.data[0] - 1.5f, q.data[1] + 0.06f, q.data[2])
                         .swizzle<1, 2, 0>(),
                     0.03f, 0.06f));
  if (d < dMin) {
    dMin = d;
    idObj = idCarsl;
    qHit = q;
  };
  q.set_swizzle<0, 2>(Rot2Cs(__ctx, q.swizzle<0, 2>(),
                             sin(pi / nRec + +f32x2(0.5f * pi, 0.f))));
  a = atan(q.data[2], -q.data[0]) / (2.f * pi);
  cRecT = (floor(nRec * (nRec * (a + 0.5f) - carslAng) + 0.5f) - 0.5f) / nRec;
  q.set_swizzle<0, 2>(Rot2Cs(
      __ctx, q.swizzle<0, 2>(),
      sin(2.f * pi * (floor(nRec * a) + 0.5f) / nRec + f32x2(0.5f * pi, 0.f))));
  q.data[0] -= -1.f;
  r = length(q.swizzle<0, 1>());
  dd = max(r - 0.55f, abs(q.data[2]) - 1.8f * tRec);
  qq = q;
  dr = max(abs(r - 0.5f * (rRec + rHole)) - 0.5f * (rRec - rHole),
           abs(q.data[2]) - tRec);
  q = p;
  q.data[0] -= -1.2f;
  r = length(q.swizzle<0, 2>()) - 1.f;
  s = length(f32x2(r, q.data[1]));
  d = max(min(max(abs(s - 0.45f) - 0.05f, -dd),
              max(abs(s - rRec - 0.02f) - 0.01f,
                  min(abs(r) - 0.1f, -q.data[1] - 0.1f))),
          q.data[1] + 0.02f);
  if (d < dMin) {
    dMin = d;
    idObj = idCarsl;
    qHit = q;
  };
  d = dr;
  if (haveRec)
    d = max(d, min(q.data[0], 4.f * tRec - abs(q.data[2])));
  q = qq;
  if (d < dMin)
    cRec = cRecT;
  if (d < dMin) {
    dMin = d;
    idObj = idRec;
    qHit = q;
  };
  if (haveRec) {
    q = p;
    q.data[0] -= 0.9f;
    q.set_swizzle<0, 1>(Rot2Cs(__ctx, q.swizzle<0, 1>(), cs.recAng));
    q.data[0] -= -1.1f;
    q.set_swizzle<1, 2>(Rot2Cs(__ctx, q.swizzle<1, 2>(), cs.recTwAng));
    d = max(abs(length(q.swizzle<0, 1>()) - 0.5f * (rRec + rHole)) -
                0.5f * (rRec - rHole),
            abs(q.data[2]) - tRec);
    q.set_swizzle<0, 1>(Rot2Cs(__ctx, q.swizzle<0, 1>(), cs.platAngS));
    if (d < dMin)
      cRec = nRec - 1.f - selRec;
    if (d < dMin) {
      dMin = d;
      idObj = idRec;
      qHit = q;
    };
  }
  q = p;
  q.data[0] -= 0.9f;
  d = PrCylDf(__ctx, q, 0.02f, 0.25f);
  if (d < dMin) {
    dMin = d;
    idObj = idGrabS;
    qHit = q;
  };
  q.data[2] = abs(q.data[2]);
  q.set_swizzle<1, 2>(q.swizzle<1, 2>() - (f32x2(-0.1f, 0.2f)));
  d = PrCylDf(__ctx, q.swizzle<0, 2, 1>(), 0.03f, 0.13f);
  if (d < dMin) {
    dMin = d;
    idObj = idSup;
    qHit = q;
  };
  q = p;
  q.data[0] -= 0.9f;
  q.set_swizzle<0, 1>(Rot2Cs(__ctx, q.swizzle<0, 1>(), cs.grabAng));
  q.data[0] -= -1.1f;
  q.set_swizzle<1, 2>(Rot2Cs(__ctx, q.swizzle<1, 2>(), cs.grabTwAng));
  d = max(max(abs(length(q.swizzle<0, 1>()) - rRec - 0.03f) - 0.03f,
              abs(q.data[2]) - 1.3f * tRec),
          -0.01f - q.data[1]);
  if (d < dMin) {
    dMin = d;
    idObj = idGrab;
    qHit = q;
  };
  d = PrCylDf(__ctx, f32x3(q.swizzle<1, 2>(), q.data[0] - 0.83f), 0.04f, 0.31f);
  if (d < dMin) {
    dMin = d;
    idObj = idGrabS;
    qHit = q;
  };
  q = p;
  q.set_swizzle<0, 1>(q.swizzle<0, 1>() - (f32x2(2.f, -0.05f - tRec)));
  q.set_swizzle<0, 2>(Rot2Cs(__ctx, q.swizzle<0, 2>(), cs.platAng));
  r = length(q.swizzle<0, 2>());
  d = min(max(r - (rRec - 0.02f), abs(q.data[1]) - 0.05f),
          max(r - rHole, abs(q.data[1]) - 0.08f));
  if (d < dMin) {
    dMin = d;
    idObj = idPlat;
    qHit = q;
  };
  q = p;
  q.set_swizzle<0, 2>(
      Rot2Cs(__ctx, q.swizzle<0, 2>() - f32x2(2.7f, 0.5f), cs.armAng));
  d = PrCylDf(__ctx, q.swizzle<0, 2, 1>(), 0.06f, 0.17f);
  q.data[1] -= 0.1f;
  d = max(d, 0.027f - length(q.swizzle<0, 1>()));
  if (d < dMin) {
    dMin = d;
    idObj = idSup;
    qHit = q;
  };
  d = PrCylDf(__ctx, q.swizzle<1, 2, 0>(), 0.02f, 0.08f);
  if (d < dMin) {
    dMin = d;
    idObj = idArm;
    qHit = q;
  };
  q.set_swizzle<1, 2>(Rot2Cs(__ctx, q.swizzle<1, 2>(), cs.armAngV));
  r = length(q.swizzle<0, 1>());
  d = min(min(max(r - 0.08f, abs(q.data[2] - 0.17f) - 0.03f),
              max(r - 0.02f, abs(q.data[2] + 0.31f) - 0.54f)),
          max(r - 0.03f, abs(q.data[2] + 0.81f) - 0.035f));
  if (d < dMin) {
    dMin = d;
    idObj = idArm;
    qHit = q;
  };
  q.set_swizzle<1, 2>(q.swizzle<1, 2>() - (f32x2(-0.04f, -0.81f)));
  d = PrCapsDf(__ctx, q.swizzle<0, 2, 1>(), 0.005f, 0.04f);
  if (d < dMin) {
    dMin = d;
    idObj = idNeed;
    qHit = q;
  };
  q = p;
  q -= f32x3(1.5f, -0.5f, -1.3f);
  d = PrRoundBoxDf(__ctx, q, f32x3(0.3f, 0.03f, 0.2f), 0.02f);
  if (d < dMin) {
    dMin = d;
    idObj = idPick;
    qHit = q;
  };
  q = p;
  q -= f32x3(0.5f, -0.5f, -1.3f);
  d = PrRoundBoxDf(__ctx, q, f32x3(0.15f, 0.03f, 0.4f), 0.02f);
  if (d < dMin) {
    dMin = d;
    idObj = idVol;
    qHit = q;
  };
  q = p;
  q.data[1] -= -0.6f;
  d = PrBoxDf(__ctx, q,
              f32x3(cabSize.swizzle<0, 2>() - 0.1f, 0.05f).swizzle<0, 2, 1>());
  d = min(d,
          PrBoxDf(__ctx, f32x3(q.data[0] - 1.8f, q.data[1] - 0.38f, q.data[2]),
                  f32x3(1.1f, 0.1f, 0.7f)));
  if (d < dMin) {
    dMin = d;
    idObj = idBase;
    qHit = q;
  };
  d = PrCylDf(
      __ctx,
      f32x3(abs(f32x2(q.data[0] - 1.8f, q.data[2])) - f32x2(0.85f, 0.4f),
            q.data[1] - 0.15f),
      0.15f, 0.13f);
  if (d < dMin) {
    dMin = d;
    idObj = idLeg;
    qHit = q;
  };
  q = p;
  q.set_swizzle<1, 2>(q.swizzle<1, 2>() -
                      (f32x2(-0.6f, cabSize.data[2] - 0.25f)));
  d = max(PrTorusDf(__ctx, q, 0.1f, cabSize.data[0] - 0.25f), -q.data[1]);
  if (d < dMin) {
    dMin = d;
    idObj = idArc;
    qHit = q;
  };
  q = p;
  q.data[1] -= -cabSize.data[1] - 0.65f;
  d = PrRoundBoxDf(__ctx, q, cabSize - 0.03f, 0.03f);
  dd = PrRoundBox2Df(__ctx, f32x2(abs(q.data[0]) - 1.5f, q.data[1]),
                     f32x2(0.82f, 1.98f), 0.03f);
  d = max(d, -max(dd, q.data[2]));
  if (d < dMin) {
    dMin = d;
    idObj = idCab;
    qHit = q;
  };
  q.data[2] -= -1.7f;
  d = max(dd, abs(q.data[2]) - 0.05f);
  if (d < dMin) {
    dMin = d;
    idObj = idSpkr;
    qHit = q;
  };
  q.set_swizzle<0, 1>((fract(3.f * q.swizzle<0, 1>() + 0.5f) - 0.5f) / 3.f);
  q.data[2] -= -0.15f;
  d = max(min(length(q.swizzle<0, 2>()), length(q.swizzle<1, 2>())) - 0.02f,
          dd);
  if (d < dMin) {
    dMin = d;
    idObj = idGril;
    qHit = q;
  };
  return dMin;
}
float ObjRay(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dHit, d;
  dHit = 0.f;
  for (int j = min(iFrame, 0); j < 160; j++) {
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
  e = f32x2(0.001f, -0.001f);
  for (int j = min(iFrame, 0); j < 4; j++) {
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
  for (int j = min(iFrame, 0); j < 30; j++) {
    h = ObjDf(__ctx, ro + d * rd);
    sh = min(sh, smoothstep(0.f, 0.05f * d, h));
    d += h;
    if (sh < 0.05f)
      break;
  }
  return 0.5f + 0.5f * sh;
}
float TrObjDf(Context &__ctx, f32x3 p) {
  f32x3 q;
  float dMin, d;
  dMin = dstFar;
  p -= objOrg;
  q = p;
  q.data[1] -= -0.67f;
  d = max(PrRoundCylDf(__ctx, q, cabSize.data[0] - 0.13f, 0.1f,
                       cabSize.data[2] - 0.13f),
          -q.data[1]);
  if (d < dMin) {
    dMin = d;
    idObj = 0;
    qHit = q;
  };
  return dMin;
}
float TrObjRay(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dHit, d;
  dHit = 0.f;
  for (int j = min(iFrame, 0); j < 120; j++) {
    d = TrObjDf(__ctx, ro + dHit * rd);
    if (d < 0.0005f || dHit > dstFar)
      break;
    dHit += d;
  }
  return dHit;
}
f32x3 TrObjNf(Context &__ctx, f32x3 p) {
  f32x4 v;
  f32x2 e;
  e = f32x2(0.001f, -0.001f);
  for (int j = min(iFrame, 0); j < 4; j++) {
    v[j] = TrObjDf(
        __ctx,
        p + ((j < 2)
                 ? ((j == 0) ? e.swizzle<0, 0, 0>() : e.swizzle<0, 1, 1>())
                 : ((j == 2) ? e.swizzle<1, 0, 1>() : e.swizzle<1, 1, 0>())));
  }
  v.data[0] = -v.data[0];
  return normalize(2.f * v.swizzle<1, 2, 3>() - dot(v, f32x4(1.f)));
}
f32x4 ObjCol(Context &__ctx, f32x2 &vf, bool &isMet) {
  f32x4 col4, cVol4;
  f32x3 c;
  f32x2 w, iw;
  float r, t, tVol;
  vf = f32x2(0.f);
  isMet = false;
  if (idObj <= idSup) {
    if (idObj == idCarsl) {
      col4 = f32x4(0.7f, 0.4f, 0.2f, 0.1f);
      isMet = true;
    } else if (idObj == idRec) {
      r = length(qHit.swizzle<0, 1>());
      if (qHit.data[1] > 0.f && r < 0.22f || r > 0.48f)
        col4 = f32x4(HsvToRgb(__ctx, f32x3(cRec / nRec, 0.8f, 1.f)), 0.1f);
      else if (qHit.data[1] < 0.f && r < 0.22f)
        col4 = f32x4(0.9f, 0.9f, 0.9f, 0.1f);
      else
        col4 = f32x4(0.4f, 0.4f, 0.45f, 0.1f) *
               (0.8f + 0.2f * step(abs(r - 0.35f), 0.06f));
      col4 *= 1.f - 0.8f * step(length(f32x2(((qHit.data[2] > 0.f)
                                                  ? qHit.data[0]
                                                  : abs(qHit.data[0]) - 0.04f),
                                             qHit.data[1] + 0.15f)),
                                0.02f);
      col4 *=
          1.f -
          0.8f *
              step(0.001f,
                   ShowIntPZ(__ctx,
                             f32x2(sign(qHit.data[2]) * qHit.data[0] - 0.075f,
                                   -(qHit.data[1] - 0.15f)),
                             0.3f * f32x2(0.5f, 0.25f), 2.f, selRec + 1.f));
      if (abs(qHit.data[0]) < 0.08f && abs(qHit.data[1] - 0.18f) < 0.006f)
        col4 *= 0.2f;
    } else if (idObj == idGrab) {
      col4 =
          f32x4(0.9f, 0.7f, 0.5f, 0.1f) *
          (0.5f + 0.5f * step(0.02f, Minv2(__ctx, abs(qHit.swizzle<0, 1>()))));
      isMet = true;
    } else if (idObj == idGrabS) {
      col4 = f32x4(0.9f, 0.7f, 0.5f, 0.1f);
      ;
      isMet = true;
    } else if (idObj == idPlat) {
      r = length(qHit.swizzle<0, 2>());
      if (r > 0.055f && r < 0.43f) {
        col4 =
            f32x4(0.4f, 0.4f, 0.5f, 0.1f) *
            (0.8f + 0.2f * smoothstep(0.01f, 0.02f,
                                      Minv2(__ctx, abs(qHit.swizzle<0, 2>()))));
        vf = f32x2(128.f, 0.2f);
      } else {
        col4 = f32x4(0.9f, 0.9f, 0.9f, 0.1f);
        if (abs(qHit.data[1]) < 0.04f)
          col4 *= 0.8f + 0.2f * smoothstep(0.02f, 0.3f,
                                           fract(32.f * (atan(qHit.data[2],
                                                              -qHit.data[0]) /
                                                             (2.f * pi) +
                                                         0.5f)));
        isMet = true;
      }
    } else if (idObj == idArm) {
      col4 = f32x4(0.9f, 0.7f, 0.5f, 0.1f);
      isMet = true;
    } else if (idObj == idNeed) {
      col4 = f32x4(0.9f, 0.9f, 0.f, -1.f);
      isMet = true;
    } else if (idObj == idSup) {
      col4 = f32x4(0.8f, 0.8f, 0.9f, 0.1f);
      isMet = true;
    }
  } else {
    if (idObj == idVol || idObj == idCab) {
      tVol = floor(16.f *
                   clamp(1.3f * Fbm1(__ctx, 8.f * floor(16.f * tCur) / 16.f) -
                             0.1f,
                         0.f, 1.f)) /
             16.f;
      cVol4 = f32x4(HsvToRgb(__ctx, f32x3(0.7f - 0.7f * tVol, 1.f, 1.f)), -1.f);
    }
    if (idObj == idLeg) {
      col4 = f32x4(0.9f, 0.9f, 1.f, 0.1f) *
             (0.8f + 0.2f * step(0.2f, fract(32.f * qHit.data[1])));
      isMet = true;
    } else if (idObj == idBase) {
      col4 = f32x4(f32x3(0.5f), 0.1f);
      if (min(abs(qHit.data[1] - 0.05f), abs(qHit.data[1] - 0.48f)) < 0.01f) {
        w = 12.f * qHit.swizzle<2, 0>();
        col4 *= mix(
            1.8f, 1.f,
            step(0.1f,
                 HexEdgeDist(__ctx, w - HexToPix(__ctx, PixToHex(__ctx, w)))));
      }
      col4 *=
          0.7f +
          0.3f * step(1.1f, length(f32x2(qHit.data[0] + 1.2f, qHit.data[2])));
    } else if (idObj == idPick) {
      col4 = f32x4(1.f, 1.f, 0.7f, -1.f);
      if (showPick) {
        col4.set_swizzle<0, 1, 2>(
            col4.swizzle<0, 1, 2>() *
            (1.f -
             0.8f * step(0.001f,
                         ShowIntPZ(
                             __ctx, qHit.swizzle<0, 2>() - f32x2(0.13f, -0.08f),
                             0.7f * f32x2(0.5f, 0.25f), 2.f, selRec + 1.f))));
        col4.set_swizzle<0, 1, 2>(
            col4.swizzle<0, 1, 2>() *
            (1.f - 0.8f * step(length(f32x2(qHit.data[0] - 0.22f,
                                            ((abSide > 0.f)
                                                 ? qHit.data[2]
                                                 : abs(qHit.data[2]) - 0.05f))),
                               0.03f)));
      }
    } else if (idObj == idVol) {
      col4 = f32x4(1.f, 1.f, 0.7f, 0.f);
      if (PrBox2Df(__ctx, qHit.swizzle<0, 2>(), f32x2(0.15f, 0.4f) - 0.01f) <
          0.f) {
        col4 = f32x4(0.2f, 0.2f, 0.2f, -1.f);
        if (showVol && (qHit.data[2] + 0.4f) / 0.8f < tVol)
          col4 = cVol4;
      }
    } else if (idObj == idArc) {
      t = fract(2.f * (atan(qHit.data[1], -qHit.data[0]) / (2.f * pi) + 0.5f) +
                0.5f * sin(0.5f * pi * tCur));
      col4 = (showVol) ? f32x4(HsvToRgb(__ctx, f32x3(t, 1.f, 1.f)), -1.f)
                       : f32x4(f32x3(1.f, 1.f, 0.7f) *
                                   (0.8f + 0.2f * sin(16.f * pi * t)),
                               -1.f);
    } else if (idObj == idCab) {
      col4 = f32x4(1.f, 0.9f, 1.f, 0.1f) *
             (0.8f + 0.2f * Fbm2(__ctx, f32x2(4.f, 1.f) *
                                            f32x2(((abs(qHit.data[0]) <
                                                    cabSize.data[0] - 0.01f)
                                                       ? qHit.data[0]
                                                       : qHit.data[2]),
                                                  qHit.data[1])));
      if (qHit.data[0] > 0.f &&
          length(f32x2(abs(qHit.data[2] - cabSize.data[2] + 0.6f) - 0.1f,
                       qHit.data[1] + cabSize.data[1] - 0.15f)) < 0.05f)
        col4 = f32x4(0.2f, 1.f, 0.2f, -1.f);
      t = Minv3(__ctx, f32x3(abs(qHit.swizzle<0, 2>()) -
                                 cabSize.swizzle<0, 2>() + 0.2f,
                             cabSize.data[1] - 0.3f - abs(qHit.data[1])));
      if (t < 0.f) {
        r = length(qHit.swizzle<1, 2>()) - 1.5f;
        if (r < 0.f) {
          w = 8.f * Rot2D(__ctx, qHit.swizzle<1, 2>(), pi / 12.f);
          iw = PixToHex(__ctx, w);
          t = mod(dot(mod(2.f * iw + iw.swizzle<1, 0>(), 3.f), f32x2(1.f, 2.f)),
                  3.f);
          c = f32x3(0.9f, 0.9f, 0.f);
          if (t > 0.f)
            c = mix(c.swizzle<0, 2, 2>(), c.swizzle<2, 0, 2>(), t - 1.f);
          c = mix(f32x3(0.f, 0.f, 1.f), c,
                  smoothstep(0.05f, 0.1f,
                             HexEdgeDist(__ctx, w - HexToPix(__ctx, iw))));
          col4 = f32x4(c, 0.1f);
        } else if (r < 0.05f)
          col4 *= 0.8f;
      } else if (t > 0.03f) {
        col4 = (showVol) ? cVol4
                         : f32x4(f32x3(1.f, 1.f, 0.9f) *
                                     (0.8f + 0.2f * sin(4.f * pi *
                                                        (qHit.data[1] + tCur))),
                                 -1.f);
      } else
        col4 *= 0.7f;
    } else if (idObj == idSpkr) {
      w = 6.f * qHit.swizzle<1, 0>();
      col4 = f32x4(0.6f, 0.2f, 0.f, 0.f) *
             (1.f +
              0.3f * step(HexEdgeDist(__ctx,
                                      w - HexToPix(__ctx, PixToHex(__ctx, w))),
                          0.07f));
      vf = f32x2(64.f, 0.5f);
    } else if (idObj == idGril) {
      col4 = f32x4(0.9f, 0.9f, 0.2f, 0.2f);
      isMet = true;
    }
  }
  return col4;
}
f32x4 InCylHit(Context &__ctx, f32x3 ro, f32x3 rd, float rad, float len) {
  f32x3 s, vn;
  float d, a, b, w, srdy;
  d = dstFar;
  vn = f32x3(0.f);
  a = dot(rd.swizzle<0, 2>(), rd.swizzle<0, 2>());
  b = dot(rd.swizzle<0, 2>(), ro.swizzle<0, 2>());
  w = b * b - a * (dot(ro.swizzle<0, 2>(), ro.swizzle<0, 2>()) - rad * rad);
  if (w > 0.f) {
    srdy = sign(rd.data[1]);
    if (a > 0.f) {
      d = (-b + sqrt(w)) / a;
      s = ro + d * rd;
    } else
      s.data[1] = len;
    if (abs(s.data[1]) < len)
      vn.set_swizzle<0, 2>(-s.swizzle<0, 2>() / rad);
    else {
      d = (-srdy * ro.data[1] + len) / abs(rd.data[1]);
      vn.data[1] = -srdy;
    }
  }
  return f32x4(d, vn);
}
f32x3 ShStagGrid(Context &__ctx, f32x2 p) {
  f32x2 q, sq, ss;
  q = p;
  if (2.f * floor(0.5f * floor(q.data[1])) != floor(q.data[1]))
    q.data[0] += 0.5f;
  sq = smoothstep(0.05f, 0.1f, abs(fract(q + 0.5f) - 0.5f));
  q = fract(q) - 0.5f;
  ss = 0.5f * smoothstep(0.3f, 0.5f, abs(q.swizzle<0, 1>())) *
       sign(q.swizzle<0, 1>());
  if (abs(q.data[0]) < abs(q.data[1]))
    ss.data[0] = 0.f;
  else
    ss.data[1] = 0.f;
  return f32x3(ss.data[0], 0.8f + 0.2f * sq.data[0] * sq.data[1], ss.data[1]);
}
f32x3 BgCol(Context &__ctx, f32x3 ro, f32x3 rd, bool &needSh) {
  f32x4 vc, db4, col4;
  f32x3 vn, rg, vnCyl;
  f32x2 qw, wSize;
  float dCyl, hy, y, b, a, ga;
  db4 = InCylHit(__ctx, ro - f32x3(0.f, bCylHt, 0.f), rd, bCylRad, bCylHt);
  dCyl = db4.data[0];
  vnCyl = db4.swizzle<1, 2, 3>();
  ro += dCyl * rd;
  needSh = false;
  if (vnCyl.data[1] == 0.f) {
    vn = vnCyl;
    hy = mod(ro.data[1] / bCylHt + 0.5f, 1.f) - 0.5f;
    a = atan(vn.data[0], -vn.data[2]) / pi;
    wSize = f32x2(0.28f, 0.33f);
    qw = abs(f32x2(mod(32.f * 0.5f * (1.f + a) + 0.5f, 1.f), abs(hy)) - 0.5f) -
         0.5f * wSize;
    if (Maxv2(__ctx, abs(qw) - wSize * f32x2(0.43f, 0.47f)) < 0.f) {
      col4 = f32x4(0.5f, 0.5f, 1.f, -1.f);
    } else {
      col4 = f32x4(0.2f, 0.4f, 0.2f, -1.f);
      if (Maxv2(__ctx, qw - 0.5f * wSize) > 0.f) {
        col4 = f32x4(0.4f, 0.2f, 0.f, 0.f);
        y = abs(abs(hy) - 0.5f) - 0.5f;
        if (y > -0.3f / 16.f) {
          col4 *= 0.8f;
          vn.set_swizzle<0, 2>(Rot2D(__ctx, vn.swizzle<0, 2>(), -pi * a));
          vn.set_swizzle<1, 2>(
              Rot2D(__ctx, vn.swizzle<1, 2>(),
                    0.15f * pi *
                        (1.f - 2.f * SmoothBump(__ctx, 0.15f, 0.25f, 0.05f,
                                                16.f * (0.5f - y))) *
                        sign(hy)));
          vn.set_swizzle<0, 2>(Rot2D(__ctx, vn.swizzle<0, 2>(), pi * a));
        } else {
          qw = 8.f * f32x2(12.f * a, 2.f * hy);
          rg = ShStagGrid(__ctx, qw);
          col4 *= rg.data[1] * (1.f - 0.2f * Noisefv2(__ctx, 32.f * qw));
          rg.set_swizzle<0, 2>(
              rg.swizzle<0, 2>() *
              (sign((abs(vn.data[0]) > abs(vn.data[2])) ? vn.data[0]
                                                        : vn.data[2])));
          if (abs(vn.data[0]) > abs(vn.data[2])) {
            if (rg.data[0] == 0.f)
              vn.set_swizzle<0, 1>(
                  Rot2D(__ctx, vn.swizzle<0, 1>(), rg.data[2]));
            else
              vn.set_swizzle<0, 2>(
                  Rot2D(__ctx, vn.swizzle<0, 2>(), rg.data[0]));
          } else {
            if (rg.data[0] == 0.f)
              vn.set_swizzle<2, 1>(
                  Rot2D(__ctx, vn.swizzle<2, 1>(), rg.data[2]));
            else
              vn.set_swizzle<2, 0>(
                  Rot2D(__ctx, vn.swizzle<2, 0>(), rg.data[0]));
          }
        }
      }
    }
  } else if (vnCyl.data[1] > 0.f) {
    vc = HexVor(__ctx, ro.swizzle<0, 2>());
    b = 1.f - smoothstep(-0.2f, -0.05f, rd.data[1]) *
                  smoothstep(0.4f, 0.8f, dCyl / (1.6f * bCylRad));
    vn = mix(f32x3(0.f, 1.f, 0.f),
             normalize(f32x3(-vc.swizzle<1, 2>(), 4.f)).swizzle<0, 2, 1>(),
             b * SmoothBump(__ctx, 0.05f, 0.13f, 0.02f, vc.data[0]));
    col4 = f32x4(
        HsvToRgb(__ctx,
                 f32x3(0.8f * floor(8.f * vc.data[3]) / 8.f,
                       0.5f * smoothstep(0.06f, 0.08f, vc.data[0]), 0.9f)) *
            (0.5f +
             0.5f * b * step(0.06f + 0.03f * vc.data[3], vc.data[0]) *
                 (1.f - 0.2f * Noisefv2(__ctx, 16.f * ro.swizzle<0, 2>()))),
        0.1f);
    needSh = true;
  } else {
    b = 0.f;
    qw = ro.swizzle<0, 2>() / bCylRad;
    ga = 2.39996f;
    for (float n = float(min(iFrame, 0)); n < 128.f; n++)
      b += 1.f -
           step(0.015f, length(qw - sqrt(n / 128.f) *
                                        sin(n * ga + f32x2(0.5f * pi, 0.f))));
    col4 = mix(f32x4(0.3f, 0.3f, 0.8f, 0.f), f32x4(1.f, 1.f, 0.4f, -1.f), b);
  }
  return (col4.data[3] >= 0.f) ? col4.swizzle<0, 1, 2>() *
                                     (0.3f + 0.7f * max(dot(vn, ltDir), 0.f))
                               : col4.swizzle<0, 1, 2>();
}
f32x3 ShowScene(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x4 col4;
  f32x3 col, vn, roo;
  f32x2 vf;
  float dstObj, dstTrObj, nDotL, sh;
  bool isMet, needSh;
  HexVorInit(__ctx);
  roo = ro;
  dstObj = ObjRay(__ctx, ro, rd);
  if (dstObj < dstFar) {
    ro += dstObj * rd;
    vn = ObjNf(__ctx, ro);
    col4 = ObjCol(__ctx, vf, isMet);
    if (vf.data[0] > 0.f)
      vn = VaryNf(__ctx, vf.data[0] * ro, vn, vf.data[1]);
    if (col4.data[3] >= 0.f) {
      nDotL = max(dot(vn, ltDir), 0.f);
      if (isMet)
        nDotL *= nDotL;
    }
  } else {
    col = BgCol(__ctx, ro, rd, needSh);
    if (needSh) {
      ro += (-ro.data[1] / rd.data[1]) * rd;
      vn = f32x3(0.f, 1.f, 0.f);
    }
  }
  sh = (dstObj < dstFar && col4.data[3] >= 0.f || dstObj >= dstFar && needSh)
           ? ObjSShadow(__ctx, ro + 0.01f * vn, ltDir)
           : 1.f;
  if (dstObj < dstFar) {
    if (col4.data[3] >= 0.f)
      col = col4.swizzle<0, 1, 2>() * (0.3f + 0.7f * sh * nDotL) +
            col4.data[3] * step(0.95f, sh) *
                pow(max(dot(ltDir, reflect(rd, vn)), 0.f), 32.f);
    else
      col = col4.swizzle<0, 1, 2>() * (0.4f + 0.6f * max(-dot(rd, vn), 0.f));
  } else {
    col = 0.2f + 0.4f * (0.5f + 0.5f * sh) * col;
  }
  ro = roo;
  dstTrObj = TrObjRay(__ctx, ro, rd);
  if (dstTrObj < min(dstObj, dstFar)) {
    ro += dstTrObj * rd;
    vn = TrObjNf(__ctx, ro);
    col = mix(col, BgCol(__ctx, ro, reflect(rd, vn), needSh),
              0.05f + 0.95f * pow(1.f - abs(dot(vn, rd)), 5.f));
  }
  return clamp(col, 0.f, 1.f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3x3 vuMat;
  f32x4 mPtr, dateCur;
  f32x3 ro, rd, vd, col;
  f32x2 canvas, uv;
  float el, az, zmFac, sr, t;
  canvas = iResolution.swizzle<0, 1>();
  uv = 2.f * fragCoord.swizzle<0, 1>() / canvas - 1.f;
  uv.data[0] *= canvas.data[0] / canvas.data[1];
  tCur = iTime;
  dateCur = iDate;
  mPtr = iMouse;
  mPtr.set_swizzle<0, 1>(mPtr.swizzle<0, 1>() / canvas - 0.5f);
  tCur = max(tCur - 1.f, 0.f);
  tCyc = 30.f;
  tCur +=
      floor(20.f * tCyc * floor(dateCur.data[3] / (20.f * tCyc)) / tCyc) * tCyc;
  tLoop = mod(tCur, tCyc);
  nLoop = 1.f + floor(tCur / tCyc);
  SetConf(__ctx);
  t = SmoothBump(__ctx, 0.15f, 0.85f, 0.1f, tLoop / tCyc);
  if (mPtr.data[2] > 0.f) {
    az = 2.f * pi * mPtr.data[0];
    el = pi * (mPtr.data[1] - 0.25f);
  } else {
    az = 0.15f * pi * sign(mod(nLoop, 2.f) - 0.5f) * sign(tLoop / tCyc - 0.5f) *
         (1.f - t);
    el = -pi * (0.25f + 0.17f * t);
  }
  el = clamp(el, -0.47f * pi, -0.05f * pi);
  bCylRad = 40.f;
  bCylHt = 15.f;
  cabSize = f32x3(3.1f, 3.f, 1.9f);
  objOrg = f32x3(0.f, 2.f * cabSize.data[1] + 0.75f, 0.f);
  ro = f32x3(0.f, objOrg.data[1] - 3.f, -0.9f * bCylRad);
  vd = normalize(objOrg + f32x3(0.f, 10.f, 0.f) - ro);
  az += atan(vd.data[2], -vd.data[0]) - 0.5f * pi;
  el += asin(vd.data[1]);
  vuMat = StdVuMat(__ctx, el, az);
  ro = vuMat * ro;
  ro.data[1] = max(ro.data[1], 1.f);
  vd.set_swizzle<0, 2>(Rot2D(__ctx, vd.swizzle<0, 2>(), 0.1f * pi));
  vd.set_swizzle<1, 2>(Rot2D(__ctx, vd.swizzle<1, 2>(), 0.2f * pi));
  ltDir = -vuMat * vd;
  zmFac = mix(4.5f, 12.f, smoothstep(0.f, 0.5f * pi, -el));
  dstFar = 100.f;
  const float naa = 1.f;
  col = f32x3(0.f);
  sr = 2.f * mod(dot(mod(floor(0.5f * (uv + 1.f) * canvas), 2.f), f32x2(1.f)),
                 2.f) -
       1.f;
  for (float a = float(min(iFrame, 0)); a < naa; a++) {
    rd =
        vuMat *
        normalize(f32x3(uv + step(1.5f, naa) *
                                 Rot2D(__ctx, f32x2(0.5f / canvas.data[1], 0.f),
                                       sr * (0.667f * a + 0.5f) * pi),
                        zmFac));
    col += (1.f / naa) * ShowScene(__ctx, ro, rd);
  }
  fragColor = f32x4(col, 1.f);
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
float PrRoundBox2Df(Context &__ctx, f32x2 p, f32x2 b, float r) {
  return length(max(abs(p) - b, 0.f)) - r;
}
float PrCylDf(Context &__ctx, f32x3 p, float r, float h) {
  return max(length(p.swizzle<0, 1>()) - r, abs(p.data[2]) - h);
}
float PrRoundCylDf(Context &__ctx, f32x3 p, float r, float rt, float h) {
  return length(max(f32x2(length(p.swizzle<0, 1>()) - r, abs(p.data[2]) - h),
                    0.f)) -
         rt;
}
float PrCapsDf(Context &__ctx, f32x3 p, float r, float h) {
  return length(p - f32x3(0.f, 0.f, clamp(p.data[2], -h, h))) - r;
}
float PrTorusDf(Context &__ctx, f32x3 p, float ri, float rc) {
  return length(f32x2(length(p.swizzle<0, 1>()) - rc, p.data[2])) - ri;
}
float Minv2(Context &__ctx, f32x2 p) { return min(p.data[0], p.data[1]); }
float Maxv2(Context &__ctx, f32x2 p) { return max(p.data[0], p.data[1]); }
float Minv3(Context &__ctx, f32x3 p) {
  return min(p.data[0], min(p.data[1], p.data[2]));
}
float Maxv3(Context &__ctx, f32x3 p) {
  return max(p.data[0], max(p.data[1], p.data[2]));
}
float SmoothMin(Context &__ctx, float a, float b, float r) {
  float h;
  h = clamp(0.5f + 0.5f * (b - a) / r, 0.f, 1.f);
  return mix(b - h * r, a, h);
}
float SmoothBump(Context &__ctx, float lo, float hi, float w, float x) {
  return (1.f - smoothstep(hi - w, hi + w, x)) * smoothstep(lo - w, lo + w, x);
}
float DigSeg(Context &__ctx, f32x2 q) {
  q = 1.f -
      smoothstep(f32x2(0.f), f32x2(0.04f, 0.07f), abs(q) - f32x2(0.13f, 0.5f));
  return q.data[0] * q.data[1];
}
float ShowDig(Context &__ctx, f32x2 q, int iv) {
  f32x2 vp, vm, vo;
  float d;
  int k, kk;
  vp = f32x2(0.5f, 0.5f);
  vm = f32x2(-0.5f, 0.5f);
  vo = vp - vm;
  if (iv == -1)
    k = 8;
  else if (iv < 2)
    k = (iv == 0) ? 119 : 36;
  else if (iv < 4)
    k = (iv == 2) ? 93 : 109;
  else if (iv < 6)
    k = (iv == 4) ? 46 : 107;
  else if (iv < 8)
    k = (iv == 6) ? 122 : 37;
  else
    k = (iv == 8) ? 127 : 47;
  q = (q - 0.5f) * f32x2(1.8f, 2.3f);
  d = 0.f;
  kk = k;
  k = kk;
  kk = k / 2;
  if (kk * 2 != k)
    d += DigSeg(__ctx, q.swizzle<1, 0>() - vo);
  k = kk;
  kk = k / 2;
  if (kk * 2 != k)
    d += DigSeg(__ctx, q.swizzle<0, 1>() - vp);
  k = kk;
  kk = k / 2;
  if (kk * 2 != k)
    d += DigSeg(__ctx, q.swizzle<0, 1>() - vm);
  k = kk;
  kk = k / 2;
  if (kk * 2 != k)
    d += DigSeg(__ctx, q.swizzle<1, 0>());
  k = kk;
  kk = k / 2;
  if (kk * 2 != k)
    d += DigSeg(__ctx, q.swizzle<0, 1>() + vm);
  k = kk;
  kk = k / 2;
  if (kk * 2 != k)
    d += DigSeg(__ctx, q.swizzle<0, 1>() + vp);
  k = kk;
  kk = k / 2;
  if (kk * 2 != k)
    d += DigSeg(__ctx, q.swizzle<1, 0>() + vo);
  return d;
}
float ShowIntPZ(Context &__ctx, f32x2 q, f32x2 cBox, float mxChar, float val) {
  float nDig, idChar, s, v;
  q = f32x2(-q.data[0], q.data[1]) / cBox;
  s = 0.f;
  if (Minv2(__ctx, q) >= 0.f && Maxv2(__ctx, q) < 1.f) {
    q.data[0] *= mxChar;
    nDig = mxChar;
    idChar = mxChar - 1.f - floor(q.data[0]);
    q.data[0] = fract(q.data[0]);
    v = max(val, 0.f) / pow(10.f, mxChar - idChar - 1.f);
    if (idChar >= mxChar - nDig)
      s = ShowDig(__ctx, q, int(mod(floor(v), 10.f)));
  }
  return s;
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
f32x2 PixToHex(Context &__ctx, f32x2 p) {
  f32x3 c, r, dr;
  c.set_swizzle<0, 2>(f32x2((1.f / sqrt3) * p.data[0] - (1.f / 3.f) * p.data[1],
                            (2.f / 3.f) * p.data[1]));
  c.data[1] = -c.data[0] - c.data[2];
  r = floor(c + 0.5f);
  dr = abs(r - c);
  r -= step(dr.swizzle<1, 2, 0>(), dr) * step(dr.swizzle<2, 0, 1>(), dr) *
       dot(r, f32x3(1.f));
  return r.swizzle<0, 2>();
}
f32x2 HexToPix(Context &__ctx, f32x2 h) {
  return f32x2(sqrt3 * (h.data[0] + 0.5f * h.data[1]), (3.f / 2.f) * h.data[1]);
}
float HexEdgeDist(Context &__ctx, f32x2 p) {
  p = abs(p);
  return (sqrt3 / 2.f) - p.data[0] +
         0.5f * min(p.data[0] - sqrt3 * p.data[1], 0.f);
}
f32x2 gVec[7], hVec[7];
void HexVorInit(Context &__ctx) {
  f32x3 e = f32x3(1.f, 0.f, -1.f);
  gVec[0] = e.swizzle<1, 1>();
  gVec[1] = e.swizzle<0, 1>();
  gVec[2] = e.swizzle<1, 0>();
  gVec[3] = e.swizzle<0, 2>();
  gVec[4] = e.swizzle<2, 1>();
  gVec[5] = e.swizzle<1, 2>();
  gVec[6] = e.swizzle<2, 0>();
  for (int k = 0; k < 7; k++)
    hVec[k] = HexToPix(__ctx, gVec[k]);
}
f32x4 HexVor(Context &__ctx, f32x2 p) {
  f32x4 sd, udm;
  f32x2 ip, fp, d, u;
  float amp;
  amp = 0.7f;
  ip = PixToHex(__ctx, p);
  fp = p - HexToPix(__ctx, ip);
  sd = f32x4(4.f);
  udm = f32x4(4.f);
  for (int k = min(iFrame, 0); k < 7; k++) {
    u = Hashv2v2(__ctx, ip + gVec[k]);
    d = hVec[k] +
        amp * (0.4f + 0.6f * u.data[0]) *
            sin(2.f * pi * (u.data[1] - 0.5f) + f32x2(0.5f * pi, 0.f)) -
        fp;
    sd.data[3] = dot(d, d);
    if (sd.data[3] < sd.data[0]) {
      sd = sd.swizzle<3, 0, 1, 3>();
      udm = f32x4(d, u);
    } else
      sd = (sd.data[3] < sd.data[1])
               ? sd.swizzle<0, 3, 1, 3>()
               : ((sd.data[3] < sd.data[2]) ? sd.swizzle<0, 1, 3, 3>() : sd);
  }
  sd.set_swizzle<0, 1, 2>(sqrt(sd.swizzle<0, 1, 2>()));
  return f32x4(SmoothMin(__ctx, sd.data[1], sd.data[2], 0.3f) - sd.data[0],
               udm.swizzle<0, 1>(), Hashfv2(__ctx, udm.swizzle<2, 3>()));
}
f32x3x3 StdVuMat(Context &__ctx, float el, float az) {
  f32x2 ori, ca, sa;
  ori = f32x2(el, az);
  ca = cos(ori);
  sa = sin(ori);
  return f32x3x3(ca.data[1], 0.f, -sa.data[1], 0.f, 1.f, 0.f, sa.data[1], 0.f,
                 ca.data[1]) *
         f32x3x3(1.f, 0.f, 0.f, 0.f, ca.data[0], -sa.data[0], 0.f, sa.data[0],
                 ca.data[0]);
}
f32x2 Rot2D(Context &__ctx, f32x2 q, float a) {
  f32x2 cs;
  cs = sin(a + f32x2(0.5f * pi, 0.f));
  return f32x2(dot(q, f32x2(cs.data[0], -cs.data[1])),
               dot(q.swizzle<1, 0>(), cs));
}
f32x2 Rot2Cs(Context &__ctx, f32x2 q, f32x2 cs) {
  return f32x2(dot(q, f32x2(cs.data[0], -cs.data[1])),
               dot(q.swizzle<1, 0>(), cs));
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
f32x3 Hashv3f(Context &__ctx, float p) {
  return fract(sin(p + f32x3(37.f, 39.f, 41.f)) * cHashM);
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
  for (int j = min(iFrame, 0); j < 4; j++)
    v[j] = Fbmn(
        __ctx,
        p + ((j < 2)
                 ? ((j == 0) ? e.swizzle<0, 1, 1>() : e.swizzle<1, 0, 1>())
                 : ((j == 2) ? e.swizzle<1, 1, 0>() : e.swizzle<1, 1, 1>())),
        n);
  g = v.swizzle<0, 1, 2>() - v.data[3];
  return normalize(n + f * (g - n * dot(n, g)));
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
