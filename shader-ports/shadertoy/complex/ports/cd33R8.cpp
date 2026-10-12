// Port of https://www.shadertoy.com/view/cd33R8; author: spacecat.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cd33R8.json.
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
f32x2 rotate2D(Context &__ctx, f32x2 uv, float angle) {
  f32x2x2 transform = f32x2x2(cos(angle), -sin(angle), sin(angle), cos(angle));
  return uv * transform;
}
float sdCircle(Context &__ctx, f32x2 p, float r, f32x2 origin) {
  return length(p - origin) - r;
}
float sdTriangle(Context &__ctx, f32x2 p, float size, float rotation,
                 f32x2 origin) {
  p = rotate2D(__ctx, p, rotation);
  p -= origin;
  p /= size;
  const float k = sqrt(3.0f);
  p.data[0] = abs(p.data[0]) - 1.0f;
  p.data[1] = p.data[1] + 1.0f / k;
  if (p.data[0] + k * p.data[1] > 0.0f)
    p = f32x2(p.data[0] - k * p.data[1], -k * p.data[0] - p.data[1]) / 2.0f;
  p.data[0] -= clamp(p.data[0], -2.0f, 0.0f);
  return -length(p) * sign(p.data[1]) * size;
}
float dot2(Context &__ctx, f32x2 v) { return dot(v, v); }
float sdBezier(Context &__ctx, f32x2 pos, f32x2 A, f32x2 B, f32x2 C) {
  f32x2 a = B - A;
  f32x2 b = A - 2.0f * B + C;
  f32x2 c = a * 2.0f;
  f32x2 d = A - pos;
  float kk = 1.0f / dot(b, b);
  float kx = kk * dot(a, b);
  float ky = kk * (2.0f * dot(a, a) + dot(d, b)) / 3.0f;
  float kz = kk * dot(d, a);
  float res = 0.0f;
  float p = ky - kx * kx;
  float p3 = p * p * p;
  float q = kx * (2.0f * kx * kx - 3.0f * ky) + kz;
  float h = q * q + 4.0f * p3;
  if (h >= 0.0f) {
    h = sqrt(h);
    f32x2 x = (f32x2(h, -h) - q) / 2.0f;
    f32x2 uv = sign(x) * pow(abs(x), f32x2(1.0f / 3.0f));
    float t = clamp(uv.data[0] + uv.data[1] - kx, 0.0f, 1.0f);
    res = dot2(__ctx, d + (c + b * t) * t);
  } else {
    float z = sqrt(-p);
    float v = acos(q / (p * z * 2.0f)) / 3.0f;
    float m = cos(v);
    float n = sin(v) * 1.732050808f;
    f32x3 t = clamp(f32x3(m + m, -n - m, n - m) * z - kx, 0.0f, 1.0f);
    res = min(dot2(__ctx, d + (c + b * t.data[0]) * t.data[0]),
              dot2(__ctx, d + (c + b * t.data[1]) * t.data[1]));
  }
  return sqrt(res);
}
float sdCat(Context &__ctx, f32x2 uv) {
  float head = sdCircle(__ctx, uv, 0.36f, f32x2(0.0f, -0.1f));
  float earLeft = sdTriangle(__ctx, uv, 0.2f, -0.6f, f32x2(-0.1f, 0.29f));
  float earRight = sdTriangle(__ctx, uv, 0.2f, 0.6f, f32x2(0.1f, 0.29f));
  float nose = sdTriangle(__ctx, uv, 0.03f, 3.14159f, f32x2(0.0f, 0.2f));
  float eyeLeft = max(sdCircle(__ctx, uv, 0.045f, f32x2(0.1f, -0.09f)),
                      -sdCircle(__ctx, uv, 0.009f, f32x2(0.095f, -0.082f)));
  float eyeRight = max(sdCircle(__ctx, uv, 0.045f, f32x2(-0.1f, -0.09f)),
                       -sdCircle(__ctx, uv, 0.009f, f32x2(-0.105f, -0.082f)));
  float mouthLeft = sdBezier(__ctx, uv, f32x2(0.0f, -0.23f),
                             f32x2(-0.05f, -0.33f), f32x2(-0.1f, -0.28f));
  float mouthRight = sdBezier(__ctx, uv, f32x2(0.0f, -0.23f),
                              f32x2(0.05f, -0.33f), f32x2(0.1f, -0.28f));
  float cat = min(head, min(earLeft, earRight));
  cat = max(cat, -nose);
  cat = max(max(cat, -eyeLeft), -eyeRight);
  cat = max(max(cat, -mouthLeft), -mouthRight);
  for (float y = -0.22f; y <= -0.17f; y += 0.02f) {
    float whiskerLeft =
        sdBezier(__ctx, uv, f32x2(-0.1f, y), f32x2(-0.25f, y + 0.03f),
                 f32x2(-0.45f, y - 0.9f * (-0.15f - y)));
    float whiskerRight =
        sdBezier(__ctx, uv, f32x2(0.1f, y), f32x2(0.25f, y + 0.03f),
                 f32x2(0.45f, y - 0.9f * (-0.15f - y)));
    cat = cat > 0.0f ? min(cat, whiskerLeft) : max(cat, -whiskerLeft);
    cat = cat > 0.0f ? min(cat, whiskerRight) : max(cat, -whiskerRight);
  }
  return cat;
}
float sdSimpleCat(Context &__ctx, f32x2 uv) {
  float head = sdCircle(__ctx, uv, 0.36f, f32x2(0.0f, -0.1f));
  float earLeft = sdTriangle(__ctx, uv, 0.2f, -0.6f, f32x2(-0.1f, 0.29f));
  float earRight = sdTriangle(__ctx, uv, 0.2f, 0.6f, f32x2(0.1f, 0.29f));
  float cat = min(head, min(earLeft, earRight));
  return cat;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  uv -= 0.5f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  uv.data[1] -= 0.03f;
  float cat = sdCat(__ctx, rotate2D(__ctx, uv, sin(iTime) / 8.0f));
  float outline = 1.0f - smoothstep(0.002f, 0.006f, abs(cat));
  float inside = step(0.0f, -cat);
  f32x3 col = f32x3(0.0f, 0.0f, 0.1f);
  f32x2 bgPatternUV =
      fract(rotate2D(__ctx, uv + iTime * 0.1f, -0.4f) * 10.0f) - 0.5f;
  float bgPattern = smoothstep(-0.01f, 0.01f, sdSimpleCat(__ctx, bgPatternUV));
  f32x3 bgColor =
      mix(f32x3(uv * 0.2f, 0.2f), f32x3(uv * 0.6f + 0.4f, 1.0f), bgPattern);
  col = mix(col, bgColor, clamp(cat * 5.0f, 0.0f, 1.0f));
  f32x3 insideColor = f32x3(uv * 0.8f + 0.8f, 1.0f);
  col = mix(col, insideColor, inside);
  f32x3 lineColor = f32x3(uv * 1.8f + 0.6f, 1.0f);
  col = mix(col, lineColor, outline);
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
