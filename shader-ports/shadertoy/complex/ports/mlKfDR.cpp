// Port of https://www.shadertoy.com/view/mlKfDR; author: LuckyLMJ.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mlKfDR.json.
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
f32x3 cameraPosition = f32x3(0, 0, -20);
f32x3 cameraDirection = f32x3(0, 0, 0);
f32x3 lightDirection = f32x3(1, 1.5f, 0);
f32x3 rotatePoint(Context &__ctx, f32x3 p, f32x3 a) {
  a = a.swizzle<1, 0, 2>();
  p.set_swizzle<1, 2>(
      f32x2(p.data[1] * cos(a.data[1]) + p.data[2] * sin(a.data[1]),
            -p.data[1] * sin(a.data[1]) + p.data[2] * cos(a.data[1])));
  p.set_swizzle<0, 2>(
      f32x2(p.data[0] * cos(a.data[0]) + p.data[2] * sin(a.data[0]),
            -(p.data[0] * sin(a.data[0]) - p.data[2] * cos(a.data[0]))));
  p.set_swizzle<0, 1>(
      f32x2(p.data[0] * cos(a.data[2]) - p.data[1] * sin(a.data[2]),
            p.data[0] * sin(a.data[2]) + p.data[1] * cos(a.data[2])));
  return p;
}
float sphereSDF(Context &__ctx, f32x3 p, f32x3 sp, float r) {
  return distance(p, sp) - r;
}
float boxSDF(Context &__ctx, f32x3 p, f32x3 sp, f32x3 b) {
  f32x3 q = abs(p - sp) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
float map(Context &__ctx, f32x3 p) {
  float dist = float(100);
  dist = p.data[1] + 4.0f;
  dist = min(dist, sphereSDF(__ctx, p, f32x3(0, -1, 0), 3.0f));
  dist = min(dist,
             sphereSDF(__ctx, p, f32x3(7, 3.0f + sin(iTime * 3.0f), 3), 2.0f));
  dist = min(dist, boxSDF(__ctx, p, f32x3(4, -3, -4), f32x3(1, 1, 1)));
  dist = min(dist, max(boxSDF(__ctx, p, f32x3(-3, 0, 5), f32x3(1, 1, 1)),
                       -sphereSDF(__ctx, p, f32x3(-3, 0, 5), 1.3f)));
  dist = min(dist, max(sphereSDF(__ctx, p, f32x3(3, -3, 5), 1.0f),
                       sphereSDF(__ctx, p, f32x3(3, -2, 5), 1.0f)));
  return dist;
}
f32x3 getColour(Context &__ctx, f32x3 p) {
  f32x3 colour = f32x3(1);
  if (p.data[1] + 4.0f < 1e-4f) {
    f32x3 fp = floor(p);
    colour =
        f32x3(mod(fp.data[0] + fp.data[1] + fp.data[2], 2.f)) + f32x3(0.5f);
  } else if (sphereSDF(__ctx, p, f32x3(0, -1, 0), 3.0f) < 1e-4f) {
    colour = f32x3(1, 0, 0);
  } else if (sphereSDF(__ctx, p, f32x3(7, 3.0f + sin(iTime * 3.0f), 3), 2.0f) <
             1e-4f) {
    colour = f32x3(1, 0, 1);
  } else if (boxSDF(__ctx, p, f32x3(4, -3, -4), f32x3(1, 1, 1)) < 1e-4f) {
    colour = f32x3(1, 0.5f, 0);
  } else if (max(boxSDF(__ctx, p, f32x3(-3, 0, 5), f32x3(1, 1, 1)),
                 -sphereSDF(__ctx, p, f32x3(-3, 0, 5), 1.3f)) < 1e-4f) {
    colour = f32x3(0, 1, 0);
  } else if (max(sphereSDF(__ctx, p, f32x3(3, -3, 5), 1.0f),
                 sphereSDF(__ctx, p, f32x3(3, -2, 5), 1.0f)) < 1e-4f) {
    colour = f32x3(0.5f, 0.6f, 1);
  }
  return colour;
}
f32x3 getNormal(Context &__ctx, f32x3 p) {
  const f32x2 h = f32x2(1e-5f, 0);
  float mapP = map(__ctx, p);
  return normalize(f32x3(map(__ctx, p + h.swizzle<0, 1, 1>()) - mapP,
                         map(__ctx, p + h.swizzle<1, 0, 1>()) - mapP,
                         map(__ctx, p + h.swizzle<1, 1, 0>()) - mapP));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 screenCoordinate =
      (-iResolution.swizzle<0, 1>() + 2.0f * fragCoord.swizzle<0, 1>()) /
      iResolution.data[1];
  lightDirection = rotatePoint(__ctx, lightDirection, f32x3(0, iTime, 0));
  lightDirection = normalize(lightDirection);
  cameraDirection = f32x3(0, -iTime * 0.1f, 0);
  cameraPosition = rotatePoint(__ctx, cameraPosition, cameraDirection);
  f32x3 rayDirection = rotatePoint(
      __ctx, normalize(f32x3(screenCoordinate, 1.5f)), cameraDirection);
  float t = 0.0f;
  f32x3 colour = f32x3(0, 0, 0);
  float light = 1.0f;
  for (int i = 0; i < 1000; i++) {
    float dist = map(__ctx, cameraPosition + t * rayDirection);
    t += dist;
    if (dist < 1e-5f) {
      f32x3 normal = getNormal(__ctx, cameraPosition + t * rayDirection);
      light = max(dot(normal, lightDirection), 0.0f);
      light = (light * (1.0f - 0.25f)) + 0.25f;
      float shadowT = 0.0f;
      f32x3 shadowBaseP = (cameraPosition + (t - 1e-3f) * rayDirection);
      for (int j = 0; j < 1000; j++) {
        float dist = map(__ctx, shadowBaseP + shadowT * lightDirection);
        shadowT += dist;
        if (dist < 1e-5f) {
          light = 0.25f;
          break;
        }
        if (shadowT > float(100)) {
          break;
        }
      }
      light *= 1.0f - (t / float(100));
      colour = getColour(__ctx, cameraPosition + t * rayDirection);
      break;
    }
    if (t > float(100)) {
      break;
    }
  }
  fragColor = f32x4(colour, 1);
  fragColor.set_swizzle<0, 1, 2>(fragColor.swizzle<0, 1, 2>() * (light));
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
