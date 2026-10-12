// Port of https://www.shadertoy.com/view/dtf3RM; author: BeRo.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtf3RM.json.
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
float sdTorus(Context &__ctx, f32x3 p, f32x2 t) {
  f32x2 q = f32x2(length(p.swizzle<0, 2>()) - t.data[0], p.data[1]);
  return length(q) - t.data[1];
}
float sdBox(Context &__ctx, f32x3 p, f32x3 b, f32x3 r) {
  f32x3 d = abs(p) - b;
  return length(max(d, 0.0f)) +
         min(max(d.data[0], max(d.data[1], d.data[2])), 0.0f);
}
float sdSphere(Context &__ctx, f32x3 p, float r) { return length(p) - r; }
f32x2 sdModernCar(Context &__ctx, f32x3 p) {
  float dist = 10000.0f;
  float materialIndex = 0.0f;
  float bodyDist = sdBox(__ctx, p - f32x3(0.0f, 0.5f, 0.0f),
                         f32x3(1.0f, 0.5f, 1.8f), f32x3(0.0f));
  dist = min(dist, bodyDist);
  materialIndex = 1.0f;
  float grillDist = sdBox(__ctx, p - f32x3(0.0f, 0.5f, 1.8f),
                          f32x3(1.0f, 0.1f, 0.2f), f32x3(0.0f));
  if (grillDist < dist) {
    dist = grillDist;
    materialIndex = 3.0f;
  }
  float flhDist = sdSphere(__ctx, p - f32x3(0.9f, 0.5f, 1.8f), 0.2f);
  if (flhDist < dist) {
    dist = flhDist;
    materialIndex = 4.0f;
  }
  float frhDist = sdSphere(__ctx, p - f32x3(-0.9f, 0.5f, 1.8f), 0.2f);
  if (frhDist < dist) {
    dist = frhDist;
    materialIndex = 4.0f;
  }
  float fltDist =
      sdTorus(__ctx, p - f32x3(0.7f, 0.0f, 1.0f), f32x2(0.2f, 0.2f));
  if (fltDist < dist) {
    dist = fltDist;
    materialIndex = 5.0f;
  }
  float frtDist =
      sdTorus(__ctx, p - f32x3(-0.7f, 0.0f, 1.0f), f32x2(0.2f, 0.2f));
  if (frtDist < dist) {
    dist = frtDist;
    materialIndex = 5.0f;
  }
  float bltDist =
      sdTorus(__ctx, p - f32x3(0.7f, 0.0f, -1.2f), f32x2(0.2f, 0.2f));
  if (bltDist < dist) {
    dist = bltDist;
    materialIndex = 5.0f;
  }
  float brtDist =
      sdTorus(__ctx, p - f32x3(-0.7f, 0.0f, -1.2f), f32x2(0.2f, 0.2f));
  if (brtDist < dist) {
    dist = brtDist;
    materialIndex = 5.0f;
  }
  return f32x2(dist, materialIndex);
}
f32x3 sdNormal(Context &__ctx, f32x3 p, float eps) {
  f32x3 n;
  n.data[0] = sdModernCar(__ctx, p + f32x3(eps, 0.0f, 0.0f)).data[0] -
              sdModernCar(__ctx, p - f32x3(eps, 0.0f, 0.0f)).data[0];
  n.data[1] = sdModernCar(__ctx, p + f32x3(0.0f, eps, 0.0f)).data[0] -
              sdModernCar(__ctx, p - f32x3(0.0f, eps, 0.0f)).data[0];
  n.data[2] = sdModernCar(__ctx, p + f32x3(0.0f, 0.0f, eps)).data[0] -
              sdModernCar(__ctx, p - f32x3(0.0f, 0.0f, eps)).data[0];
  return normalize(n);
}
f32x2 raymarchCar(Context &__ctx, f32x3 start, f32x3 direction) {
  float t = 0.0f;
  float dist = 0.0f;
  float minDist = 10000.0f;
  float materialIndex = 0.0f;
  int i = 0;
  while (i < 300 && t < 30.0f) {
    f32x3 pos = start + t * direction;
    f32x2 result = sdModernCar(__ctx, pos);
    dist = result.data[0];
    float tempMaterialIndex = result.data[1];
    if (dist < minDist) {
      minDist = dist;
      materialIndex = tempMaterialIndex;
    }
    t += dist;
    i++;
  }
  return (minDist < 1e-4f) ? f32x2(minDist, materialIndex) : f32x2(0.0f, 0.0f);
}
void generateRay(Context &__ctx, f32x3 &resultRayOrigin,
                 f32x3 &resultRayDirection, f32x3 viewOrigin, f32x3 viewTarget,
                 f32x2 uv) {
  f32x2 p = (2.0f * uv - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  resultRayOrigin = viewOrigin;
  f32x3 ww = normalize(viewTarget - viewOrigin);
  f32x3 uu = normalize(cross(ww, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 vv = normalize(cross(uu, ww));
  resultRayDirection = normalize(p.data[0] * uu + p.data[1] * vv + 2.2f * ww);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 rayOrigin = f32x3(0.0f, 1.5f, -5.0f);
  f32x3x3 rotate = f32x3x3(cos(iTime), 0.0f, sin(iTime), 0.0f, 1.0f, 0.0f,
                           -sin(iTime), 0.0f, cos(iTime));
  rayOrigin = rotate * rayOrigin;
  f32x3 viewTarget = f32x3(0.0f, 0.0f, 0.0f);
  f32x3 rayDirection;
  generateRay(__ctx, rayOrigin, rayDirection, rayOrigin, viewTarget, fragCoord);
  f32x2 result = raymarchCar(__ctx, rayOrigin, normalize(rayDirection));
  float dist = result.data[0];
  float materialIndex = result.data[1];
  f32x3 finalColor;
  if (materialIndex == 1.0f) {
    finalColor = f32x3(0.75f, 0.75f, 0.75f);
  } else if (materialIndex == 2.0f) {
    finalColor = f32x3(0.0f, 0.0f, 0.0f);
  } else if (materialIndex == 3.0f) {
    finalColor = f32x3(0.9f, 0.9f, 0.9f);
  } else if (materialIndex == 4.0f) {
    finalColor = f32x3(1.0f, 1.0f, 1.0f);
  } else if (materialIndex == 5.0f) {
    finalColor = f32x3(0.0f, 0.0f, 0.0f);
  } else {
    finalColor = f32x3(1.0f, 1.0f, 1.0f);
  }
  if (materialIndex > 0.0f) {
    f32x3 lightDirection = normalize(f32x3(0.5f, 0.5f, 0.5f));
    f32x3 normal = f32x3(0.0f, 0.0f, 0.0f);
    normal = sdNormal(__ctx, rayOrigin + rayDirection * dist, 0.001f);
    finalColor *= max(dot(lightDirection, normal), 0.0f);
  }
  fragColor = f32x4(finalColor, 1.0f);
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
