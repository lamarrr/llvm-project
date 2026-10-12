// Port of https://www.shadertoy.com/view/NtKfRc; author: rubioh.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NtKfRc.json.
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
f32x2 hash(Context &__ctx, f32x2 x) {
  f32x2 p = f32x2(dot(x, f32x2(3451.4327f, 2744.749f)),
                  dot(x, f32x2(2453.4327f, 3218.127f)));
  return fract(sin(p) * 3745.4f);
}
float make_ellipse(Context &__ctx, f32x2 U, f32x2 ab, float amp, float a) {
  f32x2 A = f32x2(cos(a), sin(a)), A2 = A * A, B2 = ab * ab, C = A * U;
  float a2b2 = B2.data[0] * B2.data[1],
        c = 2.f * C.data[0] * C.data[1] * (B2.data[1] - B2.data[0]) / a2b2;
  f32x2 Q = f32x2x2(A2.data[1], A2.data[0], A2) * B2 * U * U / a2b2;
  return smoothstep(10.f / iResolution.data[1], 0.f,
                    abs(Q.data[0] + Q.data[1] + c - amp));
}
f32x2x2 rotation_matrix(Context &__ctx, float angle) {
  return f32x2x2(cos(angle), sin(angle), -sin(angle), cos(angle));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (2.f * fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
             iResolution.data[1];
  uv /= 1.5f;
  float N = 11.f;
  f32x3 col;
  f32x3 tmp_color = f32x3(1.f, 1.f, 1.f);
  f32x2 ab = f32x2(.3f + .2f * cos(iTime), .2f);
  for (int i = 0; i < int(N); i++) {
    f32x2 st = uv;
    float angle = (2.f * 3.14159265359f * float(i) / N) + iTime + .1f;
    f32x2x2 rot = rotation_matrix(__ctx, angle);
    f32x2 tmp_ab =
        mix(ab, f32x2(.2f, .2f) + 0.05f * hash(__ctx, f32x2(float(i), angle)),
            smoothstep(0.4f, 0.6f, cos(iTime * .1f)));
    st +=
        rot * mix(f32x2(.1f, .1f),
                  f32x2(0.f, 0.f) + 0.05f * hash(__ctx, f32x2(angle, float(i))),
                  smoothstep(0.3f, 0.7f, abs(cos(iTime * .1f))));
    col += f32x3(1.f, 0.f, 0.f) *
           make_ellipse(__ctx, st, tmp_ab, .3f + .3f * cos(iTime * 2.f), angle);
  }
  for (int i = 0; i < int(N); i++) {
    f32x2 st = uv;
    float angle = (2.f * 3.14159265359f * float(i) / N) + iTime;
    f32x2x2 rot = rotation_matrix(__ctx, angle);
    f32x2 tmp_ab =
        mix(ab, f32x2(.2f, .2f) + 0.05f * hash(__ctx, f32x2(float(i), angle)),
            smoothstep(0.4f, 0.6f, cos(iTime * .1f)));
    st +=
        rot * mix(f32x2(.1f, .1f),
                  f32x2(0.f, 0.f) + 0.05f * hash(__ctx, f32x2(angle, float(i))),
                  smoothstep(0.3f, 0.7f, abs(cos(iTime * .1f))));
    col += f32x3(0.f, 1.f, 0.f) *
           make_ellipse(__ctx, st, tmp_ab, .3f + .3f * cos(iTime * 2.f), angle);
  }
  for (int i = 0; i < int(N); i++) {
    f32x2 st = uv;
    float angle = (2.f * 3.14159265359f * float(i) / N) + iTime - .1f;
    f32x2x2 rot = rotation_matrix(__ctx, angle);
    f32x2 tmp_ab =
        mix(ab, f32x2(.2f, .2f) + 0.05f * hash(__ctx, f32x2(float(i), angle)),
            smoothstep(0.4f, 0.6f, cos(iTime * .1f)));
    st +=
        rot * mix(f32x2(.1f, .1f),
                  f32x2(0.f, 0.f) + 0.05f * hash(__ctx, f32x2(angle, float(i))),
                  smoothstep(0.3f, 0.7f, abs(cos(iTime * .1f))));
    col += f32x3(0.f, 0.f, 1.f) *
           make_ellipse(__ctx, st, tmp_ab, .3f + .3f * cos(iTime * 2.f), angle);
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
