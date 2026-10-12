// Port of https://www.shadertoy.com/view/cttyDj; author: misol101.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cttyDj.json.
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
const float PI = 3.14159265359f;
const float PHASE_LENGTH = 9.f;
const float MAX_NOF_SEG = 30.f;
const float MAX_WIDTH = 0.355f;
const float FADE_TIME = 1.f;
const int NOF_PHASES = 5;
float nof_lines, nof_full_sections, main_line_width, swing_var, swing_speed,
    xTime = 0.f, yTime = 0.f;
f32x4 colorizer = f32x4(0.5f, 0.9f, 0.5f, 0.f);
void getPhaseData(Context &__ctx, float time) {
  int phase = (int(time / PHASE_LENGTH) + 0) % NOF_PHASES;
  xTime = mod(time, PHASE_LENGTH);
  nof_lines = 41.f, main_line_width = 0.8f;
  switch (phase) {
  case 0:
    nof_full_sections = 2.7f, swing_var = 2.8f, swing_speed = 1.5f;
    yTime = xTime * 0.15f;
    colorizer = f32x4(0.5f, 0.9f, 0.5f, 0.f);
    break;
  case 1:
    nof_full_sections = 1.5f, swing_var = 1.0f, swing_speed = 1.f;
    yTime = 4.6f + xTime * -0.3f;
    colorizer = f32x4(0.9f, 0.9f, 0.5f, 0.f);
    break;
  case 2:
    nof_full_sections = 5.0f, swing_var = 1.8f, swing_speed = 0.8f;
    yTime = xTime * 0.3f;
    colorizer = f32x4(0.3f, 0.7f, 1.0f, 0.f);
    break;
  case 3:
    nof_lines = 61.f, nof_full_sections = 0.6f;
    swing_var = 0.7f, swing_speed = 0.9f, main_line_width = 1.6f;
    yTime = 3.0f + xTime * -0.1f;
    colorizer = f32x4(1.0f, 0.5f, 0.5f, 0.f);
    break;
  case 4:
    nof_full_sections = 3.7f, swing_var = 1.2f, swing_speed = 2.4f;
    yTime = 4.4f + xTime * -0.42f;
    colorizer = f32x4(1.0f, 0.6f, 1.0f, 0.f);
    nof_lines = 38.f, main_line_width = 1.1f;
    break;
  }
  float ft = FADE_TIME * 0.5f;
  if (xTime < ft)
    colorizer *= (xTime / ft);
  if (xTime > PHASE_LENGTH - ft)
    colorizer *= (PHASE_LENGTH - xTime) / ft;
}
float getCosInput(Context &__ctx, float n, float w, float m) {
  float v = mod(n, PI * 2.f) / (PI * 2.f) - 0.5f;
  v *= 1.f + (MAX_WIDTH - w) * m;
  v = clamp(v + 0.5f, 0.f, 1.f);
  return v * PI * 2.f;
}
float swingWidth(Context &__ctx, float section) {
  return MAX_WIDTH *
         (0.55f + sin(xTime * swing_speed + section * swing_var) * 0.5f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x4 col = f32x4(0.f);
  getPhaseData(__ctx, iTime);
  float uvTy = uv.data[1] + yTime;
  float section = 0.f, yprogress = 0.f, ss = 0.f;
  float fullSegSize = 1.f / nof_full_sections;
  for (float i = 0.f; i < MAX_NOF_SEG; i++) {
    float w = abs(swingWidth(__ctx, i));
    float old_ss = ss;
    ss += smoothstep(0.0f, 1.0f, w / MAX_WIDTH) * fullSegSize +
          fullSegSize * 0.15f;
    if (uvTy >= old_ss && uvTy < ss) {
      section = i;
      yprogress = (uvTy - old_ss) / (ss - old_ss);
      break;
    }
  }
  float w = abs(sin(yprogress * PI) * (swingWidth(__ctx, section) + 0.02f));
  float dist = abs(uv.data[0] - 0.5f);
  if (dist < w) {
    dist /= w;
    col = f32x4(smoothstep(
        -1.f, 1.f, cos(getCosInput(__ctx, dist * PI * nof_lines, w, 4.5f))));
    col *= w * 1.9f;
  }
  float dc = MAX_WIDTH * 2.f / nof_lines * main_line_width;
  float dir = float((int(section) % 2) * 2 - 1);
  float linw =
      (sin(yprogress * PI) * (swingWidth(__ctx, section) + 0.02f - dc * 0.5f)) *
      dir;
  dist = abs(uv.data[0] - 0.5f - linw);
  if (dist < dc) {
    dist /= dc;
    f32x4 col2 =
        f32x4(smoothstep(-1.f, 1.f,
                         cos(getCosInput(__ctx, dist * PI, dist, 0.4f)))) *
        0.66f;
    col = (dist > 0.65f) ? max(col, col2) : col2;
  }
  fragColor = col * colorizer;
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
