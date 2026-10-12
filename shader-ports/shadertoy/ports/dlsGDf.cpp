// Port of https://www.shadertoy.com/view/dlsGDf; author: mrange.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dlsGDf.json.
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
const f32x3 bcol = f32x3(0.f, 1.0f, 0.25f) * 0.8f;
const float logo_radius = 0.25f;
const float logo_off = 0.25f;
const float logo_width = 0.10f;
float atan_approx(Context &__ctx, float y, float x) {
  float cosatan2 = x / (abs(x) + abs(y));
  float t = (0.5f * 3.141592654f) - cosatan2 * (0.5f * 3.141592654f);
  return y < 0.0f ? -t : t;
}
float mod1(Context &__ctx, float &p, float size) {
  float halfsize = size * 0.5f;
  float c = floor((p + halfsize) / size);
  p = mod(p + halfsize, size) - halfsize;
  return c;
}
float spiralLength(Context &__ctx, float b, float a) {
  return 0.5f * b * (a * sqrt(1.0f + a * a) + log(a + sqrt(1.0f + a * a)));
}
void spiralMod(Context &__ctx, f32x2 &p, float a) {
  f32x2 op = p;
  float b = a / (2.0f * 3.141592654f);
  float rr = length(op);
  float aa = atan(op.data[1], op.data[0]);
  rr -= aa * b;
  float nn = mod1(__ctx, rr, a);
  float sa = aa + (2.0f * 3.141592654f) * nn;
  float sl = spiralLength(__ctx, b, sa);
  p = f32x2(sl, rr);
}
float dsegmentx(Context &__ctx, f32x2 p, f32x2 dim) {
  p.data[0] = abs(p.data[0]);
  float o = 0.5f * max(dim.data[0] - dim.data[1], 0.0f);
  if (p.data[0] < o) {
    return abs(p.data[1]) - dim.data[1];
  }
  return length(p - f32x2(o, 0.0f)) - dim.data[1];
}
f32x3 digit(Context &__ctx, f32x3 col, f32x2 p, f32x3 acol, f32x3 icol,
            float aa, float n, float t) {
  const int[16] digits =
      int[16](0x7D, 0x50, 0x4F, 0x57, 0x72, 0x37, 0x3F, 0x51, 0x7F, 0x77, 0x7B,
              0x3E, 0x2D, 0x5E, 0x2F, 0x2B);
  const f32x2 dim = f32x2(0.75f, 0.075f);
  const float eps = 0.1f;
  f32x2 ap = abs(p);
  if (ap.data[0] > (0.5f + dim.data[1] + eps))
    return col;
  if (ap.data[1] > (1.0f + dim.data[1] + eps))
    return col;
  float m = mod(floor(n), 16.0f);
  int digit = digits[int(m)];
  f32x2 cp = (p - 0.5f);
  f32x2 cn = round(cp);
  f32x2 p0 = p;
  p0.data[1] -= 0.5f;
  p0.data[1] = p0.data[1] - 0.5f;
  float n0 = round(p0.data[1]);
  p0.data[1] -= n0;
  float d0 = dsegmentx(__ctx, p0, dim);
  f32x2 p1 = p;
  f32x2 n1 = sign(p1);
  p1 = abs(p1);
  p1 -= 0.5f;
  p1 = p1.swizzle<1, 0>();
  float d1 = dsegmentx(__ctx, p1, dim);
  f32x2 p2 = p;
  p2.data[1] = abs(p.data[1]);
  p2.data[1] -= 0.5f;
  p2 = abs(p2);
  float d2 = dot(normalize(f32x2(1.0f, -1.0f)), p2);
  float d = d0;
  d = min(d, d1);
  float sx = 0.5f * (n1.data[0] + 1.0f) + (n1.data[1] + 1.0f);
  float sy = -n0;
  float s = d2 > 0.0f ? (3.0f + sx) : sy;
  f32x3 scol = ((digit & (1 << int(s))) == 0) ? icol : acol;
  col = mix(col, scol, smoothstep(aa, -aa, d) * t);
  return col;
}
f32x3 digit(Context &__ctx, f32x3 col, f32x2 p, f32x3 acol, f32x3 icol, float n,
            float t) {
  f32x2 aa2 = fwidth(p);
  float aa = max(aa2.data[0], aa2.data[1]);
  return digit(__ctx, col, p, acol, icol, aa, n, t);
}
float hash(Context &__ctx, float co) {
  return fract(sin(co * 12.9898f) * 13758.5453f);
}
float hash2(Context &__ctx, f32x2 co) {
  return fract(sin(dot(co.swizzle<0, 1>(), f32x2(12.9898f, 58.233f))) *
               13758.5453f);
}
f32x2 mod2(Context &__ctx, f32x2 &p, f32x2 size) {
  f32x2 c = floor((p + size * 0.5f) / size);
  p = mod(p + size * 0.5f, size) - size * 0.5f;
  return c;
}
float box(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.0f)) + min(max(d.data[0], d.data[1]), 0.0f);
}
float circle(Context &__ctx, f32x2 p, float r) { return length(p) - r; }
float stripes(Context &__ctx, float d) {
  const float cc = 0.42f;
  d = abs(d) - logo_width * cc;
  d = abs(d) - logo_width * cc * 0.5f;
  return d;
}
f32x4 merge(Context &__ctx, f32x4 s0, f32x4 s1) {
  bool dt = s0.data[2] < s1.data[2];
  f32x4 b = dt ? s0 : s1;
  f32x4 t = dt ? s1 : s0;
  b.data[0] *= 1.0f - exp(-max(80.0f * (t.data[3]), 0.0f));
  f32x4 r = f32x4(mix(b.swizzle<0, 1>(), t.swizzle<0, 1>(), t.data[1]),
                  b.data[3] < t.data[3] ? b.data[2] : t.data[2],
                  min(b.data[3], t.data[3]));
  return r;
}
f32x4 figure_8(Context &__ctx, f32x2 p, float aa) {
  f32x2 p1 = p - f32x2(logo_off, -logo_off);
  float d1 = abs(circle(__ctx, p1, logo_radius));
  float a1 = atan_approx(__ctx, -p1.data[0], -p1.data[1]);
  float s1 = stripes(__ctx, d1);
  float o1 = d1 - logo_width;
  f32x2 p2 = p - f32x2(logo_off, logo_off);
  float d2 = abs(circle(__ctx, p2, logo_radius));
  float a2 = atan_approx(__ctx, p2.data[0], p2.data[1]);
  float s2 = stripes(__ctx, d2);
  float o2 = d2 - logo_width;
  f32x4 c0 = f32x4(smoothstep(aa, -aa, s1), smoothstep(aa, -aa, o1), a1, o1);
  f32x4 c1 = f32x4(smoothstep(aa, -aa, s2), smoothstep(aa, -aa, o2), a2, o2);
  return merge(__ctx, c0, c1);
}
f32x4 clogo(Context &__ctx, f32x2 p, float aa, float &d) {
  const f32x2x2 rot0 =
      f32x2x2(cos(3.141592654f / 4.0f), sin(3.141592654f / 4.0f),
              -sin(3.141592654f / 4.0f), cos(3.141592654f / 4.0f));
  const f32x2x2 rot1 = f32x2x2(
      cos(5.0f * 3.141592654f / 4.0f), sin(5.0f * 3.141592654f / 4.0f),
      -sin(5.0f * 3.141592654f / 4.0f), cos(5.0f * 3.141592654f / 4.0f));
  float sgn = sign(p.data[1]);
  p *= sgn;
  f32x4 s0 = figure_8(__ctx, p, aa);
  f32x4 s1 = figure_8(__ctx, p * rot0, aa);
  f32x4 s2 = figure_8(__ctx, p - f32x2(-0.5f, 0.0f), aa);
  f32x4 s3 = figure_8(__ctx, p * rot1, aa);
  const float off = -3.141592654f;
  s1.data[2] -= off;
  s3.data[2] -= off;
  f32x4 s = s0;
  s = merge(__ctx, s, s1);
  s = merge(__ctx, s, s2);
  s = merge(__ctx, s, s3);
  d = s.data[3];
  return f32x4(mix(0.025f * bcol, bcol, s.data[0]), s.data[1]);
}
f32x3 logoEffect(Context &__ctx, f32x3 col, f32x2 p, f32x2 pp, float aa) {
  float d;
  f32x4 ccol = clogo(__ctx, p, aa, d);
  const float period = (2.0f * 3.141592654f) * 10.0f;
  float ss = sin(period * d - iTime * (2.0f * 3.141592654f) / 10.0f);
  const float off = 0.2f;
  float doff = period * aa * cos(off);
  col = mix(col, ccol.swizzle<0, 1, 2>(), ccol.data[3]);
  return col;
}
f32x3 spiralEffect(Context &__ctx, f32x3 col, f32x2 p, f32x2 pp, float aa) {
  f32x2 sp = p;
  spiralMod(__ctx, sp, .5f);
  f32x2 dp = sp;
  float dz = 0.0125f;
  dp /= dz;
  aa /= dz;
  float dny = mod1(__ctx, dp.data[1], 3.06f);
  float dhy = hash(__ctx, dny + 1234.5f);
  dp.data[0] = -dp.data[0];
  float ltm = (iTime + 1234.5f) * mix(2.0f, 10.0f, (dhy)) * 0.125f;
  dp.data[0] -= ltm;
  float opx = dp.data[0];
  float dnx = mod1(__ctx, dp.data[0], 1.5f);
  const float stepfx = 0.125f * 0.25f;
  float fx = -2.0f * stepfx * ltm + stepfx * dnx;
  float fnx = floor(fx);
  float ffx = fract(fx);
  float dht = hash(__ctx, fnx);
  float dhx = hash(__ctx, dnx);
  float dh = fract(dht + dhx + dhy);
  float l = length(p);
  float t = smoothstep(0.4f, 0.5f, l);
  const f32x3 hcol = clamp(1.5f * sqrt(bcol) + f32x3(0.2f), 0.0f, 1.0f);
  const f32x3 acol = bcol;
  const f32x3 icol = acol * 0.1f;
  float fo = (smoothstep(0.0f, 1.0f, ffx));
  float ff = smoothstep(1.0f - 2.0f * sqrt(stepfx), 1.0f, ffx * ffx);
  col =
      digit(__ctx, col, dp, mix(acol, hcol, ff), icol, aa, 100.0f * dh, fo * t);
  return col;
}
f32x3 glowEffect(Context &__ctx, f32x3 col, f32x2 p, f32x2 pp, float aa) {
  float d = length(p);
  col += 0.25f * bcol * exp(-9.0f * max(d - 2.0f / 3.0f, 0.0f));
  return col;
}
f32x3 effect(Context &__ctx, f32x2 p, f32x2 pp) {
  float aa = 2.0f / iResolution.data[1];
  f32x3 col = f32x3(0.0f);
  col = spiralEffect(__ctx, col, p, pp, aa);
  col = glowEffect(__ctx, col, p, pp, aa);
  col = logoEffect(__ctx, col,
                   p * f32x2x2(cos(-0.05f * iTime), sin(-0.05f * iTime),
                               -sin(-0.05f * iTime), cos(-0.05f * iTime)),
                   pp, aa);
  col *= smoothstep(1.25f, 0.5f, length(pp));
  col = sqrt(col);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 q = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 p = -1.f + 2.f * q;
  f32x2 pp = p;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 col = effect(__ctx, p, pp);
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
