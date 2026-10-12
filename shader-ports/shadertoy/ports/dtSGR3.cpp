// Port of https://www.shadertoy.com/view/dtSGR3; author: mla.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtSGR3.json.
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
  Texture2D channel3;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
bool alert = false;
void assert(Context &__ctx, bool b) {
  if (!b)
    alert = true;
}
const float PI = 3.14159f;
const int CHAR_A = 65;
const int CHAR_D = 68;
const int CHAR_R = 82;
const int CHAR_X = 88;
const int CHAR_Y = 89;
const int CHAR_Z = 90;
unsigned ihash(Context &__ctx, unsigned x) {
  x += 0x12345678u;
  x ^= x >> 16;
  x *= 0x7feb352dU;
  x ^= x >> 15;
  x *= 0x846ca68bU;
  x ^= x >> 16;
  return x;
}
float rand(Context &__ctx, int n) {
  return float(ihash(__ctx, unsigned(n))) / exp2(32.0f);
}
float xor (Context & __ctx, float d0, float d1) {
  return max(min(d0, d1), -max(d0, d1));
} float repCircle(Context &__ctx, f32x2 p, float t, f32x2 centre) {
  p -= centre;
  float r = length(p);
  float i = round(r / t);
  if (i == 0.0f)
    i = 1.0f;
  float s = t * i;
  if ((texelFetch(__ctx.channel3, i32x2((CHAR_R), 2)).data[0] != 0.0f))
    s += 0.5f * t * (rand(__ctx, int(i)) - 0.5f);
  float sgn = mod(i, 2.0f) == 0.0f ? -1.0f : 1.0f;
  assert(__ctx, sgn < 0.0f);
  return sgn * (r - s);
}
float repSquare(Context &__ctx, f32x2 p, float t, f32x2 centre) {
  p -= centre;
  p = abs(p);
  float r = min(p.data[0], p.data[1]);
  float i = round(r / t);
  if (i == 0.0f)
    i = 1.0f;
  float s = t * i;
  if ((texelFetch(__ctx.channel3, i32x2((CHAR_R), 2)).data[0] != 0.0f))
    s += 0.5f * t * (rand(__ctx, int(i) * 255) - 0.5f);
  float sgn = mod(i, 2.0f) == 0.0f ? -1.0f : 1.0f;
  float dx = s - p.data[0];
  float dy = s - p.data[1];
  float d = dx > 0.0f && dy > 0.0f ? distance(p, f32x2(s)) : max(dx, dy);
  return sgn * d;
}
f32x3 drawScene(Context &__ctx, f32x2 p) {
  float px = fwidth(p.data[0]);
  float t = 0.25f * iTime;
  float d1 =
      repCircle(__ctx, p, 0.5f, f32x2(sin(t) * 1.0f, cos(0.414f * t) * 1.5f));
  float d2 =
      repSquare(__ctx, p, 0.5f, f32x2(-sin(0.618f * t) * 1.5f, cos(t)) * 1.0f);
  float d = xor(__ctx, d1, d2);
  f32x3 col = f32x3(smoothstep(-0.5f * px, 0.5f * px, d));
  if ((texelFetch(__ctx.channel3, i32x2((CHAR_D), 2)).data[0] != 0.0f))
    col *= 0.75f - 0.25f * cos(0.5f * PI * d / px);
  if ((texelFetch(__ctx.channel3, i32x2((CHAR_X), 2)).data[0] != 0.0f))
    col = mix(f32x3(1, 0, 0), col, smoothstep(0.0f, px, abs(d) - 0.005f));
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  p *= 4.0f;
  f32x3 col = drawScene(__ctx, p);
  col = pow(col, f32x3(0.4545f));
  if ((texelFetch(__ctx.channel3, i32x2((CHAR_A), 2)).data[0] != 0.0f) && alert)
    col.data[2] = 1.0f - col.data[2];
  fragColor = f32x4(col, 1);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 3) texture_2d<f32x4> channel3,
               __binding(1, 3) sampler sampling3) {
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
  Context __ctx{{channel3, sampling3}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
