// Port of https://www.shadertoy.com/view/fsScWz; author: magnias.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fsScWz.json.
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
f32x3x3 rotateX(Context &__ctx, float theta) {
  float c = cos(theta);
  float s = sin(theta);
  return f32x3x3(f32x3(1, 0, 0), f32x3(0, c, -s), f32x3(0, s, c));
}
f32x3x3 rotateY(Context &__ctx, float theta) {
  float c = cos(theta);
  float s = sin(theta);
  return f32x3x3(f32x3(c, 0, s), f32x3(0, 1, 0), f32x3(-s, 0, c));
}
f32x3x3 rotateZ(Context &__ctx, float theta) {
  float c = cos(theta);
  float s = sin(theta);
  return f32x3x3(f32x3(c, -s, 0), f32x3(s, c, 0), f32x3(0, 0, 1));
}
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
float sdf(Context &__ctx, f32x3 p) {
  f32x3 c = f32x3(3);
  f32x3 modPos = mod(p + .5f * c, c) - .5f * c;
  f32x3 pos = f32x3(modPos.data[0], p.data[1], modPos.data[2]);
  pos = p;
  float d = min(sdBox(__ctx, pos, f32x3(1)),
                sdBox(__ctx, pos - f32x3(0, -2, 0), f32x3(15, 1, 15)));
  d = max(d, -(length(f32x3(pos.data[0], pos.data[1] + sin(iTime / 2.f),
                            pos.data[2])) -
               1.2f));
  for (int x = -1; x <= 1; x++)
    for (int z = -1; z <= 1; z++) {
      if (x == 0 && z == 0)
        continue;
      d = max(d, -(length(f32x3(pos.data[0] + 3.4f * float(x),
                                pos.data[1] - sin(iTime / 2.f),
                                pos.data[2] + 3.4f * float(z))) -
                   1.2f));
    }
  return d;
}
f32x3 calcNormal(Context &__ctx, f32x3 p) {
  const float h = 0.0001f;
  const f32x2 k = f32x2(1, -1);
  return normalize(
      k.swizzle<0, 1, 1>() * sdf(__ctx, p + k.swizzle<0, 1, 1>() * h) +
      k.swizzle<1, 1, 0>() * sdf(__ctx, p + k.swizzle<1, 1, 0>() * h) +
      k.swizzle<1, 0, 1>() * sdf(__ctx, p + k.swizzle<1, 0, 1>() * h) +
      k.swizzle<0, 0, 0>() * sdf(__ctx, p + k.swizzle<0, 0, 0>() * h));
}
float ray_march(Context &__ctx, f32x3 origin, f32x3 direction) {
  f32x3 pos = origin;
  if (sdf(__ctx, pos) < 0.f) {
    return -1.f;
  }
  float dist = 0.f;
  for (int i = 0; i < 100; i++) {
    float step_dist = sdf(__ctx, pos);
    dist += step_dist;
    pos = origin + dist * direction;
    if (step_dist < 0.0001f) {
      return dist;
    }
  }
  return -1.f;
}
bool is_edge(Context &__ctx, f32x3 position) {
  f32x3 p = f32x3(0);
  float totalSdf = 0.f;
  float possible_max = 0.f;
  float scale = 0.015f;
  float minVal = -1.f * scale;
  float maxVal = 1.f * scale;
  float stepSize = .5f * scale;
  for (p.data[0] = minVal; p.data[0] <= maxVal; p.data[0] += stepSize)
    for (p.data[1] = minVal; p.data[1] <= maxVal; p.data[1] += stepSize)
      for (p.data[2] = minVal; p.data[2] <= maxVal; p.data[2] += stepSize) {
        if (p == f32x3(0.f))
          continue;
        float weight = (abs(p.data[0]) * abs(p.data[1]) * abs(p.data[2]));
        if (sdf(__ctx, position + p) > 0.f)
          totalSdf += weight;
        else
          totalSdf -= weight;
        possible_max += weight;
      }
  totalSdf /= possible_max;
  if (totalSdf > 0.5f)
    return true;
  return false;
}
f32x3 march_scene(Context &__ctx, f32x3 origin, f32x3 direction) {
  float dist = ray_march(__ctx, origin, direction);
  if (dist == -1.f) {
    return f32x3(.1f);
  }
  f32x3 pos = origin + direction * dist;
  f32x3 lightdir = normalize(f32x3(1, -.6f, 1));
  f32x3 lightcolor = f32x3(1, .3f, .8f);
  f32x3 color = f32x3(1, 0, 0) * lightcolor *
                max(dot(-lightdir, calcNormal(__ctx, pos)), 0.1f);
  if (true) {
    float dist_to_light =
        ray_march(__ctx, pos + -f32x3(20) * lightdir, lightdir);
    if (dist_to_light < 20.f - 0.001f) {
      color = f32x3(.05f);
    }
  }
  if (false && is_edge(__ctx, pos)) {
    return f32x3(0.34f);
  }
  return color;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() /
                 f32x2(iResolution.data[0], iResolution.data[1]) * 2.0f -
             1.0f;
  float aspect = iResolution.data[0] / iResolution.data[1];
  uv *= f32x2(6, 6.f / aspect);
  f32x3 worldpos = f32x3(uv.data[0], uv.data[1], 0.f);
  worldpos -= f32x3(0, 0, 20);
  f32x3 worlddir = normalize(f32x3(0, 0, 1));
  f32x3x3 rot =
      rotateX(__ctx, -0.4f) * rotateY(__ctx, 0.4f) * rotateZ(__ctx, 0.0f);
  worldpos = rot * worldpos;
  worlddir = rot * worlddir;
  f32x3 color = f32x3(0);
  color = march_scene(__ctx, worldpos, worlddir);
  fragColor = f32x4(color, 1.0f);
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
