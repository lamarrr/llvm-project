// Port of https://www.shadertoy.com/view/DtfXzB; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtfXzB.json.
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
float hash(Context &__ctx, float a, float b) {
  int x = floatBitsToInt(a * a / 7.f) ^ floatBitsToInt(a),
      y = floatBitsToInt(b * b / 7.f) ^ floatBitsToInt(b);
  return float((x * x + y) * (y * y - x) - x) / 2.14e9f;
}
f32x3 erot(Context &__ctx, f32x3 p, f32x3 ax, float ro) {
  return mix(dot(ax, p) * ax, p, cos(ro)) + cross(ax, p) * sin(ro);
}
float cc(Context &__ctx, float a, float b) {
  float f = tanh(a * cos(b)) / tanh(a);
  return sign(f) * pow(abs(f), 0.25f);
}
float cs(Context &__ctx, float a, float b) {
  float f = tanh(a * sin(b)) / tanh(a);
  return sign(f) * pow(abs(f), 0.25f);
}
float h21(Context &__ctx, f32x2 a) {
  return fract(sin(dot(a.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
float mlength(Context &__ctx, f32x2 uv) {
  return max(abs(uv.data[0]), abs(uv.data[1]));
}
float mlength(Context &__ctx, f32x3 uv) {
  return max(max(abs(uv.data[0]), abs(uv.data[1])), abs(uv.data[2]));
}
float sfloor(Context &__ctx, float a, float b) {
  return floor(b - .5f) + .5f +
         .5f * tanh(a * (fract(b - .5f) - .5f)) / tanh(.5f * a);
}
float smin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.f, 1.f);
  return mix(b, a, h) - k * h * (1.f - h);
}
float smax(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f - 0.5f * (b - a) / k, 0.f, 1.f);
  return mix(b, a, h) + k * h * (1.f - h);
}
float getFreq(Context &__ctx, float x) {
  return texture(__ctx.channel0, f32x2(floor(x * 64.0f + 1.f) / 64.0f, 0))
      .data[0];
}
float getFreq_smooth(Context &__ctx, float x) {
  float index = floor(x * 64.0f) / 64.0f;
  float next = floor(x * 64.0f + 1.0f) / 64.0f;
  return mix(getFreq(__ctx, index), getFreq(__ctx, next),
             smoothstep(0.0f, 1.0f, fract(x * 64.0f)));
}
f32x3 hsv2rgb(Context &__ctx, f32x3 c) {
  f32x3 rgb = clamp(
      abs(mod(c.data[0] * 6.0f + f32x3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f,
      0.0f, 1.0f);
  rgb = rgb * rgb * (3.0f - 2.0f * rgb);
  return c.data[2] * mix(f32x3(1.0f), rgb, c.data[1]);
}
float seg(Context &__ctx, f32x3 p, f32x3 a, f32x3 b) {
  f32x3 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h);
}
f32x3 ori(Context &__ctx) {
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float r = 5.5f;
  float a = 0.25f * iTime;
  f32x3 ro = f32x3(r * cos(a), 2.f + 0.5f * cos(iTime), r * sin(a));
  ro = mix(ro, f32x3(4, 2, 2.f * cos(a)),
           .5f + .5f * tanh(4.f * cos(0.125f * iTime)) / tanh(4.f));
  return ro;
}
f32x2 solve(Context &__ctx, f32x2 p, float r1, float r2) {
  float h = dot(p, p);
  float w = h + r1 * r1 - r2 * r2;
  float s = max(4.0f * r1 * r1 * h - w * w, 0.0f);
  return (w * p + f32x2(-p.data[1], p.data[0]) * sqrt(s)) * 0.5f / h;
}
f32x3 getKnee(Context &__ctx, f32x3 hip, f32x3 foot, f32x3 toe, float rot) {
  f32x3 vy = normalize(hip - foot);
  f32x3 toeDir = foot - toe;
  f32x3 vx = normalize(toeDir - dot(toeDir, vy) / dot(vy, vy) * vy);
  f32x2 vHip = f32x2(0, length(hip - foot));
  f32x2 vKnee = solve(__ctx, vHip, 0.45f, 0.4f);
  return foot + vKnee.data[0] * vx + vKnee.data[1] * vy;
}
float stride(Context &__ctx, float offset) {
  float t = mod(iTime + offset, 2.f);
  float mt = min(t, 1.f);
  return 0.02f + 0.2f * pow(4.f * mt * (1.f - mt), 2.f);
}
f32x2 map(Context &__ctx, f32x3 p) {
  float t = iTime;
  float ct = cos(3.1415926535f * t);
  float st = sin(3.1415926535f * t);
  float c2t = cos(2.f * 3.1415926535f * t);
  float s2t = sin(2.f * 3.1415926535f * t);
  float mx = .5f - .5f *
                       tanh(8.f * sin(0.08f * 3.1415926535f + 0.25f * iTime)) /
                       tanh(8.f);
  float mx2 = .5f - .5f *
                        tanh(4.f * sin(0.08f * 3.1415926535f + 0.5f * iTime)) /
                        tanh(4.f);
  float y = 0.02f * dot(p.swizzle<0, 2>(), p.swizzle<0, 2>());
  p.data[1] = mix(p.data[1], abs(p.data[1]) + 0.005f, mx2);
  float sc = mix(2.5f, 4.f, 1.f - mx2);
  p.set_swizzle<0, 2>(mix(p.swizzle<0, 2>(),
                          mod(p.swizzle<0, 2>() - 0.5f * sc, sc) - 0.5f * sc,
                          mx));
  float zClamp = clamp(2.f * p.data[2], -3.1415926535f, 3.1415926535f);
  float terrain = 1.f - tanh(4.f * cos(zClamp)) / tanh(4.f);
  terrain *= 0.5f + 0.5f * cos(p.data[2] + t) * cos(p.data[0] + t);
  p.data[1] += y;
  float pd = p.data[1];
  pd += mix(0.5f * terrain, 0.f, mx);
  f32x3 hip = f32x3(0, 0.82f - 0.04f * s2t, 0);
  f32x3 hipL = hip - f32x3(-0.05f * ct, -0.025f * st, 0.125f);
  f32x3 hipR = hip + f32x3(-0.05f * ct, -0.025f * st, 0.125f);
  f32x3 footL = f32x3(0.3f * ct, stride(__ctx, 0.f), -0.2f);
  f32x3 footR = f32x3(-0.3f * ct, stride(__ctx, 1.f), 0.2f);
  f32x3 toeOffset = f32x3(0.14f, 0, 0);
  f32x3 toeL = footL - toeOffset;
  f32x3 toeR = footR - toeOffset;
  f32x3 kneeL = getKnee(__ctx, hipL, footL, toeL, 0.f);
  f32x3 kneeR = getKnee(__ctx, hipR, footR, toeR, 0.f);
  f32x3 neck = hip + f32x3(0.05f * c2t, 0.55f, 0.05f * st);
  f32x3 head = neck + f32x3(0.05f * ct, 0.31f, 0);
  f32x3 shoulderL = neck + f32x3(-0.05f * ct, -0.05f * st, 0.23f);
  f32x3 shoulderR = neck - f32x3(-0.05f * ct, -0.05f * st, 0.23f);
  float eaL = -3.1415926535f / 2.f - 0.4f * 3.1415926535f * ct;
  float eaR = -3.1415926535f / 2.f + 0.4f * 3.1415926535f * ct;
  f32x3 elbowL = shoulderL + f32x3(0.3f * cos(eaL), 0.3f * sin(eaL), 0.14f);
  f32x3 elbowR = shoulderR + f32x3(0.3f * cos(eaR), 0.3f * sin(eaR), -0.14f);
  eaL -= 0.4f * 3.1415926535f * (.25f + .5f * st);
  eaR -= 0.4f * 3.1415926535f * (.25f + .5f * st);
  f32x3 handL = elbowL + f32x3(0.3f * cos(eaL), 0.3f * sin(eaL), 0.09f);
  f32x3 handR = elbowR + f32x3(0.3f * cos(eaR), 0.3f * sin(eaR), -0.09f);
  f32x3 eyeL = head + f32x3(-0.08f, 0, -0.05f);
  f32x3 eyeR = head + f32x3(-0.08f, 0, 0.05f);
  float boneRadius = mix(0.04f, 0.2f, 1.f - mx2);
  float headDist = length(p - head) - 0.1f;
  float headNeckDist = seg(__ctx, p, head, neck) - boneRadius;
  float neckHipDist = seg(__ctx, p, neck, hip) - boneRadius;
  float shoulderDist = seg(__ctx, p, shoulderL, shoulderR) - boneRadius;
  f32x2 shoulderElbowDist = f32x2(seg(__ctx, p, shoulderL, elbowL),
                                  seg(__ctx, p, shoulderR, elbowR)) -
                            boneRadius;
  f32x2 elbowHandDist =
      f32x2(seg(__ctx, p, elbowL, handL), seg(__ctx, p, elbowR, handR)) -
      boneRadius;
  f32x2 handDist = f32x2(length(p - handL), length(p - handR)) - boneRadius;
  float hipDist = seg(__ctx, p, hipL, hipR) - 0.02f;
  f32x2 hipKneeDist =
      f32x2(seg(__ctx, p, hipL, kneeL), seg(__ctx, p, hipR, kneeR)) -
      boneRadius;
  f32x2 kneeFootDist =
      f32x2(seg(__ctx, p, kneeL, footL), seg(__ctx, p, kneeR, footR)) -
      boneRadius;
  f32x2 footToeDist =
      f32x2(seg(__ctx, p, footL, toeL), seg(__ctx, p, footR, toeR)) -
      boneRadius;
  f32x2 eyeDist = f32x2(length(p - eyeL), length(p - eyeR)) - 0.025f;
  float sa = mix(0.2f, 0.4f, mx2);
  float bd = hipDist;
  bd = smin(__ctx, bd,
            smin(__ctx, hipKneeDist.data[0], hipKneeDist.data[1], 0.5f * sa),
            sa);
  bd = smin(__ctx, bd, min(kneeFootDist.data[0], kneeFootDist.data[1]), sa);
  bd =
      smin(__ctx, bd, min(footToeDist.data[0], footToeDist.data[1]), 0.5f * sa);
  bd = smin(__ctx, bd, neckHipDist, sa);
  bd = smin(__ctx, bd, headDist, sa);
  float ad = shoulderDist;
  ad =
      smin(__ctx, ad, min(shoulderElbowDist.data[0], shoulderElbowDist.data[1]),
           0.5f * sa);
  ad = smin(__ctx, ad, min(elbowHandDist.data[0], elbowHandDist.data[1]),
            0.5f * sa);
  ad = smin(__ctx, ad, min(handDist.data[0], handDist.data[1]), 0.75f * sa);
  bd = smin(__ctx, bd, ad, 0.25f * sa);
  float ed = min(eyeDist.data[0], eyeDist.data[1]);
  if (ed < bd)
    return f32x2(ed, 0.25f);
  if (bd < pd)
    return f32x2(bd, 0.5f);
  return f32x2(pd, 1);
}
f32x3 march(Context &__ctx, f32x3 ro, f32x3 rd, float z) {
  float d = 0.f;
  float s = sign(z);
  int steps = 0;
  float mat = 0.f;
  for (int i = 0; i < 400; i++) {
    f32x3 p = ro + rd * d;
    f32x2 m = map(__ctx, p);
    m.data[0] *=
        0.9f + 0.1f * hash(__ctx, hash(__ctx, p.data[0], p.data[2]), p.data[1]);
    if (s != sign(m.data[0])) {
      z *= 0.5f;
      s = sign(m.data[0]);
    }
    if (abs(m.data[0]) < .001f || d > 100.f) {
      steps = i + 1;
      mat = m.data[1];
      break;
    }
    d += m.data[0] * z;
  }
  return f32x3(min(d, 100.f), steps, mat);
}
f32x3 norm(Context &__ctx, f32x3 p) {
  float d = map(__ctx, p).data[0];
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = d - f32x3(map(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                      map(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                      map(__ctx, p - e.swizzle<1, 1, 0>()).data[0]);
  return normalize(n);
}
f32x3 dir(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i);
  return d;
}
float AO(Context &__ctx, f32x3 p, f32x3 n) {
  float occ = 0.f;
  float sc = 1.f;
  for (float i = 0.f; i < 5.f; i++) {
    float h = 0.015f + 0.015f * i;
    float d = map(__ctx, p + h * n).data[0];
    occ += (h - d) * sc;
    sc *= 0.95f;
  }
  return clamp(1.f - 3.f * occ, 0.f, 1.f);
}
float shadow(Context &__ctx, f32x3 ro, f32x3 rd) {
  float res = 1.f;
  float t = .001f;
  for (int i = 0; i < 24; i++) {
    float h = map(__ctx, ro + rd * t).data[0];
    float s = clamp(32.f * h / t, 0.f, 1.f);
    res = min(res, s);
    t += clamp(h, 0.01f, 0.2f);
    if (res < .001f || t > 100.f)
      break;
  }
  res = clamp(res, 0.0f, 1.0f);
  return smoothstep(0.f, 1.f, res);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 ro = ori(__ctx);
  float zoomMix = tanh(0.15f * iTime);
  float zm = mix(5.6f, 1.6f, zoomMix);
  float aimHeight = mix(1.5f, 1.15f, zoomMix);
  f32x3 rd = dir(__ctx, uv, ro, f32x3(0, aimHeight, 0), zm);
  f32x3 col = f32x3(0);
  float fr = getFreq_smooth(__ctx, 0.5f);
  f32x3 myCol = hsv2rgb(__ctx, f32x3(2.f * fr, 0.25f + 0.5f * fr, 1.f));
  f32x3 m = march(__ctx, ro, rd, 1.f);
  float d = m.data[0];
  f32x3 p = ro + rd * d;
  if (d < 100.f) {
    f32x3 n = norm(__ctx, p);
    f32x3 r = reflect(rd, n);
    f32x3 ld = normalize(f32x3(1, 2, 3));
    float dif = dot(n, ld) * .5f + .5f;
    float spec = pow(dif, 16.f);
    float fres = pow(1.f + dot(rd, n), 5.f);
    col = f32x3(dif);
    col = mix(col, f32x3(1), fres);
    if (m.data[2] == 1.f) {
      float xmot =
          0.25f * 3.1415926535f *
          (iTime - .25f * sin(2.f * 3.1415926535f * iTime) / 3.1415926535f);
      col *=
          0.75f + 0.25f * abs(step(mod(2.f * (p.data[0] - xmot), 1.f), 0.5f) -
                              step(mod(2.f * p.data[2], 1.f), 0.5f));
    }
    if (m.data[2] == 0.5f) {
      f32x3 tx = texture(__ctx.channel2, n).swizzle<0, 1, 2>();
      n = abs(n);
      f32x3 c1 = texture(__ctx.channel1, .5f + .5f * p.swizzle<0, 1>())
                     .swizzle<0, 1, 2>();
      f32x3 c2 = texture(__ctx.channel1, .5f + .5f * p.swizzle<1, 2>())
                     .swizzle<0, 1, 2>();
      f32x3 c3 = texture(__ctx.channel1, .5f + .5f * p.swizzle<2, 0>())
                     .swizzle<0, 1, 2>();
      f32x3 c = c1 * n.data[2] + c2 * n.data[0] + c3 * n.data[1];
      col *= c;
      col *=
          f32x3(.82f + .18f * tanh(4.f * cos(36.f * p.data[0] - 4.f * iTime)) /
                           tanh(4.f));
    }
    float sh = shadow(__ctx, p + 20.f * .001f * n, ld);
    col = mix(col, 0.9f * col * (.5f + .5f * myCol), 1.f - sh);
  }
  col *= 1.f / cosh(0.5f * length(p.swizzle<0, 2>()));
  col = mix(col, myCol, 1.f - exp(-0.01f * m.data[1]));
  col = tanh(1.4f * col);
  fragColor = f32x4(col, 1.0f);
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
