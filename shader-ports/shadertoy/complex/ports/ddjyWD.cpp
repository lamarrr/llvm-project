// Port of https://www.shadertoy.com/view/ddjyWD; author: etdeagle.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ddjyWD.json.
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

const float PI = 3.1415926535897932384626433832795f;
f32x3 xorCols(Context &__ctx, f32x3 col0, f32x3 col1) {
  f32x3 col = f32x3(int(col0.data[0] * 255.0f) ^ int(col1.data[0] * 255.0f),
                    int(col0.data[1] * 255.0f) ^ int(col1.data[1] * 255.0f),
                    int(col0.data[2] * 255.0f) ^ int(col1.data[2] * 255.0f));
  return col / 255.0f;
}
float rand(Context &__ctx, f32x2 co) {
  return fract(sin(dot(co, f32x2(12.9898f, 78.233f))) * 43758.5453f);
}
f32x3 rgb_to_hsl(Context &__ctx, f32x3 rgb) {
  f32x3 hsl;
  float fmin = min(min(rgb.data[0], rgb.data[1]), rgb.data[2]);
  float fmax = max(max(rgb.data[0], rgb.data[1]), rgb.data[2]);
  float delta = fmax - fmin;
  hsl.data[2] = (fmax + fmin) / 2.0f;
  if (delta == 0.0f) {
    hsl.data[0] = 0.0f;
    hsl.data[1] = 0.0f;
  } else {
    if (hsl.data[2] < 0.5f)
      hsl.data[1] = delta / (fmax + fmin);
    else
      hsl.data[1] = delta / (2.0f - fmax - fmin);
    float deltaR = (((fmax - rgb.data[0]) / 6.0f) + (delta / 2.0f)) / delta;
    float deltaG = (((fmax - rgb.data[1]) / 6.0f) + (delta / 2.0f)) / delta;
    float deltaB = (((fmax - rgb.data[2]) / 6.0f) + (delta / 2.0f)) / delta;
    if (rgb.data[0] == fmax)
      hsl.data[0] = deltaB - deltaG;
    else if (rgb.data[1] == fmax)
      hsl.data[0] = (1.0f / 3.0f) + deltaR - deltaB;
    else if (rgb.data[2] == fmax)
      hsl.data[0] = (2.0f / 3.0f) + deltaG - deltaR;
    if (hsl.data[0] < 0.0f)
      hsl.data[0] += 1.0f;
    else if (hsl.data[0] > 1.0f)
      hsl.data[0] -= 1.0f;
  }
  return hsl;
}
f32x3 drawTriangle(Context &__ctx, f32x2 suv, f32x2 rot, float side) {
  float height = sqrt(side * side - (side * side / 4.0f));
  f32x2 p1 = f32x2(-side / 2.0f, 0);
  f32x2 p2 = f32x2(0, height);
  f32x2 p3 = f32x2(side / 2.0f, 0);
  p1 = f32x2(p1.data[0] * rot.data[0] - p1.data[1] * rot.data[1],
             p1.data[0] * rot.data[1] + p1.data[1] * rot.data[0]);
  p2 = f32x2(p2.data[0] * rot.data[0] - p2.data[1] * rot.data[1],
             p2.data[0] * rot.data[1] + p2.data[1] * rot.data[0]);
  p3 = f32x2(p3.data[0] * rot.data[0] - p3.data[1] * rot.data[1],
             p3.data[0] * rot.data[1] + p3.data[1] * rot.data[0]);
  f32x2 barycenter = (p1 + p2 + p3) / 3.0f;
  f32x2 v1 = p2 - p1;
  f32x2 v2 = p3 - p1;
  f32x2 v3 = suv - p1 + barycenter;
  float dot11 = dot(v1, v1);
  float dot12 = dot(v1, v2);
  float dot13 = dot(v1, v3);
  float dot22 = dot(v2, v2);
  float dot23 = dot(v2, v3);
  float invDenom = 1.0f / (dot11 * dot22 - dot12 * dot12);
  float u = (dot22 * dot13 - dot12 * dot23) * invDenom;
  float v = (dot11 * dot23 - dot12 * dot13) * invDenom;
  f32x2 uv = f32x2(u, v);
  f32x3 col = f32x3(0);
  if ((uv.data[0] >= 0.0f) && (uv.data[1] >= 0.0f) &&
      (uv.data[0] + uv.data[1] < 1.0f)) {
    float t1 = uv.data[0];
    float t2 = uv.data[1];
    float x =
        (1.0f +
         pow(sin(t1 * sin(t1 * 8.0f) + pow(sin(t1 * t1), 1.f) * 5.0f), 2.0f)) /
        2.0f;
    float y = (1.0f + cos(cos(t2) * 18.0f)) / 2.0f;
    float z = 1.0f - x * x - y * x;
    col = f32x3(1.0f);
  }
  return col;
}
f32x3 xorTriangles(Context &__ctx, f32x2 suv0, f32x2 rot0, float side0,
                   f32x3 col0, f32x2 suv1, f32x2 rot1, float side1,
                   f32x3 col1) {
  f32x3 cola = drawTriangle(__ctx, suv0, rot0, side0);
  f32x3 colb = drawTriangle(__ctx, suv1, rot1, side1);
  if (cola.data[0] == 0.0f) {
    cola = col0;
  }
  if (colb.data[0] == 0.0f) {
    colb = col1;
  }
  f32x3 col = xorCols(__ctx, cola, colb);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  uv = 2.0f * uv - 1.0f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 col = f32x3(0);
  float r = length(uv);
  if (r < 6.0f) {
    float a = atan(uv.data[1], uv.data[0]);
    f32x2 suv = f32x2(cos(a), sin(a)) * r;
    f32x2 rot = f32x2(cos(2.0f * PI / 6.0f), sin(2.0f * PI / 6.0f));
    float side = 1.2f;
    f32x2 rot3 = f32x2(1.0f, 0);
    f32x2 rot4 = f32x2(-1.0f, 0);
    float t0 = 1.0f;
    float t1 = 1.4f;
    float t2 = 1.5f;
    float t3 = 1.8f;
    f32x3 turquoise = f32x3(71.f, 224.f, 204.f) / 255.0f;
    f32x3 ochre = f32x3(239.f, 192.f, 102.f) / 255.0f;
    f32x3 blue = f32x3(24.f, 88.f, 178.f) / 255.0f;
    f32x3 green = f32x3(10, 81, 51) / 255.0f;
    f32x3 colors[6];
    colors[0] = f32x3(0.14461228f, 0.15513487f, 0.27088341f);
    colors[1] = f32x3(0.87665264f, 0.56123238f, 0.49973392f);
    colors[2] = f32x3(0.06986726f, 0.44971911f, 0.75165264f);
    colors[3] = f32x3(1.00000000f, 0.85636869f, 0.50259627f);
    colors[4] = f32x3(0.97761418f, 0.93624878f, 0.73902827f);
    colors[5] = f32x3(0.16251525f, 0.29011418f, 0.25204154f);
    f32x3 col00 = f32x3(0.14461228f, 0.15513487f, 0.27088341f);
    f32x3 col01 = f32x3(0.14461228f, 0.15513487f, 0.27088341f);
    f32x3 colx = f32x3(0.00000000f, 0.00000000f, 0.00000000f);
    int rings = 5;
    int ngon = 4;
    side = 0.9f;
    for (int i = 0; i < rings; i++) {
      t0 = 0.4f + float(i) / 3.0f;
      for (int j = 0; j < ngon; j++) {
        f32x2 localRot = f32x2(cos(float(j) * 2.0f * PI / float(ngon)),
                               sin(float(j) * 2.0f * PI / float(ngon)));
        colx = xorCols(__ctx, colx,
                       xorTriangles(__ctx, suv, rot3, side * t0, col00, suv,
                                    localRot, side * t0, col01));
      }
    }
    col = colx;
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
