// Port of https://www.shadertoy.com/view/dljGD3; author: drliquidglitch.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dljGD3.json.
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
f32x3 hsv(Context &__ctx, float h, float s, float v) {
  f32x3 rgb =
      clamp(abs(mod(h * 6.0f + f32x3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f,
            0.0f, 1.0f);
  rgb = rgb * rgb * (3.0f - 2.0f * rgb);
  return v * mix(f32x3(1.0f), rgb, s);
}
f32x2x2 rotate2D(Context &__ctx, float t) {
  return f32x2x2(cos(t), sin(t), -sin(t), cos(t));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float time = iTime;
  fragCoord.data[0] += 140.0f * cos(iTime) * sin(0.01f * fragCoord.data[1]);
  fragCoord.data[1] += 88.88f * cos(iTime) * sin(0.01f * fragCoord.data[0]);
  fragColor = f32x4(0);
  f32x2 uv = 0.3f * (2.0f * fragCoord - iResolution.swizzle<0, 1>()) /
             iResolution.data[1];
  f32x3 ro = f32x3(0, 0, -0.6f);
  f32x3 rd = f32x3(uv, 1);
  float t = 0.0f;
  f32x2 m =
      2.0f * (iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - 0.5f);
  for (float i = 0.0f; i < 1e2f; i++) {
    f32x3 p = ro + t * rd - i / 2e5f;
    p.data[2] -= abs(sin(iTime * 0.1f));
    p.set_swizzle<1, 2>(p.swizzle<1, 2>() *
                        (rotate2D(__ctx, sin(iTime * 0.1f))));
    p.set_swizzle<0, 2>(p.swizzle<0, 2>() *
                        (rotate2D(__ctx, cos(iTime * 0.1f))));
    p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (rotate2D(__ctx, 0.2f)));
    float r = length(p);
    float e = asin(-p.data[2] / r) - 0.1f / r;
    float rot = cos(iTime * 0.2f);
    f32x3 q = f32x3(log(r) - time, e, rot * atan(p.data[0], p.data[1]));
    for (float scale = 1.0f; scale < 1e2f; scale += scale) {
      e += pow(abs(dot(sin(q.swizzle<1, 0, 2>() * scale), cos(q * scale))),
               0.2f) /
           scale;
    }
    t += e * r * 0.1f;
    if (t > 50.0f)
      break;
    float k = max(e * r * 1e4f, 0.7f);
    k = pow(k, 0.4f);
    fragColor.set_swizzle<0, 1, 2>(
        fragColor.swizzle<0, 1, 2>() +
        (hsv(__ctx, 0.4f - .02f / r, k, 0.02f / exp(k))));
  }
  fragColor *= 2.0f / (1.0f + fragColor);
  fragColor = pow(fragColor, f32x4(0.4545f));
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
