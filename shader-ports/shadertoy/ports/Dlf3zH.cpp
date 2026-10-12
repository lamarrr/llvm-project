// Port of https://www.shadertoy.com/view/Dlf3zH; author: PlaNex.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Dlf3zH.json.
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
const int ROWS = 12, COLUMNS = 20;
const f32x4 BACKGROUND = f32x4(80, 57, 76, 255) / 255.f,
            SQUARECOLOR = f32x4(255, 239, 150, 255) / 255.f;
const float DIAGONALTOSIDE = 1.4142f;
struct square {
  f32x2 centre;
  float sideLength;
  float radians;
  f32x4 color;
};
float getAngle(Context &__ctx, f32x2 origin, f32x2 dir) {
  dir = dir - origin;
  return atan(dir.data[0], dir.data[1]);
}
bool isInSquare(Context &__ctx, square checkSq, f32x2 coords) {
  float sqSin = sin(checkSq.radians);
  float sqCos = cos(checkSq.radians);
  f32x2x2 rotationMatrix = f32x2x2(sqCos, -sqSin, sqSin, sqCos);
  coords = (coords - checkSq.centre) * rotationMatrix;
  coords = checkSq.centre + coords;
  coords = abs(coords - checkSq.centre);
  return max(coords.data[0], coords.data[1]) < checkSq.sideLength / 2.f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  fragColor = BACKGROUND;
  f32x2 COUNT = f32x2(COLUMNS, ROWS),
        BetweenSq = iResolution.swizzle<0, 1>() / (COUNT + 1.f),
        diag = BetweenSq * DIAGONALTOSIDE / 2.f;
  float squareSide = min(diag.data[0], diag.data[1]) * 0.9f;
  f32x2 Pos = floor(fragCoord / BetweenSq + .5f) - .5f;
  if (Pos.data[0] > 0.f && Pos.data[1] > 0.f && Pos.data[0] < COUNT.data[0] &&
      Pos.data[1] < COUNT.data[1]) {
    Pos = Pos * BetweenSq + BetweenSq / 2.f;
    square sq =
        square(Pos, squareSide, getAngle(__ctx, Pos, iMouse.swizzle<0, 1>()),
               SQUARECOLOR);
    if (isInSquare(__ctx, sq, fragCoord))
      fragColor = sq.color;
  }
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
