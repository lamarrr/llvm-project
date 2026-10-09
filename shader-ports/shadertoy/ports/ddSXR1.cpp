// Port of https://www.shadertoy.com/view/ddSXR1; author: Shane.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ddSXR1.json.
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
  Texture2D channel0;
  Texture2D channel1;
  TextureCube channel2;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

f32x2x2 rot2(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, -s, s, c);
}
float hash21(Context &__ctx, f32x2 p) {
  return fract(sin(dot(p, f32x2(27.617f, 57.643f))) * 43758.5453f);
}
float hash31(Context &__ctx, f32x3 p) {
  return fract(sin(dot(p, f32x3(113.619f, 57.583f, 27.897f))) * 43758.5453f);
}
float smin(Context &__ctx, float a, float b, float r) {
  float f = max(0.f, 1.f - abs(b - a) / r);
  return min(a, b) - r * .25f * f * f;
}
float smax(Context &__ctx, float a, float b, float k) {
  float f = max(0.f, 1.f - abs(b - a) / k);
  return max(a, b) + k * .25f * f * f;
}
float distLineS(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  b -= a;
  return dot(p - a, f32x2(-b.data[1], b.data[0]) / length(b));
}
const f32x2 s = f32x2(1, 1.732f);
const f32x2[6] vID = f32x2[6](f32x2(-6, -2), f32x2(-6, 2), f32x2(0, 4),
                              f32x2(6, 2), f32x2(6, -2), f32x2(0, -4));
const f32x2[6] eID = f32x2[6](f32x2(-6, 0), f32x2(-3, 3), f32x2(3, 3),
                              f32x2(6, 0), f32x2(3, -3), f32x2(-3, -3));
