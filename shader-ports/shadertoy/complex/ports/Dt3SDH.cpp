// Port of https://www.shadertoy.com/view/Dt3SDH; author: iq.
// Dataset license metadata: mit; original retained in upstream/Dt3SDH.json.
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
float sdCircle(Context &__ctx, f32x2 p, f32x2 c, float r) {
  return length(p - c) - r;
}
float sdLine(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h);
}
f32x2 opUnion(Context &__ctx, f32x2 m, float d, float a) {
  return (d < m.data[0]) ? f32x2(d, a) : m;
}
void intersectCircle(Context &__ctx, f32x2 ro, f32x2 rd, float rad, f32x4 &p1,
                     f32x4 &p2) {
  float b = dot(ro, rd);
  float c = dot(ro, ro) - rad * rad;
  float h = b * b - c;
  if (h > 0.0f) {
    h = sqrt(h);
    p1 = f32x4(ro + (-b + h) * rd, ro);
    p2 = f32x4(ro + (-b - h) * rd, ro);
  } else {
    h = sqrt(-h);
    p1 = f32x4(ro - b * rd, ro + h * rd);
    p2 = f32x4(ro - b * rd, ro - h * rd);
  }
}
void get_ray(Context &__ctx, f32x2 &ro, f32x2 &rd, float t) {
  float an = 3.1415927f * (0.5f + 0.2f * sin(t * 0.735f + 4.0f));
  ro = f32x2(-1.0f * cos(t * 0.525f), -1.0f + 0.1f * sin(t * 1.5f));
  rd = f32x2(cos(an), sin(an));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  float px = 2.0f / iResolution.data[1];
  p.data[1] -= 0.4f;
  const float rad = 0.4f;
  const float cth = 0.03f;
  f32x3 col = f32x3(0.2f - 0.1f * length(p * 0.5f));
  {
    float d = sdCircle(__ctx, p, f32x2(0.0f, 0.0f), rad);
    col =
        mix(col, f32x3(0.75f), 1.0f - smoothstep(0.0f, 0.01f, abs(d) - 0.005f));
  }
  {
    f32x2 dr = f32x2(1e20f, 1.0f);
    f32x2 di = f32x2(1e20f, 1.0f);
    f32x4 op1;
    f32x4 op2;
    const int num = 256;
    for (int i = 0; i < num; i++) {
      float a = float(i) / float(num);
      f32x2 ro, rd;
      get_ray(__ctx, ro, rd, iTime - 3.0f * a);
      f32x4 p1, p2;
      intersectCircle(__ctx, ro, rd, rad, p1, p2);
      if (i > 0) {
        dr = opUnion(
            __ctx, dr,
            min(sdLine(__ctx, p, p1.swizzle<0, 1>(), op1.swizzle<0, 1>()),
                sdLine(__ctx, p, p2.swizzle<0, 1>(), op2.swizzle<0, 1>())),
            a);
        di = opUnion(
            __ctx, di,
            min(sdLine(__ctx, p, p1.swizzle<2, 3>(), op1.swizzle<2, 3>()),
                sdLine(__ctx, p, p2.swizzle<2, 3>(), op2.swizzle<2, 3>())),
            a);
      }
      op1 = p1;
      op2 = p2;
    }
    col = mix(col, f32x3(0.0f, 0.7f, 1.2f),
              smoothstep(1.0f, 0.7f, dr.data[1]) *
                  (1.0f - smoothstep(0.0f, 0.01f, dr.data[0])));
    col = mix(col, f32x3(1.2f, 0.7f, 0.0f),
              smoothstep(1.0f, 0.7f, di.data[1]) *
                  (1.0f - smoothstep(0.0f, 0.01f, di.data[0])));
  }
  f32x2 ro, rd;
  get_ray(__ctx, ro, rd, iTime);
  {
    {
      float d = sdLine(__ctx, p, ro - rd * 10.0f, ro + rd * 10.0f);
      col = mix(col, f32x3(1.0f), 0.25f * (1.0f - smoothstep(0.0f, 0.008f, d)));
    }
    {
      f32x2 rdp = f32x2(-rd.data[1], rd.data[0]);
      f32x2 tip = ro + rd * 0.2f;
      float d = sdLine(__ctx, p, ro, tip);
      d = min(d,
              sdLine(__ctx, p, tip, tip + 0.05f * normalize(rdp - 2.0f * rd)));
      d = min(d,
              sdLine(__ctx, p, tip, tip + 0.05f * normalize(-rdp - 2.0f * rd)));
      col = mix(col, f32x3(0.75f), 1.0f - smoothstep(0.005f, 0.01f, d));
    }
  }
  {
    f32x4 p1, p2;
    intersectCircle(__ctx, ro, rd, rad, p1, p2);
    float dr = min(sdCircle(__ctx, p, p1.swizzle<0, 1>(), cth),
                   sdCircle(__ctx, p, p2.swizzle<0, 1>(), cth));
    float di = min(sdCircle(__ctx, p, p1.swizzle<2, 3>(), cth),
                   sdCircle(__ctx, p, p2.swizzle<2, 3>(), cth));
    col =
        mix(col, f32x3(0.0f, 0.7f, 1.2f), 1.0f - smoothstep(0.005f, 0.01f, dr));
    col =
        mix(col, f32x3(1.2f, 0.7f, 0.0f), 1.0f - smoothstep(0.005f, 0.01f, di));
  }
  col += sin(fragCoord.data[0] * 114.0f) * sin(fragCoord.data[1] * 211.1f) /
         512.0f;
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
