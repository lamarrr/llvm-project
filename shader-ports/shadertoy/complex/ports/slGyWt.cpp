// Port of https://www.shadertoy.com/view/slGyWt; author: iq.
// Dataset license metadata: mit; original retained in upstream/slGyWt.json.
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
f32x3 sdgCircle(Context &__ctx, f32x2 p, f32x2 c, float r) {
  p -= c;
  float l = length(p);
  return f32x3(l - r, p / l);
}
f32x3 sdgBox(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 w = abs(p) - b;
  f32x2 s = f32x2(p.data[0] < 0.0f ? -1 : 1, p.data[1] < 0.0f ? -1 : 1);
  float g = max(w.data[0], w.data[1]);
  f32x2 q = max(w, 0.0f);
  float l = length(q);
  return f32x3(
      (g > 0.0f) ? l : g,
      s * ((g > 0.0f) ? q / l
                      : ((w.data[0] > w.data[1]) ? f32x2(1, 0) : f32x2(0, 1))));
}
float cro(Context &__ctx, f32x2 a, f32x2 b) {
  return a.data[0] * b.data[1] - a.data[1] * b.data[0];
}
float map(Context &__ctx, f32x2 p) {
  f32x2 off = 0.1f * sin(iTime + f32x2(0.0f, 2.0f));
  float d = max(sdgBox(__ctx, p, f32x2(0.3f, 0.6f)).data[0],
                -sdgCircle(__ctx, p, f32x2(0.0f, 0.2f) + off, 0.4f).data[0]);
  if (d > 0.0f) {
    f32x2 op = p;
    for (int i = 0; i < 512; i++) {
      float d1 = sdgBox(__ctx, p, f32x2(0.3f, 0.6f)).data[0];
      f32x2 g1 = sdgBox(__ctx, p, f32x2(0.3f, 0.6f)).swizzle<1, 2>();
      float d2 = sdgCircle(__ctx, p, f32x2(0.0f, 0.2f) + off, 0.4f).data[0];
      f32x2 g2 =
          sdgCircle(__ctx, p, f32x2(0.0f, 0.2f) + off, 0.4f).swizzle<1, 2>();
      if (max(abs(d1), abs(d2)) < 0.001f)
        break;
      p -= 0.5f * (d1 * g1 + d2 * g2);
    }
    float d3 = length(p - op);
    f32x2 g1 = sdgBox(__ctx, p, f32x2(0.3f, 0.6f)).swizzle<1, 2>();
    f32x2 g2 =
        sdgCircle(__ctx, p, f32x2(0.0f, 0.2f) + off, 0.4f).swizzle<1, 2>();
    float no = cro(__ctx, g1, g2);
    if (min(cro(__ctx, op - p, g1) * no, cro(__ctx, op - p, g2) * no) > 0.0f)
      d = d3;
  };
  return d;
}
f32x2 gra(Context &__ctx, f32x2 p) {
  const float e = 0.0002f;
  return f32x2(map(__ctx, p + f32x2(e, 0.0f)) - map(__ctx, p - f32x2(e, 0.0f)),
               map(__ctx, p + f32x2(0.0f, e)) -
                   map(__ctx, p - f32x2(0.0f, e))) /
         (2.0f * e);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = (2.0f * iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
            iResolution.data[1];
  float d = map(__ctx, p);
  f32x3 col = (d > 0.0f) ? f32x3(0.9f, 0.6f, 0.3f) : f32x3(0.5f, 0.85f, 1.0f);
  col *= 1.0f - exp2(-32.0f * abs(d));
  col *= 0.8f + 0.2f * cos(128.0f * abs(d));
  col = mix(col, f32x3(1.0f), 1.0f - smoothstep(0.002f, 0.005f, abs(d)));
  {
  }
  if (iMouse.data[2] > 0.001f) {
    d = map(__ctx, m);
    col = mix(
        col, f32x3(1.0f, 1.0f, 0.0f),
        1.0f - smoothstep(0.0f, 0.005f, abs(length(p - m) - abs(d)) - 0.0025f));
    col = mix(col, f32x3(1.0f, 1.0f, 0.0f),
              1.0f - smoothstep(0.0f, 0.005f, length(p - m) - 0.015f));
  }
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
