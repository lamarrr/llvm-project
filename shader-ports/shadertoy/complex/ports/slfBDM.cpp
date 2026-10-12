// Port of https://www.shadertoy.com/view/slfBDM; author: Heksi.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/slfBDM.json.
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

struct camera {
  f32x3 origin;
  f32x3 lookVec;
  f32x3 rightVec;
  f32x3 upVec;
};
struct rayMarchHit {
  bool hasHit;
  int marchCount;
  f32x3 pos;
  f32x3 normal;
  float distTraveled;
  float closestDistance;
};
float rand(Context &__ctx, f32x2 n) {
  return fract(sin(dot(n, f32x2(12.9898f, 4.1414f))) * 43758.5453f);
}
float noise(Context &__ctx, f32x2 p) {
  f32x2 ip = floor(p);
  f32x2 u = fract(p);
  u = u * u * (3.0f - 2.0f * u);
  float res =
      mix(mix(rand(__ctx, ip), rand(__ctx, ip + f32x2(1.0f, 0.0f)), u.data[0]),
          mix(rand(__ctx, ip + f32x2(0.0f, 1.0f)),
              rand(__ctx, ip + f32x2(1.0f, 1.0f)), u.data[0]),
          u.data[1]);
  return res * res;
}
float distToSphere(Context &__ctx, f32x3 c, float r, f32x3 p) {
  return length(c - p) - r;
}
float distToSphereMod(Context &__ctx, f32x3 c, float r, f32x3 p) {
  return length(c - p) - r;
}
float distToGround(Context &__ctx, f32x3 p, float h) {
  float r = distToSphere(__ctx, f32x3(0, -20, 0), 20.f, p) +
            noise(__ctx, p.swizzle<0, 1>() * 2.f + iTime) / 20.f +
            noise(__ctx, p.swizzle<1, 2>() * 2.f + iTime) / 20.f +
            noise(__ctx, p.swizzle<0, 2>() * 2.f + iTime) / 20.f;
  return r;
}
float sdf(Context &__ctx, f32x3 p) {
  const int objectCount = 2;
  float dists[objectCount];
  float smallestDistance = 1e10f;
  dists[0] = distToSphereMod(__ctx, f32x3(0, 0.5f, 0), 1.f, p);
  dists[1] = distToGround(__ctx, p, -0.4f);
  for (int i = 0; i < objectCount; i++) {
    smallestDistance = min(smallestDistance, dists[i]);
  }
  return smallestDistance;
}
f32x3 getNormalAt(Context &__ctx, f32x3 p) {
  float d = sdf(__ctx, p);
  f32x2 e = f32x2(0.01f, 0);
  return normalize(d - f32x3(sdf(__ctx, p - e.swizzle<0, 1, 1>()),
                             sdf(__ctx, p - e.swizzle<1, 0, 1>()),
                             sdf(__ctx, p - e.swizzle<1, 1, 0>())));
}
rayMarchHit rayMarch(Context &__ctx, f32x3 origin, f32x3 direction) {
  rayMarchHit hit = rayMarchHit(false, 0, origin, f32x3(0), 0.f, 0.f);
  for (int i = 0; i < 800; i++) {
    float distToScene = sdf(__ctx, hit.pos);
    if (distToScene < hit.closestDistance) {
      hit.closestDistance = distToScene;
    }
    hit.marchCount += 1;
    hit.distTraveled += distToScene;
    hit.pos = origin + direction * hit.distTraveled;
    hit.normal = getNormalAt(__ctx, hit.pos);
    if (distToScene <= 0.00001f) {
      hit.hasHit = true;
      break;
    }
    if (hit.distTraveled >= 50.f) {
      break;
    }
  }
  return hit;
}
f32x3 getSunCol(Context &__ctx, f32x3 normal, f32x3 lightDir) {
  f32x3 sunColor = f32x3(0);
  sunColor += max(smoothstep(0.99f, 1.f, dot(normal, lightDir)), 0.f);
  sunColor += max(smoothstep(0.9f, 1.f, dot(normal, lightDir)), 0.f) / 3.f;
  return sunColor;
}
f32x3 getSkyCol(Context &__ctx, f32x3 normal, f32x3 lightDir) {
  f32x3 skyCol =
      mix(f32x3(0.2f, 0.2f, 0.9f), f32x3(0.4f, 0.7f, 1), dot(normal, lightDir));
  skyCol = mix(skyCol,
               mix(skyCol, f32x3(1.f),
                   noise(__ctx, normal.swizzle<0, 2>() * 5.f) *
                       noise(__ctx, normal.swizzle<0, 2>() * 1.f) * .2f),
               smoothstep(-1.f, 1.f, dot(normal, f32x3(0, 1, 0))));
  return skyCol + getSunCol(__ctx, normal, lightDir);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  uv -= 0.5f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  uv *= f32x2(1, -1);
  f32x2 muv = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  camera cam =
      camera(f32x3(-cos(muv.data[0] * 10.f) * 7.f, muv.data[1] * 20.f - 10.f,
                   -sin(muv.data[0] * 10.f) * 7.f),
             f32x3(0), f32x3(0), f32x3(0));
  cam.lookVec = normalize(f32x3(0, 0.5f, 0) - cam.origin);
  cam.rightVec = cross(cam.lookVec, f32x3(0, 1, 0));
  cam.upVec = cross(cam.lookVec, cam.rightVec);
  f32x3 lightDir = normalize(f32x3(0.5f, 2, 1));
  f32x3 rayDir = normalize(cam.lookVec + cam.rightVec * uv.data[0] +
                           cam.upVec * uv.data[1]);
  f32x3 col = getSkyCol(__ctx, rayDir, lightDir);
  rayMarchHit marchHit = rayMarch(__ctx, cam.origin, rayDir);
  if (marchHit.hasHit) {
    f32x3 reflected =
        rayDir - (2.f * dot(rayDir, marchHit.normal) * marchHit.normal);
    col = f32x3(0.5f, 0.5f, 0.5f);
    col = mix(col, getSkyCol(__ctx, reflected, lightDir),
              smoothstep(0.5f, 1.f, 1.f - dot(marchHit.normal, reflected)));
    col += getSunCol(__ctx, reflected, lightDir);
    col *= max(dot(marchHit.normal, lightDir), 0.f);
    rayMarchHit shadow =
        rayMarch(__ctx, marchHit.pos - rayDir / 100.f, lightDir);
    if (shadow.hasHit) {
      col *= 0.f;
    }
    col = mix(col, getSkyCol(__ctx, rayDir, lightDir),
              smoothstep(0.6f, 1.f, marchHit.distTraveled / 50.f));
  }
  fragColor = f32x4(col, 1);
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