float getHex(Context &__ctx, f32x2 p, float r, float sf) {
  const f32x3 k = f32x3(.5f, -.8660254f, .57735f);
  p = abs(p);
  p -= 2.f * min(dot(k.swizzle<0, 1>(), p), 0.f) * k.swizzle<0, 1>();
  r -= sf;
  return length(p - f32x2(r, clamp(p.data[1], -k.data[2] * r, k.data[2] * r))) *
             sign(p.data[0] - r) -
         sf;
}
float getTri(Context &__ctx, f32x2 p, float r) {
  const float k = sqrt(3.0f);
  p.data[0] = abs(p.data[0]) - r;
  p.data[1] = p.data[1] + r / k;
  if (p.data[0] + k * p.data[1] > 0.f)
    p = f32x2(p.data[0] - k * p.data[1], -k * p.data[0] - p.data[1]) / 2.f;
  p.data[0] -= clamp(p.data[0], -2.f * r, 0.f);
  return -length(p) * sign(p.data[1]);
}
f32x4 getGrid(Context &__ctx, f32x2 p) {
  f32x4 ip = floor(f32x4(p / s, p / s - .5f));
  f32x4 q = p.swizzle<0, 1, 0, 1>() -
            f32x4(ip.swizzle<0, 1>() + .5f, ip.swizzle<2, 3>() + 1.f) *
                s.swizzle<0, 1, 0, 1>();
  return dot(q.swizzle<0, 1>(), q.swizzle<0, 1>()) <
                 dot(q.swizzle<2, 3>(), q.swizzle<2, 3>())
             ? f32x4(q.swizzle<0, 1>(), ip.swizzle<0, 1>() * 12.f)
             : f32x4(q.swizzle<2, 3>(), ip.swizzle<2, 3>() * 12.f + 6.f);
}
f32x2 polRot(Context &__ctx, f32x2 p, float &na, float aN) {
  float a = atan(p.data[1], p.data[0]);
  na = mod(floor(a / 6.2831f * aN), aN);
  float ia = (na + .5f) / aN;
  p *= rot2(__ctx, -ia * 6.2831f);
  return p;
}
const float scale = 1.f / 2.f;
float objID;
f32x3 tex3D(Context &__ctx, sampler2D t, f32x3 p, f32x3 n) {
  n = max(abs(n) - .2f, .001f);
  n /= length(n);
  f32x3 tx = texture(t, p.swizzle<1, 2>()).swizzle<0, 1, 2>();
  f32x3 ty = texture(t, p.swizzle<2, 0>()).swizzle<0, 1, 2>();
  f32x3 tz = texture(t, p.swizzle<0, 1>()).swizzle<0, 1, 2>();
  return f32x3x3(tx * tx, ty * ty, tz * tz) * n;
}
f32x3 getCol(Context &__ctx, sampler2D s2D, f32x2 p) {
  f32x3 tx = texture(s2D, p * scale / 16.f * s).swizzle<0, 1, 2>();
  return tx * tx;
}
float ht;
float getHeight(Context &__ctx, f32x2 p) {
  f32x3 col = getCol(__ctx, __ctx.channel0, p);
  if (fract(dot(col, f32x3(1)) * 143.5273f) < .2f)
    return -1.f;
  float h = dot(col, f32x3(.299f, .587f, .114f));
  ht = smoothstep(.9f, .94f, sin(6.2831f * h * 4.f + iTime));
  return h * .25f * (1.f + ht) + .025f;
}
float opExtrusion(Context &__ctx, float sdf, float pz, float h) {
  f32x2 w = f32x2(sdf, abs(pz) - h);
  return min(max(w.data[0], w.data[1]), 0.f) + length(max(w, 0.f));
}
f32x4 gVal;
f32x3 glow, lCol;
float map(Context &__ctx, f32x3 p) {
  float fl = abs(p.data[2] - .5f) - .5f;
  float gridScale = scale;
  f32x4 p4 = getGrid(__ctx, p.swizzle<0, 1>() / gridScale);
  float gHx = getHex(__ctx, p4.swizzle<0, 1>(), .5f, 0.f);
  const f32x2 sDiv12 = s / 12.f;
  const float ew = .0f;
  const float ang = atan(sqrt(3.f) / 5.f);
  const float invR = 1.f / sqrt(7.f);
  f32x2 q = p4.swizzle<0, 1>();
  q = rot2(__ctx, ang) * q;
  float hR = 1.5f * invR;
  float ctrHx = getHex(__ctx, q, hR, 0.f);
  float tR = hR / .8660254f;
  float ctrTri = getTri(__ctx, rot2(__ctx, 3.14159f / 6.f) * q, tR);
  ctrTri = min(ctrTri, getTri(__ctx, rot2(__ctx, -3.14159f / 6.f) * q, tR));
  ctrTri = max(ctrTri, ctrHx);
  float[6] flLn, flLn2, d6;
  for (int i = 0; i < 6; i++) {
    f32x2 vi = vID[i] * sDiv12;
    flLn[i] = dot(q, f32x2(-vi.data[1], vi.data[0]) * sqrt(3.f));
  }
  int j = int(mod(floor(hash21(__ctx, p4.swizzle<2, 3>()) * 36.f), 2.f));
  f32x2[6] cID6;
  for (int i = min(iFrame, 0); i < 6; i += 2) {
    int ip0 = (i + j) % 6;
    int ip1 = (i + 2 + j) % 6;
    d6[i] = (max(ctrTri, max(flLn[ip0], -flLn[ip1])) + ew) * gridScale;
    cID6[i] = p4.swizzle<2, 3>() + eID[i] * invR * 2.f;
  }
  float svHt = -1e5f;
  float df = 1e5f;
  for (int i = min(iFrame, 0); i < 6; i += 2) {
    float hi = getHeight(__ctx, cID6[i]);
    float di = opExtrusion(__ctx, d6[i], p.data[2] + hi / 2.f, hi / 2.f);
    di += d6[i] * .125f;
    if (di < df) {
      df = di;
      gVal = f32x4(d6[i], hi, cID6[i]);
      svHt = ht;
    }
  }
  float na;
  q = polRot(__ctx, q, na, 6.f);
  q.data[0] -= tR;
  q *= rot2(__ctx, 3.14159f / 6.f);
  float smHx = getHex(__ctx, q, invR / 2.f - ew, 0.f);
  smHx *= gridScale;
  int ind = int(mod(8.f - na, 6.f));
  int jn = int(mod(
      floor(hash21(__ctx, p4.swizzle<2, 3>() + eID[ind] * 2.f) * 36.f), 2.f));
  int ind2 = int(mod(7.f - floor((na + float(jn)) / 2.f) * 2.f, 6.f));
  f32x2 smHxID =
      p4.swizzle<2, 3>() + eID[(ind) % 6] * 2.f - eID[(ind2) % 6] * invR * 2.f;
  float hgt = getHeight(__ctx, smHxID);
  float dB = opExtrusion(__ctx, smHx, p.data[2] + hgt / 2.f, hgt / 2.f);
  dB += smHx * .125f;
  if (dB < df) {
    df = dB;
    gVal = f32x4(smHx, hgt, smHxID);
    svHt = ht;
  }
  objID = fl < df ? 1.f : 0.f;
  lCol = f32x3(0);
  if (df < .25f && objID < .5f) {
    f32x3 gCol = mix(f32x3(1, .1f, .3f), f32x3(1.f, .3f, .1f),
                     hash21(__ctx, gVal.swizzle<2, 3>() + .12f));
    lCol = svHt * gCol * smoothstep(0.f, .5f, -(gVal.data[0]));
  }
  return min(fl, df);
}
float trace(Context &__ctx, f32x3 ro, f32x3 rd) {
  float t = hash31(__ctx, fract(ro / 7.319f) + rd) * .1f, d;
  glow = f32x3(0);
  for (int i = min(iFrame, 0); i < 96; i++) {
    d = map(__ctx, ro + rd * t);
    if (abs(d) < .001f || t > 20.f)
      break;
    glow += lCol;
    t += min(d * .7f, .1f);
  }
  return min(t, 20.f);
}
f32x3 getNormal(Context &__ctx, f32x3 p, float t) {
  float sgn = 1.f;
  f32x3 e = f32x3(.004f, 0, 0), mp = e.swizzle<2, 2, 2>();
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
  const int maxIterationsShad = 32;
  ro += n * .0015f;
  f32x3 rd = lp - ro;
  float shade = 1.f;
  float t = 0.f;
  float end = max(length(rd), .0001f);
  rd /= end;
  for (int i = min(iFrame, 0); i < maxIterationsShad; i++) {
    float d = map(__ctx, ro + rd * t);
    shade = min(shade, k * d / t);
    t += clamp(d, .005f, .1f);
    if (d < 0.f || t > end)
      break;
  }
  return max(shade, 0.f);
}
float calcAO(Context &__ctx, f32x3 p, f32x3 n) {
  float sca = 3.f, occ = 0.f;
  for (int i = 0; i < 5; i++) {
    float hr = float(i + 1) * .15f / 5.f;
    float d = map(__ctx, p + n * hr);
    occ += (hr - d) * sca;
    sca *= .7f;
  }
  return clamp(1.f - occ, 0.f, 1.f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - iResolution.swizzle<0, 1>() * .5f) / iResolution.data[1];
  f32x3 ro = f32x3(cos(iTime / 4.f) * .15f,
                   sin(iTime / 4.f) * .15f + iTime / 8.f, -1.5f);
  f32x3 lk = f32x3(0, iTime / 8.f + .6f, 0);
  f32x3 lp = lk + f32x3(-.5f, 1.5f, -1);
  float FOV = 1.f;
  f32x3 fwd = normalize(lk - ro);
  f32x3 rgt = normalize(cross(f32x3(0, 1, 0), fwd));
  f32x3 up = cross(fwd, rgt);
  f32x3 rd = normalize(uv.data[0] * rgt + uv.data[1] * up + fwd / FOV);
  float t = trace(__ctx, ro, rd);
  float svObjID = objID;
  f32x4 svVal = gVal;
  f32x3 col = f32x3(0);
  if (t < 20.f) {
    f32x3 sp = ro + rd * t;
    f32x3 sn = getNormal(__ctx, sp, t);
    f32x3 ld = lp - sp;
    float lDist = max(length(ld), .001f);
    ld /= lDist;
    float sh = softShadow(__ctx, sp, lp, sn, 16.f);
    float ao = calcAO(__ctx, sp, sn);
    float atten = 1.f / (1.f + lDist * .05f);
    float diff = max(dot(sn, ld), 0.f);
    float spec = pow(max(dot(reflect(ld, sn), rd), 0.f), 32.f);
    float Schlick = pow(1.f - max(dot(rd, normalize(rd + ld)), 0.f), 5.f);
    float freS = mix(.15f, 1.f, Schlick);
    f32x3 texCol = f32x3(.6f);
    f32x3 txP = sp;
    float sf = 1.5f / iResolution.data[1];
    float ew = .005f;
    if (svObjID < .5f) {
      float d = svVal.data[0];
      float h = svVal.data[1];
      float dz = txP.data[2] + h;
      f32x3 fCol = getCol(__ctx, __ctx.channel0, svVal.swizzle<2, 3>() / 1.f);
      fCol = f32x3(1.35f) * (dot(fCol, f32x3(.299f, .587f, .114f)) + .25f);
      f32x3 tx = tex3D(__ctx, __ctx.channel1, sp - f32x3(0, 0, -h), sn);
      f32x3 hCol = mix(f32x3(1, .1f, .3f), f32x3(1.f, .3f, .1f),
                       hash21(__ctx, svVal.swizzle<2, 3>() + .12f));
      f32x3 tx2 = mix(tx * f32x3(.8f, 1, 1.2f) * (hCol * .5f + 1.25f), tx,
                      min(glow.data[0] * 2.f, 1.f));
      f32x3 sideCol = f32x3(.35f);
      texCol = mix(sideCol, f32x3(0), 1.f - smoothstep(0.f, sf, dz - ew));
      texCol = mix(texCol, sideCol, 1.f - smoothstep(0.f, sf, d + ew));
      texCol = mix(texCol, f32x3(0), 1.f - smoothstep(0.f, sf, d + .04f));
      texCol = mix(texCol * (tx * 2.f + .05f), fCol * (tx2 * 2.f + .05f),
                   1.f - smoothstep(0.f, sf, d + .04f + ew * 2.f));
    } else {
      texCol = f32x3(.7f);
      f32x3 tx = tex3D(__ctx, __ctx.channel1, sp, sn);
      texCol *= tx + .05f;
      float d = abs(svVal.data[0]) - ew * 1.5f;
      if (svVal.data[1] > 0.f)
        texCol = mix(texCol, f32x3(0), 1.f - smoothstep(0.f, sf, d));
    }
    f32x3 hv = normalize(-rd + ld);
    f32x3 ref = reflect(rd, sn);
    f32x3 refTx = texture(__ctx.channel2, ref).swizzle<0, 1, 2>();
    refTx *= refTx;
    refTx = (texCol * 1.5f + .66f) * refTx;
    float spRef = pow(max(dot(hv, sn), 0.f), 8.f);
    float rf = (svObjID == 1.f) ? .25f : 1.f;
    texCol += spRef * refTx * rf * 1.f;
    col = texCol *
          (diff * sh + .3f + f32x3(1, .97f, .92f) * spec * freS * 2.f * sh);
    col *= f32x3(1.1f, 1, .9f);
    col =
        col / 2.f + col * glow.swizzle<2, 1, 0>() / f32x3(1.1f, 1, .9f) * 40.f;
    col *= ao * atten;
  }
  col = mix(col, f32x3(0), smoothstep(0.f, .99f, t / 20.f));
  fragColor = f32x4(sqrt(max(col, 0.f)), 1);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0,
               __binding__(0, 1) texture_2d<f32x4> channel1,
               __binding__(1, 1) sampler sampling1,
               __binding__(0, 2) texture_cube<f32x4> channel2,
               __binding__(1, 2) sampler sampling2) {
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
  Context __ctx{
      {channel0, sampling0}, {channel1, sampling1}, {channel2, sampling2}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
