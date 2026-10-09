// Port of https://www.shadertoy.com/view/mtK3RV; author: NikolaErceg.
// Dataset license metadata: cc0-1.0; original retained in upstream/mtK3RV.json.
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
void rot(Context &__ctx, f32x2 &p, float a) {
  float c = cos(a);
  float s = sin(a);
  p = f32x2(c * p.data[0] + s * p.data[1], -s * p.data[0] + c * p.data[1]);
}
float plane(Context &__ctx, f32x2 p, f32x2 n, float m) { return dot(p, n) + m; }
float circle(Context &__ctx, f32x2 p, float r) { return length(p) - r; }
float holey(Context &__ctx, float d, float k) { return abs(d) - k; }
float tanh2(Context &__ctx, float x) {
  if (abs(x) > 80.0f) {
    return sign(x);
  } else {
    return tanh(x);
  }
}
float nfield(Context &__ctx, f32x2 p, f32x2 c) {
  f32x2 u = p;
  float a = 0.0f;
  float s = 3.0f;
  for (int i = 0; i < 25; ++i) {
    float m = dot(u, u);
    u = mix((.5f / 0.0125f) * u * u + 0.0125f * .5f, abs(u),
            step(0.f, abs(u) - 0.0125f)) /
            m +
        c;
    u *= pow(s, 0.95f);
    a += pow(s, 18.0f) * m;
    s *= 0.75f;
  }
  return -tanh2(__ctx, 0.125f * a);
}
f32x3 normal(Context &__ctx, f32x2 p, f32x2 c) {
  f32x2 v;
  f32x2 w;
  f32x2 e = f32x2(2.0f / iResolution.data[1], 0);
  f32x3 n;
  n.data[0] = nfield(__ctx, p + e.swizzle<0, 1>(), c) -
              nfield(__ctx, p - e.swizzle<0, 1>(), c);
  n.data[1] = 1.0f * e.data[0];
  n.data[2] = nfield(__ctx, p + e.swizzle<1, 0>(), c) -
              nfield(__ctx, p - e.swizzle<1, 0>(), c);
  return normalize(n);
}
f32x3 field(Context &__ctx, f32x2 p, f32x2 c) {
  f32x2 u = p;
  float a = 0.0f;
  float s = 3.0f;
  f32x2 tc = f32x2(0.5f, 0.3f);
  rot(__ctx, tc, (0.25f * 3.141592654f) * iTime / 0.1f);
  f32x2 tpn = normalize(f32x2(1.0f));
  float tpm = 0.0f + 1.4f * tanh(length(p));
  float tcd = 1E10f;
  float tcp = 1E10f;
  for (int i = 0; i < 18; ++i) {
    float m = dot(u, u);
    u = mix((.5f / 0.0125f) * u * u + 0.0125f * .5f, abs(u),
            step(0.f, abs(u) - 0.0125f)) /
            m +
        c;
    tcd = min(tcd, holey(__ctx, circle(__ctx, u - tc, 0.05f), -0.1f));
    tcp = min(tcp, holey(__ctx, plane(__ctx, u, tpn, tpm), -0.1f));
    u *= pow(s, 0.5f);
    a += pow(s, 1.0f) * m;
    s *= 0.75f;
  }
  return f32x3(tanh(0.125f * a), tanh(tcd), tanh(tcp));
}
f32x3 postProcess(Context &__ctx, f32x3 col, f32x2 q) {
  col = pow(clamp(col, 0.0f, 1.0f), f32x3(.75f));
  col = col * 0.6f + 0.4f * col * col * (3.0f - 2.0f * col);
  col = mix(col, f32x3(dot(col, f32x3(0.33f))), -0.4f);
  col *= 0.5f + 0.5f * pow(19.0f * q.data[0] * q.data[1] * (1.0f - q.data[0]) *
                               (1.0f - q.data[1]),
                           0.7f);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 q = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 p = -1.f + 2.f * q;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  float currentPeriod = mod(floor(iTime / 0.1f), 2.0f);
  float timeInPeriod = mod(iTime, 0.1f);
  p *= 0.25f + (0.05f * timeInPeriod) + pow(1.35f, currentPeriod);
  f32x2 c = f32x2(-0.5f, -0.55f);
  f32x3 gp = f32x3(p.data[0], 1.0f * tanh(1.0f - (length(p))), p.data[1]);
  f32x3 lp1 = f32x3(-1.0f, 1.5f, 1.0f);
  f32x3 ld1 = normalize(lp1 - gp);
  f32x3 lp2 = f32x3(1.0f, 1.5f, 1.0f);
  f32x3 ld2 = normalize(lp2 - gp);
  f32x3 f = field(__ctx, p, c);
  f32x3 n = normal(__ctx, p, c);
  float diff1 = max(dot(ld1, n), 0.0f);
  float diff2 = max(dot(ld2, n), 0.0f);
  f32x3 col = f32x3(0.0f);
  const f32x3 dcol1 = f32x3(0.3f, 0.5f, 0.7f).swizzle<0, 1, 2>();
  const f32x3 dcol2 = 0.5f * f32x3(0.7f, 0.5f, 0.3f).swizzle<0, 1, 2>();
  const f32x3 scol1 = 0.5f * f32x3(1.0f);
  const f32x3 scol2 = 0.5f * 0.5f * f32x3(1.0f);
  col += diff1 * dcol1;
  col += diff2 * dcol2;
  col += scol1 * pow(diff1, 10.0f);
  col += scol2 * pow(diff2, 3.0f);
  col -= f32x3(tanh(f.data[1] - 0.1f));
  col += 0.5f * (diff1 + diff2) *
         (1.25f * pow(f32x3(f.data[2]), 5.0f * f32x3(1.0f, 4.0f, 5.0f)));
  col = postProcess(__ctx, col, q);
  const float fade = 0.0f;
  float fadeIn = smoothstep(0.0f, fade, timeInPeriod);
  float fadeOut = 1.0f - smoothstep(0.1f - fade, 0.1f, timeInPeriod);
  col *= fadeIn * fadeOut;
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
