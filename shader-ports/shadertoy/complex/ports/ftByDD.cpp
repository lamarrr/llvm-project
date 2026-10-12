// Port of https://www.shadertoy.com/view/ftByDD; author: skal.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ftByDD.json.
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
f32x2 hash2(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(f32x3(p.swizzle<0, 1, 0>()) * f32x3(.1021f, .1013f, .0977f));
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.swizzle<0, 0>() + p3.swizzle<1, 2>()) * p3.swizzle<2, 1>());
}
float SegmentDistance(Context &__ctx, f32x2 p1, f32x2 p2) {
  f32x2 dir = p2 - p1;
  float norm = 1.f / dot(dir, dir);
  float frac = clamp(norm * dot(dir, -p1), 0.0f, 1.0f);
  return length(p1 + frac * dir);
}
float CellDistance(Context &__ctx, f32x2 p1, f32x2 p2) {
  f32x2 mid = 0.5f * (p1 + p2);
  f32x2 dir = normalize(p2 - p1);
  return dot(mid, dir);
}
f32x2 GetCenter(Context &__ctx, f32x2 id, float theta) {
  f32x2 phi = 6.283185f * hash2(__ctx, id) + theta;
  return 0.5f + 0.5f * sin(phi);
}
float VoronoiDistance(Context &__ctx, f32x2 p, float t, int n, f32x2 &p1) {
  f32x2 cell = floor(p);
  f32x2 sub_pos = fract(p);
  float d1 = 1.e6f, d2 = 1.e7f;
  f32x2 p2;
  for (int j = -n; j <= n; ++j) {
    for (int i = -n; i <= n; ++i) {
      f32x2 offset = f32x2(float(i), float(j));
      f32x2 center = offset + GetCenter(__ctx, cell + offset, t) - sub_pos;
      float d = dot(center, center);
      if (d < d1) {
        d2 = d1;
        d1 = d;
        p2 = p1;
        p1 = center;
      } else if (d < d2) {
        d2 = d;
        p2 = center;
      }
    }
  }
  return SegmentDistance(__ctx, p1, p2);
}
float smin(Context &__ctx, float a, float b, float k) {
  float h = max(k - abs(a - b), 0.0f) / k;
  return min(a, b) - h * h * k * 0.25f;
}
float VoronoiMin(Context &__ctx, f32x2 p, float t, int n, f32x2 p1, float k) {
  f32x2 cell = floor(p);
  f32x2 sub_pos = fract(p);
  float d_min = 1.e6f;
  for (int j = -n; j <= n; ++j) {
    for (int i = -n; i <= n; ++i) {
      f32x2 offset = f32x2(float(i), float(j));
      f32x2 p = offset + GetCenter(__ctx, cell + offset, t) - sub_pos;
      if (dot(p - p1, p - p1) > 0.001f) {
        d_min = smin(__ctx, CellDistance(__ctx, p1, p), d_min, k);
      }
    }
  }
  return d_min;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = 6.f * fragCoord / iResolution.data[1];
  float theta = iTime * 1.2f;
  float blend = mix(0.01f, 0.38f, 0.5f + 0.5f * sin(5.f * theta));
  f32x2 p1;
  float d12 = VoronoiDistance(__ctx, uv, theta, 1, p1);
  float d = VoronoiMin(__ctx, uv, theta, 2, p1, blend);
  f32x3 col = f32x3(0.f);
  if (true) {
    col = mix(f32x3(0.2f, 0.2f, 0.7f), col, smoothstep(0.00f, 0.06f, d12));
  }
  if (true) {
    float iso_d = d * (0.5f + 0.5f * sin(60.f * d + iTime * 5.f));
    float amp = 0.5f + 0.5f * sin(2.f * theta * 0.5f);
    col = mix(col, amp * f32x3(0.76f, 0.45f, 0.96f),
              smoothstep(0.0f, 0.3f, iso_d));
  }
  if (true) {
    col = mix(f32x3(0.35f, 0.65f, 0.20f), col, smoothstep(0.00f, 0.02f, d));
  }
  if (true) {
    float d = dot(p1, p1);
    col = mix(f32x3(0.97f, 0.8f, 0.3f), col, smoothstep(0.002f, 0.005f, d));
  }
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
