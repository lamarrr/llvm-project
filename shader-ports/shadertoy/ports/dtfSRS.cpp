// Port of https://www.shadertoy.com/view/dtfSRS; author: Bricktronic.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtfSRS.json.
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
struct Context {
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

const float EPS = 0.001f;
const f32x3 lightDir = f32x3(0.5f, 1, 0.25f);
const f32x3 lightColor = f32x3(1.0f, 1.0f, 1.0f);
const f32x3 ambient = f32x3(0.4f, 0.4f, 0.4f);
f32x3 cameraPos;
struct Ray {
  f32x3 Pos;
  f32x3 Dir;
  f32x4 Color;
  float Offset;
};
float sdSphere(Context &__ctx, f32x3 p, f32x3 p2, float s) {
  return length(p - p2) - s;
}
f32x3x3 camRotation(Context &__ctx, f32x2 m) {
  m.data[1] = -m.data[1];
  f32x2 s = sin(m);
  f32x2 c = cos(m);
  f32x3x3 rotX = f32x3x3(1.0f, 0.0f, 0.0f, 0.0f, c.data[1], s.data[1], 0.0f,
                         -s.data[1], c.data[1]);
  f32x3x3 rotY = f32x3x3(c.data[0], 0.0f, -s.data[0], 0.0f, 1.0f, 0.0f,
                         s.data[0], 0.0f, c.data[0]);
  return rotY * rotX;
}
float mandelBulb(Context &__ctx, f32x3 pos) {
  float Power = 6.0f + sin(iTime * 0.5f) * 2.0f;
  f32x3 z = pos;
  float dr = 1.0f;
  float r = 0.0f;
  for (int i = 0; i < 4; i++) {
    r = length(z);
    if (r > 1.15f)
      break;
    float theta = acos(z.data[2] / r);
    float phi = atan(z.data[1], z.data[0]);
    dr = pow(r, Power - 1.0f) * Power * dr + 1.0f;
    float zr = pow(r, Power);
    theta = theta * Power;
    phi = phi * Power;
    z = zr * f32x3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
    z += pos;
  }
  return 0.5f * log(r) * r / dr;
}
float sceneDist(Context &__ctx, f32x3 p) {
  return mix(mandelBulb(__ctx, p), sdSphere(__ctx, p, f32x3(0, 0, 0), 1.0f),
             max(sin(iTime * 0.25f), 0.0f));
}
f32x3 sceneNormal(Context &__ctx, f32x3 p) {
  const f32x3 small_step = f32x3(0.001f, 0.0f, 0.0f);
  float gradient_x = sceneDist(__ctx, p + small_step.swizzle<0, 1, 1>()) -
                     sceneDist(__ctx, p - small_step.swizzle<0, 1, 1>());
  float gradient_y = sceneDist(__ctx, p + small_step.swizzle<1, 0, 1>()) -
                     sceneDist(__ctx, p - small_step.swizzle<1, 0, 1>());
  float gradient_z = sceneDist(__ctx, p + small_step.swizzle<1, 1, 0>()) -
                     sceneDist(__ctx, p - small_step.swizzle<1, 1, 0>());
  f32x3 normal = f32x3(gradient_x, gradient_y, gradient_z);
  return normalize(normal);
}
f32x3 lightScene(Context &__ctx, f32x3 pos) {
  f32x3 norm = normalize(sceneNormal(__ctx, pos));
  float diff = max(dot(norm, lightDir), 0.0f);
  f32x3 diffuse = diff * lightColor;
  f32x3 viewDir = normalize(cameraPos - pos);
  f32x3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32.0f);
  f32x3 specular = 0.002f * spec * lightColor;
  return (ambient + diffuse + specular);
}
void march(Context &__ctx, Ray &ray) {
  float totalDist = 0.0f;
  for (int i = 0; i < 100; i++) {
    float dist = sceneDist(__ctx, ray.Pos);
    if (dist < EPS) {
      ray.Color =
          f32x4(f32x3(0.45f, 0.75f, 0.85f) * lightScene(__ctx, ray.Pos), 1);
      break;
    }
    float travel = min(dist, 0.025f);
    ray.Pos += ray.Dir * (travel * ray.Offset);
    totalDist += travel;
  }
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  float div = (iResolution.data[0] / iResolution.data[1]);
  f32x2 sUV = f32x2((uv.data[0] * div) - 0.4f, uv.data[1]);
  float rot = iTime * 0.2f;
  cameraPos = f32x3(-sin(rot) * 2.0f, 0, -cos(rot) * 2.0f);
  f32x3x3 cameraDir = camRotation(__ctx, -f32x2(rot, 0));
  Ray ray;
  ray.Pos = cameraPos;
  ray.Dir = f32x3((sUV * 2.0f - 1.0f), 1) * cameraDir;
  ray.Dir = normalize(ray.Dir);
  ray.Offset = (length(texture(__ctx.channel0, uv * 1.0f)) * 0.1f) + 1.0f;
  ray.Color = f32x4(ambient, 1);
  march(__ctx, ray);
  fragColor = f32x4(ray.Color.swizzle<0, 1, 2>(), 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
  Context __ctx{{channel0, sampling0}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
