// Port of https://www.shadertoy.com/view/cllXzN; author: QuantumSuper.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cllXzN.json.
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
f32x3[3] lines;
float rect2(Context &__ctx, f32x2 myVar, f32x2 lim1, f32x2 lim2) {
  return dot(step(min(lim1, lim2), myVar), step(myVar, max(lim1, lim2)));
}
float sRect2(Context &__ctx, f32x2 myVar, f32x2 lim1, f32x2 lim2) {
  f32x2 myMin = min(lim1, lim2);
  f32x2 myMax = max(lim1, lim2);
  return dot(smoothstep(myMin, myMin + .05f, myVar),
             smoothstep(myMax, myMax - .05f, myVar));
}
float getLine(Context &__ctx, f32x2 pos, f32x3 def) {
  return dot(def, f32x3(pos, 1.f));
}
float getDist(Context &__ctx, f32x2 pos) {
  float minDist = 1.f;
  float someDist;
  for (int n = 0; n < 3; n++) {
    minDist = min(minDist, abs(getLine(__ctx, pos, lines[n])));
  }
  return minDist;
}
f32x2 guessNormal(Context &__ctx, f32x2 pos) {
  f32x2 tangent = f32x2(0.02f, 0);
  f32x2 normal = getDist(__ctx, pos) -
                 f32x2(getDist(__ctx, pos - tangent.swizzle<0, 1>()),
                       getDist(__ctx, pos - tangent.swizzle<1, 0>()));
  return normalize(normal);
}
float rayMarch(Context &__ctx, f32x2 rayOrigin, f32x2 rayDir) {
  float rayLength = 0.1f;
  float radCirc;
  if (length(rayOrigin) > .5f)
    rayLength += length(rayOrigin) - .5f;
  for (int n = 0; n < 100; n++) {
    radCirc = getDist(__ctx, rayOrigin + rayLength * rayDir);
    rayLength += radCirc;
    if (rayLength > 1.f || abs(radCirc) < 0.02f)
      break;
  }
  return rayLength;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (2.f * fragCoord - iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]);
  float animTime = 2.133333f * iTime;
  uv.data[1] += .1f + .1f * sin(animTime / 12.f);
  uv *= 1.f + .05f * sin(animTime / 12.f);
  f32x3 col = f32x3(0);
  lines = f32x3[](f32x3(0.f, 1.f, .5f * sin(1.0471975512f / 2.f)),
                  f32x3(sin(1.0471975512f), cos(1.0471975512f),
                        -.5f * sin(1.0471975512f / 2.f)),
                  f32x3(-sin(1.0471975512f), cos(1.0471975512f),
                        -.5f * sin(1.0471975512f / 2.f)));
  float bw = 0.f;
  bw += smoothstep(.01f, .0f, abs(getLine(__ctx, uv, lines[0])));
  bw += smoothstep(.01f, .0f, abs(getLine(__ctx, uv, lines[1])));
  bw += smoothstep(.01f, .0f, abs(getLine(__ctx, uv, lines[2])));
  bw *= smoothstep(.001f, .0f,
                   uv.data[0] * uv.data[0] + uv.data[1] * uv.data[1] - 0.25f);
  col += f32x3(bw);
  f32x2 normal, pos, dir, pos2, dir2;
  float amp, spread;
  float[] eta = float[](0.68f - .1f * 1.2598f, 0.68f - .1f * 1.3660f,
                        0.68f - .1f * 1.4863f, 0.68f - .1f * 1.7666f,
                        0.68f - .1f * 2.3124f, 0.68f - .1f * 2.6122f,
                        0.68f - .1f * 2.9032f);
  f32x3[] myCol =
      f32x3[](.3f * abs(cos(animTime - .3f)) * f32x3(1, 0, 0),
              .2f * abs(cos(animTime - .2f)) * f32x3(.895f, .446f, 0),
              .2f * abs(cos(animTime - .1f)) * f32x3(.707f, .707f, 0),
              .2f * abs(cos(animTime)) * f32x3(0, 1, 0),
              .2f * abs(cos(animTime + .1f)) * f32x3(0, 0, 1),
              .05f * abs(cos(animTime + .2f)) * f32x3(.5f, 0, .866f),
              .05f * abs(cos(animTime + .3f)) * f32x3(.574f, 0, .819f));
  for (int rayCol = 0; rayCol < myCol.length(); rayCol++) {
    for (int rayT = 1; rayT < 4; rayT++) {
      pos = f32x2(-1.1f, .03f + .05f * sin(animTime / 8.f));
      dir = normalize(
          f32x2(1.f + .01f * float(rayCol), .2f + .1f * sin(animTime / 8.f)));
      amp = 1.f;
      spread = 0.01f;
      for (int n = 0; n < 5; n++) {
        pos2 = pos + rayMarch(__ctx, pos, dir) * dir;
        col += amp * sRect2(__ctx, uv, pos, pos2) *
               smoothstep(spread, .0f,
                          abs(getLine(__ctx, uv,
                                      f32x3(dir.data[1], -dir.data[0],
                                            pos.data[1] * dir.data[0] -
                                                pos.data[0] * dir.data[1])))) *
               myCol[rayCol];
        spread += .01f * length(pos2 - pos);
        normal = guessNormal(__ctx, pos2 - 0.02f * dir);
        if (int(mod(float(n + rayT) + mod(float(n), 2.f) * float(rayT) / 2.f,
                    2.f)) == 0) {
          dir2 = reflect(dir, normal);
          amp *= 0.4f;
        } else {
          dir2 = refract(dir, normal, eta[rayCol]);
          eta[rayCol] = 1.f / eta[rayCol];
          amp *= 0.8f;
        }
        if (pos2.data[0] * pos2.data[0] + pos2.data[1] * pos2.data[1] > .25f)
          break;
        pos = pos2;
        dir = dir2;
      }
    }
  }
  col = pow(col, f32x3(.4545f));
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
