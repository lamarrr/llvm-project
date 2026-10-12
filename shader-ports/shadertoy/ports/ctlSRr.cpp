// Port of https://www.shadertoy.com/view/ctlSRr; author: NohatCoder.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctlSRr.json.
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

void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float boxsize = min(iResolution.data[0] * 0.5f, iResolution.data[1] * 0.5f);
  f32x2 centre;
  int quadrant = 0;
  if (fragCoord.data[0] * 2.0f < iResolution.data[0]) {
    centre.data[0] = iResolution.data[0] * 0.25f;
  } else {
    centre.data[0] = iResolution.data[0] * 0.75f;
    quadrant += 1;
  }
  if (fragCoord.data[1] * 2.0f < iResolution.data[1]) {
    centre.data[1] = iResolution.data[1] * 0.25f;
  } else {
    centre.data[1] = iResolution.data[1] * 0.75f;
    quadrant += 2;
  }
  float time = iTime;
  f32x3 c = f32x3(1.0f, 0.8f, 0.8f);
  if (quadrant == 1) {
    time = floor(time * 200.0f) / 200.0f;
    c = f32x3(1.0f, 1.0f, 1.0f);
  }
  if (quadrant == 2) {
    time = floor(time * 100.0f) / 100.0f;
    c = f32x3(0.8f, 1.0f, 0.8f);
  }
  if (quadrant == 3) {
    time = floor(time * 50.0f) / 50.0f;
    c = f32x3(0.8f, 0.8f, 1.0f);
  }
  time *= 1.0f;
  f32x2 along = f32x2(sin(time), cos(time));
  f32x2 normal = f32x2(cos(time), -sin(time));
  f32x2 end1 = centre + along * boxsize * 0.4f;
  f32x2 end2 = centre - along * boxsize * 0.4f;
  float radius = boxsize * 0.05f;
  float radiuss = radius * radius;
  float sumd = 0.0f;
  float sumdp = 0.0f;
  float a;
  float b;
  for (a = -0.5f; a < 1.0f; a += 1.0f) {
    for (b = -0.5f; b < 1.0f; b += 1.0f) {
      f32x2 pixelcorner = fragCoord + f32x2(a, b);
      float d = distance(end1, pixelcorner) - radius;
      d = min(d, distance(end2, pixelcorner) - radius);
      float inlinesegment = dot(pixelcorner - centre, along);
      if (abs(inlinesegment) < boxsize * 0.4f) {
        d = min(d, abs(dot(normal, pixelcorner - centre)) - radius);
      }
      sumd += d;
      sumdp += abs(d);
    }
  }
  float fill = ((-sumd / sumdp) + 1.0f) * 0.5f;
  fragColor = f32x4(c * fill, 1.0f);
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
